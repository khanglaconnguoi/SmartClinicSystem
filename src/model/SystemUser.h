#pragma once
#include "IAuthenticatable.h"
#include "dto/StaffDTOs.h"
#include <QString>
#include <QDateTime>
#include <vector>
#include <memory>



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
        const QPixmap&  avatar,
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

    int         getAccountId()    const override { return m_staffId; }
    QString     getStaffCode()    const override { return m_staffCode; }
    AccountType getAccountType()  const override { return AccountType::Staff; }
    UserRole    getRole()         const override { return m_role; }
    

    QString     getFullName()     const override { return m_fullName; }
    QPixmap     getAvatar()       const override { return m_avatar; }

    // ── Staff-specific: KHÔNG có ở IAuthenticatable ──────────

    QString  getPasswordHash()  const { return m_passwordHash; }

    // Trạng thái hoạt động của nhân viên
    bool isActive()        const { return m_isActive; }
    void setActive(bool active)  { m_isActive = active; }

    // Cập nhật profile — staff management operations
    // void setFullName(const QString& name) { m_fullName = name.trimmed(); }
    // void setPassword(const QString& plainPassword); // hash rồi lưu vào m_passwordHash

    // Public profile — trả về StaffPublicProfileDTO (staff-specific DTO)
    // PatientAccount sẽ có toPublicProfile() trả về kiểu khác → không đặt ở interface
    virtual QStringList getMenuItems()                       const = 0;
    virtual bool        canAccess(const QString& moduleCode) const = 0;
    virtual QString     getDisplayRole()                     const = 0;

    bool verifyPassword(const QString& plainPassword) const override;
    virtual std::unique_ptr<StaffPublicProfileDTO> toPublicProfile() const = 0;
};
