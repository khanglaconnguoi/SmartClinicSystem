#pragma once
#include "Patient.h"

class OutPatient : public Patient {
public:
    using Patient::Patient; // Kế thừa constructor từ Patient

    PatientPriority getPriority() const override;
    QString getBillingType() const override;
    QString getStatusLabel() const override;
    double getBaseFee() const override;
    PatientType getType() const override;
};
