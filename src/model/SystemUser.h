#pragma once
#include "IAuthenticatable.h"
#include "CommonEnums.h"
#include <QString>
#include <QDateTime>
#include <vector>
#include <memory>

enum class UserRole { Admin, Doctor, Nurse, Receptionist };


UserRole roleFromString(const QString& role);
QString roleToString(UserRole role);

class SystemUser : public IAuthenticatable {
protected:
    int         m_staffId;
    QString     m_staffCode;
    QString     m_username;
    QString     m_passwordHash;   // Lưu hash, không lưu plain text
    QString     m_fullName;
    UserRole    m_role;
    bool        m_isActive;

public:
    explicit SystemUser(
        int staffId, 
        const QString& staffCode,
        const QString& passwordHash, 
        const QString& fullName, 
        UserRole role,
        bool isActive
    ) :
        m_staffId(staffId), 
        m_staffCode(staffCode),
        m_passwordHash(passwordHash), 
        m_fullName(fullName), 
        m_role(role),
        m_isActive(isActive)

    {}
    virtual ~SystemUser() = default;

    // --- Getters ---
    QString    getFullName()        const           { return m_fullName; }
    UserRole   getRole()            const           { return m_role; }
    bool       isActive()           const           { return m_isActive; }

    // --- Setters ---
    void setActive(bool active) { m_isActive = active; }
    void setFullName(const QString& name) { m_fullName = name; }

    // --- Business Logic ---
    bool verifyPassword(const QString& plainPassword) const;
    void setPassword(const QString& plainPassword);

    // --- Implement IAuthenticatable ---
    int         getAccountId()    const override { return m_staffId; }
    QString     getUsername()     const override { return m_username; }
    QString     getPasswordHash() const override { return m_passwordHash; }
    AccountType getAccountType()  const override { return AccountType::Staff; }

    // --- Pure Virtual ---
    std::vector<QString> getMenuItems()         const override = 0;
    bool canAccess(const QString& moduleCode)   const override = 0;
    QString getDisplayRole()                    const override = 0;
};
