#pragma once
#include "Patient.h"

class EmergencyPatient : public Patient {
public:
    EmergencyPatient() = default;
    EmergencyPatient(const QString &fullName, const QDate &birthDate, Gender gender,
                     const QString &phoneNumber, const QString &address,
                     const QString &citizenId = {}, const QString &email = {},
                     const QString &insurance = {}, int severity = 5);

    PatientPriority getPriority() const override;
    QString getBillingType() const override;
    QString getStatusLabel() const override;
    double getBaseFee() const override;
    PatientType getType() const override;

    int severity() const;
    void setSeverity(int severity);

private:
    int m_severity = 5; // 1-10 (10 = critical)
};
