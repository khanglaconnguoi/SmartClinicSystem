/**
 * @file    StaffDTOs.h
 * @brief   Data Transfer Objects cho Staff module
 *          Chỉ là data containers — không có behavior
 */
#pragma once
#include <QString>
#include <QDate>
#include <QPixmap>
#include <QBuffer>
#include "model/CommonEnums.h"

// ── INPUT DTOs ───────────────────────────────────────────────────────
// Dùng khi đọc thông tin nhân viên từ form đăng kí / edit (UI → Service)
// UI chịu trách nhiệm:
// - lấy thông tin trong form rồi dựa theo role để đóng thành DTO và gọi hàm phù hợp

struct StaffInputDTO {
    QString     fullName;
    QPixmap     avatar;
    Gender      gender;
    QDate       dateOfBirth;
    QString     citizenId;
    QString     phoneNumber;
    QString     email;
    QString     address;
    int         departmentId;
    QString     shift;
    // KHÔNG có: staffCode, plainPassword, passwordHash, hireDate

    virtual ~StaffInputDTO() = default;
};

struct DoctorInputDTO : public StaffInputDTO {
    QString specialty;
    QString licenseNumber;
    int     experienceYears;
    int     consultationFee;
    QString bio;
};

struct NurseInputDTO : public StaffInputDTO {
    QString nurseLevel;
    QString certification;
};

struct ReceptionistInputDTO : public StaffInputDTO {
    QString shift;
};

// ── INSERT DTOs ───────────────────────────────────────────────────────
// Dùng khi TẠO MỚI nhân viên (Service → Repository)
// Service chịu trách nhiệm:
//   - Gán staffCode (generated)
//   - Gán passwordHash (generated)
//   - Gán hireDate (tự lấy ngày hôm nay)

struct StaffInsertDTO {
    QString     staffCode; 
    QString     passwordHash;
    QString     fullName;
    QByteArray  avatarBytes;
    QString     role;
    QString     gender;
    QString     dateOfBirth;    // "yyyy-MM-dd"
    QString     citizenId;
    QString     phoneNumber;
    QString     email;
    QString     address;
    int         departmentId;
    QString     hireDate;       // "yyyy-MM-dd"
    QString     shift;

    StaffInsertDTO() = default;
    virtual ~StaffInsertDTO() = default;
    StaffInsertDTO(const StaffInputDTO& inputInformation, 
                   const QString& generatedStaffCode, 
                   const QString& generatedPasswordHash,
                   UserRole inputRole)
      : staffCode(generatedStaffCode.trimmed()),
        passwordHash(generatedPasswordHash),
        fullName(inputInformation.fullName.trimmed()),
        role(userRoleToEn(inputRole).trimmed()),
        gender(genderToEn(inputInformation.gender).trimmed()),
        dateOfBirth(inputInformation.dateOfBirth.toString("yyyy-MM-dd")),
        citizenId(inputInformation.citizenId.trimmed()),
        phoneNumber(inputInformation.phoneNumber.trimmed()),
        email(inputInformation.email.trimmed()), // Làm sạch & Chuẩn hóa email
        address(inputInformation.address.trimmed()),
        departmentId(inputInformation.departmentId),
        hireDate(QDate::currentDate().toString("yyyy-MM-dd")), 
        shift(inputInformation.shift.trimmed()) {

        if (!inputInformation.avatar.isNull()) {
            QBuffer buffer(&avatarBytes);
            buffer.open(QIODevice::WriteOnly);
            inputInformation.avatar.save(&buffer, "PNG"); // Chuẩn hóa ảnh về dạng định dạng PNG thô
        }
    }
};

struct DoctorInsertDTO : public StaffInsertDTO {
    QString specialty;
    QString licenseNumber;
    int     experienceYears;
    int     consultationFee;
    QString bio;

    DoctorInsertDTO() = default;
    ~DoctorInsertDTO() override = default;
    DoctorInsertDTO(const DoctorInputDTO& inputInformation,
                    const QString& generatedStaffCode,
                    const QString& generatedPasswordHash)
      : StaffInsertDTO(inputInformation, generatedStaffCode, generatedPasswordHash, UserRole::Doctor),
        specialty(inputInformation.specialty.trimmed()),
        licenseNumber(inputInformation.licenseNumber.trimmed()),
        experienceYears(inputInformation.experienceYears),
        consultationFee(inputInformation.consultationFee),
        bio(inputInformation.bio.trimmed()) {}
};

