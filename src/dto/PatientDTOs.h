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

// ═══════════════════════════════════════════════════════════════════════════
// ALLERGY DTOs  (map với bảng `patient_allergies`)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Dùng để INSERT một dị ứng vào bảng `patient_allergies`.
 */
struct AllergyInsertDTO {
  int patientId;        // patient_id    NOT NULL  (tự gán bởi Service/Repo)
  QString allergenName; // allergen_name NOT NULL  COLLATE NOCASE  UNIQUE per patient
  QString severity;     // severity      NOT NULL  DEFAULT 'MODERATE'  CHECK: MILD|MODERATE|SEVERE
  QString notes;        // notes         nullable
};

/**
 * @brief Kết quả đọc từ bảng `patient_allergies`.
 */
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

/**
 * @brief Dùng để INSERT / UPSERT bản ghi bảo hiểm vào `patient_insurance`.
 */
struct InsuranceInsertDTO {
  int patientId;                 // patient_id       NOT NULL  (tự gán bởi Service/Repo)
  QString providerName;          // provider_name    NOT NULL
  QString policyNumber;          // policy_number    NOT NULL
  QString insuranceType;         // insurance_type   NOT NULL  DEFAULT 'BHYT'  CHECK: BHYT|PRIVATE|OTHER
  double coveragePercent = 80.0; // coverage_percent NOT NULL  DEFAULT 80
  QString validFrom;             // valid_from       nullable  (yyyy-MM-dd)
  QString validTo;               // valid_to         nullable  (yyyy-MM-dd)
  QString notes;                 // notes            nullable
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
  // ── patients ─────────────────────────────────────────────────────────────
  QString fullName;  // full_name               NOT NULL
  QDate dateOfBirth; // date_of_birth            NOT NULL
  Gender gender;     // gender                   NOT NULL  CHECK: MALE|FEMALE|OTHER
  QString citizenId; // citizen_id               nullable  UNIQUE
  QString phone;     // phone                    NOT NULL
  QString email;     // email                    NOT NULL  (DB: NOT NULL, dù có thể rỗng)
  QString address;   // address                  NOT NULL
  QString bloodType; // blood_type               NOT NULL  DEFAULT 'UNKNOWN'  CHECK: A+|A-|B+|B-|AB+|AB-|O+|O-|UNKNOWN
  // QString allergies; // → dùng QList<AllergyInsertDTO> riêng biệt
  QString insurance;             // → raw string bảo hiểm từ UI (Service sẽ parse)
  PatientType type;              // default_patient_type  NOT NULL  DEFAULT 'OUTPATIENT'  CHECK: OUTPATIENT|INPATIENT|EMERGENCY
  QString emergencyContactName;  // emergency_contact_name  NOT NULL
  QString emergencyContactPhone; // emergency_contact_phone NOT NULL

  virtual ~PatientInputDTO() = default;
};

/**
 * @brief Thêm thông tin nội trú từ form UI.
 *        Map với bảng `in_patients`.
 */
struct InPatientInputDTO : public PatientInputDTO {
  // ── in_patients ───────────────────────────────────────────────────────────
  std::optional<int> roomId;          // room_id        nullable  FK → rooms
  std::optional<int> doctorId;        // doctor_id      nullable  FK → staff
  QDate admissionDate;                // admission_date NOT NULL  DEFAULT datetime('now')
  std::optional<QDate> dischargeDate; // discharge_date nullable  CHECK: >= admission_date
  QString reason;                     // reason         nullable
};

/**
 * @brief Thêm thông tin cấp cứu từ form UI.
 *        Map với bảng `emergency_patients_admissions`.
 */
struct EmergencyPatientInputDTO : public PatientInputDTO {
  // ── emergency_patients ────────────────────────────────────────────────────
  std::optional<int> roomId;          // room_id              nullable  FK → rooms
  std::optional<int> doctorId;        // doctor_id            nullable  FK → staff
  QString injuryCause;                // injury_cause         nullable
  QString injuryDescription;          // injury_description   nullable
  QDate admissionDate;                // admission_date       NOT NULL  DEFAULT datetime('now')
  std::optional<QDate> dischargeDate; // discharge_date       nullable  CHECK: >= admission_date
};

/**
 * @brief Bệnh nhân ngoại trú – không có thêm trường nào ngoài base.
 *        Map với bảng `out_patients`.
 */
struct OutPatientInputDTO : public PatientInputDTO {
  // ── out_patients ──────────────────────────────────────────────────────────
  std::optional<int> doctorId; // doctor_id  nullable  FK → staff
  // status  NOT NULL  DEFAULT 'REGISTERED'  CHECK: REGISTERED|WAITING FOR TREATMENT|TREATMENT|DISCHARGED
  //         → do Repository tự gán, UI không cần nhập
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

  // ── từ PatientInputDTO ──────────────────────────────────────────────────
  QString fullName;              // full_name               NOT NULL
  QString dateOfBirth;           // date_of_birth           NOT NULL  (yyyy-MM-dd)
  QString gender;                // gender                  NOT NULL  CHECK: MALE|FEMALE|OTHER
  QString citizenId;             // citizen_id              nullable  UNIQUE
  QString phone;                 // phone                   NOT NULL
  QString email;                 // email                   NOT NULL
  QString address;               // address                 NOT NULL
  QString bloodType;             // blood_type              NOT NULL  DEFAULT 'UNKNOWN'  CHECK: A+|A-|B+|B-|AB+|AB-|O+|O-|UNKNOWN
  QString type;                  // default_patient_type    NOT NULL  DEFAULT 'OUTPATIENT'  CHECK: OUTPATIENT|INPATIENT|EMERGENCY
  QString emergencyContactName;  // emergency_contact_name  NOT NULL
  QString emergencyContactPhone; // emergency_contact_phone NOT NULL

