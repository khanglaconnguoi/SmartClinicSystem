#include "Doctor.h"

bool Doctor::isEligibleToPrescribe() const {
    return m_isActive && !m_licenseNumber.trimmed().isEmpty();
}

bool Doctor::matchesSpecialty(const QString& requiredSpecialty) const {
    if (requiredSpecialty.trimmed().isEmpty()) return true;
    return m_specialty.contains(requiredSpecialty.trimmed(), Qt::CaseInsensitive);
}

bool Doctor::hasValidConsultationFee() const {
    return m_consultationFee > 0;
}