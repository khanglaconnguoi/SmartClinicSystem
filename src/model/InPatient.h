#pragma once

#include "Patient.h"

class InPatient : public Patient {
public:
  InPatient();
  InPatient(int patientId, const QString &patientCode, const QString &fullName,
            const QDate &dateOfBirth, const QString &gender,
            const QString &citizenId, const QString &phone,
            const QString &email, const QString &address,
            const QString &bloodType, const QString &allergies,
            const QString &insurance, PatientType type,
            const QString &emergencyContactName,
            const QString &emergencyContactPhone, bool is_deleted,
            const QString &admissionId, const QString &roomId,
            const QString &admittingDoctorId, const QDate &admissionDate,
            const QDate &dischargeDate, const QString &reason,
            const QString &status)
      : Patient(patientId, patientCode, fullName, dateOfBirth, gender,
                citizenId, phone, email, address, bloodType, allergies,
                insurance, type, emergencyContactName, emergencyContactPhone,
                is_deleted) {
    m_admissionId = admissionId;
    m_roomId = roomId;
    m_admittingDoctorId = admittingDoctorId;
    m_admissionDate = admissionDate;
    m_dischargeDate = dischargeDate;
    m_reason = reason;
    m_status = status;
  }

  PatientPriority getPriority() const override;
  QString getBillingType() const override;
  QString getStatusLabel() const override;
  double getBaseFee() const override;

private:
  QString m_admissionId;
  QString m_roomId;
  QString m_admittingDoctorId;
  QDate m_admissionDate;
  QDate m_dischargeDate;
  QString m_reason;
  QString m_status;
};