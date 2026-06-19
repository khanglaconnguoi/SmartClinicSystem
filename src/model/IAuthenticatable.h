#pragma once
#include <QString>
#include <vector>
enum class AccountType { Staff, Patient };

class IAuthenticatable {
public:
    virtual ~IAuthenticatable() = default;

    // --- Định danh & xác thực ---
    virtual int         getAccountId()      const = 0;
    virtual QString     getPasswordHash()   const = 0;
    virtual AccountType getAccountType()    const = 0;

    // --- Đa hình điều khiển UI (dùng cho MainWindow) ---
    // virtual std::vector<QString> getMenuItems()       const = 0;
    // virtual bool canAccess(const QString& moduleCode) const = 0;
    virtual QString getDisplayRole()                  const = 0;
};
