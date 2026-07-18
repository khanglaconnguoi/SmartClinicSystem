/**
 * @file    model/Pharmacist.h
 * @brief   Domain model cho Dược sĩ — định danh, RBAC, business validation
 *          Hàm isEligibleToDispense() là guard chính mà PharmacyService
 *          gọi trước khi cho phép cấp phát đơn thuốc.
 */
#pragma once
#include "SystemUser.h"

class Pharmacist : public SystemUser {
private:
    QString m_licenseNumber;    // Số chứng chỉ hành nghề dược
    QString m_pharmacySection;  // Khu vực làm việc: "Dược nội trú" | "Dược ngoại trú" | "Kho dược"
    int     m_experienceYears;

public:
    explicit Pharmacist(
        int            staffId,
        const QString& staffCode,
        const QString& passwordHash,
        const QString& fullName,
        const QPixmap& avatar,
        UserRole       role,
        bool           isActive,
        bool           mustChangePassword,
        const QString& licenseNumber,
        const QString& pharmacySection,
        int            experienceYears
    ) :
        SystemUser(staffId, staffCode, passwordHash, fullName,
                   avatar, role, isActive, mustChangePassword),
        m_licenseNumber(licenseNumber),
        m_pharmacySection(pharmacySection),
        m_experienceYears(experienceYears)
    {}

    ~Pharmacist() override = default;

    // ── Getters ───────────────────────────────────────────────────
    QString getLicenseNumber()   const { return m_licenseNumber; }
    QString getPharmacySection() const { return m_pharmacySection; }
    int     getExperienceYears() const { return m_experienceYears; }

    // ── Override SystemUser — RBAC ────────────────────────────────
    QStringList getMenuItems() const override {
        // Dược sĩ chỉ cần: Dashboard (hàng đợi cấp phát), Pharmacy (kho + đơn thuốc chờ)
        // KHÔNG có: Appointments, Queue (bệnh nhân), Schedule (lịch bác sĩ)
        return { "Dashboard", "Pharmacy", "Analytics" };
    }

    bool canAccess(const QString& moduleCode) const override {
        return getMenuItems().contains(moduleCode);
    }

    QString getDisplayRole() const override {
        if (!m_pharmacySection.isEmpty()) {
            return "Dược sĩ — " + m_pharmacySection;
        }
        return "Dược sĩ";
    }

    std::unique_ptr<StaffPublicProfileDTO> toPublicProfile() const override;

    // ── Domain Validation ─────────────────────────────────────────

    /**
     * @brief Guard chính trước khi cấp phát thuốc.
     *        PharmacyService::dispensePrescription() gọi hàm này đầu tiên.
     *        Điều kiện: tài khoản đang hoạt động + có số chứng chỉ hành nghề dược.
     */
    bool isEligibleToDispense() const;

    /**
     * @brief Dược sĩ có phụ trách khu vực phù hợp với đơn thuốc không?
     *        Hữu ích khi bệnh viện phân chia "Dược nội trú" vs "Dược ngoại trú".
     *        Truyền rỗng = không phân biệt khu vực.
     */
    bool coversSection(const QString& requiredSection) const;
};