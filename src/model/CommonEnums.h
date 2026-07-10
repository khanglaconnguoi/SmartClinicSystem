#pragma once

#include <QString>
#include <QStringList>

enum class Gender { Male, Female, Other };

enum class PatientType { Outpatient, Inpatient, Emergency };

enum class PatientPriority { Low, Normal, High, Critical };

enum class OutPatientState {
  Registered,
  WaitingForTreatment,
  Treatment,
  Discharged
};

// helper
enum class InPatientState { Admitted, Discharged, Transferred };

enum class EmergencyPatientState { Emergency, Discharged, Transferred };

enum class MedicationCategory {
  Antibiotics,
  Analgesics,
  Antipyretics,
  Cardiovascular,
  Antidiabetics,
  Vitamins,
  Respiratory,
  Gastrointestinal,
  Dermatological
};

inline QString genderToString(Gender gender) {
  switch (gender) {
  case Gender::Male:
    return "MALE";
  case Gender::Female:
    return "FEMALE";
  case Gender::Other:
    return "OTHER";
  }
  return "OTHER";
}

inline Gender genderFromString(const QString &genderStr) {
  if (genderStr == "MALE")
    return Gender::Male;
  if (genderStr == "FEMALE")
    return Gender::Female;
  return Gender::Other;
}

inline QString patientTypeToString(PatientType type) {
  switch (type) {
  case PatientType::Outpatient:
    return "Ngoại trú";
  case PatientType::Inpatient:
    return "Nội trú";
  case PatientType::Emergency:
    return "Cấp cứu";
  }
  return "Ngoại trú";
}

inline PatientType patientTypeFromString(const QString &str) {
  if (str == "Ngoại trú" || str == "OUTPATIENT" || str == "OutPatient")
    return PatientType::Outpatient;
  if (str == "Nội trú" || str == "INPATIENT" || str == "InPatient")
    return PatientType::Inpatient;
  if (str == "Cấp cứu" || str == "EMERGENCY" || str == "Emergency")
    return PatientType::Emergency;
  return PatientType::Outpatient;
}

inline QString patientPriorityToString(PatientPriority priority) {
  switch (priority) {
  case PatientPriority::Low:
    return "Thấp";
  case PatientPriority::Normal:
    return "Bình thường";
  case PatientPriority::High:
    return "Cao";
  case PatientPriority::Critical:
    return "Khẩn cấp";
  }
  return "Bình thường";
}

inline PatientPriority patientPriorityFromString(const QString &str) {
  if (str == "Thấp" || str == "LOW" || str == "Low")
    return PatientPriority::Low;
  if (str == "Bình thường" || str == "NORMAL" || str == "Normal")
    return PatientPriority::Normal;
  if (str == "Cao" || str == "HIGH" || str == "High")
    return PatientPriority::High;
  if (str == "Khẩn cấp" || str == "CRITICAL" || str == "Critical")
    return PatientPriority::Critical;
  return PatientPriority::Normal;
}

inline QString outPatientStateToString(OutPatientState state) {
  switch (state) {
  case OutPatientState::Registered:
    return "Đã đăng ký";
  case OutPatientState::WaitingForTreatment:
    return "Chờ điều trị";
  case OutPatientState::Treatment:
    return "Đang điều trị";
  case OutPatientState::Discharged:
    return "Đã xuất viện";
  }
  return "Đã đăng ký";
}

inline OutPatientState outPatientStateFromString(const QString &str) {
  if (str == "Đã đăng ký" || str == "REGISTERED")
    return OutPatientState::Registered;
  if (str == "Chờ điều trị" || str == "WAITING FOR TREATMENT")
    return OutPatientState::WaitingForTreatment;
  if (str == "Đang điều trị" || str == "TREATMENT")
    return OutPatientState::Treatment;
  if (str == "Đã xuất viện" || str == "DISCHARGED")
    return OutPatientState::Discharged;
  return OutPatientState::Registered;
}

inline QString inPatientStateToString(InPatientState state) {
  switch (state) {
  case InPatientState::Admitted:
    return "Đã nhập viện";
  case InPatientState::Discharged:
    return "Đã xuất viện";
  case InPatientState::Transferred:
    return "Chuyển viện";
  }
  return "Đã nhập viện";
}

inline InPatientState inPatientStateFromString(const QString &str) {
  if (str == "Đã nhập viện" || str == "ADMITTED")
    return InPatientState::Admitted;
  if (str == "Đã xuất viện" || str == "DISCHARGED")
    return InPatientState::Discharged;
  if (str == "Chuyển viện" || str == "TRANSFERED" || str == "TRANSFERRED")
    return InPatientState::Transferred;
  return InPatientState::Admitted;
}

