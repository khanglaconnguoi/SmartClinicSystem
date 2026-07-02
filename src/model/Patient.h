// File: model/patient.h
#pragma once
#include "CommonEnums.h"
#include <QDate>
#include <QString>

class Patient {
protected:
  int m_patientId;
  QString m_patientCode; // Mã bệnh nhân (VD: BN-2024-0001)
  QString m_fullName;
  QDate m_dateOfBirth;
  QString m_gender;
  QString m_citizenId;
  QString m_phone;
  QString m_email;
  QString m_address;
  QString m_bloodType;
  QString m_allergies; // Danh sách dị ứng, phân cách bởi ","
  QString m_insurance;
  PatientType m_type;
  QString m_emergencyContactName;
  QString m_emergencyContactPhone;

public:
  explicit Patient(int patientId, const QString &patientCode,
                   const QString &fullName, const QDate &dateOfBirth,
                   const QString &gender, const QString &citizenId,
                   const QString &phone, const QString &email,
                   const QString &address, const QString &bloodType,
                   const QString &allergies, const QString &insurance,
                   PatientType type, const QString &emergencyContactName,
                   const QString &emergencyContactPhone);
  virtual ~Patient() = default;

  // --- Getters phổ biến ---
  int getPatientId() const { return m_patientId; }
  QString getPatientCode() const { return m_patientCode; }
  QString getFullName() const { return m_fullName; }
  QDate getDateOfBirth() const { return m_dateOfBirth; }
  int getAge() const;
  QString getAllergies() const { return m_allergies; }
  QString getType() const { return PatientTypeToString(m_type); }

  // --- Pure Virtual (Đa hình) ---
  virtual PatientPriority getPriority() const = 0;
  virtual QString getBillingType() const = 0;
  virtual QString getStatusLabel() const = 0;
  virtual double getBaseFee() const = 0;

  // --- Utility ---
  bool hasAllergy(const QString &medicationName) const;
  static QString generatePatientCode(int id);
};