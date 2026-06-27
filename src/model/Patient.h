// File: model/patient.h
#pragma once
#include "model/CommonEnums.h"
#include <QString>
#include <QDate>
#include <vector>

enum class PatientType   { OUTPATIENT, INPATIENT, EMERGENCY };
enum class PatientPriority { LOW, NORMAL, HIGH, CRITICAL };

// class Patient {
// protected:
//     int         m_patientId;
//     QString     m_patientCode;     // Mã bệnh nhân (VD: BN-2024-0001)
//     QString     m_fullName;
//     QString     m_dateOfBirth;
//     Gender      m_gender;
//     QString     m_phone;
//     QString     m_address;
//     QString     m_bloodType;
//     QString     m_allergies;       // Danh sách dị ứng, phân cách bởi ","
//     PatientType m_type;

// public:
//     explicit Patient(const QString& fullName, const QDate& dob, PatientType type);
//     virtual ~Patient() = default;

//     // --- Getters phổ biến ---
//     int         getPatientId()   const { return m_patientId; }
//     QString     getPatientCode() const { return m_patientCode; }
//     QString     getFullName()    const { return m_fullName; }
//     QDate       getDateOfBirth() const { return m_dateOfBirth; }
//     int         getAge()         const;
//     QString     getAllergies()   const { return m_allergies; }
//     PatientType getType()        const { return m_type; }

//     // --- Pure Virtual (Đa hình) ---
//     virtual PatientPriority getPriority()       const = 0;
//     virtual QString         getBillingType()    const = 0;
//     virtual QString         getStatusLabel()    const = 0;
//     virtual double          getBaseFee()        const = 0;

//     // --- Utility ---
//     bool hasAllergy(const QString& medicationName) const;
//     static QString generatePatientCode(int id);
// };
