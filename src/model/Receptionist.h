#pragma once
#include "SystemUser.h"

class Receptionist : public SystemUser {
public:
    explicit Receptionist(
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

    ~Receptionist() override = default;

    QStringList getMenuItems() const override {
        return { "Dashboard", "Appointments", "Patients", "Billing", "Messages" };
    }

    bool canAccess(const QString& moduleCode) const override {
        return getMenuItems().contains(moduleCode);
    }

    QString getDisplayRole() const override {
        return "Receptionist";
    }

    std::unique_ptr<StaffPublicProfileDTO> toPublicProfile() const override {
        auto dto = std::make_unique<StaffPublicProfileDTO>();
        dto->staffId = m_staffId;
        dto->fullName = m_fullName;
        dto->displayRole = getDisplayRole();
        return dto;
    }
};
