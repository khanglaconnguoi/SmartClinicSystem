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
    QString     gender;
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
    int     roomId = 0;
};

struct NurseInputDTO : public StaffInputDTO {
    QString nurseLevel;
    QString certification;
};

struct ReceptionistInputDTO : public StaffInputDTO {};

struct PharmacistInputDTO : public StaffInputDTO {
    QString licenseNumber;
    QString pharmacySection;
    int     experienceYears;
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
};

struct DoctorInsertDTO : public StaffInsertDTO {
    QString specialty;
    QString licenseNumber;
    int     experienceYears;
    int     consultationFee;
    QString bio;
    int     roomId = 0;
};

struct NurseInsertDTO : public StaffInsertDTO {
    QString nurseLevel;
    QString certification;
};

struct ReceptionistInsertDTO : public StaffInsertDTO {};


struct PharmacistInsertDTO : public StaffInsertDTO {
    QString licenseNumber;
    QString pharmacySection;
    int     experienceYears = 0;
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
};

struct DoctorUpdateDTO : public StaffUpdateDTO {
    QString specialty;
    QString licenseNumber;
    int     experienceYears;
    int     consultationFee;
    QString bio;
    int     roomId = 0;
};

struct NurseUpdateDTO : public StaffUpdateDTO {
    QString nurseLevel;
    QString certification;
};

struct ReceptionistUpdateDTO : public StaffUpdateDTO {};

struct PharmacistUpdateDTO : public StaffUpdateDTO {
    QString licenseNumber;
    QString pharmacySection;
    int     experienceYears;
};

// ── SEARCH CRITERIA ───────────────────────────────────────────────────
#include <optional>

#include "Pagination.h"

struct StaffSearchCriteria {
    // Nhóm 1: Text search (LIKE trên staff_code hoặc full_name)
    QString searchKey;

    // Nhóm 2: Dropdown filter
    std::optional<UserRole> role;
    int departmentId = -1;  // -1 = tất cả
    QString shift;          // rỗng = tất cả

    // Nhóm 3: Status filter
    bool onlyActive = true;
    bool includeDeleted = false;

    // Nhóm 4: Phân trang (1-indexed)
    int page = 1;       ///< Trang hiện tại (1-indexed)
    int pageSize = 20;  ///< Số bản ghi / trang. 0 = không phân trang (trả về tất cả)

    virtual ~StaffSearchCriteria() = default;
};

struct DoctorSearchCriteria : public StaffSearchCriteria {
    QString specialty;

    DoctorSearchCriteria() { role = UserRole::Doctor; }
};

struct NurseSearchCriteria : public StaffSearchCriteria {
    QString nurseLevel;

    NurseSearchCriteria() { role = UserRole::Nurse; }
};

struct ReceptionistSearchCriteria : public StaffSearchCriteria {
    ReceptionistSearchCriteria() { role = UserRole::Receptionist; }
};

struct PharmacistSearchCriteria : public StaffSearchCriteria {
    QString pharmacySection;

    PharmacistSearchCriteria() { role = UserRole::Pharmacist; }
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
    QString  gender;
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
    int      roomId = 0;
    QString  roomNumber; // Hiển thị tên phòng

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
        dto->roomId          = roomId;
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

struct PharmacistProfileDTO : public StaffProfileDTO {
    QString licenseNumber;
    QString pharmacySection;
    int     experienceYears;

    ~PharmacistProfileDTO() override = default;


    std::unique_ptr<StaffInputDTO> toInputDTO() const override {
        auto dto = std::make_unique<PharmacistInputDTO>();
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
        // Pharmacist-specific
        dto->licenseNumber   = licenseNumber;
        dto->pharmacySection = pharmacySection;
        dto->experienceYears = experienceYears;
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

struct PharmacistPublicProfileDTO : public StaffPublicProfileDTO {
    // Pharmacist public
    QString licenseNumber;
    QString pharmacySection;
    int     experienceYears;
};

// ── LEAVE MANAGEMENT DTOs ──────────────────────────────────────────

struct LeaveBalanceDTO {
    int staffId = 0;
    int year = 0;
    int totalDays = 0;
    int usedDays = 0;
};

struct LeaveRequestDTO {
    int requestId = 0;
    int staffId = 0;
    QString staffCode;
    QString fullName;
    QDate startDate;
    QDate endDate;
    QString reason;
    QString status;
};