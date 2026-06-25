/**
 * @file    PatientRepository.h
 * @brief   Data access layer cho bảng patients (SQLite).
 */
#pragma once

#include "model/MedicalRecord.h"
#include "model/Patient.h"
#include <QDate>
#include <QSqlQuery>
#include <QString>
#include <memory>
#include <optional>

// ── Forward declarations
// ──────────────────────────────────────────────────────
class QSqlDatabase;
struct PatientSearchCriteria;

// ─────────────────────────────────────────────────────────────────────────────
// DTOs
// ─────────────────────────────────────────────────────────────────────────────

/*
 * Dùng struct vì DTO chỉ là "túi dữ liệu" thuần tuý:
 * không có invariant cần bảo vệ, không có behaviour —
 * default public phù hợp hơn class.
 */

struct PatientInsertDTO {
  QString patientCode;
  QString fullName;
  QDate birthDate;
  Gender gender;
  QString phoneNumber;
  QString address;
  QString bloodType;
  QString allergies;
  QString medicalHistory;
  QString citizenId;
  QString email;
  QString insurance;
  bool isActive = true;
  PatientStateType state = PatientStateType::Registered;
  PatientType type = PatientType::OutPatient;
};

struct OutPatientInsertDTO : public PatientInsertDTO {
  OutPatientInsertDTO() { type = PatientType::OutPatient; }
};

struct InPatientInsertDTO : public PatientInsertDTO {
  QString roomNo;
  QDate admitDate;
  InPatientInsertDTO() { type = PatientType::InPatient; }
};

struct EmergencyPatientInsertDTO : public PatientInsertDTO {
  int severity = 0;
  EmergencyPatientInsertDTO() { type = PatientType::Emergency; }
};

struct MedicalRecordInsertDTO {
  int patientId;
  int doctorId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QString treatment;
  QString testResults;
  QDateTime nextVisitDate; // invalid QDateTime = không có
};

struct PatientSearchCriteria {
  // ── Nhóm 1: Text search ───────────────────────────────────────
  QString nameKeyword;
  QString phoneNumber;
  QString patientCode;
  QString citizenId;

  // ── Nhóm 2: Dropdown filter ───────────────────────────────────
  QString bloodType;
  std::optional<PatientType> patientType;
  std::optional<PatientStateType> state;

  // ── Nhóm 3: Age range filter (lọc sau khi query) ──────────────
  int minAge = -1; // -1 = không giới hạn
  int maxAge = -1;

  // ── Nhóm 4: Status filter ─────────────────────────────────────
  bool onlyActive = true;
  bool includeDeleted = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// Repository
// ─────────────────────────────────────────────────────────────────────────────

class PatientRepository {
public:
  PatientRepository() = default;

  // ── Insert ────────────────────────────────────────────────────
  bool insertPatient(const PatientInsertDTO &patient);
  bool insertOutPatient(const OutPatientInsertDTO &patient);
  bool insertInPatient(const InPatientInsertDTO &patient);
  bool insertEmergencyPatient(const EmergencyPatientInsertDTO &patient);

  // ── Update ────────────────────────────────────────────────────
  bool updatePatient(const PatientInsertDTO &patient, int patientId);
  bool updateInPatient(const InPatientInsertDTO &patient, int patientId);
  bool updateEmergencyPatient(const EmergencyPatientInsertDTO &patient,
                              int patientId);

  // ── Soft delete / restore ─────────────────────────────────────
  bool deactivate(int patientId);
  bool reactivate(int patientId);

  // ── Lookup ────────────────────────────────────────────────────
  std::optional<std::shared_ptr<Patient>> findById(int patientId) const;
  std::optional<std::shared_ptr<Patient>>
  findByPatientCode(const QString &patientCode) const;

  // ── Search / list ─────────────────────────────────────────────
  QList<std::shared_ptr<Patient>>
  search(const PatientSearchCriteria &criteria) const;

  // ── Medical records ───────────────────────────────────────────
  bool addMedicalRecord(const MedicalRecordInsertDTO &record);
  QList<MedicalRecord> getRecordsByPatientId(int patientId) const;

  // ── Helpers ───────────────────────────────────────────────────
  static std::optional<QString> getLatestCodeByYear(int year);

private:
  // Tách phần insert base ra riêng để tái dùng
  bool insertPatientBase(const PatientInsertDTO &dto, int &patientId);

  std::shared_ptr<Patient> mapRowToPatient(const QSqlQuery &query) const;

  // Serialize / deserialize VitalSigns <-> pipe-separated string
  static QString vitalsToString(const VitalSigns &v);
  static VitalSigns vitalsFromString(const QString &s);
};