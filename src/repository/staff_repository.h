#pragma once
#include "../model/system_user.h"
#include "database_manager.h"
#include <QString>
#include <QDateTime>
#include <vector>
#include <memory>
#include <optional>


class StaffRepository{
private:
    std::shared_ptr<SystemUser> mapRowToUser(const QSqlQuery& query) const;
public:
    // CRUD
    // bool insert(const std::shared_ptr<SystemUser>& user);
    // bool update(const std::shared_ptr<SystemUser>& user);
    // bool deactivate(const std::shared_ptr<SystemUser>& user);
    // bool reactivate(const std::shared_ptr<SystemUser>& user);
    
    // Tìm kiếm theo ID / username phục vụ Auth Service
    //std::optional<std::shared_ptr<SystemUser>> findById(int userId) const;
    std::optional<std::shared_ptr<SystemUser>> findByUsername(const QString& username) const;
    //bool usernameExists(const QString& username) const;
};