#include "Doctor.h"


std::unique_ptr<StaffPublicProfileDTO> Doctor::toPublicProfile() const {
    auto dto = std::make_unique<DoctorPublicProfileDTO>();
    dto->staffId         = m_staffId;
    dto->fullName        = m_fullName;
    dto->displayRole     = getDisplayRole();
    dto->specialty       = m_specialty;
    dto->licenseNumber   = m_licenseNumber;
    dto->experienceYears = m_experienceYears;
    dto->consultationFee = m_consultationFee;
    dto->bio             = m_bio;
    return dto;  // unique_ptr<DoctorPublicProfileDTO> → implicit upcast → unique_ptr<StaffPublicProfileDTO>
}


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

