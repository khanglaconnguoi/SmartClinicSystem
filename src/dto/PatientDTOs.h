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
 *    emergency_patients            – bệnh nhân cấp cứu
 *    patient_allergies             – dị ứng (1-n)
 *    patient_insurance             – bảo hiểm (1-1)
 */

#pragma once

#include "model/CommonEnums.h"
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QString>
#include <optional>
#include <qcoreapplication.h>
#include <qhashfunctions.h>

// ═══════════════════════════════════════════════════════════════════════════
// ALLERGY DTOs  (map với bảng `patient_allergies`)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Dùng để INSERT một dị ứng vào bảng `patient_allergies`.
 */
struct AllergyInputDTO {
  std::optional<int> ingredientId;
  QString allergenName; // tên chất gây dị ứng
  QString severity;     // 'MILD' | 'MODERATE' | 'SEVERE'
  QString notes;        // ghi chú thêm (tùy chọn)
};

/**
 * @brief Kết quả đọc từ bảng `patient_allergies`.
 */
struct AllergyResultDTO {
  int allergyId;
  std::optional<int> ingredientId;
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

/**
 * @brief Dùng để INSERT / UPSERT bản ghi bảo hiểm vào `patient_insurance`.
 */
struct InsuranceInputDTO {
  QString providerName;          // tên công ty bảo hiểm
  QString policyNumber;          // số thẻ / hợp đồng
  QString insuranceType;         // 'BHYT' | 'PRIVATE' | 'OTHER'
  double coveragePercent = 80.0; // % chi trả
  QDate validFrom;               // ngày hiệu lực  (yyyy-MM-dd)
  QDate validTo;                 // ngày hết hạn   (yyyy-MM-dd)
  QString notes;
};

struct InsuranceInsertDTO {
  QString providerName;
  QString policyNumber;
  QString insuranceType;
  double coveragePercent = 80.0;
  QString validFrom;
  QString validTo;
  QString notes;
};

/**
 * @brief Kết quả đọc từ bảng `patient_insurance`.
 */
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
  QString fullName;  // full_name           NOT NULL
  QDate dateOfBirth; // date_of_birth        NOT NULL
  QString gender;    // gender               NOT NULL
  QString citizenId; // citizen_id           UNIQUE (nullable)
  QString phone;     // phone                (nullable)
  QString email;     // email                (nullable)
  QString address;   // address              (nullable)
  QString bloodType; // blood_type           DEFAULT 'UNKNOWN'
  QList<AllergyInputDTO> allergies;
  InsuranceInputDTO insurance;   // insurance            (nullable, future use)
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
  int roomId;                         // room_id              (nullable FK)
  int doctorId;                       // admitting_doctor_id  (nullable FK)
  QDate admissionDate;                // admission_date       NOT NULL
  std::optional<QDate> dischargeDate; // discharge_date       (nullable)
  QString reason;                     // reason               (nullable)
};

/**
 * @brief Thêm thông tin cấp cứu từ form UI.
 *        Map với bảng `emergency_patients_admissions`.
 */
struct EmergencyPatientInputDTO : public PatientInputDTO {
  int roomId;                         // room_id              (nullable FK)
  int doctorId;                       // emergency_doctor_id  (nullable FK)
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
  int doctorId;
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

  // -- di ung & bao hiem -----------------------------------------
  QList<AllergyInputDTO> allergies; // danh sach di ung (co the rong)
  InsuranceInsertDTO insurance;     // bao hiem (da convert QDate->QString)