  // ── dị ứng & bảo hiểm ─────────────────────────────────────────────────
  QList<AllergyInsertDTO> allergies;           // patient_allergies  nullable  (danh sách, có thể rỗng)
  std::optional<InsuranceInsertDTO> insurance; // patient_insurance  nullable  (nullopt nếu không có)


  PatientInsertDTO() = default;
  virtual ~PatientInsertDTO() = default;
  PatientInsertDTO(const PatientInputDTO &inputInformation,
                   const QString &generatePatientCode)
      : patientCode(generatePatientCode.trimmed()),
        fullName(inputInformation.fullName.trimmed()),
        dateOfBirth(inputInformation.dateOfBirth.toString("yyyy-MM-dd")),
        gender(genderToString(inputInformation.gender)),
        citizenId(inputInformation.citizenId.trimmed()),
        phone(inputInformation.phone.trimmed()),
        email(inputInformation.email.trimmed()),
        address(inputInformation.address.trimmed()),
        bloodType(inputInformation.bloodType.trimmed()),
        type(inputInformation.type == PatientType::Inpatient
                 ? "INPATIENT"
                 : (inputInformation.type == PatientType::Emergency
                        ? "EMERGENCY"
                        : "OUTPATIENT")),
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
  // ── in_patients ───────────────────────────────────────────────────────────
  std::optional<int> roomId;   // room_id        nullable  FK → rooms
  std::optional<int> doctorId; // doctor_id      nullable  FK → staff
  QString admissionDate;       // admission_date NOT NULL  (yyyy-MM-dd)
  QString dischargeDate;       // discharge_date nullable  CHECK: >= admission_date
  QString reason;              // reason         nullable
  QString status;              // status         NOT NULL  DEFAULT 'ADMITTED'  CHECK: ADMITTED|DISCHARGED|TRANSFERRED
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
  // ── emergency_patients ────────────────────────────────────────────────────
  std::optional<int> roomId;   // room_id             nullable  FK → rooms
  std::optional<int> doctorId; // doctor_id           nullable  FK → staff
  QString injuryCause;         // injury_cause        nullable
  QString injuryDescription;   // injury_description  nullable
  QString admissionDate;       // admission_date      NOT NULL  DEFAULT datetime('now')  (yyyy-MM-dd)
  QString dischargeDate;       // discharge_date      nullable  CHECK: >= admission_date
  QString status;              // status              NOT NULL  DEFAULT 'EMERGENCY'  CHECK: EMERGENCY|DISCHARGED|TRANSFERRED

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
  int patientId; // PK – NOT NULL  (bắt buộc để xác định bản ghi)

  // ── patients (tất cả NOT NULL trong DB) ──────────────────────────────────
  QString fullName;              // full_name               NOT NULL
  QString dateOfBirth;           // date_of_birth           NOT NULL  (yyyy-MM-dd)
  QString gender;                // gender                  NOT NULL  CHECK: MALE|FEMALE|OTHER
  QString citizenId;             // citizen_id              nullable  UNIQUE
  QString phone;                 // phone                   NOT NULL
  QString email;                 // email                   NOT NULL
  QString address;               // address                 NOT NULL
  QString bloodType;             // blood_type              NOT NULL  DEFAULT 'UNKNOWN'  CHECK: A+|A-|B+|B-|AB+|AB-|O+|O-|UNKNOWN
  QString emergencyContactName;  // emergency_contact_name  NOT NULL
  QString emergencyContactPhone; // emergency_contact_phone NOT NULL
  PatientUpdateDTO() = default;
  virtual ~PatientUpdateDTO() = default;
  PatientUpdateDTO(const PatientInputDTO &inputInformation, int pId) {
    this->patientId = pId;
    this->fullName = inputInformation.fullName.trimmed();
    this->dateOfBirth = inputInformation.dateOfBirth.toString("yyyy-MM-dd");
    this->gender = genderToString(inputInformation.gender);
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
  // ── out_patients ──────────────────────────────────────────────────────────
  QString status;              // status    NOT NULL  CHECK: REGISTERED|WAITING FOR TREATMENT|TREATMENT|DISCHARGED
  std::optional<int> doctorId; // doctor_id nullable  FK → staff
  OutPatientUpdateDTO(const OutPatientInputDTO &inputInformation, int pId,
                      const QString &newStatus = "REGISTERED")
      : PatientUpdateDTO(inputInformation, pId), status(newStatus),
        doctorId(inputInformation.doctorId) {}
};

/**
 * @brief UPDATE bảng `in_patients`.
 */
struct InPatientUpdateDTO : public PatientUpdateDTO {
  // ── in_patients ───────────────────────────────────────────────────────────
  std::optional<int> roomId;   // room_id        nullable  FK → rooms
  std::optional<int> doctorId; // doctor_id      nullable  FK → staff
  QString admissionDate;       // admission_date NOT NULL  (yyyy-MM-dd)
  QString dischargeDate;       // discharge_date nullable  CHECK: >= admission_date
  QString reason;              // reason         nullable
  QString status;              // status         NOT NULL  CHECK: ADMITTED|DISCHARGED|TRANSFERRED
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
  // ── emergency_patients ────────────────────────────────────────────────────
  std::optional<int> roomId;   // room_id             nullable  FK → rooms
  std::optional<int> doctorId; // doctor_id           nullable  FK → staff
  QString injuryCause;         // injury_cause        nullable
  QString injuryDescription;   // injury_description  nullable
  QString admissionDate;       // admission_date      NOT NULL  (yyyy-MM-dd)
  QString dischargeDate;       // discharge_date      nullable  CHECK: >= admission_date
  QString status;              // status              NOT NULL  CHECK: EMERGENCY|DISCHARGED|TRANSFERRED
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