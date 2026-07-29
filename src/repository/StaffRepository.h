#pragma once
#include <QList>
#include <QSqlQuery>
#include <QString>
#include <memory>
#include <optional>
#include "model/SystemUser.h"
#include "dto/StaffDTOs.h"
#include "DatabaseManager.h"

class StaffRepository {
private:
    std::shared_ptr<SystemUser> mapRowToUser(const QSqlQuery& query) const;
    std::unique_ptr<StaffProfileDTO> queryProfile(const QString& whereClause, const QVariantList& params) const;
    bool insertStaffBase(const StaffInsertDTO& staff, int& staffId);
public:
    StaffRepository() = default;
    ~StaffRepository() = default;
    // CRUD
    bool insertStaff(const StaffInsertDTO& staff);
    bool insertDoctor(const DoctorInsertDTO& doctor);
    bool insertNurse(const NurseInsertDTO& nurse);
    bool insertPharmacist(const PharmacistInsertDTO& pharmacist);


    bool updateStaff(const StaffUpdateDTO& staff);
    bool updateDoctor(const DoctorUpdateDTO& doctor);
    bool updateNurse(const NurseUpdateDTO& nurse);
    bool updatePharmacist(const PharmacistUpdateDTO& pharmacist);


    bool deactivate(int staffId);
    bool reactivate(int staffId);


    static std::optional<QString> getLatestStaffCodeByYear(int year);

    // // --- Danh sách & Tìm kiếm ---
    QList<std::shared_ptr<SystemUser>> search(const StaffSearchCriteria& criteria) const;

    // Tìm kiếm profile theo ID / staff code phục vụ Auth Service
    std::shared_ptr<SystemUser> findById(int staffId) const;
    std::shared_ptr<SystemUser> findByStaffCode(const QString& staffCode) const;
    // std::vector<std::shared_ptr<SystemUser>> findAll(bool includeInactive = false) const;

    std::unique_ptr<StaffProfileDTO> findProfileById(int staffId) const;
    std::unique_ptr<StaffProfileDTO> findProfileByStaffCode(const QString& staffCode) const;


    // // --- Kiểm tra uniqueness ---
    bool existsByCitizenId(const QString& citizenId, int excludeStaffId = -1) const;
    bool existsByEmail(const QString& email, int excludeStaffId = -1) const;
    bool existsByPhoneNumber(const QString& licenseNumber, int excludeStaffId = -1) const;
    bool existsByLicenseNumber(const QString& licenseNumber, int excludeStaffId = -1) const; 
    bool existsByStaffId(int staffId) const;

    // // --- Phân trang (cho QTableView) ---
    // std::vector<std::shared_ptr<SystemUser>> findPaged(int offset, int limit) const;
    // int countTotal(bool includeInactive = false) const;

    // --- Leave Management ---
    LeaveBalanceDTO getLeaveBalance(int staffId, int year) const;
    bool createLeaveRequest(int staffId, const QDate& startDate, const QDate& endDate, const QString& reason) const;
    QList<LeaveRequestDTO> getPendingLeaveRequests() const;
    QList<LeaveRequestDTO> getLeaveHistory(int staffId) const;
    bool updateLeaveRequestStatus(int requestId, const QString& status) const;
    bool approveLeaveRequest(int requestId) const;
    bool rejectLeaveRequest(int requestId, int staffId, int year, int days) const;
    std::optional<LeaveRequestDTO> getLeaveRequestById(int requestId) const;
    bool isStaffOnLeave(int staffId, const QDate& date) const;

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