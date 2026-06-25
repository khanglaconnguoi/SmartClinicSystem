#include "EmergencyPatient.h"

EmergencyPatient::EmergencyPatient(const QString &fullName, const QDate &birthDate, Gender gender,
                                   const QString &phoneNumber, const QString &address,
                                   const QString &citizenId, const QString &email,
                                   const QString &insurance, int severity)
    : Patient(fullName, birthDate, gender, phoneNumber, address, citizenId, email, insurance),
      m_severity(severity) {}

PatientPriority EmergencyPatient::getPriority() const {
    return PatientPriority::Critical;
}

QString EmergencyPatient::getBillingType() const {
    return QStringLiteral("Cấp cứu");
}

QString EmergencyPatient::getStatusLabel() const {
    return QStringLiteral("Bệnh nhân cấp cứu");
}

double EmergencyPatient::getBaseFee() const {
    return 1000000.0; // Phí cấp cứu
}

PatientType EmergencyPatient::getType() const {
    return PatientType::Emergency;
}

int EmergencyPatient::severity() const { return m_severity; }
void EmergencyPatient::setSeverity(int severity) { m_severity = severity; }
