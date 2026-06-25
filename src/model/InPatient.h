#pragma once
#include "Patient.h"

class InPatient : public Patient {
public:
    InPatient() = default;
    InPatient(const QString &fullName, const QDate &birthDate, Gender gender,
              const QString &phoneNumber, const QString &address,
              const QString &citizenId = {}, const QString &email = {},
              const QString &insurance = {}, const QString &roomNo = {});

    PatientPriority getPriority() const override;
    QString getBillingType() const override;
    QString getStatusLabel() const override;
    double getBaseFee() const override;
    PatientType getType() const override;

    QString roomNo() const;
    void setRoomNo(const QString &roomNo);
    QDate admitDate() const;
    void setAdmitDate(const QDate &date);

private:
    QString m_roomNo;
    QDate m_admitDate;
};
