#pragma once
#include "IAuthenticatable.h"
#include <QString>
#include <QPixmap>

class PatientUser : public IAuthenticatable {
private:
    int m_patientId;
    QString m_fullName;
    QPixmap m_avatar;

public:
    PatientUser(int id, const QString& fullName)
        : m_patientId(id), m_fullName(fullName) {
        // Tạo avatar mặc định màu xanh dương cho bệnh nhân
        m_avatar = QPixmap(36, 36);
        m_avatar.fill(QColor("#2563EB"));
    }

    virtual ~PatientUser() override = default;

    // Implement IAuthenticatable
    int getAccountId() const override { return m_patientId; }
    QString getPasswordHash() const override { return ""; } // Không cần cho mock
    AccountType getAccountType() const override { return AccountType::Patient; }
    QString getFullName() const override { return m_fullName; }
    QPixmap getAvatar() const override { return m_avatar; }

    QStringList getMenuItems() const override {
        return {"Tổng quan", "Lịch hẹn", "Hồ sơ bệnh án", "Đơn thuốc"};
    }
    bool canAccess(const QString& /*moduleCode*/) const override { return true; }
    QString getDisplayRole() const override { return "PATIENT"; }
};
