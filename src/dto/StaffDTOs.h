/**
 * @file    StaffDTOs.h
 * @brief   Data Transfer Objects cho Staff module
 *          Chỉ là data containers — không có behavior
 */
#pragma once
#include <QString>
#include <QDate>
#include <QPixmap>
#include "model/CommonEnums.h"
#include "model/SystemUser.h"

// ── INPUT DTOs ───────────────────────────────────────────────────────
// Dùng khi đọc thông tin nhân viên mới từ form đăng kí (UI → Service)
// UI chịu trách nhiệm:
// - lấy thông tin trong form rồi dựa theo role để đóng thành DTO và gọi hàm phù hợp

struct StaffInputDTO {
    QString     fullName;
    QPixmap     avatar;
    UserRole    role;
    Gender      gender;
    QDate       dateOfBirth;
    QString     citizenId;
    QString     phoneNumber;
    QString     email;
    QString     address;
    int         departmentId;
    QString     shift;
    // KHÔNG có: staffCode, plainPassword, passwordHash, hireDate
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

// ── INSERT DTOs ───────────────────────────────────────────────────────
// Dùng khi TẠO MỚI nhân viên (Service → Repository)
// Service chịu trách nhiệm:
//   - Gán staffCode (generated)
//   - Gán passwordHash (generated)
//   - Gán hireDate (tự lấy ngày hôm nay)

struct StaffInsertDTO {
    QString  staffCode; 
    QString  passwordHash;
    QString  fullName;
    QPixmap  avatar;
    UserRole role;
    Gender   gender;
    QString  dateOfBirth;    // "yyyy-MM-dd"
    QString  citizenId;
    QString  phoneNumber;
    QString  email;
    QString  address;
    int      departmentId;
    QString  hireDate;       // "yyyy-MM-dd"
    QString  shift;
};

struct DoctorInsertDTO : public StaffInsertDTO {
    QString specialty;
    QString licenseNumber;
    int     experienceYears;
    int     consultationFee;
    QString bio;
};

struct NurseInsertDTO : public StaffInsertDTO {
    QString nurseLevel;
    QString certification;
};

// ── UPDATE DTO ────────────────────────────────────────────────────────
// Dùng khi CẬP NHẬT thông tin — chỉ chứa field được phép sửa
// Không có: staffCode (key), role (không đổi), createdAt (bất biến)

struct StaffUpdateDTO {
    int     staffId;         // để biết UPDATE WHERE staff_id = ?

    // Những trường thông tin có thể thay đổi
    QString     fullName;           // ADMIN
    QPixmap     avatar;             // ADMIN
    UserRole    role;               // ADMIN
    Gender      gender;             // ADMIN
    QString     dateOfBirth;        // ADMIN
    QString     citizenId;          // ADMIN
    QString     phoneNumber;        // ADMIN & DOCTOR
    QString     email;              // ADMIN & DOCTOR
    QString     address;            // ADMIN & DOCTOR
    int         departmentId;       // ADMIN   
    QString     shift;              // ADMIN

    StaffUpdateDTO() = default;
    StaffUpdateDTO(const StaffInputDTO& inputInformation, int inputId)
        : staffId(inputId),
          fullName(inputInformation.fullName.trimmed()),
          avatar(inputInformation.avatar),
          role(inputInformation.role),
          gender(inputInformation.gender),
          dateOfBirth(inputInformation.dateOfBirth.toString("yyyy-MM-dd")),
          citizenId(inputInformation.citizenId.trimmed()),
          phoneNumber(inputInformation.phoneNumber.trimmed()),
          email(inputInformation.email.trimmed()),
          address(inputInformation.address.trimmed()),
          departmentId(inputInformation.departmentId),
          shift(inputInformation.shift) {}
};

struct DoctorUpdateDTO : public StaffUpdateDTO {
    QString specialty;
    QString licenseNumber;
    int     experienceYears;
    int     consultationFee;
    QString bio;

    DoctorUpdateDTO() = default;
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
    NurseUpdateDTO(const NurseInputDTO inputInformation, int inputId)
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
