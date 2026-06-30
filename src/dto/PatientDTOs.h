/**
 * @file    patientDTOs.h
 * @brief   Data Transfer Objects cho Patient module
 *          Chỉ là data containers — không có behavior
 *
 *  Luồng dữ liệu:
 *    UI  →  (InputDTO)   →  Service
 *    Service  →  (InsertDTO / UpdateDTO)  →  Repository  →  DB
 *    DB  →  (ResultDTO)  →  Repository  →  Service  →  UI
 *
 *  Bảng DB tương ứng (đọc từ DatabaseManager.cpp):
 *    patients                      – thông tin chung
 *    out_patients                  – bệnh nhân ngoại trú
 *    in_patients                   – bệnh nhân nội trú
 *    emergency_patients_admissions – bệnh nhân cấp cứu
 */

#pragma once

#include "model/CommonEnums.h"
#include <QDate>
#include <QDateTime>
#include <QString>
#include <optional>
#include <qcoreapplication.h>

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1 – INPUT DTOs  (UI → Service)
//   UI đọc form rồi đóng thành DTO, truyền lên Service.
//   Không chứa patientId, patientCode, created_at, updated_at
//   vì những trường đó do Service/Repository tự sinh.
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Thông tin cơ bản bệnh nhân từ form UI.
 *        Map với bảng `patients`.
 */
struct PatientInputDTO {
  // ── patients ──────────────────────────────────────────
  QString fullName;              // full_name           NOT NULL
  QDate dateOfBirth;             // date_of_birth        NOT NULL
  Gender gender;                 // gender               NOT NULL
  QString citizenId;             // citizen_id           UNIQUE (nullable)
  QString phone;                 // phone                (nullable)
  QString email;                 // email                (nullable)
  QString address;               // address              (nullable)
  QString bloodType;             // blood_type           DEFAULT 'UNKNOWN'
  PatientType type;              // default_patient_type DEFAULT 'OUTPATIENT'
  QString emergencyContactName;  // emergency_contact_name  (nullable)
  QString emergencyContactPhone; // emergency_contact_phone (nullable)

  virtual ~PatientInputDTO() = default;
};

/**
 * @brief Thêm thông tin nội trú từ form UI.
 *        Map với bảng `in_patients`.
 */
struct InPatientInputDTO : public PatientInputDTO {
  std::optional<int> roomId;          // room_id             (nullable FK)
  std::optional<int> doctorId;        // admitting_doctor_id (nullable FK)
  QDate admissionDate;                // admission_date       NOT NULL
  std::optional<QDate> dischargeDate; // discharge_date       (nullable)
  QString reason;                     // reason               (nullable)
};

/**
 * @brief Thêm thông tin cấp cứu từ form UI.
 *        Map với bảng `emergency_patients_admissions`.
 */
struct EmergencyPatientInputDTO : public PatientInputDTO {
  std::optional<int> roomId;          // room_id              (nullable FK)
  std::optional<int> doctorId;        // emergency_doctor_id  (nullable FK)
  QString injuryCause;                // injury_cause         (nullable)
  QString injuryDescription;          // injury_description   (nullable)
  QDate admissionDate;                // admission_date       NOT NULL
  std::optional<QDate> dischargeDate; // discharge_date       (nullable)
};

/**
 * @brief Bệnh nhân ngoại trú – không có thêm trường nào ngoài base.
 *        Map với bảng `out_patients`.
 */
struct OutPatientInputDTO : public PatientInputDTO {
  std::optional<int> doctorId;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2 – INSERT DTOs  (Service → Repository)
//   Service chịu trách nhiệm:
//     - Sinh patientCode
//     - Lấy ngày hiện tại làm created_at / admission_date nếu không có
//     - Gán trạng thái ban đầu cho từng loại
//   Repository nhận DTO này và thực hiện INSERT vào DB.
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief INSERT vào bảng `patients`.
 *        Tất cả NON-NULL fields trong bảng phải có mặt ở đây.
 */
struct PatientInsertDTO {
  // ── sinh bởi Service ──────────────────────────────────
  QString patientCode; // patient_code         NOT NULL UNIQUE

  // ── từ PatientInputDTO ────────────────────────────────
  QString fullName;              // full_name            NOT NULL
  QString dateOfBirth;           // date_of_birth        NOT NULL
  QString gender;                // gender               NOT NULL
  QString citizenId;             // citizen_id           (nullable)
  QString phone;                 // phone                (nullable)
  QString email;                 // email                (nullable)
  QString address;               // address              (nullable)
  QString bloodType;             // blood_type           DEFAULT 'UNKNOWN'
  QString type;                  // default_patient_type DEFAULT 'OUTPATIENT'
  QString emergencyContactName;  // emergency_contact_name  (nullable)
  QString emergencyContactPhone; // emergency_contact_phone (nullable)

