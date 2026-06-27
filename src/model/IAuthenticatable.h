#pragma once
#include <QString>
#include <vector>
#include <QPixmap>
enum class AccountType { Staff, Patient };

class IAuthenticatable {
public:
    virtual ~IAuthenticatable() = default;

    // --- Định danh & xác thực ---
    virtual int         getAccountId()      const = 0;
    virtual QString     getPasswordHash()   const = 0;
    virtual AccountType getAccountType()    const = 0;
    virtual QString     getFullName()       const = 0;
    virtual QPixmap     getAvatar()         const = 0;

    // --- Đa hình điều khiển UI (dùng cho MainWindow) ---
    virtual QStringList getMenuItems()                const = 0;
    virtual bool canAccess(const QString& moduleCode) const = 0;
    virtual QString getDisplayRole()                  const = 0;
};
