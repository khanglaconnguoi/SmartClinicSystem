#pragma once
#include "i_authenticatable.h"
#include <QString>
#include <QDateTime>
#include <vector>
#include <memory>

enum class UserRole { ADMIN, DOCTOR, NURSE, RECEPTIONIST };

class SystemUser : public IAuthenticatable {
protected:
    int         m_id;
    QString     m_username;
    QString     m_passwordHash;   // Lưu hash, không lưu plain text
    QString     m_fullName;
    UserRole    m_role;
    bool        m_isActive;
    QDateTime   m_createdAt;

public:
    explicit SystemUser(int id, const QString& username, const QString& passwordHash, const QString& fullName, UserRole role) : 
        m_id(id), m_username(username), m_passwordHash(passwordHash), m_fullName(fullName), m_role(role) {};
    virtual ~SystemUser() = default;

    // --- Getters ---
    QString    getPasswordHash()    const override  { return m_passwordHash; }
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
    int         getAccountId()    const override { return m_id; }
    QString     getUsername()     const override { return m_username; }
    // QString     getPasswordHash() const override { return m_passwordHash; }
    AccountType getAccountType()  const override { return AccountType::STAFF; }

    // --- Pure Virtual ---
    std::vector<QString> getMenuItems()         const override = 0;
    bool canAccess(const QString& moduleCode)   const override = 0;
    QString getDisplayRole()                    const override = 0;
};