inline QString emergencyPatientStateToString(EmergencyPatientState state) {
  switch (state) {
  case EmergencyPatientState::Emergency:
    return "Cấp cứu";
  case EmergencyPatientState::Discharged:
    return "Đã xuất viện";
  case EmergencyPatientState::Transferred:
    return "Chuyển viện";
  }
  return "Cấp cứu";
}

inline EmergencyPatientState
emergencyPatientStateFromString(const QString &str) {
  if (str == "Cấp cứu" || str == "EMERGENCY")
    return EmergencyPatientState::Emergency;
  if (str == "Đã xuất viện" || str == "DISCHARGED")
    return EmergencyPatientState::Discharged;
  if (str == "Chuyển viện" || str == "TRANSFERED" || str == "TRANSFERRED")
    return EmergencyPatientState::Transferred;
  return EmergencyPatientState::Emergency;
}

inline QString categoryToString(MedicationCategory category) {
  switch (category) {
  case MedicationCategory::Antibiotics:
    return "Kháng sinh";
  case MedicationCategory::Analgesics:
    return "Giảm đau";
  case MedicationCategory::Antipyretics:
    return "Hạ sốt";
  case MedicationCategory::Cardiovascular:
    return "Tim mạch";
  case MedicationCategory::Antidiabetics:
    return "Trị tiểu đường";
  case MedicationCategory::Vitamins:
    return "Vitamin & Thực phẩm chức năng";
  case MedicationCategory::Respiratory:
    return "Hô hấp";
  case MedicationCategory::Gastrointestinal:
    return "Tiêu hóa";
  case MedicationCategory::Dermatological:
    return "Thuốc bôi da liễu";
  default:
    return "Khác";
  }
}

inline MedicationCategory categoryFromString(const QString &str) {
  if (str == "Kháng sinh")
    return MedicationCategory::Antibiotics;
  if (str == "Giảm đau")
    return MedicationCategory::Analgesics;
  if (str == "Hạ sốt")
    return MedicationCategory::Antipyretics;
  if (str == "Tim mạch")
    return MedicationCategory::Cardiovascular;
  if (str == "Trị tiểu đường")
    return MedicationCategory::Antidiabetics;
  if (str == "Vitamin & Thực phẩm chức năng")
    return MedicationCategory::Vitamins;
  if (str == "Hô hấp")
    return MedicationCategory::Respiratory;
  if (str == "Tiêu hóa")
    return MedicationCategory::Gastrointestinal;
  if (str == "Thuốc bôi da liễu")
    return MedicationCategory::Dermatological;
  return MedicationCategory::Vitamins;
}

inline QStringList getAllCategories() {
  return {"Kháng sinh", "Giảm đau",       "Hạ sốt",
          "Tim mạch",   "Trị tiểu đường", "Vitamin & Thực phẩm chức năng",
          "Hô hấp",     "Tiêu hóa",       "Thuốc bôi da liễu"};
}

enum class InvoiceStatus { Unpaid, Paid, Cancelled };

inline QString invoiceStatusToString(InvoiceStatus status) {
  switch (status) {
  case InvoiceStatus::Unpaid:
    return "Chưa thanh toán";
  case InvoiceStatus::Paid:
    return "Đã thanh toán";
  case InvoiceStatus::Cancelled:
    return "Đã hủy";
  }
  return "Chưa thanh toán";
}

inline InvoiceStatus invoiceStatusFromString(const QString &str) {
  if (str == "Chưa thanh toán" || str == "UNPAID" || str == "Unpaid")
    return InvoiceStatus::Unpaid;
  if (str == "Đã thanh toán" || str == "PAID" || str == "Paid")
    return InvoiceStatus::Paid;
  if (str == "Đã hủy" || str == "CANCELLED" || str == "Cancelled")
    return InvoiceStatus::Cancelled;
  return InvoiceStatus::Unpaid;
}




enum class UserRole { Admin, Doctor, Nurse, Receptionist };

inline QString roleToString(UserRole role){
    switch(role) {
        case UserRole::Admin:         return "ADMIN";
        case UserRole::Doctor:        return "DOCTOR";
        case UserRole::Nurse:         return "NURSE";
        case UserRole::Receptionist:  return "RECEPTIONIST";
    }
    return "ADMIN";
}

inline UserRole roleFromString(const QString& roleStr){
    if(roleStr == "ADMIN")          return UserRole::Admin;
    if(roleStr == "DOCTOR")         return UserRole::Doctor;
    if(roleStr == "NURSE")          return UserRole::Nurse;
    if(roleStr == "RECEPTIONIST")   return UserRole::Receptionist;
    return UserRole::Admin;
}

