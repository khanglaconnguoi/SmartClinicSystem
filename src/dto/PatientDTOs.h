/**
 * @file    PatientDTOs.h
 * @brief   Data Transfer Objects cho Patient module
 *          Chỉ là data containers — không có behavior
 */

#pragma once

#include "model/CommonEnums.h"
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QString>
#include <optional>

// ═══════════════════════════════════════════════════════════════════════════
// ALLERGY DTOs  (map với bảng `patient_allergies`)
// ═══════════════════════════════════════════════════════════════════════════

struct AllergyInsertDTO {
  int patientId;
  QString allergenName; 
  QString severity;     // 'MILD' | 'MODERATE' | 'SEVERE'
  QString notes;        
};

struct AllergyResultDTO {
  int allergyId;
  QString allergenName;
  QString severity;
  QString notes;
  bool isActive;
  QString recordedAt;
  QString updatedAt;
};

// ═══════════════════════════════════════════════════════════════════════════
// INSURANCE DTOs  (map với bảng `patient_insurance`)
// ═══════════════════════════════════════════════════════════════════════════

struct InsuranceInsertDTO {
  int patientId;
  QString providerName;          
  QString policyNumber;          
  QString insuranceType;         
  double coveragePercent = 80.0; 
  QString validFrom;             
  QString validTo;               
  QString notes;
};

struct InsuranceResultDTO {
  int insuranceId = 0;
  QString providerName;
  QString policyNumber;
  QString insuranceType;
  double coveragePercent = 0.0;
  QString validFrom;
  QString validTo;
  QString notes;
  bool isActive = false;
  QString createdAt;
  QString updatedAt;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1 – INPUT DTOs  (UI → Service)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientInputDTO {
  QString fullName;              
  QDate   dateOfBirth;           
  QString gender;                // Dùng QString thay cho Enum Gender (đã loại bỏ)
  QString citizenId;             
  QString phone;                 
  QString email;                 
  QString address;               
  QString bloodType;             
  QString allergies;             
  QString insurance;             
  PatientType type;              // Giữ PatientType làm Enum class (Nhóm 3)
  QString emergencyContactName;  
  QString emergencyContactPhone; 

  virtual ~PatientInputDTO() = default;
};

struct InPatientInputDTO : public PatientInputDTO {
  std::optional<int> roomId;          
  std::optional<int> doctorId;        
  QDate admissionDate;                
  std::optional<QDate> dischargeDate; 
  QString reason;                     
};

struct EmergencyPatientInputDTO : public PatientInputDTO {
  std::optional<int> roomId;          
  std::optional<int> doctorId;        
  QString injuryCause;                
  QString injuryDescription;          
  QDate admissionDate;                
  std::optional<QDate> dischargeDate; 
};

struct OutPatientInputDTO : public PatientInputDTO {
  std::optional<int> doctorId;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2 – INSERT DTOs  (Service → Repository)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientInsertDTO {
  QString patientCode; 
  QString fullName;              
  QString dateOfBirth;           
  QString gender;                
  QString citizenId;             
  QString phone;                 
  QString email;                 
  QString address;               
  QString bloodType;             
  QString type;                  
  QString emergencyContactName;  
  QString emergencyContactPhone; 

  QList<AllergyInsertDTO> allergies; 
  std::optional<InsuranceInsertDTO> insurance; 

  virtual ~PatientInsertDTO() = default;
};

struct OutPatientInsertDTO : public PatientInsertDTO {
  std::optional<int> doctorId; 
  QString status;              
};

struct InPatientInsertDTO : public PatientInsertDTO {
  std::optional<int> roomId;   
  std::optional<int> doctorId; 
  QString admissionDate;       
  QString dischargeDate;       
  QString reason;              
  QString status;              
};

struct EmergencyPatientInsertDTO : public PatientInsertDTO {
  std::optional<int> roomId;   
  std::optional<int> doctorId; 
  QString injuryCause;         
  QString injuryDescription;   
  QString admissionDate;       
  QString dischargeDate;       
  QString status;              
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3 – UPDATE DTOs  (Service → Repository)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientUpdateDTO {
  int patientId; 

  QString fullName;
  QString dateOfBirth;
  QString gender;
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  QString emergencyContactName;
  QString emergencyContactPhone;

  virtual ~PatientUpdateDTO() = default;
};

struct OutPatientUpdateDTO : public PatientUpdateDTO {
  QString status;
  std::optional<int> doctorId;
};

struct InPatientUpdateDTO : public PatientUpdateDTO {
  std::optional<int> roomId;
  std::optional<int> doctorId;
  QString admissionDate;
  QString dischargeDate;
  QString reason;
  QString status;
};

struct EmergencyPatientUpdateDTO : public PatientUpdateDTO {
  std::optional<int> roomId;
  std::optional<int> doctorId;
  QString injuryCause;
  QString injuryDescription;
  QString admissionDate;
  QString dischargeDate;
  QString status;
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 4 – RESULT DTOs  (Repository → Service → UI)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientDetailDTO {
  int patientId;
  QString patientCode;
  QString fullName;
  QDate dateOfBirth;
  QString gender;                // Dùng QString
  QString citizenId;
  QString phone;
  QString email;
  QString address;
  QString bloodType;
  PatientType defaultPatientType;
  QString emergencyContactName;
  QString emergencyContactPhone;
  bool isDeleted;
  QDateTime createdAt;
  QDateTime updatedAt;

  QList<AllergyResultDTO> allergies; 
  std::optional<InsuranceResultDTO> insurance; 

  PatientType currentType; 
  QString status;          

  std::optional<int> roomId;          
  std::optional<int> doctorId;        
  std::optional<QDate> admissionDate; 
  std::optional<QDate> dischargeDate; 

  QString reason;            
  QString injuryCause;       
  QString injuryDescription; 
};

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 5 – FIND / FILTER DTOs  (UI → Service → Repository)
// ═══════════════════════════════════════════════════════════════════════════

struct PatientSearchCriteria {
  QString searchKey;

  std::optional<PatientType> type; 
  int roomId = -1;                 

  QString status; 
  bool onlyActive = true;
  bool includeDeleted = false;

  std::optional<QDate> fromDate;
  std::optional<QDate> toDate;

  int limit = 50;
  int offset = 0;
};

struct PatientSearchResultDTO {
  int patientId;
  QString patientCode;
  QString fullName;
  QDate dateOfBirth;
  QString gender;                // Dùng QString
  QString phone;

  PatientType type;    
  QString statusLabel; 
  QString roomId;      
};
