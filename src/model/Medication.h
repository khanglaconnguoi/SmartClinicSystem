/**
 * @file    model/Medication.h
 * @brief   Domain model cho thuốc trong kho dược
 *          Chứa domain validation không cần DB — PharmacyService gọi trước khi INSERT
 */
#pragma once
#include "dto/MedicationDTOs.h"
#include <QString>
#include <QDate>

class Medication {
private:
    int m_medicationId;
    QString m_brandName;
    QList<MedicationIngredientDTO> m_ingredients;
    QList<QString> m_categories;  // Một thuốc thuộc nhiều danh mục
    QString m_unit;
    double m_unitPrice;
    int m_stockQuantity;
    int m_minimumStock;
    int m_reorderThreshold;
    QDate m_expiryDate;
    QString m_manufacturer;
    QString m_description;
    bool m_isActive;

public:
    explicit Medication(
        int medicationId,
        const QString& brandName,
        const QList<MedicationIngredientDTO>& ingredients,
        const QList<QString>& categories,
        const QString& unit,
        double unitPrice,
        int stockQuantity,
        int minimumStock,
        int reorderThreshold,
        const QDate& expiryDate,
        const QString& manufacturer,
        const QString& description,
        bool isActive
    ) :
        m_medicationId(medicationId),
        m_brandName(brandName),
        m_ingredients(ingredients),
        m_categories(categories),
        m_unit(unit),
        m_unitPrice(unitPrice),
        m_stockQuantity(stockQuantity),
        m_minimumStock(minimumStock),
        m_reorderThreshold(reorderThreshold),
        m_expiryDate(expiryDate),
        m_manufacturer(manufacturer),
        m_description(description),
        m_isActive(isActive)
    {}

    ~Medication() = default;

    int getMedicationId() const { return m_medicationId; }
    QString getBrandName() const { return m_brandName; }
    QList<MedicationIngredientDTO> getIngredients() const { return m_ingredients; }
    QList<QString> getCategories() const { return m_categories; }
    QString getUnit() const { return m_unit; }
    double getUnitPrice() const { return m_unitPrice; }
    int getStockQuantity() const { return m_stockQuantity; }
    int getMinimumStock() const { return m_minimumStock; }
    int getReorderThreshold() const { return m_reorderThreshold; }
    QDate getExpiryDate() const { return m_expiryDate; }
    QString getManufacturer() const { return m_manufacturer; }
    QString getDescription() const { return m_description; }
    bool isActive() const { return m_isActive; }

    // ── Domain Validation (thuần logic, không cần DB) ────────────────

    /**
     * @brief Còn đủ số lượng để cấp phát không?
     *        PharmacyService::createPrescription() gọi TRƯỚC khi tạo PrescriptionItem.
     * @param requiredQty Số lượng cần cấp phát
     */
    bool isAvailable(int requiredQty) const;

    /**
     * @brief Tồn kho dưới ngưỡng reorder chưa?
     *        PharmacyService dùng để trigger cảnh báo Observer → Admin/Dược sĩ.
     */
    bool isLowStock() const;

    /** stock <= minimum_stock — cảnh báo nghiêm trọng: gần hết */
    bool isCriticalStock() const;

    /**
     * @brief Đã hết hạn sử dụng chưa?
     */
    bool isExpired() const;

    /**
     * @brief Sắp hết hạn trong vòng N ngày không?
     * @param daysThreshold Số ngày cảnh báo trước (mặc định 30 ngày)
     */
    bool isExpiringSoon(int daysThreshold = 30) const;

    /**
     * @brief ✅ Kiểm tra kết hợp: có thể kê đơn ngay bây giờ không?
     *        = isActive + không hết hạn + còn đủ số lượng
     *        Doctor::prescribeMedication() và PharmacyService đều dùng hàm này
     *        để guard trước khi cho phép kê đơn.
     * @param requiredQty Số lượng cần kê
     */
    bool isEligibleForPrescription(int requiredQty) const;

    MedicationSummaryDTO toSummary() const;
};