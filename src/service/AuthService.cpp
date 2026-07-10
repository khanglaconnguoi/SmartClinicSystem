#include "AuthService.h"

LoginResult AuthService::login(const QString& staffCode, const QString& plainPassword) {
    std::shared_ptr<SystemUser> user = m_staffRepo->findByStaffCode(staffCode);

    if (!user || !user->isActive() || !user->verifyPassword(plainPassword)) {
        return LoginResult{false, false};
    }

    UserSession::getInstance().setCurrentAccount(user);
    return {true, user->mustChangePassword()};
}