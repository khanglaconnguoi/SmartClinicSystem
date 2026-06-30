/**
 * @file    PatientRepository.cpp
 * @brief   Implementación của PatientRepository – chỉ INSERT.
 *
 *  Lưu ý về cột DB:
 *    patients                       patient_code, full_name, date_of_birth,
 *                                   gender, citizen_id, phone, email, address,
 *                                   blood_type, default_patient_type,
 *                                   emergency_contact_name,
 * emergency_contact_phone out_patients                   patient_id (FK),
 * doctor_id, status in_patients                    patient_id (FK),
 * admitting_doctor_id, room_id, admission_date, discharge_date, reason, status
 *    emergency_patients_admissions  patient_id (FK), room_id, doctor_id,
 *                                   injury_cause, injury_description,
 *                                   admission_date, discharge_date, status
 */

#include "PatientRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QVariant>
#include <QtDebug>

// ─────────────────────────────────────────────────────────────────────────────
// Private helper
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertBasePatient(const PatientInsertDTO &dto,
                                          int &patientId) {
  const QString sql = R"(
    INSERT INTO patients (
        patient_code,
        full_name,
        date_of_birth,
        gender,
        citizen_id,
        phone,
        email,
        address,
        blood_type,
        default_patient_type,
        emergency_contact_name,
        emergency_contact_phone
    )
    VALUES (?,?,?,?,?,?,?,?,?,?,?,?)
  )";

  const QVariantList params = {
      dto.patientCode,
      dto.fullName,
      dto.dateOfBirth,
      dto.gender,
      dto.citizenId.isEmpty() ? QVariant(QVariant::String) : dto.citizenId,
      dto.phone.isEmpty() ? QVariant(QVariant::String) : dto.phone,
      dto.email.isEmpty() ? QVariant(QVariant::String) : dto.email,
      dto.address.isEmpty() ? QVariant(QVariant::String) : dto.address,
      dto.bloodType,
      dto.type,
      dto.emergencyContactName.isEmpty() ? QVariant(QVariant::String)
                                         : dto.emergencyContactName,
      dto.emergencyContactPhone.isEmpty() ? QVariant(QVariant::String)
                                          : dto.emergencyContactPhone,
  };

  if (!DatabaseManager::getInstance().executeQuery(sql, params)) {
    qWarning()
        << "PatientRepository::insertBasePatient - INSERT patients thất bại";
    return false;
  }

  QSqlQuery lastId =
      DatabaseManager::getInstance().selectQuery("SELECT last_insert_rowid()");
  if (!lastId.isActive() || !lastId.next()) {
    qWarning() << "PatientRepository::insertBasePatient - Không lấy được "
                  "last_insert_rowid";
    return false;
  }
  patientId = lastId.value(0).toInt();
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public – insert out patient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertOutPatient(const OutPatientInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();

  if (!db.beginTransaction()) {
    qWarning()
        << "PatientRepository::insertOutPatient - Không mở được transaction";
    return false;
  }

  int patientId = 0;
  if (!insertBasePatient(dto, patientId)) {
    db.rollbackTransaction();
    return false;
  }

  const QString sql = R"(
    INSERT INTO out_patients (
        patient_id,
        doctor_id,
        status
    )
    VALUES (?,?,?)
  )";

  const QVariantList params = {
      patientId,
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value())
                               : QVariant(QVariant::Int),
      dto.status,
  };

  if (!db.executeQuery(sql, params)) {
    db.rollbackTransaction();
    qWarning()
        << "PatientRepository::insertOutPatient - INSERT out_patients thất bại";
    return false;
  }

  if (!db.commitTransaction()) {
    qWarning() << "PatientRepository::insertOutPatient - Commit thất bại";
    return false;
  }
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public – insert in patient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertInPatient(const InPatientInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();

  if (!db.beginTransaction()) {
    qWarning()
        << "PatientRepository::insertInPatient - Không mở được transaction";
    return false;
  }

  int patientId = 0;
  if (!insertBasePatient(dto, patientId)) {
    db.rollbackTransaction();
    return false;
  }

  const QString sql = R"(
    INSERT INTO in_patients (
        patient_id,
        room_id,
        doctor_id,
        admission_date,
        discharge_date,
        reason,
        status
    )
    VALUES (?,?,?,?,?,?,?)
  )";

  const QVariantList params = {
      patientId,
      dto.roomId.has_value() ? QVariant(dto.roomId.value())
                             : QVariant(QVariant::Int),
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value())
                               : QVariant(QVariant::Int),
      dto.admissionDate,
      dto.dischargeDate.isEmpty() ? QVariant(QVariant::String)
                                  : dto.dischargeDate,
      dto.reason.isEmpty() ? QVariant(QVariant::String) : dto.reason,
      dto.status,
  };

  if (!db.executeQuery(sql, params)) {
    db.rollbackTransaction();
    qWarning()
        << "PatientRepository::insertInPatient - INSERT in_patients thất bại";
    return false;
  }

  if (!db.commitTransaction()) {
    qWarning() << "PatientRepository::insertInPatient - Commit thất bại";
    return false;
  }
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public – insert emergency patient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertEmergencyPatient(
    const EmergencyPatientInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();

  if (!db.beginTransaction()) {
    qWarning() << "PatientRepository::insertEmergencyPatient - Không mở được "
                  "transaction";
    return false;
  }

  int patientId = 0;
  if (!insertBasePatient(dto, patientId)) {
    db.rollbackTransaction();
    return false;
  }

  const QString sql = R"(
    INSERT INTO emergency_patients (
        patient_id,
        room_id,
        doctor_id,
        injury_cause,
        injury_description,
        admission_date,
        discharge_date,
        status
    )
    VALUES (?,?,?,?,?,?,?,?)
  )";

  const QVariantList params = {
      patientId,
      dto.roomId.has_value() ? QVariant(dto.roomId.value())
                             : QVariant(QVariant::Int),
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value())
                               : QVariant(QVariant::Int),
      dto.injuryCause.isEmpty() ? QVariant(QVariant::String) : dto.injuryCause,
      dto.injuryDescription.isEmpty() ? QVariant(QVariant::String)
                                      : dto.injuryDescription,
      dto.admissionDate,
      dto.dischargeDate.isEmpty() ? QVariant(QVariant::String)
                                  : dto.dischargeDate,
      dto.status,
  };

  if (!db.executeQuery(sql, params)) {
    db.rollbackTransaction();
    qWarning() << "PatientRepository::insertEmergencyPatient - INSERT "
                  "emergency_patients_admissions thất bại";
    return false;
  }

  if (!db.commitTransaction()) {
    qWarning() << "PatientRepository::insertEmergencyPatient - Commit thất bại";
    return false;
  }
  return true;
}