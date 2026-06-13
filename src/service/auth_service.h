#include "../repository/staff_repository.h"
#include "../model/i_authenticatable.h"
#include <memory>

class AuthService {
private:
    std::shared_ptr<StaffRepository>   m_staffRepo;
    //std::shared_ptr<PatientRepository> m_patientRepo; chưa dùng đến

public:
    // Trả về interface chung — KHÔNG trả về SystemUser* trực tiếp
    std::optional<std::shared_ptr<IAuthenticatable>> login(
        const QString& username, const QString& plainPassword);

    void logout();
    std::shared_ptr<IAuthenticatable> getCurrentAccount() const;
};