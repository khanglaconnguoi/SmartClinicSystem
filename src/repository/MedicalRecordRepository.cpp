#include "MedicalRecordRepository.h"
#include "DatabaseManager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QtDebug>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

MedicalRecordRepository::MedicalRecordRepository(
    std::shared_ptr<PatientRepository> patientRepo)
    : m_patientRepository(patientRepo) {}

// ─────────────────────────────────────────────────────────────────────────────
// Private helper: insert allergies
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Gán patientId vào từng item rồi ủy qua
 * PatientRepository::insertAllergies(). Nếu m_patientRepository == nullptr hoặc
 * items rỗng, trả true ngay.
 */
bool MedicalRecordRepository::insertNewAllergies(
    int patientId, const QList<AllergyInsertDTO> &items) {
  if (!m_patientRepository || items.isEmpty())
    return true;

  QList<AllergyInsertDTO> filled;
  filled.reserve(items.size());
  for (AllergyInsertDTO item : items) {
    item.patientId = patientId;
    filled.append(item);
  }
  return m_patientRepository->insertAllergies(filled);
}

int MedicalRecordRepository::insertMedicalRecord(
    const MedicalRecordInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction()) {
    return -1;
  }

  const QString sql = R"(
    INSERT INTO medical_records (
      patient_id, doctor_id, appointment_id,
      visit_datetime, temperature, blood_pressure, heart_rate, weight, height,
      chief_complaint, clinical_notes, treatment, next_visit_date
    ) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)
  )";

  QVariantList params = {
      dto.patientId,
      dto.doctorId,
      dto.appointmentId,
      dto.visitDateTime,
      dto.vitals.temperature,
      dto.vitals.bloodPressure,
      dto.vitals.heartRate,
      dto.vitals.weight,
      dto.vitals.height,
      dto.chiefComplaint,
      dto.clinicalNotes,
      dto.treatment,
      dto.nextVisitDate.has_value() ? QVariant(dto.nextVisitDate.value())
                                    : QVariant(QVariant::Date)};

  QSqlQuery query(db.database());
  if (!query.prepare(sql)) {
    db.rollbackTransaction();
    return -1;
  }
  for (const QVariant &param : params) {
    query.addBindValue(param);
  }
  if (!query.exec()) {
    db.rollbackTransaction();
    return -1;
  }

  int recordId = query.lastInsertId().toInt();
  if (recordId <= 0) {
    db.rollbackTransaction();
    return -1;
  }

  if (!insertDiagnoses(recordId, dto.diagnoses)) {
    db.rollbackTransaction();
    return -1;
  }

  // Commit hồ sơ bệnh án + chẩn đoán trước
  if (!db.commitTransaction()) {
    return -1;
  }

  // Insert dị ứng NGOÀI transaction chính (best-effort):
  // Nếu thất bại (vd: allergen_name đã tồn tại), chỉ log warning —
  // KHÔNG rollback hồ sơ bệnh án vì dữ liệu lâm sàng đã được ghi an toàn.
  if (!dto.newAllergies.isEmpty()) {
    if (!insertNewAllergies(dto.patientId, dto.newAllergies)) {
      qWarning()
          << "insertMedicalRecord: insertNewAllergies thất bại (best-effort) "
             "— record_id="
          << recordId << "vẫn được lưu.";
    }
  }

  return recordId;
}

bool MedicalRecordRepository::updateMedicalRecord(
    const MedicalRecordUpdateDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction()) {
    return false;
  }

  const QString sql = R"(
    UPDATE medical_records
    SET doctor_id = ?, appointment_id = ?, visit_datetime = ?,
        temperature = ?, blood_pressure = ?, heart_rate = ?, weight = ?, height = ?,
        chief_complaint = ?, clinical_notes = ?, treatment = ?, next_visit_date = ?
    WHERE record_id = ?
  )";

  QVariantList params = {
      dto.doctorId,
      dto.appointmentId,
      dto.visitDateTime,
      dto.vitals.temperature,
      dto.vitals.bloodPressure,
      dto.vitals.heartRate,
      dto.vitals.weight,
      dto.vitals.height,
      dto.chiefComplaint,
      dto.clinicalNotes,
      dto.treatment,
      dto.nextVisitDate.has_value() ? QVariant(dto.nextVisitDate.value())
                                    : QVariant(QVariant::Date),
      dto.recordId};

  if (!db.executeQuery(sql, params).isActive()) {
    db.rollbackTransaction();
    return false;
  }

  // Delete old diagnoses and insert new ones
  const QString delDiagSql = "DELETE FROM diagnoses WHERE record_id = ?";
  if (!db.executeQuery(delDiagSql, {dto.recordId}).isActive()) {
    db.rollbackTransaction();
    return false;
  }

  if (!insertDiagnoses(dto.recordId, dto.diagnoses)) {
    db.rollbackTransaction();
    return false;
  }

  return db.commitTransaction();
}

