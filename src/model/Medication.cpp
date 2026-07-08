#include "Medication.h"




bool Medication::isAvailable(int requiredQty) const {
    return m_isActive && (m_stockQuantity >= requiredQty);
}

bool Medication::isLowStock() const {
    return m_stockQuantity <= m_reorderThreshold;
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