  PatientInsertDTO() = default;
  virtual ~PatientInsertDTO() = default;
  PatientInsertDTO(const PatientInputDTO &inputInformation,
                   const QString &generatePatientCode, const QString &patientType)
      : patientCode(generatePatientCode.trimmed()),
        fullName(inputInformation.fullName.trimmed()),
        dateOfBirth(inputInformation.dateOfBirth.toString("yyyy-MM-dd")),
        gender(inputInformation.gender),
        citizenId(inputInformation.citizenId.trimmed()),
        phone(inputInformation.phone.trimmed()),
        email(inputInformation.email.trimmed()),
        address(inputInformation.address.trimmed()),
        bloodType(inputInformation.bloodType.trimmed()), type(patientType.trimmed()),
        emergencyContactName(inputInformation.emergencyContactName.trimmed()),
        emergencyContactPhone(
            inputInformation.emergencyContactPhone.trimmed()),
        allergies(inputInformation.allergies) {
    // Convert InsuranceInputDTO (QDate) -> InsuranceInsertDTO (QString) tai day
    // de repo khong can xu ly logic chuyen doi nua.
    const auto &ins = inputInformation.insurance;
    insurance.providerName   = ins.providerName;
    insurance.policyNumber   = ins.policyNumber;
    insurance.insuranceType  = ins.insuranceType;
    insurance.coveragePercent = ins.coveragePercent;
    insurance.validFrom = ins.validFrom.isValid()
                              ? ins.validFrom.toString("yyyy-MM-dd")
                              : QString();
    insurance.validTo = ins.validTo.isValid()
                            ? ins.validTo.toString("yyyy-MM-dd")
                            : QString();
    insurance.notes = ins.notes;
  }
};


/**
 * @brief INSERT vào bảng `out_patients`.
 *        Repository tự gán status = 'REGISTERED'.
 */
struct OutPatientInsertDTO : public PatientInsertDTO {
  int doctorId;   // doctor_id            (nullable)
  QString status; // status  DEFAULT 'REGISTERED'

  OutPatientInsertDTO(const OutPatientInputDTO &inputInformation,
                      const QString &generatePatientCode)
      : PatientInsertDTO(inputInformation, generatePatientCode,
                         patientTypeToEn(PatientType::Outpatient)),
        doctorId(inputInformation.doctorId),
        status(outPatientStateToEn(OutPatientState::Registered)) {}
};

/**
 * @brief INSERT vào bảng `in_patients`.
 *        Repository tự gán status = 'ADMITTED'.
 */
struct InPatientInsertDTO : public PatientInsertDTO {
  int roomId;            // room_id              (nullable)
  int doctorId;          // admitting_doctor_id  (nullable)
  QString admissionDate; // admission_date       NOT NULL
  QString dischargeDate; // discharge_date       (nullable)
  QString reason;        // reason               (nullable)
  QString status;        // status  DEFAULT 'ADMITTED'
  InPatientInsertDTO(const InPatientInputDTO &inputInformation,
                     const QString &generatePatientCode)
      : PatientInsertDTO(inputInformation, generatePatientCode,
                         patientTypeToEn(PatientType::Inpatient)),
        roomId(inputInformation.roomId), doctorId(inputInformation.doctorId),
        admissionDate(inputInformation.admissionDate.toString("yyyy-MM-dd")),
        dischargeDate(inputInformation.dischargeDate.value_or(QDate()).toString(
            "yyyy-MM-dd")),
        reason(inputInformation.reason.trimmed()),
        status(inPatientStateToEn(InPatientState::Admitted)) {}
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
      : PatientInsertDTO(inputInformation, generatePatientCode,
                         patientTypeToEn(PatientType::Emergency)),
        roomId(inputInformation.roomId), doctorId(inputInformation.doctorId),
        injuryCause(inputInformation.injuryCause.trimmed()),
        injuryDescription(inputInformation.injuryDescription.trimmed()),
        admissionDate(inputInformation.admissionDate.toString("yyyy-MM-dd")),
        dischargeDate(inputInformation.dischargeDate.value_or(QDate()).toString(
            "yyyy-MM-dd")),
        status(emergencyPatientStateToEn(EmergencyPatientState::Emergency)) {}
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
  QString dateOfBirth;
  QString gender;
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  QString emergencyContactName;
  QString emergencyContactPhone;
  PatientUpdateDTO() = default;
  virtual ~PatientUpdateDTO() = default;
  PatientUpdateDTO(const PatientInputDTO &inputInformation, int pId) {
    this->patientId = pId;
    this->fullName = inputInformation.fullName.trimmed();
    this->dateOfBirth = inputInformation.dateOfBirth.toString("yyyy-MM-dd");
    this->gender = inputInformation.gender;
    this->citizenId = inputInformation.citizenId.trimmed();
    this->phone = inputInformation.phone.trimmed();
    this->email = inputInformation.email.trimmed();
    this->address = inputInformation.address.trimmed();
    this->bloodType = inputInformation.bloodType;
    this->emergencyContactName = inputInformation.emergencyContactName;
    this->emergencyContactPhone = inputInformation.emergencyContactPhone;
  }
};

