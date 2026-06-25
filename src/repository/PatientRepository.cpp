#include "PatientRepository.h"
#include "DatabaseManager.h"
#include "model/EmergencyPatient.h"
#include "model/InPatient.h"
#include "model/MedicalRecord.h"
#include "model/OutPatient.h"
#include <QDate>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

QString PatientRepository::vitalsToString(const VitalSigns &v) {
  return QString("%1|%2|%3|%4|%5|%6")
      .arg(v.temperature)
      .arg(v.bloodPressureSystolic)
      .arg(v.bloodPressureDiastolic)
      .arg(v.heartRate)
      .arg(v.weight)
      .arg(v.height);
}

VitalSigns PatientRepository::vitalsFromString(const QString &s) {
  VitalSigns v;
  QStringList parts = s.split('|');
  if (parts.size() == 6) {
    v.temperature = parts[0].toDouble();
    v.bloodPressureSystolic = parts[1].toInt();
    v.bloodPressureDiastolic = parts[2].toInt();
    v.heartRate = parts[3].toInt();
    v.weight = parts[4].toDouble();
    v.height = parts[5].toDouble();
  }
  return v;
}

// ─────────────────────────────────────────────────────────────────────────────
// insertPatientBase — dùng chung cho mọi loại patient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertPatientBase(const PatientInsertDTO &dto,
                                          int &patientId) {
  QString sql = R"(
        INSERT INTO patients (
            patient_code,
            full_name,
            birth_date,
            gender,
            phone_number,
            address,
            blood_type,
            allergies,
            medical_history,
            citizen_id,
            email,
            insurance,
            is_active,
            state,
            patient_type
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

  QVariantList params = {dto.patientCode,
                         dto.fullName,
                         dto.birthDate.toString("yyyy-MM-dd"),
                         static_cast<int>(dto.gender),
                         dto.phoneNumber,
                         dto.address,
                         dto.bloodType,
                         dto.allergies,
                         dto.medicalHistory,
                         dto.citizenId,
                         dto.email,
                         dto.insurance,
                         dto.isActive ? 1 : 0,
                         static_cast<int>(dto.state),
                         static_cast<int>(dto.type)};

  if (!DatabaseManager::getInstance().executeQuery(sql, params))
    return false;

  QSqlQuery lastId =
      DatabaseManager::getInstance().selectQuery("SELECT last_insert_rowid()");
  if (!lastId.next())
    return false;
  patientId = lastId.value(0).toInt();
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Insert
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertPatient(const PatientInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  int patientId = 0;
  if (!insertPatientBase(dto, patientId)) {
    db.rollbackTransaction();
    qWarning() << "PatientRepository::insertPatient - Lỗi ghi bảng patients";
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool PatientRepository::insertOutPatient(const OutPatientInsertDTO &dto) {
  // OutPatient không có bảng phụ, dùng thẳng base
  return insertPatient(dto);
}

bool PatientRepository::insertInPatient(const InPatientInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  int patientId = 0;
  if (!insertPatientBase(dto, patientId)) {
    db.rollbackTransaction();
    qWarning() << "PatientRepository::insertInPatient - Lỗi ghi bảng patients";
    return false;
  }

  QString sql = R"(
        INSERT INTO inpatient_profiles (
            patient_id,
            room_no,
            admit_date
        )
        VALUES (?, ?, ?)
    )";

  QVariantList params = {patientId, dto.roomNo,
                         dto.admitDate.toString("yyyy-MM-dd")};

  if (!db.executeQuery(sql, params)) {
    db.rollbackTransaction();
    qWarning() << "PatientRepository::insertInPatient - Lỗi ghi bảng "
                  "inpatient_profiles";
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool PatientRepository::insertEmergencyPatient(
    const EmergencyPatientInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  int patientId = 0;
  if (!insertPatientBase(dto, patientId)) {
    db.rollbackTransaction();
    qWarning()
        << "PatientRepository::insertEmergencyPatient - Lỗi ghi bảng patients";
    return false;
  }

  QString sql = R"(
        INSERT INTO emergency_profiles (
            patient_id,
            severity
        )
        VALUES (?, ?)
    )";

  QVariantList params = {patientId, dto.severity};

  if (!db.executeQuery(sql, params)) {
    db.rollbackTransaction();
    qWarning() << "PatientRepository::insertEmergencyPatient - Lỗi ghi bảng "
                  "emergency_profiles";
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::updatePatient(const PatientInsertDTO &dto,
                                      int patientId) {
  QString sql = R"(
        UPDATE patients SET
            patient_code    = ?,
            full_name       = ?,
            birth_date      = ?,
            gender          = ?,
            phone_number    = ?,
            address         = ?,
            blood_type      = ?,
            allergies       = ?,
            medical_history = ?,
            citizen_id      = ?,
            email           = ?,
            insurance       = ?,
            is_active       = ?,
            state           = ?,
            patient_type    = ?,
            updated_at      = datetime('now')
        WHERE id = ?
    )";

  QVariantList params = {dto.patientCode,
                         dto.fullName,
                         dto.birthDate.toString("yyyy-MM-dd"),
                         static_cast<int>(dto.gender),
                         dto.phoneNumber,
                         dto.address,
                         dto.bloodType,
                         dto.allergies,
                         dto.medicalHistory,
                         dto.citizenId,
                         dto.email,
                         dto.insurance,
                         dto.isActive ? 1 : 0,
                         static_cast<int>(dto.state),
                         static_cast<int>(dto.type),
                         patientId};

  if (!DatabaseManager::getInstance().executeQuery(sql, params))
    return false;
  return true;
}

bool PatientRepository::updateInPatient(const InPatientInsertDTO &dto,
                                        int patientId) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  if (!updatePatient(dto, patientId)) {
    db.rollbackTransaction();
    return false;
  }

  QString sql = R"(
        UPDATE inpatient_profiles
        SET room_no    = ?,
            admit_date = ?
        WHERE patient_id = ?
    )";

  QVariantList params = {dto.roomNo, dto.admitDate.toString("yyyy-MM-dd"),
                         patientId};

  if (!db.executeQuery(sql, params)) {
    db.rollbackTransaction();
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool PatientRepository::updateEmergencyPatient(
    const EmergencyPatientInsertDTO &dto, int patientId) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  if (!updatePatient(dto, patientId)) {
    db.rollbackTransaction();
    return false;
  }

  QString sql = R"(
        UPDATE emergency_profiles
        SET severity = ?
        WHERE patient_id = ?
    )";

  QVariantList params = {dto.severity, patientId};

  if (!db.executeQuery(sql, params)) {
    db.rollbackTransaction();
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Soft delete / restore
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::deactivate(int patientId) {
  DatabaseManager &db = DatabaseManager::getInstance();
  QString sql = "UPDATE patients SET is_active = 0 WHERE id = ?";

  if (!db.beginTransaction())
    return false;
  if (!db.executeQuery(sql, {patientId})) {
    db.rollbackTransaction();
    qWarning() << "PatientRepository::deactivate - Lỗi update bảng patients";
    return false;
  }
  if (!db.commitTransaction())
    return false;
  return true;
}

bool PatientRepository::reactivate(int patientId) {
  DatabaseManager &db = DatabaseManager::getInstance();
  QString sql = "UPDATE patients SET is_active = 1 WHERE id = ?";

  if (!db.beginTransaction())
    return false;
  if (!db.executeQuery(sql, {patientId})) {
    db.rollbackTransaction();
    qWarning() << "PatientRepository::reactivate - Lỗi update bảng patients";
    return false;
  }
  if (!db.commitTransaction())
    return false;
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Lookup
// ─────────────────────────────────────────────────────────────────────────────

std::optional<std::shared_ptr<Patient>>
PatientRepository::findById(int patientId) const {
  QString sql = R"(
        SELECT  p.*,
                ip.room_no, ip.admit_date,
                ep.severity
        FROM    patients p
        LEFT JOIN inpatient_profiles  ip ON p.id = ip.patient_id
        LEFT JOIN emergency_profiles  ep ON p.id = ep.patient_id
        WHERE   p.id = ? AND p.is_active = 1
    )";

  QSqlQuery query =
      DatabaseManager::getInstance().selectQuery(sql, {patientId});
  if (!query.next())
    return std::nullopt;
  return mapRowToPatient(query);
}

std::optional<std::shared_ptr<Patient>>
PatientRepository::findByPatientCode(const QString &patientCode) const {
  QString sql = R"(
        SELECT  p.*,
                ip.room_no, ip.admit_date,
                ep.severity
        FROM    patients p
        LEFT JOIN inpatient_profiles  ip ON p.id = ip.patient_id
        LEFT JOIN emergency_profiles  ep ON p.id = ep.patient_id
        WHERE   p.patient_code = ? AND p.is_active = 1
    )";

  QSqlQuery query =
      DatabaseManager::getInstance().selectQuery(sql, {patientCode});
  if (!query.next())
    return std::nullopt;
  return mapRowToPatient(query);
}

// ─────────────────────────────────────────────────────────────────────────────
// Search
// ─────────────────────────────────────────────────────────────────────────────

QList<std::shared_ptr<Patient>>
PatientRepository::search(const PatientSearchCriteria &criteria) const {
  QString sql = R"(
        SELECT  p.*,
                ip.room_no, ip.admit_date,
                ep.severity
        FROM    patients p
        LEFT JOIN inpatient_profiles  ip ON p.id = ip.patient_id
        LEFT JOIN emergency_profiles  ep ON p.id = ep.patient_id
        WHERE 1 = 1
    )";

  QVariantList params;

  if (!criteria.nameKeyword.trimmed().isEmpty()) {
    sql += " AND p.full_name LIKE ?";
    params << "%" + criteria.nameKeyword.trimmed() + "%";
  }

  if (!criteria.phoneNumber.trimmed().isEmpty()) {
    sql += " AND p.phone_number LIKE ?";
    params << "%" + criteria.phoneNumber.trimmed() + "%";
  }

  if (!criteria.patientCode.trimmed().isEmpty()) {
    sql += " AND p.patient_code LIKE ?";
    params << "%" + criteria.patientCode.trimmed() + "%";
  }

  if (!criteria.citizenId.trimmed().isEmpty()) {
    sql += " AND p.citizen_id LIKE ?";
    params << "%" + criteria.citizenId.trimmed() + "%";
  }

  if (!criteria.bloodType.trimmed().isEmpty()) {
    sql += " AND p.blood_type = ?";
    params << criteria.bloodType.trimmed();
  }

  if (criteria.patientType.has_value()) {
    sql += " AND p.patient_type = ?";
    params << static_cast<int>(criteria.patientType.value());
  }

  if (criteria.state.has_value()) {
    sql += " AND p.state = ?";
    params << static_cast<int>(criteria.state.value());
  }

  if (criteria.onlyActive) {
    sql += " AND p.is_active = 1";
  }

  if (!criteria.includeDeleted) {
    sql += " AND p.is_active != -1"; // -1 = soft-deleted hoàn toàn (nếu dùng)
  }

  sql += " ORDER BY p.full_name ASC";

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);

  QList<std::shared_ptr<Patient>> result;
  while (query.next()) {
    auto p = mapRowToPatient(query);
    if (!p)
      continue;

    // Age filter — lọc ở tầng app vì SQL không có cột age trực tiếp
    if (criteria.minAge >= 0 && p->getAge() < criteria.minAge)
      continue;
    if (criteria.maxAge >= 0 && p->getAge() > criteria.maxAge)
      continue;

    result.append(p);
  }
  return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Medical records
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::addMedicalRecord(const MedicalRecordInsertDTO &dto) {
  QString sql = R"(
        INSERT INTO medical_records (
            patient_id,
            doctor_id,
            visit_date,
            vitals,
            chief_complaint,
            clinical_notes,
            treatment,
            test_results,
            next_visit
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

  QVariantList params = {dto.patientId,
                         dto.doctorId,
                         dto.visitDateTime.toString(Qt::ISODate),
                         vitalsToString(dto.vitals),
                         dto.chiefComplaint,
                         dto.clinicalNotes,
                         dto.treatment,
                         dto.testResults,
                         dto.nextVisitDate.isValid()
                             ? QVariant(dto.nextVisitDate.toString(Qt::ISODate))
                             : QVariant()};

  return DatabaseManager::getInstance().executeQuery(sql, params);
}

QList<MedicalRecord>
PatientRepository::getRecordsByPatientId(int patientId) const {
  QString sql = "SELECT * FROM medical_records WHERE patient_id = ? ORDER BY "
                "visit_date DESC";
  QSqlQuery query =
      DatabaseManager::getInstance().selectQuery(sql, {patientId});

  QList<MedicalRecord> records;
  while (query.next()) {
    MedicalRecord rec;
    rec.setRecordId(query.value("id").toInt());
    rec.setVisitDateTime(QDateTime::fromString(
        query.value("visit_date").toString(), Qt::ISODate));
    rec.setChiefComplaint(query.value("chief_complaint").toString());
    rec.setClinicalNotes(query.value("clinical_notes").toString());
    rec.setTreatment(query.value("treatment").toString());
    rec.setTestResults(query.value("test_results").toString());
    rec.setVitals(vitalsFromString(query.value("vitals").toString()));

    QVariant nextVisit = query.value("next_visit");
    if (!nextVisit.isNull()) {
      rec.setNextVisitDate(
          QDateTime::fromString(nextVisit.toString(), Qt::ISODate));
    }

    records.append(rec);
  }
  return records;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

std::optional<QString> PatientRepository::getLatestCodeByYear(int year) {
  QString sql = R"(
        SELECT  patient_code
        FROM    patients
        WHERE   substr(patient_code, 2, 2) = substr(CAST(:year AS TEXT), -2)
        ORDER BY CAST(substr(patient_code, 4, 2) AS INTEGER) DESC
        LIMIT 1
    )";

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {year});
  if (!query.next())
    return std::nullopt;

  QVariant val = query.value("patient_code");
  if (val.isNull())
    return std::nullopt;
  return val.toString();
}

std::shared_ptr<Patient>
PatientRepository::mapRowToPatient(const QSqlQuery &query) const {
  PatientType type =
      static_cast<PatientType>(query.value("patient_type").toInt());

  std::shared_ptr<Patient> p;
  switch (type) {
  case PatientType::InPatient: {
    auto inP = std::make_shared<InPatient>();
    if (query.record().indexOf("room_no") != -1)
      inP->setRoomNo(query.value("room_no").toString());
    if (query.record().indexOf("admit_date") != -1)
      inP->setAdmitDate(QDate::fromString(query.value("admit_date").toString(),
                                          "yyyy-MM-dd"));
    p = inP;
    break;
  }
  case PatientType::Emergency: {
    auto emP = std::make_shared<EmergencyPatient>();
    if (query.record().indexOf("severity") != -1)
      emP->setSeverity(query.value("severity").toInt());
    p = emP;
    break;
  }
  case PatientType::OutPatient:
  default:
    p = std::make_shared<OutPatient>();
    break;
  }

  p->setId(query.value("id").toInt());
  p->setPatientCode(query.value("patient_code").toString());
  p->setFullName(query.value("full_name").toString());
  p->setBirthDate(
      QDate::fromString(query.value("birth_date").toString(), "yyyy-MM-dd"));
  p->setGender(static_cast<Gender>(query.value("gender").toInt()));
  p->setPhoneNumber(query.value("phone_number").toString());
  p->setAddress(query.value("address").toString());
  p->setBloodType(query.value("blood_type").toString());
  p->setAllergies(query.value("allergies").toString());
  p->setMedicalHistory(query.value("medical_history").toString());
  p->setCitizenId(query.value("citizen_id").toString());
  p->setEmail(query.value("email").toString());
  p->setInsurance(query.value("insurance").toString());
  p->setIsActive(query.value("is_active").toInt() == 1);
  p->setState(static_cast<PatientStateType>(query.value("state").toInt()));
  return p;
}
