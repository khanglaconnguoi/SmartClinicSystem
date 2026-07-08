#pragma once
#include "CommonEnums.h"
#include "IAuthenticatable.h"
#include <QDateTime>
#include <QPixmap>
#include <QString>
#include <memory>
#include <vector>


enum class UserRole { Admin, Doctor, Nurse, Receptionist };

UserRole roleFromString(const QString &role);
QString roleToString(UserRole role);

class SystemUser : public IAuthenticatable {
protected:
  int m_staffId;
  QString m_staffCode;
  QString m_passwordHash;
  QString m_fullName;
  QPixmap m_avatar;
  UserRole m_role;
  bool m_isActive;

public:
  explicit SystemUser(int staffId, const QString &staffCode,
                      const QString &passwordHash, const QString &fullName,
                      QPixmap avatar, UserRole role, bool isActive)
      : m_staffId(staffId), m_staffCode(staffCode),
        m_passwordHash(passwordHash), m_fullName(fullName), m_avatar(avatar),
        m_role(role), m_isActive(isActive)

  {}
  virtual ~SystemUser() = default;

  // --- Getters ---
  int getStaffId() const { return m_staffId; }
  QString getStaffCode() const { return m_staffCode; }
  UserRole getRole() const { return m_role; }
  bool isActive() const { return m_isActive; }

  // --- Setters ---
  void setActive(bool active) { m_isActive = active; }
  void setFullName(const QString &name) { m_fullName = name; }

  // --- Business Logic ---
  bool verifyPassword(const QString &plainPassword) const;
  void setPassword(const QString &plainPassword);

  // --- Implement IAuthenticatable ---
  int getAccountId() const override { return m_staffId; }
  QString getPasswordHash() const override { return m_passwordHash; }
  AccountType getAccountType() const override { return AccountType::Staff; }
  QString getFullName() const override { return m_fullName; }
  QPixmap getAvatar() const override { return m_avatar; }

  // --- Virtual methods (default implementation) ---
  QStringList getMenuItems() const override { return {}; }
  bool canAccess(const QString &moduleCode) const override {
    (void)moduleCode;
    return true;
  }
  QString getDisplayRole() const override { return roleToString(m_role); }
};