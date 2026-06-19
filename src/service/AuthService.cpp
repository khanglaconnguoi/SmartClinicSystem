#include "bcrypt.h"
#include "AuthService.h"

std::optional<std::shared_ptr<IAuthenticatable>> AuthService::login(const QString& staffCode, const QString& plainPassword) {
    std::optional<std::shared_ptr<SystemUser>> user = m_staffRepo -> findByStaffCode(staffCode);

    if (!user || bcrypt::validatePassword(plainPassword.toStdString(), user->get()->getPasswordHash().toStdString())) {
        return std::nullopt;
    }
    else {
        return user;
    }
}