#pragma once
#include "IAuthenticatable.h"
#include "CommonEnums.h"
#include <QString>
#include <QDateTime>
#include <QPixmap>
#include <vector>
#include <memory>

enum class UserRole { Admin, Doctor, Nurse, Receptionist };


UserRole roleFromString(const QString& role);
QString roleToString(UserRole role);

class SystemUser : public IAuthenticatable {
protected:
    int         m_staffId;
    QString     m_staffCode;
    QString     m_passwordHash;   // Lưu hash, không lưu plain text
    QString     m_fullName;
    QPixmap     m_avatar;
    UserRole    m_role;
    bool        m_isActive;

public:
    explicit SystemUser(
        int             staffId, 
        const QString&  staffCode,
        const QString&  passwordHash, 
        const QString&  fullName, 
        QPixmap         avatar,
        UserRole        role,
        bool            isActive
    ) :
        m_staffId(staffId), 
        m_staffCode(staffCode),
        m_passwordHash(passwordHash), 
        m_fullName(fullName), 
        m_avatar(avatar),
        m_role(role),
        m_isActive(isActive)

    {}
    virtual ~SystemUser() = default;

    // --- Getters ---
    UserRole   getRole()      const           { return m_role; }
    bool       isActive()     const           { return m_isActive; }
    

    // --- Setters ---
    void setActive(bool active) { m_isActive = active; }
    void setFullName(const QString& name) { m_fullName = name; }

    // --- Business Logic ---
    bool verifyPassword(const QString& plainPassword) const;
    void setPassword(const QString& plainPassword);

    // --- Implement IAuthenticatable ---
    int         getAccountId()    const override { return m_staffId; }
    QString     getPasswordHash() const override { return m_passwordHash; }
    AccountType getAccountType()  const override { return AccountType::Staff; }
    QString     getFullName()     const override { return m_fullName; }
    QPixmap     getAvatar()       const override { return m_avatar; }

    // --- Pure Virtual ---
    QStringList getMenuItems()                  const override = 0;
    bool canAccess(const QString& moduleCode)   const override = 0;
    QString getDisplayRole()                    const override = 0;

};