/**
 * @brief UPDATE bảng `out_patients` (chỉ status thay đổi theo nghiệp vụ).
 */
struct OutPatientUpdateDTO : public PatientUpdateDTO {
  QString status;
  std::optional<int> doctorId;
  OutPatientUpdateDTO(const OutPatientInputDTO &inputInformation, int pId,
                      const QString &newStatus = "REGISTERED")
      : PatientUpdateDTO(inputInformation, pId), status(newStatus),
        doctorId(inputInformation.doctorId) {}
};

/**
 * @brief UPDATE bảng `in_patients`.
 */
struct InPatientUpdateDTO : public PatientUpdateDTO {
  std::optional<int> roomId;
  std::optional<int> doctorId;
  QString admissionDate;
  QString dischargeDate;
  QString reason;
  QString status;
  InPatientUpdateDTO(const InPatientInputDTO &inputInformation, int pId,
                     const QString &newStatus = "ADMITTED")
      : PatientUpdateDTO(inputInformation, pId),
        roomId(inputInformation.roomId), doctorId(inputInformation.doctorId),
        admissionDate(inputInformation.admissionDate.toString("yyyy-MM-dd")),
        dischargeDate(inputInformation.dischargeDate.value_or(QDate()).toString(
            "yyyy-MM-dd")),
        reason(inputInformation.reason), status(newStatus) {}
};

/**
 * @brief UPDATE bảng `emergency_patients_admissions`.
 */
struct EmergencyPatientUpdateDTO : public PatientUpdateDTO {
  std::optional<int> roomId;
  std::optional<int> doctorId;
  QString injuryCause;
  QString injuryDescription;
  QString admissionDate;
  QString dischargeDate;
  QString status;
  EmergencyPatientUpdateDTO(const EmergencyPatientInputDTO &inputInformation,
                            int pId, const QString &newStatus = "EMERGENCY")
      : PatientUpdateDTO(inputInformation, pId),
        roomId(inputInformation.roomId), doctorId(inputInformation.doctorId),
        injuryCause(inputInformation.injuryCause),
        injuryDescription(inputInformation.injuryDescription),
        admissionDate(inputInformation.admissionDate.toString("yyyy-MM-dd")),
        dischargeDate(inputInformation.dischargeDate.value_or(QDate()).toString(
            "yyyy-MM-dd")),
        status(newStatus) {}
};

// //
// ═══════════════════════════════════════════════════════════════════════════
// // SECTION 4 – RESULT DTOs  (Repository → Service → UI)
// //   Kết quả đọc từ DB. Bao gồm tất cả cột, kể cả PK và timestamp.
// //   Dùng cho getById, getByCode, getAll, search.
// //
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Kết quả lấy chi tiết một bệnh nhân (Dùng cho getById).
 *        Cấu trúc làm phẳng (flatten) tất cả các trường để UI dễ dàng bind dữ
 * liệu.
 */
struct PatientDetailDTO {
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
  PatientType defaultPatientType;
  QString emergencyContactName;
  QString emergencyContactPhone;
  bool isDeleted;
  QDateTime createdAt;
  QDateTime updatedAt;

  // ── dị ứng & bảo hiểm ─────────────────────────────────
  QList<AllergyResultDTO> allergies; // từ patient_allergies
  std::optional<InsuranceResultDTO>
      insurance; // từ patient_insurance (nullopt nếu chưa có)

  // ── thông tin chuyên biệt từ bảng con ──────────────────
  PatientType currentType; // OUTPATIENT / INPATIENT / EMERGENCY
  QString status;          // Trạng thái từ bảng con (REGISTERED, ADMITTED...)

