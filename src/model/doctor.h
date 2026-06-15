#pragma once
#include "SystemUser.h"

class Doctor : public SystemUser {
private:
    QString m_specialty;        // Chuyên khoa
    QString m_licenseNumber;    // Số chứng chỉ hành nghề
    int     m_experienceYears;
    int     m_consultationFee;  // Phí khám mặc định

public:
    // Doctor(int id, const QString code, const QString& username, const QString& passwordHash, const QString& fullName, const QString& specialty, const QString& licenseNo, int experienceYears, int consultationFee) : 
    //     SystemUser(id, username, passwordHash, fullName, UserRole::Doctor), 
    //     m_specialty(specialty), 
    //     m_licenseNumber(licenseNo), 
    //     m_experienceYears(experienceYears), 
    //     m_consultationFee(consultationFee) {}

    // --- Getters ---
    QString getSpecialty()       const { return m_specialty; }
    QString getLicenseNumber()   const { return m_licenseNumber; }
    double  getConsultationFee() const { return m_consultationFee; }

    // --- Override từ SystemUser ---
    std::vector<QString> getMenuItems()           const override;
    bool canAccess(const QString& moduleCode)     const override;
    QString getDisplayRole()                      const override { return "Bác sĩ"; }

    // --- Hành vi đặc thù của Doctor ---
    // bool prescribeMedication(int patientId, const std::vector<PrescriptionItem>& items);
    // bool createMedicalRecord(int patientId, const MedicalRecord& record);
};
