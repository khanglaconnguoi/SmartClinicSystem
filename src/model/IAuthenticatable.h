#pragma once
#include <QString>
#include <vector>
#include <QPixmap>
#include "CommonEnums.h"
enum class AccountType { Staff, Patient };

class IAuthenticatable {
public:
    virtual ~IAuthenticatable() = default;

    // ── Định danh ─────────────────────────────────────────────
    virtual int         getAccountId()    const = 0;
    virtual QString     getStaffCode()     const = 0;  // staffCode với staff, email với patient
    virtual AccountType getAccountType()  const = 0;  // STAFF | PATIENT
    virtual UserRole    getRole()         const = 0;

    // ── RBAC (MainWindow + ViewRegistry) ─────────────────────
    virtual QStringList getMenuItems()                       const = 0;
    virtual bool        canAccess(const QString& moduleCode) const = 0;
    virtual QString     getDisplayRole()                     const = 0;

    // ── Display (MainWindow header + sidebar) ─────────────────
    virtual QString     getFullName()    const = 0;
    virtual QPixmap     getAvatar()      const = 0;

    // ── Xác thực ─────────────────────────────────────────────
    // AuthService gọi sau khi tìm được account từ DB
    virtual bool        verifyPassword(const QString& plainPassword) const = 0;
};