  std::optional<int> roomId;          // Dùng cho INPATIENT / EMERGENCY
  std::optional<int> doctorId;        // Bác sĩ phụ trách (Dùng chung)
  std::optional<QDate> admissionDate; // Dùng cho INPATIENT / EMERGENCY
  std::optional<QDate> dischargeDate; // Dùng cho INPATIENT / EMERGENCY

  QString reason;            // Lý do nhập viện (INPATIENT)
  QString injuryCause;       // Nguyên nhân tai nạn (EMERGENCY)
  QString injuryDescription; // Mô tả chấn thương (EMERGENCY)
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5 – FIND / FILTER DTOs  (UI → Service → Repository)
//   Dùng để truyền tiêu chí tìm kiếm, lọc danh sách bệnh nhân.
//   Các trường để rỗng / std::nullopt nghĩa là không lọc theo trường đó.
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Tiêu chí tìm kiếm bệnh nhân (mọi loại:
 * OutPatient/InPatient/Emergency).
 *
 *  Tất cả trường để rỗng / std::nullopt / -1 nghĩa là "không lọc theo
 *  trường đó". Service sẽ validate (vd. fromDate <= toDate) trước khi
 *  truyền xuống Repository.
 */
struct PatientSearchCriteria {
  // ── TEXT search ───────────────────────────────────────
  // So khớp (LIKE) với: full_name, patient_code, citizen_id, phone
  QString searchKey;

  // ── Dropdown filter ───────────────────────────────────
  std::optional<PatientType> type; // nullopt = tìm cả 3 loại
  int roomId = -1;                 // -1 = tất cả phòng

  // ── Status filter ─────────────────────────────────────
  QString status; // rỗng = tất cả trạng thái (REGISTERED/ADMITTED/...)
  bool onlyActive = true;
  bool includeDeleted = false;

  // ── Date range filter (theo admission_date) ──────────
  std::optional<QDate> fromDate;
  std::optional<QDate> toDate;

  // ── Phân trang ────────────────────────────────────────
  int limit = 50;
  int offset = 0;
};

/**
 * @brief Một dòng kết quả tìm kiếm bệnh nhân.
 *
 *  Vì OutPatient/InPatient/EmergencyPatient nằm ở 3 bảng con khác cột
 *  nhau, DTO này được "làm phẳng" để UI hiển thị thống nhất trên 1
 *  QTableView, bất kể bệnh nhân thuộc loại nào.
 */
struct PatientSearchResultDTO {
  int patientId;
  QString patientCode;
  QString fullName;
  QDate dateOfBirth;
  Gender gender;
  QString phone;

  PatientType type;    // OUTPATIENT / INPATIENT / EMERGENCY
  QString statusLabel; // status tương ứng lấy từ bảng con
  QString roomId;      // rỗng nếu là OutPatient (không có phòng)
};
// struct OutPatientFindDTO {
//   QString keyword; // tìm theo full_name, patient_code, citizen_id
//   std::optional<OutPatientState>
//       status; // lọc theo trạng thái (nullopt = tất cả)
// };
// struct OutPatientFindDTO {
//   QString keyword; // tìm theo full_name, patient_code, citizen_id
//   std::optional<OutPatientState>
//       status; // lọc theo trạng thái (nullopt = tất cả)
// };

// /**
//  * @brief Tìm kiếm / lọc bệnh nhân nội trú.
//  */
// struct InPatientFindDTO {
//   QString keyword;                      // full_name, patient_code,
//   citizen_id std::optional<InPatientState> status; // nullopt = tất cả
//   std::optional<int> roomId;            // lọc theo phòng
//   std::optional<int> admittingDoctorId;
// };

// /**
//  * @brief Tìm kiếm / lọc bệnh nhân cấp cứu.
//  */
// struct EmergencyPatientFindDTO {
//   QString keyword; // full_name, patient_code, citizen_id
//   std::optional<EmergencyPatientState> status; // nullopt = tất cả
//   std::optional<int> roomId;
//   std::optional<int> emergencyDoctorId;
// };