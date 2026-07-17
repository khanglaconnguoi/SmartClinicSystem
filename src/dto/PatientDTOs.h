/**
 * @file    PatientDTOs.h
 * @brief   Data Transfer Objects cho Patient module
 *          Chỉ là data containers — không có behavior
 */

#pragma once

#include "model/CommonEnums.h"
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QString>
#include <optional>

// ═══════════════════════════════════════════════════════════════════════════
// ALLERGY DTOs  (map với bảng `patient_allergies`)
// ═══════════════════════════════════════════════════════════════════════════

struct AllergyInsertDTO {
  int patientId;
  QString allergenName;
  QString severity; // 'MILD' | 'MODERATE' | 'SEVERE'
  QString notes;
};

struct AllergyResultDTO {
  int allergyId;
  QString allergenName;
  QString severity;
  QString notes;
  bool isActive;
  QString recordedAt;
  QString updatedAt;
};

// ═══════════════════════════════════════════════════════════════════════════
// INSURANCE DTOs  (map với bảng `patient_insurance`)
// ═══════════════════════════════════════════════════════════════════════════

struct InsuranceInsertDTO {
  int patientId;         // patient_id       NOT NULL  (tự gán bởi Service/Repo)
  QString providerName;  // provider_name    NOT NULL
  QString policyNumber;  // policy_number    NOT NULL
  QString insuranceType; // insurance_type   NOT NULL  DEFAULT 'BHYT'  CHECK:
                         // BHYT|PRIVATE|OTHER
  double coveragePercent = 80.0; // coverage_percent NOT NULL  DEFAULT 80
  QString validFrom;             // valid_from       nullable  (yyyy-MM-dd)
  QString validTo;               // valid_to         nullable  (yyyy-MM-dd)
  QString notes;                 // notes            nullable
};

