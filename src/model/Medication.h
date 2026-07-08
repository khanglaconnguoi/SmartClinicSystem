// File: model/Medication.h
#pragma once
#include <QString>
#include <QDate>

class Medication {
private:
    int     m_medicationId;
    QString m_medicationCode;
    QString m_medicationName;
    int     m_categoryId;
    QString m_unit;
    double  m_unitPrice;
    int     m_stockQuantity;
    int     m_reorderThreshold;
    QDate   m_expiryDate;
    QString m_manufacturer;
    bool    m_isActive;

public:
    explicit Medication(
        int     medicationId,
        QString medicationCode,
        QString medicationName,
        int     categoryId,
        QString unit,
        double  unitPrice,
        int     stockQuantity,
        int     reorderThreshold,
        QDate   expiryDate,
        QString manufacturer,
        bool    isActive
    ) : 
        m_medicationId(medicationId),
        m_medicationCode(medicationCode),
        m_medicationName(medicationName),
        m_categoryId(categoryId),
        m_unit(unit),
        m_unitPrice(unitPrice),
        m_stockQuantity(stockQuantity),
        m_reorderThreshold(reorderThreshold),
        m_expiryDate(expiryDate),
        m_manufacturer(manufacturer),
        m_isActive(isActive)
    {}
    ~Medication() = default;

    // ── Getters ──────────────────────────────────────────────────
    int     getMedicationId()    const { return m_medicationId; }
    QString getMedicationCode()  const { return m_medicationCode; }
    QString getMedicationName()  const { return m_medicationName; }
    double  getUnitPrice()       const { return m_unitPrice; }
    int     getStockQuantity()   const { return m_stockQuantity; }
    QString getUnit()            const { return m_unit; }
    QDate   getExpiryDate()      const { return m_expiryDate; }
    bool    isActive()           const { return m_isActive; }

    // ── Domain Validation (không cần DB) ─────────────────────────
    /**
     * @brief Còn đủ số lượng để cấp phát không?
     *        PharmacyService gọi trước khi tạo prescription item.
     */
    bool isAvailable(int requiredQty) const;

    /**
     * @brief Tồn kho dưới ngưỡng reorder chưa?
     *        PharmacyService dùng để trigger cảnh báo hết hàng.
     */
    bool isLowStock() const;

    /**
     * @brief Đã hết hạn sử dụng chưa?
     */
    bool isExpired() const;

    /**
     * @brief Sắp hết hạn trong vòng N ngày không?
     * @param daysThreshold Số ngày cảnh báo trước (mặc định 30 ngày)
     */
    bool isExpiringSoon(int daysThreshold = 30) const;
};