bool MedicalRecordRepository::softDeleteMedicalRecord(int recordId) {
  DatabaseManager &db = DatabaseManager::getInstance();
  const QString sql =
      "UPDATE medical_records SET is_deleted = 1 WHERE record_id = ?";
  return db.executeQuery(sql, {recordId}).isActive();
}

bool MedicalRecordRepository::insertDiagnoses(
    int recordId, const QList<Diagnosis> &diagnoses) {
  DatabaseManager &db = DatabaseManager::getInstance();
  const QString sql = R"(
    INSERT INTO diagnoses (record_id, icd_code, description, severity)
    VALUES (?,?,?,?)
  )";

  for (const Diagnosis &d : diagnoses) {
    QVariantList params = {recordId, d.icdCode, d.description,
                           d.severity.toUpper()};
    if (!db.executeQuery(sql, params).isActive()) {
      return false;
    }
  }
  return true;
}

std::optional<MedicalRecordResultDTO>
MedicalRecordRepository::findById(int recordId) {
  DatabaseManager &db = DatabaseManager::getInstance();
  const QString sql =
      "SELECT * FROM medical_records WHERE record_id = ? AND is_deleted = 0";
  QSqlQuery query = db.selectQuery(sql, {recordId});

  if (!query.next()) {
    return std::nullopt;
  }

  MedicalRecordResultDTO dto;
  dto.recordId = query.value("record_id").toInt();
  dto.patientId = query.value("patient_id").toInt();
  dto.doctorId = query.value("doctor_id").toInt();
  dto.appointmentId = query.value("appointment_id").toInt();

  dto.visitDateTime = query.value("visit_datetime").toDateTime();
  dto.vitals.temperature = query.value("temperature").toDouble();
  dto.vitals.bloodPressure = query.value("blood_pressure").toString();
  dto.vitals.heartRate = query.value("heart_rate").toInt();
  dto.vitals.weight = query.value("weight").toDouble();
  dto.vitals.height = query.value("height").toDouble();
  dto.chiefComplaint = query.value("chief_complaint").toString();
  dto.clinicalNotes = query.value("clinical_notes").toString();
  dto.treatment = query.value("treatment").toString();

  QVariant nextVisit = query.value("next_visit_date");
  dto.nextVisitDate = nextVisit.isNull()
                          ? std::nullopt
                          : std::make_optional(nextVisit.toDate());

  const QString diagSql = "SELECT * FROM diagnoses WHERE record_id = ?";
  QSqlQuery diagQuery = db.selectQuery(diagSql, {recordId});
  while (diagQuery.next()) {
    Diagnosis d;
    d.icdCode = diagQuery.value("icd_code").toString();
    d.description = diagQuery.value("description").toString();
    d.severity = diagQuery.value("severity").toString();
    dto.diagnoses.append(d);
  }

  return dto;
}

