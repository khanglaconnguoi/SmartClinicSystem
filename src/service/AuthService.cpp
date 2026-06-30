#include "AuthService.h"
#include "ext/bcrypt.h"
#include "../model/PatientUser.h"

std::optional<std::shared_ptr<IAuthenticatable>> AuthService::login(
        const QString& staffCode, const QString& plainPassword) {
    // Cho phép đăng nhập giả lập vai trò bệnh nhân bằng tài khoản "patient" hoặc bắt đầu bằng "pat"
    if (staffCode.toLower() == "patient" || staffCode.toLower().startsWith("pat")) {
        return std::make_shared<PatientUser>(1001, "Nguyễn Văn Bệnh Nhân");
    }

    std::optional<std::shared_ptr<SystemUser>> user = m_staffRepo->findByStaffCode(staffCode);

    if (!user || !bcrypt::validatePassword(plainPassword.toStdString(), user->get()->getPasswordHash().toStdString()))
        return std::nullopt;
    else return user;
}