  PatientInsertDTO() = default;
  virtual ~PatientInsertDTO() = default;
  PatientInsertDTO(const PatientInputDTO &inputInformation,
                   const QString &generatePatientCode)
      : patientCode(generatePatientCode.trimmed()),
        fullName(inputInformation.fullName.trimmed()),
        dateOfBirth(inputInformation.dateOfBirth.toString("yyyy-MM-dd")),
        gender(GenderToString(inputInformation.gender)),
        citizenId(inputInformation.citizenId.trimmed()),
        phone(inputInformation.phone.trimmed()),
        email(inputInformation.email.trimmed()),
        address(inputInformation.address.trimmed()),
        bloodType(inputInformation.bloodType.trimmed()),
        type(PatientTypeToString(inputInformation.type)),
        emergencyContactName(inputInformation.emergencyContactName.trimmed()),
        emergencyContactPhone(
            inputInformation.emergencyContactPhone.trimmed()) {}
};

/**
 * @brief INSERT vào bảng `out_patients`.
 *        Repository tự gán status = 'REGISTERED'.
 */
struct OutPatientInsertDTO : public PatientInsertDTO {
  std::optional<int> doctorId; // doctor_id            (nullable)
  QString status;              // status  DEFAULT 'REGISTERED'

  OutPatientInsertDTO(const OutPatientInputDTO &inputInformation,
                      const QString &generatePatientCode)
      : PatientInsertDTO(inputInformation, generatePatientCode),
        doctorId(inputInformation.doctorId), status("REGISTERED") {}
};

/**
 * @brief INSERT vào bảng `in_patients`.
 *        Repository tự gán status = 'ADMITTED'.
 */
struct InPatientInsertDTO : public PatientInsertDTO {
  std::optional<int> roomId;   // room_id              (nullable)
  std::optional<int> doctorId; // admitting_doctor_id  (nullable)
  QString admissionDate;       // admission_date       NOT NULL
  QString dischargeDate;       // discharge_date       (nullable)
  QString reason;              // reason               (nullable)
  QString status;              // status  DEFAULT 'ADMITTED'
  InPatientInsertDTO(const InPatientInputDTO &inputInformation,
                     const QString &generatePatientCode)
      : PatientInsertDTO(inputInformation, generatePatientCode),
        roomId(inputInformation.roomId), doctorId(inputInformation.doctorId),
        admissionDate(inputInformation.admissionDate.toString("yyyy-MM-dd")),
        dischargeDate(inputInformation.dischargeDate.value_or(QDate()).toString(
            "yyyy-MM-dd")),
        reason(inputInformation.reason.trimmed()), status("ADMITTED") {}
};

/**
 * @brief INSERT vào bảng `emergency_patients_admissions`.
 *        Repository tự gán status = 'EMERGENCY'.
 */
struct EmergencyPatientInsertDTO : public PatientInsertDTO {
  std::optional<int> roomId;   // room_id              (nullable)
  std::optional<int> doctorId; // emergency_doctor_id  (nullable)
  QString injuryCause;         // injury_cause         (nullable)
  QString injuryDescription;   // injury_description   (nullable)
  QString admissionDate;       // admission_date       NOT NULL
  QString dischargeDate;       // discharge_date       (nullable)
  QString status;              // status  DEFAULT 'EMERGENCY'

  EmergencyPatientInsertDTO(const EmergencyPatientInputDTO &inputInformation,
                            const QString &generatePatientCode)
      : PatientInsertDTO(inputInformation, generatePatientCode),
        roomId(inputInformation.roomId), doctorId(inputInformation.doctorId),
        injuryCause(inputInformation.injuryCause.trimmed()),
        injuryDescription(inputInformation.injuryDescription.trimmed()),
        admissionDate(inputInformation.admissionDate.toString("yyyy-MM-dd")),
        dischargeDate(inputInformation.dischargeDate.value_or(QDate()).toString(
            "yyyy-MM-dd")),
        status("EMERGENCY") {}
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3 – UPDATE DTOs  (Service → Repository)
//   Chứa patientId để xác định bản ghi cần cập nhật.
//   updated_at do trigger SQLite tự cập nhật.
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief UPDATE bảng `patients` (thông tin cơ bản).
 */
struct PatientUpdateDTO {
  int patientId; // PK – bắt buộc

