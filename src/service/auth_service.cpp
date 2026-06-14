#include "bcrypt.h"
#include "auth_service.h"

std::optional<std::shared_ptr<IAuthenticatable>> AuthService::login(const QString& username, const QString& plainPassword) {
    std::optional<std::shared_ptr<SystemUser>> user = m_staffRepo -> findByUsername(username);

    if (!user || bcrypt::validatePassword(plainPassword.toStdString(), user->get()->getPasswordHash().toStdString())) {
        return std::nullopt;
    }
    else {
        return user;
    }
}