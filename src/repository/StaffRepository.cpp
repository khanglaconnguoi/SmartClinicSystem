#include "StaffRepository.h"
#include "..\model\Doctor.h"


bool StaffRepository::insert(const StaffInsertDTO& user) {
    // QString insert = R"( 
    //     INSERT INTO staff 
    // )"
}






std::shared_ptr<SystemUser> StaffRepository::mapRowToUser(const QSqlQuery& query) const {
    // int id = query.value("staff_id").toInt();
    // QString username = query.value("username").toString();
    // QString passwordHash = query.value("passwordHash").toString();
    // QString full_name = query.value("full_name").toString();
    // UserRole role = stringToRole(query.value("role").toString());

    // switch(role) {
    //     case UserRole::DOCTOR:
    //         return std::make_shared<Doctor>(id, username, passwordHash, full_name, role);
    //         break;
    //     // case UserRole::NURSE:
    //     //     return std::make_shared<Nurse>(id, username, passwordHash, full_name, role);
    //     // case UserRole::RECEPTIONIST:
    //     //     return std::make_shared<Receptionist>(id, username, passwordHash, full_name, role);
    // }

}

std::optional<std::shared_ptr<SystemUser>> StaffRepository::findByUsername(const QString& username) const {
    // QString findByUsername = R"(
    //     SELECT s.staff_id, s.full_name, s.role, l.username, l.passwordHash 
    //     FROM staffs s 
    //     INNER JOIN login_information l ON s.staff_id = l.staff_id 
    //     WHERE l.username = ?"
    // )";

    // QSqlQuery query = DatabaseManager::getInstance().selectQuery(findByUsername, { username });
    
    // if(query.next()){
    //     return mapRowToUser(query);
    // }

    // return std::nullopt;
}

std::optional<std::shared_ptr<SystemUser>> StaffRepository::findById(int userId) const {
    // QString findById = R"(
    //     SELECT s.staff_id, s.full_name, s.role, l.username, l.passwordHash 
    //     FROM staffs s 
    //     INNER JOIN login_information l ON s.staff_id = l.staff_id 
    //     WHERE l.staff_id = ?"
    // )";

    // QSqlQuery query = DatabaseManager::getInstance().selectQuery(findById, { userId });
    
    // if(query.next()){
    //     return mapRowToUser(query);
    // }

    // return std::nullopt;
}


