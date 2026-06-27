#pragma once
#include "SystemUser.h"
#include "MedicalRecord.h"

class Doctor : public SystemUser {
private:
    QString m_specialty;        // Chuyên khoa
    QString m_licenseNumber;    // Số chứng chỉ hành nghề
    int     m_experienceYears;
    int     m_consultationFee;  // Phí khám mặc định
    QString m_bio;

public:
    explicit Doctor(
        int             staffId, 
        const QString&  staffCode,
        const QString&  passwordHash, 
        const QString&  fullName, 
        const QPixmap&  avatar,
        UserRole        role, 
        bool            isActive,
        const QString&  specialty,
        const QString&  licenseNumber,
        int             experienceYears,
        int             consultationFee,
        const QString&  bio
    ) : 
        SystemUser(
            staffId, 
            staffCode,
            passwordHash, 
            fullName, 
            avatar,
            role, 
            isActive
        ), 
        m_specialty(specialty), 
        m_licenseNumber(licenseNumber), 
        m_experienceYears(experienceYears), 
        m_consultationFee(consultationFee),
        m_bio(bio)
    {}

    ~Doctor() override = default;

    // --- Getters ---
    QString getSpecialty()         const { return m_specialty; }
    QString getLicenseNumber()     const { return m_licenseNumber; }
    int     getExperienceYears()   const { return m_experienceYears; }
    double  getConsultationFee()   const { return m_consultationFee; }
    QString getBio()               const { return m_bio; }
    
    // ── Setters (Admin/Doctor cập nhật hồ sơ — qua StaffService) ─
    // void setSpecialty(const QString& specialty);
    // void setConsultationFee(double fee);
    // void setExperienceYears(int years);
    // void setBio(const QString& bio);


    // --- Override từ SystemUser ---
    QStringList getMenuItems() const override {
        return { "Dashboard", "Appointments", "Schedule", "Messages" };
    }

    bool canAccess(const QString& moduleCode) const override {
        return getMenuItems().contains(moduleCode);
    }

    QString getDisplayRole() const override {
        if (!m_specialty.isEmpty()) {
            return "Doctor " + m_specialty;
        }
        return "Doctor";
    }

    // ── Domain Validation (thuần business logic, không cần DB) ───
    /**
     * @brief Bác sĩ có đủ điều kiện kê đơn không?
     *        PharmacyService gọi hàm này trước khi tạo prescription.
     *        Điều kiện: tài khoản đang hoạt động + có số chứng chỉ hành nghề.
     */
    bool isEligibleToPrescribe() const;

    /**
     * @brief Chuyên khoa của bác sĩ có phù hợp với yêu cầu không?
     *        QueueService dùng để lọc bác sĩ phù hợp khi xếp hàng.
     * @param requiredSpecialty Chuyên khoa cần tìm, rỗng = chấp nhận tất cả
     */
    bool matchesSpecialty(const QString& requiredSpecialty) const;

    /**
     * @brief Phí khám có hợp lệ để lập hóa đơn không?
     *        BillingService dùng trước khi tạo invoice item "CONSULTATION".
     */
    bool hasValidConsultationFee() const;

    // --- Hành vi đặc thù của Doctor ---
    // bool prescribeMedication(int patientId, const std::vector<PrescriptionItem>& items);
    // bool createMedicalRecord(int patientId, const MedicalRecord& record);
};
