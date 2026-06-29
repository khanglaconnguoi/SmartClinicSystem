#pragma once
#include "repository/StaffRepository.h"
#include "model/SystemUser.h"
#include "UserSession.h"
#include <memory>
#include <optional>


class AuthService {
private:
    std::shared_ptr<StaffRepository> m_staffRepo;
    // std::shared_ptr<PatientRepository>  m_patientRepo; chưa dùng đến
public:
    explicit AuthService(std::shared_ptr<StaffRepository> staffRepo)
        : m_staffRepo(staffRepo) {}
    ~AuthService() = default;
    // Trả về interface chung — KHÔNG trả về SystemUser* trực tiếp
    bool login(const QString& staffCode, const QString& plainPassword);
    void logout() {
        UserSession::getInstance().clear();
    }
};