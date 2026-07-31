#include "Medication.h"

bool Medication::isAvailable(int requiredQty) const {
    return m_isActive && (m_stockQuantity >= requiredQty);
}

bool Medication::isLowStock() const {
    return m_stockQuantity <= m_reorderThreshold;
}

bool Medication::isCriticalStock() const {
    return m_stockQuantity <= m_minimumStock;
}

bool Medication::isExpired() const {
    if (!m_expiryDate.isValid()) {
        return false;
    }
    return m_expiryDate < QDate::currentDate();
}

bool Medication::isExpiringSoon(int daysThreshold) const {
    if (!m_expiryDate.isValid()) {
        return false;
    }
    
    int daysLeft = QDate::currentDate().daysTo(m_expiryDate);
    
    return (daysLeft >= 0) && (daysLeft <= daysThreshold);
}

bool Medication::isEligibleForPrescription(int requiredQty) const {
    return m_isActive && !isExpired() && isAvailable(requiredQty);
}




MedicationSummaryDTO Medication::toSummary() const {
    MedicationSummaryDTO dto;
    dto.medicationId = m_medicationId;
    dto.brandName = m_brandName;
    dto.categories = m_categories;
    dto.unit = m_unit;
    dto.unitPrice = m_unitPrice;
    dto.stockQuantity = m_stockQuantity;
    dto.minimumStock = m_minimumStock;
    dto.reorderThreshold = m_reorderThreshold;
    dto.expiryDate = m_expiryDate;
    dto.manufacturer = m_manufacturer;
    dto.description = m_description;
    dto.isActive = m_isActive;
    
    dto.isCriticalStock = this->isCriticalStock();
    dto.isLowStock = this->isLowStock();
    dto.isExpiringSoon = this->isExpiringSoon();
    
    dto.ingredients = m_ingredients; 
    return dto;
}