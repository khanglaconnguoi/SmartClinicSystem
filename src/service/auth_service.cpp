#include "auth_service.h"

std::optional<std::shared_ptr<IAuthenticatable>> AuthService::login(const QString& username, const QString& plainPassword) {
    // kiểm tra username tồn tại hay không -> hash mật khẩu -> kiểm tra mật khẩu (dùng method của repository để gọi db)
    std::optional<std::shared_ptr<SystemUser>> user = m_staffRepo -> findByUsername(username);

    //QString passwordHash = ... ;

    // if(!user || user -> m_passwordHash != passwordHash)
    //     qDebug() << "Incorrect username or password!";
    //     return std::nullopt;
    // }
    // else{
    //     qDebug() << "Welcome back!";
    //     return user;
    // }

}