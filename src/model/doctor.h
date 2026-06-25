#pragma once
#include "SystemUser.h"

class Doctor : public SystemUser {
private:
    QString m_specialty;        // Chuyên khoa
    QString m_licenseNumber;    // Số chứng chỉ hành nghề
    int     m_experienceYears;
    int     m_consultationFee;  // Phí khám mặc định

public:
    Doctor(
        int             staffId, 
        const QString&  staffCode,
        const QString&  passwordHash, 
        const QString&  fullName, 
        QPixmap         avatar,
        UserRole        role, 
        bool            isActive,
        const QString&  specialty,
        const QString&  licenseNumber,
        int             experienceYears,
        int             consultationFee
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
        m_consultationFee(consultationFee) 
    {}

    // --- Getters ---
    QString getSpecialty()       const { return m_specialty; }
    QString getLicenseNumber()   const { return m_licenseNumber; }
    double  getConsultationFee() const { return m_consultationFee; }

    // --- Override từ SystemUser ---
    QStringList getMenuItems() const override {
        return { "Dashboard", "Appointments", "Schedule", "Messages" };
    }

    bool canAccess(const QString& moduleCode) const override {
        return getMenuItems().contains(moduleCode);
    }

    QString getDisplayRole() const override { return "Doctor"; }

    // --- Hành vi đặc thù của Doctor ---
    // bool prescribeMedication(int patientId, const std::vector<PrescriptionItem>& items);
    // bool createMedicalRecord(int patientId, const MedicalRecord& record);
};
