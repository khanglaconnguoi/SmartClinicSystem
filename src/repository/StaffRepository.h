#pragma once
#include <QDateTime>
#include <QString>
#include <memory>
#include <optional>
#include <vector>

#include "../model/SystemUser.h"
#include "DatabaseManager.h"

/*
DTO: Data Transfer Object dùng để lưu trữ thông tin cần đẩy xuống database
*/

/*
Sử dụng struct vì nó chỉ là một cấu trúc
để lưu thông tin đẩy vào database
chứ không phải 1 lớp đối tượng
*/

struct StaffInsertDTO {
    QString staffCode;
    QString passwordHash;
    QString fullName;
    UserRole role;
    Gender gender;
    QString dateOfBirth;
    QString nationalId;
    QString phoneNumber;
    QString email;
    QString address;
    int departmentId;
    QString hireDate;
    QString shift;
};

struct DoctorInsertDTO : public StaffInsertDTO {
    QString specialty;
    QString licenseNumber;
    int experienceYears;
    int consultationFee;
    QString bio;
};

struct NurseInsertDTO : public StaffInsertDTO {
    QString nurseLevel;
    QString certification;
};

class StaffRepository {
   private:
    std::shared_ptr<SystemUser> mapRowToUser(const QSqlQuery& query) const;

    bool insertStaffBase(const StaffInsertDTO& staff, int& staffId);
public:
    // CRUD
    bool insertStaff(const StaffInsertDTO& staff);
    bool insertDoctor(const DoctorInsertDTO& doctor);
    bool insertNurse(const NurseInsertDTO& nurse);

    bool update(const StaffInsertDTO& user);
    bool deactivate(const StaffInsertDTO& user);
    bool reactivate(const StaffInsertDTO& user);

    // Tìm kiếm theo ID / username phục vụ Auth Service
    std::optional<std::shared_ptr<SystemUser>> findById(int userId) const;
    std::optional<std::shared_ptr<SystemUser>> findByStaffCode(const QString& staffCode) const;
};