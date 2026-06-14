#include "staff_repository.h"


std::optional<std::shared_ptr<SystemUser>> StaffRepository::findByUsername(const QString& username) const{
    QString findByUsername = R"(
        SELECT s.staff_id, s.full_name, s.role, l.username, l.passwordHash 
        FROM staffs s 
        INNER JOIN login_information l ON s.staff_id = l.staff_id 
        WHERE l.username = ?"
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(findByUsername, { username });
    
    if(query.next()){
        int id = query.value("staff_id").toInt();
        QString full_name = query.value("full_name").toString();
        QString role = query.value("role").toString();
        QString username = query.value("username").toString();
        QString passwordHash = query.value("passwordHash").toString();
        return std::make_shared<SystemUser>(id, username, passwordHash, full_name, role);
    }

    return std::nullopt;
}
