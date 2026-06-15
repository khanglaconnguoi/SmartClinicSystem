#include "../repository/StaffRepository.h"
#include "../model/IAuthenticatable.h"
#include <memory>
#include <optional>

class AuthService {
private:
    std::shared_ptr<StaffRepository>    m_staffRepo;
    //std::shared_ptr<PatientRepository>  m_patientRepo; chưa dùng đến

public:
    // Trả về interface chung — KHÔNG trả về SystemUser* trực tiếp
    std::optional<std::shared_ptr<IAuthenticatable>> login(const QString& username, const QString& plainPassword);

    void logout();
    std::shared_ptr<IAuthenticatable> getCurrentAccount() const;

    
};