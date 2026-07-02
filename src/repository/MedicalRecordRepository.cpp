#include "MedicalRecordRepository.h"
#include "DatabaseManager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QtDebug>

int MedicalRecordRepository::insertMedicalRecord(const MedicalRecordInsertDTO &dto) {
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
    dto.appointmentId.has_value() ? QVariant(dto.appointmentId.value()) : QVariant(QVariant::Int),
    dto.visitDateTime,
    dto.vitals.temperature,
    dto.vitals.bloodPressure,
    dto.vitals.heartRate,
    dto.vitals.weight,
    dto.vitals.height,
    dto.chiefComplaint,
    dto.clinicalNotes,
    dto.treatment,
    dto.nextVisitDate.has_value() ? QVariant(dto.nextVisitDate.value()) : QVariant(QVariant::Date)
  };

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

  if (!db.commitTransaction()) {
    return -1;
  }

  return recordId;
}

bool MedicalRecordRepository::updateMedicalRecord(const MedicalRecordUpdateDTO &dto) {
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
    dto.appointmentId.has_value() ? QVariant(dto.appointmentId.value()) : QVariant(QVariant::Int),
    dto.visitDateTime,
    dto.vitals.temperature,
    dto.vitals.bloodPressure,
    dto.vitals.heartRate,
    dto.vitals.weight,
    dto.vitals.height,
    dto.chiefComplaint,
    dto.clinicalNotes,
    dto.treatment,
    dto.nextVisitDate.has_value() ? QVariant(dto.nextVisitDate.value()) : QVariant(QVariant::Date),
    dto.recordId
  };

  if (!db.executeQuery(sql, params)) {
    db.rollbackTransaction();
    return false;
  }

  // Delete old diagnoses and insert new ones
  const QString delDiagSql = "DELETE FROM diagnoses WHERE record_id = ?";
  if (!db.executeQuery(delDiagSql, {dto.recordId})) {
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
  const QString sql = "UPDATE medical_records SET is_deleted = 1 WHERE record_id = ?";
  return db.executeQuery(sql, {recordId});
}

bool MedicalRecordRepository::insertDiagnoses(int recordId, const QList<Diagnosis> &diagnoses) {
  DatabaseManager &db = DatabaseManager::getInstance();
  const QString sql = R"(
    INSERT INTO diagnoses (record_id, icd_code, description, severity)
    VALUES (?,?,?,?)
  )";

  for (const Diagnosis &d : diagnoses) {
    QVariantList params = {
      recordId,
      d.icdCode,
      d.description,
      d.severity.toUpper()
    };
    if (!db.executeQuery(sql, params)) {
      return false;
    }
  }
  return true;
}

std::optional<MedicalRecordResultDTO> MedicalRecordRepository::findById(int recordId) {
  DatabaseManager &db = DatabaseManager::getInstance();
  const QString sql = "SELECT * FROM medical_records WHERE record_id = ? AND is_deleted = 0";
  QSqlQuery query = db.selectQuery(sql, {recordId});

  if (!query.next()) {
    return std::nullopt;
  }

  MedicalRecordResultDTO dto;
  dto.recordId = query.value("record_id").toInt();
  dto.patientId = query.value("patient_id").toInt();
  dto.doctorId = query.value("doctor_id").toInt();

  QVariant appt = query.value("appointment_id");
  dto.appointmentId = appt.isNull() ? std::nullopt : std::make_optional(appt.toInt());

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
  dto.nextVisitDate = nextVisit.isNull() ? std::nullopt : std::make_optional(nextVisit.toDate());

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

QList<MedicalRecordResultDTO> MedicalRecordRepository::getHistoryByPatientId(int patientId) {
  DatabaseManager &db = DatabaseManager::getInstance();
  const QString sql = "SELECT * FROM medical_records WHERE patient_id = ? AND is_deleted = 0 ORDER BY visit_datetime DESC";
  QSqlQuery query = db.selectQuery(sql, {patientId});

  QList<MedicalRecordResultDTO> results;
  while (query.next()) {
    MedicalRecordResultDTO dto;
    dto.recordId = query.value("record_id").toInt();
    dto.patientId = query.value("patient_id").toInt();
    dto.doctorId = query.value("doctor_id").toInt();

    QVariant appt = query.value("appointment_id");
    dto.appointmentId = appt.isNull() ? std::nullopt : std::make_optional(appt.toInt());

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
    dto.nextVisitDate = nextVisit.isNull() ? std::nullopt : std::make_optional(nextVisit.toDate());

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