struct InsuranceResultDTO {
  int insuranceId = 0;
  QString providerName;
  QString policyNumber;
  QString insuranceType;
  double coveragePercent = 0.0;
  QString validFrom;
  QString validTo;
  QString notes;
  bool isActive = false;
  QString createdAt;
  QString updatedAt;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1 – INPUT DTOs  (UI → Service)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientInputDTO {
  // ── patients ─────────────────────────────────────────────────────────────
  QString fullName;  // full_name               NOT NULL
  QDate dateOfBirth; // date_of_birth            NOT NULL
  Gender gender; // gender                   NOT NULL  CHECK: MALE|FEMALE|OTHER
  QString citizenId; // citizen_id               nullable  UNIQUE
  QString phone;     // phone                    NOT NULL
  QString email; // email                    NOT NULL  (DB: NOT NULL, dù có thể
                 // rỗng)
  QString address;   // address                  NOT NULL
  QString bloodType; // blood_type               NOT NULL  DEFAULT 'UNKNOWN'
                     // CHECK: A+|A-|B+|B-|AB+|AB-|O+|O-|UNKNOWN
  // QString allergies; // → dùng QList<AllergyInsertDTO> riêng biệt
  QString insurance; // → raw string bảo hiểm từ UI (Service sẽ parse)
  PatientType type;  // default_patient_type  NOT NULL  DEFAULT 'OUTPATIENT'
                     // CHECK: OUTPATIENT|INPATIENT|EMERGENCY
  QString emergencyContactName;  // emergency_contact_name  NOT NULL
  QString emergencyContactPhone; // emergency_contact_phone NOT NULL

  virtual ~PatientInputDTO() = default;
};

struct InPatientInputDTO : public PatientInputDTO {
  // ── in_patients ───────────────────────────────────────────────────────────
  std::optional<int> roomId;   // room_id        nullable  FK → rooms
  std::optional<int> doctorId; // doctor_id      nullable  FK → staff
  QDate admissionDate; // admission_date NOT NULL  DEFAULT datetime('now')
  std::optional<QDate>
      dischargeDate; // discharge_date nullable  CHECK: >= admission_date
  QString reason;    // reason         nullable
};

struct EmergencyPatientInputDTO : public PatientInputDTO {
  // ── emergency_patients ────────────────────────────────────────────────────
  std::optional<int> roomId;   // room_id              nullable  FK → rooms
  std::optional<int> doctorId; // doctor_id            nullable  FK → staff
  QString injuryCause;         // injury_cause         nullable
  QString injuryDescription;   // injury_description   nullable
  QDate admissionDate; // admission_date       NOT NULL  DEFAULT datetime('now')
  std::optional<QDate>
      dischargeDate; // discharge_date       nullable  CHECK: >= admission_date
};

struct OutPatientInputDTO : public PatientInputDTO {
  // ── out_patients ──────────────────────────────────────────────────────────
  std::optional<int> doctorId; // doctor_id  nullable  FK → staff
  // status  NOT NULL  DEFAULT 'REGISTERED'  CHECK: REGISTERED|WAITING FOR
  // TREATMENT|TREATMENT|DISCHARGED
  //         → do Repository tự gán, UI không cần nhập
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2 – INSERT DTOs  (Service → Repository)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientInsertDTO {
  QString patientCode;
  QString fullName;
  QString dateOfBirth;
  QString gender;
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  QString type;
  QString emergencyContactName;
  QString emergencyContactPhone;

  QList<AllergyInsertDTO> allergies;
  std::optional<InsuranceInsertDTO> insurance;

  virtual ~PatientInsertDTO() = default;
};

struct OutPatientInsertDTO : public PatientInsertDTO {
  std::optional<int> doctorId;
  QString status;
};

struct InPatientInsertDTO : public PatientInsertDTO {
  std::optional<int> roomId;
  std::optional<int> doctorId;
  QString admissionDate;
  QString dischargeDate;
  QString reason;
  QString status;
};

struct EmergencyPatientInsertDTO : public PatientInsertDTO {
  std::optional<int> roomId;
  std::optional<int> doctorId;
  QString injuryCause;
  QString injuryDescription;
  QString admissionDate;
  QString dischargeDate;
  QString status;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3 – UPDATE DTOs  (Service → Repository)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientUpdateDTO {
  int patientId;

  QString fullName;
  QString dateOfBirth;
  QString gender;
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  QString emergencyContactName;
  QString emergencyContactPhone;

  virtual ~PatientUpdateDTO() = default;
};

struct OutPatientUpdateDTO : public PatientUpdateDTO {
  QString status;
  std::optional<int> doctorId;
};

struct InPatientUpdateDTO : public PatientUpdateDTO {
  std::optional<int> roomId;
  std::optional<int> doctorId;
  QString admissionDate;
  QString dischargeDate;
  QString reason;
  QString status;
};

struct EmergencyPatientUpdateDTO : public PatientUpdateDTO {
  std::optional<int> roomId;
  std::optional<int> doctorId;
  QString injuryCause;
  QString injuryDescription;
  QString admissionDate;
  QString dischargeDate;
  QString status;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4 – RESULT DTOs  (Repository → Service → UI)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientDetailDTO {
  int patientId;
  QString patientCode;
  QString fullName;
  QDate dateOfBirth;
  QString gender; // Dùng QString
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  PatientType defaultPatientType;
  QString emergencyContactName;
  QString emergencyContactPhone;
  bool isDeleted;
  QDateTime createdAt;
  QDateTime updatedAt;

  QList<AllergyResultDTO> allergies;
  std::optional<InsuranceResultDTO> insurance;

  PatientType currentType;
  QString status;

  std::optional<int> roomId;
  std::optional<int> doctorId;
  std::optional<QDate> admissionDate;
  std::optional<QDate> dischargeDate;

  QString reason;
  QString injuryCause;
  QString injuryDescription;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5 – FIND / FILTER DTOs  (UI → Service → Repository)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientSearchCriteria {
  QString searchKey;

  std::optional<PatientType> type;
  int roomId = -1;

  QString status;
  bool onlyActive = true;
  bool includeDeleted = false;

  std::optional<QDate> fromDate;
  std::optional<QDate> toDate;

  int limit = 50;
  int offset = 0;
};

struct PatientSearchResultDTO {
  int patientId;
  QString patientCode;
  QString fullName;
  QDate dateOfBirth;
  QString gender; // Dùng QString
  QString phone;

  PatientType type;
  QString statusLabel;
  QString roomId;
};