  QString fullName;
  QDate dateOfBirth;
  Gender gender;
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  PatientType defaultPatientType;
  QString emergencyContactName;
  QString emergencyContactPhone;
};

/**
 * @brief UPDATE bảng `out_patients` (chỉ status thay đổi theo nghiệp vụ).
 */
struct OutPatientUpdateDTO {
  int patientId; // PK
  OutPatientState status;
};

/**
 * @brief UPDATE bảng `in_patients`.
 */
struct InPatientUpdateDTO {
  int patientId; // PK
  std::optional<int> roomId;
  std::optional<int> admittingDoctorId;
  QDate admissionDate;
  std::optional<QDate> dischargeDate;
  QString reason;
  InPatientState status;
};

/**
 * @brief UPDATE bảng `emergency_patients_admissions`.
 */
struct EmergencyPatientUpdateDTO {
  int patientId; // PK
  std::optional<int> roomId;
  std::optional<int> emergencyDoctorId;
  QString injuryCause;
  QString injuryDescription;
  QDate admissionDate;
  std::optional<QDate> dischargeDate;
  EmergencyPatientState status;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4 – RESULT DTOs  (Repository → Service → UI)
//   Kết quả đọc từ DB. Bao gồm tất cả cột, kể cả PK và timestamp.
//   Dùng cho getById, getByCode, getAll, search.
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Kết quả JOIN patients + out_patients.
 */
struct OutPatientDTO {
  // ── patients ──────────────────────────────────────────
  int patientId;
  QString patientCode;
  QString fullName;
  QDate dateOfBirth;
  Gender gender;
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  QString emergencyContactName;
  QString emergencyContactPhone;

  // ── out_patients ──────────────────────────────────────
  OutPatientState
      status; // REGISTERED | WAITING FOR TREATMENT | TREATMENT | DISCHARGED

  // ── metadata ──────────────────────────────────────────
  QDateTime createdAt;
  QDateTime updatedAt;
};

/**
 * @brief Kết quả JOIN patients + in_patients.
 */
struct InPatientDTO {
  // ── patients ──────────────────────────────────────────
  int patientId;
  QString patientCode;
  QString fullName;
  QDate dateOfBirth;
  Gender gender;
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  QString emergencyContactName;
  QString emergencyContactPhone;

  // ── in_patients ───────────────────────────────────────
  std::optional<int> roomId;
  std::optional<int> admittingDoctorId;
  QDate admissionDate;
  std::optional<QDate> dischargeDate;
  QString reason;
  InPatientState status; // ADMITTED | DISCHARGED | TRANSFERRED

  // ── metadata ──────────────────────────────────────────
  QDateTime createdAt;
  QDateTime updatedAt;
};

/**
 * @brief Kết quả JOIN patients + emergency_patients_admissions.
 */
struct EmergencyPatientDTO {
  // ── patients ──────────────────────────────────────────
  int patientId;
  QString patientCode;
  QString fullName;
  QDate dateOfBirth;
  Gender gender;
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  QString emergencyContactName;
  QString emergencyContactPhone;

  // ── emergency_patients_admissions ─────────────────────
  std::optional<int> roomId;
  std::optional<int> emergencyDoctorId;
  QString injuryCause;
  QString injuryDescription;
  QDate admissionDate;
  std::optional<QDate> dischargeDate;
  EmergencyPatientState status; // EMERGENCY | DISCHARGED | TRANSFERRED

  // ── metadata ──────────────────────────────────────────
  QDateTime createdAt;
  QDateTime updatedAt;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5 – FIND / FILTER DTOs  (UI → Service → Repository)
//   Dùng để truyền tiêu chí tìm kiếm, lọc danh sách bệnh nhân.
//   Các trường để rỗng / std::nullopt nghĩa là không lọc theo trường đó.
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Tìm kiếm / lọc bệnh nhân ngoại trú.
 */
struct OutPatientFindDTO {
  QString keyword; // tìm theo full_name, patient_code, citizen_id
  std::optional<OutPatientState>
      status; // lọc theo trạng thái (nullopt = tất cả)
};

/**
 * @brief Tìm kiếm / lọc bệnh nhân nội trú.
 */
struct InPatientFindDTO {
  QString keyword;                      // full_name, patient_code, citizen_id
  std::optional<InPatientState> status; // nullopt = tất cả
  std::optional<int> roomId;            // lọc theo phòng
  std::optional<int> admittingDoctorId;
};

/**
 * @brief Tìm kiếm / lọc bệnh nhân cấp cứu.
 */
struct EmergencyPatientFindDTO {
  QString keyword; // full_name, patient_code, citizen_id
  std::optional<EmergencyPatientState> status; // nullopt = tất cả
  std::optional<int> roomId;
  std::optional<int> emergencyDoctorId;
};