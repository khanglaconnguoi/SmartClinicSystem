#include "OutPatient.h"

PatientPriority OutPatient::getPriority() const {
    return PatientPriority::Normal;
}

QString OutPatient::getBillingType() const {
    return QStringLiteral("Ngoại trú");
}

QString OutPatient::getStatusLabel() const {
    return QStringLiteral("Bệnh nhân ngoại trú");
}

double OutPatient::getBaseFee() const {
    return 150000.0; // Phí khám cơ bản
}

PatientType OutPatient::getType() const {
    return PatientType::OutPatient;
}
