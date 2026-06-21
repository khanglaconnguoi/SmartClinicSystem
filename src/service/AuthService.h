#pragma once
#include <memory>
#include <optional>

#include "repository/StaffRepository.h"

class AuthService {
   private:
    std::shared_ptr<StaffRepository> m_staffRepo;
    // std::shared_ptr<PatientRepository>  m_patientRepo; chưa dùng đến

   public:
    // Trả về interface chung — KHÔNG trả về SystemUser* trực tiếp
    std::optional<std::shared_ptr<IAuthenticatable>> login(
            const QString& staffCode, const QString& plainPassword);

    void logout();
    std::shared_ptr<IAuthenticatable> getCurrentAccount() const;
};