struct NurseInsertDTO : public StaffInsertDTO {
    QString nurseLevel;
    QString certification;

    NurseInsertDTO() = default;
    ~NurseInsertDTO() override = default;
    NurseInsertDTO(const NurseInputDTO& inputInformation,
                   const QString& generatedStaffCode,
                   const QString& generatedPasswordHash)
      : StaffInsertDTO(inputInformation, generatedStaffCode, generatedPasswordHash, UserRole::Nurse),
        nurseLevel(inputInformation.nurseLevel.trimmed()),
        certification(inputInformation.certification.trimmed()) {}
};

// ── UPDATE DTO ────────────────────────────────────────────────────────
// Dùng khi CẬP NHẬT thông tin — chỉ chứa field được phép sửa
// Không có: staffCode (key), role (không đổi), createdAt (bất biến)

struct StaffUpdateDTO {
    int     staffId;         // để biết UPDATE WHERE staff_id = ?

    // Những trường thông tin có thể thay đổi
    QString     fullName;           // ADMIN
    QByteArray  avatarBytes;        // ADMIN
    QString     gender;             // ADMIN
    QString     dateOfBirth;        // ADMIN
    QString     citizenId;          // ADMIN
    QString     phoneNumber;        // ADMIN & DOCTOR
    QString     email;              // ADMIN & DOCTOR
    QString     address;            // ADMIN & DOCTOR
    int         departmentId;       // ADMIN   
    QString     shift;              // ADMIN

    StaffUpdateDTO() = default;
    virtual ~StaffUpdateDTO() = default;
    StaffUpdateDTO(const StaffInputDTO& inputInformation, int inputId)
      : staffId(inputId),
        fullName(inputInformation.fullName.trimmed()),
        gender(genderToString(inputInformation.gender).trimmed()),
        dateOfBirth(inputInformation.dateOfBirth.toString("yyyy-MM-dd")),
        citizenId(inputInformation.citizenId.trimmed()),
        phoneNumber(inputInformation.phoneNumber.trimmed()),
        email(inputInformation.email.trimmed()),
        address(inputInformation.address.trimmed()),
        departmentId(inputInformation.departmentId),
        shift(inputInformation.shift) {
            
        if (!inputInformation.avatar.isNull()) {
            QBuffer buffer(&avatarBytes);
            buffer.open(QIODevice::WriteOnly);
            inputInformation.avatar.save(&buffer, "PNG"); // Chuẩn hóa ảnh về dạng định dạng PNG thô
        }
    }
};

struct DoctorUpdateDTO : public StaffUpdateDTO {
    QString specialty;
    QString licenseNumber;
    int     experienceYears;
    int     consultationFee;
    QString bio;

    DoctorUpdateDTO() = default;
    ~DoctorUpdateDTO() override = default;
    DoctorUpdateDTO(const DoctorInputDTO& inputInformation, int inputId)
      : StaffUpdateDTO(inputInformation, inputId),
        specialty(inputInformation.specialty.trimmed()),
        licenseNumber(inputInformation.licenseNumber.trimmed()),
        experienceYears(inputInformation.experienceYears),
        consultationFee(inputInformation.consultationFee),
        bio(inputInformation.bio.trimmed()) {}
};

struct NurseUpdateDTO : public StaffUpdateDTO {
    QString nurseLevel;
    QString certification;

    NurseUpdateDTO() = default;
    ~NurseUpdateDTO() override = default;
    NurseUpdateDTO(const NurseInputDTO& inputInformation, int inputId)
      : StaffUpdateDTO(inputInformation, inputId),
        nurseLevel(inputInformation.nurseLevel.trimmed()),
        certification(inputInformation.certification.trimmed()) {}
};

// ── SEARCH CRITERIA ───────────────────────────────────────────────────
#include <optional>

struct StaffSearchCriteria {
    // Nhóm 1: Text search (LIKE trên staff_code hoặc full_name)
    QString searchKey;

    // Nhóm 2: Dropdown filter
    std::optional<UserRole> role;
    QString  specialty;
    int      departmentId = -1;   // -1 = tất cả
    QString  shift;               // rỗng = tất cả

    // Nhóm 3: Status filter
    bool onlyActive     = true;
    bool includeDeleted = false;
};


// =====================================================================
// PROFILE DTOs — Repository -> Service -> View (READ ONLY)
// Khong dung cho INSERT hay UPDATE
// =====================================================================

