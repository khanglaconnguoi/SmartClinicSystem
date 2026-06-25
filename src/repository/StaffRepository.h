#pragma once
#include <QDateTime>
#include <QSqlQuery>
#include <QString>
#include <QPixmap>
#include <memory>
#include <optional>

#include "model/SystemUser.h"

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
    QPixmap avatar;
    UserRole role;
    Gender gender;
    QString dateOfBirth;
    QString citizenId;
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

struct StaffSearchCriteria {

    // ── Nhóm 1: Text search ─────────────────────────────────────
    QString searchKey;

    // ── Nhóm 2: Dropdown filter ──────────────────────────────────
    std::optional<UserRole> role;       
    QString specialty;                  
    int     departmentId = -1;          
    QString shift;                      

    // ── Nhóm 3: Status filter ────────────────────────────────────
    bool onlyActive     = true;
    bool includeDeleted = false;
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

    bool updateStaff(const StaffInsertDTO& staff);
    bool updateDoctor(const DoctorInsertDTO& doctor);
    bool updateNurse(const NurseInsertDTO& nurse);

    bool deactivate(int staffId);
    bool reactivate(int staffId);

    // Tìm kiếm theo ID / staff code phục vụ Auth Service
    std::optional<std::shared_ptr<SystemUser>> findById(int staffId) const;
    std::optional<std::shared_ptr<SystemUser>> findByStaffCode(const QString& staffCode) const;

    static std::optional<QString> getLatestIdByYear(int year);

    // // --- Danh sách & Tìm kiếm ---
    // std::vector<std::shared_ptr<SystemUser>> findAll(bool includeInactive = false) const;
    QList<std::shared_ptr<SystemUser>> search(const StaffSearchCriteria& criteria) const;
    // std::vector<std::shared_ptr<SystemUser>> findByRole(UserRole role) const;

    // // --- Phân trang (cho QTableView) ---
    // std::vector<std::shared_ptr<SystemUser>> findPaged(int offset, int limit) const;
    // int countTotal(bool includeInactive = false) const;

    // // --- Mật khẩu ---
    // bool updatePasswordHash(int userId, const QString& newHash);

    // // --- Audit Log (tính năng nâng cao Module 1) ---
    // bool logLoginAttempt(int userId, const QString& ipAddress, bool success, const QString& moduleAccessed = "");
    // std::vector<LoginLogEntry> getLoginHistory(int userId, int limit = 50) const;

    // // --- Thống kê (cho Dashboard) ---
    // int countByRole(UserRole role) const;
    // int countActiveStaff() const;
};