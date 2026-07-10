#pragma once
#include "Patient.h"

class EmergencyPatient : public Patient {
public:
  EmergencyPatient();
  EmergencyPatient(int patientId, const QString &patientCode,
                   const QString &fullName, const QDate &dateOfBirth,
                   const QString &gender, const QString &citizenId,
                   const QString &phone, const QString &email,
                   const QString &address, const QString &bloodType,
                   const QString &allergies, const QString &insurance,
                   PatientType type, const QString &emergencyContactName,
                   const QString &emergencyContactPhone,
                   const QString &emergencyRoomId,
                   const QString &emergencyDoctorId, const QString &injuryCause,
                   const QString &injuryDescription, const QDate &admissionDate,
                   const QDate &dischargeDate, EmergencyPatientState status)
      : Patient(patientId, patientCode, fullName, dateOfBirth, gender,
                citizenId, phone, email, address, bloodType, allergies,
                insurance, type, emergencyContactName, emergencyContactPhone) {
    m_emergencyRoomId = emergencyRoomId;
    m_emergencyDoctorId = emergencyDoctorId;
    m_injuryCause = injuryCause;
    m_injuryDescription = injuryDescription;
    m_admissionDate = admissionDate;
    m_dischargeDate = dischargeDate;
    m_status = status;
  }
  PatientPriority getPriority() const override;
  QString getBillingType() const override;
  QString getStatusLabel() const override;
  double getBaseFee() const override;
  QString getEmergencyRoomId() const;
  void setEmergencyRoomId(const QString &emergencyRoomId);
  QString getEmergencyDoctorId() const;
  void setEmergencyDoctorId(const QString &emergencyDoctorId);
  QString getInjuryCause() const;
  void setInjuryCause(const QString &injuryCause);
  QString getInjuryDescription() const;
  void setInjuryDescription(const QString &injuryDescription);
  QDate getAdmissionDate() const;
  void setAdmissionDate(const QDate &admissionDate);
  QDate getDischargeDate() const;
  void setDischargeDate(const QDate &dischargeDate);
  EmergencyPatientState getStatus() const;
  void setStatus(EmergencyPatientState status);

private:
  QString m_emergencyRoomId;   // Phòng cấp cứu được chỉ định
  QString m_emergencyDoctorId; // Bác sĩ cấp cứu phụ trách
  QString m_injuryCause;       // Nguyên nhân chấn thương (tai nạn, ngộ độc...)
  QString m_injuryDescription; // Mô tả sơ bộ chấn thương/vấn đề
  QDate m_admissionDate;
  QDate m_dischargeDate;
  EmergencyPatientState m_status;
};