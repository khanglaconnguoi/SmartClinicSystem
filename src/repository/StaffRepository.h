#pragma once
#include <QList>
#include <QSqlQuery>
#include <QString>
#include <memory>
#include <optional>
#include "model/SystemUser.h"
#include "dto/StaffDTOs.h"

class StaffRepository {
private:
    std::shared_ptr<SystemUser> mapRowToUser(const QSqlQuery& query) const;

    bool insertStaffBase(const StaffInsertDTO& staff, int& staffId);
public:
    // CRUD
    bool insertStaff(const StaffInsertDTO& staff);
    bool insertDoctor(const DoctorInsertDTO& doctor);
    bool insertNurse(const NurseInsertDTO& nurse);

    bool updateStaff(const StaffUpdateDTO& staff);
    bool updateDoctor(const DoctorUpdateDTO& doctor);
    bool updateNurse(const NurseUpdateDTO& nurse);

    bool deactivate(int staffId);
    bool reactivate(int staffId);


    static std::optional<QString> getLatestStaffCodeByYear(int year);

    // // --- Danh sách & Tìm kiếm ---
    // Tìm kiếm theo ID / staff code phục vụ Auth Service
    std::optional<std::shared_ptr<SystemUser>> findById(int staffId) const;
    std::optional<std::shared_ptr<SystemUser>> findByStaffCode(const QString& staffCode) const;
    // std::vector<std::shared_ptr<SystemUser>> findAll(bool includeInactive = false) const;
    QList<std::shared_ptr<SystemUser>> search(const StaffSearchCriteria& criteria) const;
    // std::vector<std::shared_ptr<SystemUser>> findByRole(UserRole role) const;

    // // --- Kiểm tra uniqueness ---
    bool existsByCitizenId(const QString& citizenId, int excludeStaffId = -1) const;
    bool existsByEmail(const QString& email, int excludeStaffId = -1) const;
    bool existsByPhoneNumber(const QString& licenseNumber, int excludeStaffId = -1) const;
    bool existsByLicenseNumber(const QString& licenseNumber, int excludeStaffId = -1) const; 
    bool existsByStaffId(int staffId) const;

    // // --- Phân trang (cho QTableView) ---
    // std::vector<std::shared_ptr<SystemUser>> findPaged(int offset, int limit) const;
    // int countTotal(bool includeInactive = false) const;

    // --- Mật khẩu ---
    bool updatePasswordInformation(
            int userId, const QString& newHash, bool mustChangePassword = false);

    // // --- Audit Log (tính năng nâng cao Module 1) ---
    // bool logLoginAttempt(int userId, const QString& ipAddress, bool success, const QString& moduleAccessed = "");
    // std::vector<LoginLogEntry> getLoginHistory(int userId, int limit = 50) const;

    // // --- Thống kê (cho Dashboard) ---
    // int countByRole(UserRole role) const;
    // int countActiveStaff() const;
};