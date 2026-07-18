#pragma once
#include "SystemUser.h"

class Nurse : public SystemUser {
private:
    QString m_nurseLevel;
    QString m_certification;
public:
    explicit Nurse(
        int             staffId, 
        const QString&  staffCode,
        const QString&  passwordHash, 
        const QString&  fullName, 
        const QPixmap&  avatar,
        UserRole        role,
        bool            isActive,
        bool            mustChangePassword,
        const QString&  nurseLevel,
        const QString&  certification
    ) : SystemUser(
            staffId, 
            staffCode,
            passwordHash, 
            fullName, 
            avatar,
            role, 
            isActive,
            mustChangePassword
        ),
        m_nurseLevel(nurseLevel),
        m_certification(certification)
    {}

    ~Nurse() override = default;

    QString getNurseLevel() const { return m_nurseLevel; }
    QString getCertification() const { return m_certification; }

    QStringList getMenuItems() const override {
        return { "Dashboard", "Patients", "Schedule", "Messages" };
    }

    bool canAccess(const QString& moduleCode) const override {
        return getMenuItems().contains(moduleCode);
    }

    QString getDisplayRole() const override {
        return "Nurse";
    }

    std::unique_ptr<StaffPublicProfileDTO> toPublicProfile() const override {
        auto dto = std::make_unique<NursePublicProfileDTO>();
        dto->staffId = m_staffId;
        dto->fullName = m_fullName;
        dto->displayRole = getDisplayRole();
        dto->nurseLevel = m_nurseLevel;
        return dto;
    }
};
