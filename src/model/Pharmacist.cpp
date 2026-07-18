/**
 * @file    model/Pharmacist.cpp
 */
#include "Pharmacist.h"

std::unique_ptr<StaffPublicProfileDTO> Pharmacist::toPublicProfile() const {
    // Pharmacist dùng base StaffPublicProfileDTO — không cần subclass riêng
    // vì thông tin công khai của dược sĩ không có gì đặc biệt hơn base
    auto dto = std::make_unique<StaffPublicProfileDTO>();
    dto->staffId     = m_staffId;
    dto->fullName    = m_fullName;
    dto->displayRole = getDisplayRole();
    return dto;
}

bool Pharmacist::isEligibleToDispense() const {
    // Giống isEligibleToPrescribe() của Doctor về mặt cấu trúc,
    // nhưng tách riêng vì nghiệp vụ khác nhau — Doctor kê đơn, Pharmacist cấp phát
    return m_isActive && !m_licenseNumber.trimmed().isEmpty();
}

bool Pharmacist::coversSection(const QString& requiredSection) const {
    if (requiredSection.trimmed().isEmpty()) return true;
    return m_pharmacySection.contains(requiredSection.trimmed(), Qt::CaseInsensitive);
}