QList<MedicalRecordResultDTO>
MedicalRecordRepository::getHistoryByPatientId(int patientId) {
  DatabaseManager &db = DatabaseManager::getInstance();
  const QString sql = "SELECT * FROM medical_records WHERE patient_id = ? AND "
                      "is_deleted = 0 ORDER BY visit_datetime DESC";
  QSqlQuery query = db.selectQuery(sql, {patientId});

  QList<MedicalRecordResultDTO> results;
  while (query.next()) {
    MedicalRecordResultDTO dto;
    dto.recordId = query.value("record_id").toInt();
    dto.patientId = query.value("patient_id").toInt();
    dto.doctorId = query.value("doctor_id").toInt();
    dto.appointmentId = query.value("appointment_id").toInt();
    dto.visitDateTime = query.value("visit_datetime").toDateTime();
    dto.vitals.temperature = query.value("temperature").toDouble();
    dto.vitals.bloodPressure = query.value("blood_pressure").toString();
    dto.vitals.heartRate = query.value("heart_rate").toInt();
    dto.vitals.weight = query.value("weight").toDouble();
    dto.vitals.height = query.value("height").toDouble();
    dto.chiefComplaint = query.value("chief_complaint").toString();
    dto.clinicalNotes = query.value("clinical_notes").toString();
    dto.treatment = query.value("treatment").toString();

    QVariant nextVisit = query.value("next_visit_date");
    dto.nextVisitDate = nextVisit.isNull()
                            ? std::nullopt
                            : std::make_optional(nextVisit.toDate());

    const QString diagSql = "SELECT * FROM diagnoses WHERE record_id = ?";
    QSqlQuery diagQuery = db.selectQuery(diagSql, {dto.recordId});
    while (diagQuery.next()) {
      Diagnosis d;
      d.icdCode = diagQuery.value("icd_code").toString();
      d.description = diagQuery.value("description").toString();
      d.severity = diagQuery.value("severity").toString();
      dto.diagnoses.append(d);
    }

    results.append(dto);
  }
  return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// Search
// ─────────────────────────────────────────────────────────────────────────────

QString MedicalRecordRepository::buildSearchWhereClause(
    const MedicalRecordSearchCriteria &criteria,
    QVariantList &outParams) const {
  QStringList conditions;

  // is_deleted: mặc định bỏ qua bản ghi đã xoá.
  if (!criteria.includeDeleted) {
    conditions << "mr.is_deleted = 0";
  }

  // searchKey: LIKE trên chief_complaint và clinical_notes.
  // Escape ký tự đặc biệt '%' và '_' để tránh LIKE sai kết quả.
  if (!criteria.searchKey.trimmed().isEmpty()) {
    conditions << "(mr.chief_complaint LIKE ? OR mr.clinical_notes LIKE ?)";
    QString escaped = criteria.searchKey.trimmed();
    escaped.replace('%', "\\%").replace('_', "\\_");
    const QString likeValue = "%" + escaped + "%";
    outParams << likeValue << likeValue;
  }

  if (criteria.patientId != -1) {
    conditions << "mr.patient_id = ?";
    outParams << criteria.patientId;
  }

  if (criteria.doctorId != -1) {
    conditions << "mr.doctor_id = ?";
    outParams << criteria.doctorId;
  }

  if (criteria.fromDate.has_value() && criteria.fromDate->isValid()) {
    conditions << "DATE(mr.visit_datetime) >= ?";
    outParams << criteria.fromDate->toString("yyyy-MM-dd");
  }

  if (criteria.toDate.has_value() && criteria.toDate->isValid()) {
    conditions << "DATE(mr.visit_datetime) <= ?";
    outParams << criteria.toDate->toString("yyyy-MM-dd");
  }

  if (conditions.isEmpty())
    return "1 = 1";

  return conditions.join(" AND ");
}

QList<MedicalRecordSummaryDTO> MedicalRecordRepository::searchMedicalRecords(
    const MedicalRecordSearchCriteria &criteria) {
  QVariantList params;
  QString whereClause = buildSearchWhereClause(criteria, params);

  const QString sql = QString(R"(
    SELECT mr.record_id, mr.patient_id, mr.doctor_id,
           mr.visit_datetime, mr.chief_complaint, mr.is_deleted
    FROM   medical_records mr
    WHERE  %1
    ORDER  BY mr.visit_datetime DESC
    LIMIT  ? OFFSET ?
  )")
                          .arg(whereClause);

  params << criteria.limit << criteria.offset;

  QList<MedicalRecordSummaryDTO> results;
  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);

  if (!query.isActive()) {
    qWarning()
        << "MedicalRecordRepository::searchMedicalRecords - Query thất bại";
    return results;
  }

  while (query.next()) {
    MedicalRecordSummaryDTO dto;
    dto.recordId = query.value("record_id").toInt();
    dto.patientId = query.value("patient_id").toInt();
    dto.doctorId = query.value("doctor_id").toInt();
    dto.visitDateTime = QDateTime::fromString(
        query.value("visit_datetime").toString(), Qt::ISODate);
    dto.chiefComplaint = query.value("chief_complaint").toString();
    dto.isDeleted = query.value("is_deleted").toInt() == 1;
    results.append(dto);
  }
  return results;
}

int MedicalRecordRepository::countSearchResults(
    const MedicalRecordSearchCriteria &criteria) {
  QVariantList params;
  QString whereClause = buildSearchWhereClause(criteria, params);

  const QString sql = QString(R"(
    SELECT COUNT(*)
    FROM   medical_records mr
    WHERE  %1
  )")
                          .arg(whereClause);

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (query.next())
    return query.value(0).toInt();
  return 0;
}