/**
 * @brief Full profile — chinh chu xem ho so ca nhan
 *        Load tu DB bang findProfileById()
 *        Chua ca field read-only (staffCode, hireDate...) va editable field
 */
struct StaffProfileDTO {
    // ── Read-only fields (hien thi, khong cho sua truc tiep) ─────────
    int      staffId;
    QString  staffCode;
    UserRole role;
    bool     isActive;
    QDate    hireDate;          // ngay vao lam
    QString  departmentName;    // JOIN tu bang departments

    // ── Editable fields (map 1-1 voi StaffInputDTO) ────────────
    QString  fullName;
    QPixmap  avatar;
    Gender   gender;
    QDate    dateOfBirth;
    QString  citizenId;
    QString  phoneNumber;
    QString  email;
    QString  address;
    int      departmentId;
    QString  shift;

    virtual ~StaffProfileDTO() = default;

    // ── Helper: chuyen sang StaffInputDTO de pre-fill form chinh sua ─
    // UI goi ham nay khi nguoi dung bam "Chinh sua"
    virtual std::unique_ptr<StaffInputDTO> toInputDTO() const {
        auto dto = std::make_unique<StaffInputDTO>();
        dto->fullName     = fullName;
        dto->avatar       = avatar;
        dto->gender       = gender;
        dto->dateOfBirth  = dateOfBirth;
        dto->citizenId    = citizenId;
        dto->phoneNumber  = phoneNumber;
        dto->email        = email;
        dto->address      = address;
        dto->departmentId = departmentId;
        dto->shift        = shift;
        return dto;
    }
};


struct DoctorProfileDTO : public StaffProfileDTO {
    QString  specialty;
    QString  licenseNumber;
    int      experienceYears;
    int      consultationFee;
    QString  bio;

    ~DoctorProfileDTO() override = default;

    std::unique_ptr<StaffInputDTO> toInputDTO() const override {
        auto dto = std::make_unique<DoctorInputDTO>();
        dto->fullName     = fullName;
        dto->avatar       = avatar;
        dto->gender       = gender;
        dto->dateOfBirth  = dateOfBirth;
        dto->citizenId    = citizenId;
        dto->phoneNumber  = phoneNumber;
        dto->email        = email;
        dto->address      = address;
        dto->departmentId = departmentId;
        dto->shift        = shift;
        // Doctor-specific
        dto->specialty       = specialty;
        dto->licenseNumber   = licenseNumber;
        dto->experienceYears = experienceYears;
        dto->consultationFee = consultationFee;
        dto->bio             = bio;
        return dto;
    }
};

struct NurseProfileDTO : public StaffProfileDTO {
    QString  nurseLevel;
    QString  certification;

    ~NurseProfileDTO() override = default;


    std::unique_ptr<StaffInputDTO> toInputDTO() const override {
        auto dto = std::make_unique<NurseInputDTO>();
        dto->fullName     = fullName;
        dto->avatar       = avatar;
        dto->gender       = gender;
        dto->dateOfBirth  = dateOfBirth;
        dto->citizenId    = citizenId;
        dto->phoneNumber  = phoneNumber;
        dto->email        = email;
        dto->address      = address;
        dto->departmentId = departmentId;
        dto->shift        = shift;
        // Nurse-specific
        dto->nurseLevel   = nurseLevel;
        dto->certification = certification;
        return dto;
    }
};

/**
 * @brief Public profile — xem thong tin cua nguoi khac
 *        Khong chua thong tin ca nhan nhay cam (CCCD, email, dien thoai, dia chi...)
 *        Duoc tao tu SystemUser::toPublicProfile() — khong can query DB them
 */
struct StaffPublicProfileDTO {
    int     staffId;
    QString fullName;
    QString displayRole;     // "Bac si Tim mach", "Y ta", "Le tan"
    //QString departmentName;  // co the de rong neu chua co join
    virtual ~StaffPublicProfileDTO() = default;
};

struct DoctorPublicProfileDTO : public StaffPublicProfileDTO {
    // Doctor public (hop le de hien thi voi benh nhan/dong nghiep)
    QString specialty;
    QString licenseNumber;
    int     experienceYears = 0;
    int     consultationFee = 0;
    QString bio;
};

struct NursePublicProfileDTO : public StaffPublicProfileDTO {
    // Nurse public
    QString nurseLevel;     // "JUNIOR" | "SENIOR" | "HEAD"
};