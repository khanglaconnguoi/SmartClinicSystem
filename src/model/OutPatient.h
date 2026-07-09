#pragma once

#include "Patient.h"

class OutPatient : public Patient {
public:
  OutPatient();
  explicit OutPatient(int patientId, const QString &patientCode,
                      const QString &fullName, const QDate &dateOfBirth,
                      const QString &gender, const QString &citizenId,
                      const QString &phone, const QString &email,
                      const QString &address, const QString &bloodType,
                      const QString &allergies, const QString &insurance,
                      PatientType type, const QString &emergencyContactName,
                      const QString &emergencyContactPhone)
      : Patient(patientId, patientCode, fullName, dateOfBirth, gender,
                citizenId, phone, email, address, bloodType, allergies,
                insurance, type, emergencyContactName, emergencyContactPhone) {}
  PatientPriority getPriority() const override;
  QString getBillingType() const override;
  QString getStatusLabel() const override;
  double getBaseFee() const override;
};