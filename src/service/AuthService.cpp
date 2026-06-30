#include "AuthService.h"
#include "ext/bcrypt.h"
#include "../model/PatientUser.h"
#include "../repository/DatabaseManager.h"
#include <QSqlQuery>

std::optional<std::shared_ptr<IAuthenticatable>> AuthService::login(
        const QString& staffCode, const QString& plainPassword) {
    // Cho phép đăng nhập giả lập vai trò bệnh nhân bằng tài khoản "patient" hoặc bắt đầu bằng "pat"
    if (staffCode.toLower() == "patient" || staffCode.toLower().startsWith("pat")) {
        QString codeToQuery = (staffCode.toLower() == "patient") ? "PAT-1001" : staffCode.toUpper();
        QSqlQuery query = DatabaseManager::getInstance().selectQuery(
            "SELECT patient_id, full_name FROM patients WHERE patient_code = ?", {codeToQuery});
        if (query.next()) {
            int patId = query.value(0).toInt();
            QString fullName = query.value(1).toString();
            return std::make_shared<PatientUser>(patId, fullName);
        }
    }

    std::optional<std::shared_ptr<SystemUser>> user = m_staffRepo->findByStaffCode(staffCode);

    if (!user || !bcrypt::validatePassword(plainPassword.toStdString(), user->get()->getPasswordHash().toStdString()))
        return std::nullopt;
    else return user;
}