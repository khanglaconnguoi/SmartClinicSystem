#pragma once
#include "SystemUser.h"

class Admin : public SystemUser {
public:
    explicit Admin(
        int             staffId, 
        const QString&  staffCode,
        const QString&  passwordHash, 
        const QString&  fullName, 
        const QPixmap&  avatar,
        UserRole        role,
        bool            isActive,
        bool            mustChangePassword
    ) : SystemUser(
            staffId, 
            staffCode,
            passwordHash, 
            fullName, 
            avatar,
            role, 
            isActive,
            mustChangePassword
        ) 
    {}

    ~Admin() override = default;

    QStringList getMenuItems() const override {
        return { "Dashboard", "Staff", "Patients", "Reports", "Settings" };
    }

    bool canAccess(const QString& moduleCode) const override {
        return true; // Admin can access everything
    }

    QString getDisplayRole() const override {
        return "Administrator";
    }

    std::unique_ptr<StaffPublicProfileDTO> toPublicProfile() const override {
        auto dto = std::make_unique<StaffPublicProfileDTO>();
        dto->staffId = m_staffId;
        dto->fullName = m_fullName;
        dto->displayRole = getDisplayRole();
        return dto;
    }
};
