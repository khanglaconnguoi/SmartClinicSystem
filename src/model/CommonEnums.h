#pragma once

#include <QString>

enum class Gender { Male, Female, Other };

enum class PatientType { OUTPATIENT, INPATIENT, EMERGENCY };

enum class PatientPriority { LOW, NORMAL, HIGH, CRITICAL };

enum class OutPatientState {
  REGISTERED,
  WAITING_FOR_TREATMENT,
  TREATMENT,
  DISCHARGED
};

// helper
enum class InPatientState { ADMITTED, DISCHARGED, TRANSFERED };

enum class EmergencyPatientState { EMERGENCY, DISCHARGED, TRANSFERRED };

inline QString GenderToString(Gender gender) {
  switch (gender) {
  case Gender::Male:
    return QString("MALE");
  case Gender::Female:
    return QString("FEMALE");
  case Gender::Other:
    return QString("OTHER");
  default:
    return QString("UNKNOWN");
  }
}

inline QString PatientTypeToString(PatientType type) {
  switch (type) {
  case PatientType::OUTPATIENT:
    return QString("OUTPATIENT");
  case PatientType::INPATIENT:
    return QString("INPATIENT");
  case PatientType::EMERGENCY:
    return QString("EMERGENCY");
  default:
    return QString("UNKNOWN");
  }
}

inline InPatientState stringToInPatientState(const QString &str) {
  if (str == "ADMITTED")
    return InPatientState::ADMITTED;
  if (str == "DISCHARGED")
    return InPatientState::DISCHARGED;
  if (str == "TRANSFERED")
    return InPatientState::TRANSFERED;
  return InPatientState::ADMITTED;
}

inline PatientType stringToPatientType(const QString &str) {
  if (str == "OutPatient")
    return PatientType::OUTPATIENT;
  if (str == "InPatient")
    return PatientType::INPATIENT;
  if (str == "Emergency")
    return PatientType::EMERGENCY;
  return PatientType::OUTPATIENT;
}

inline PatientPriority stringToPatientPriority(const QString &str) {
  if (str == "Low")
    return PatientPriority::LOW;
  if (str == "Normal")
    return PatientPriority::NORMAL;
  if (str == "High")
    return PatientPriority::HIGH;
  if (str == "Critical")
    return PatientPriority::CRITICAL;
  return PatientPriority::NORMAL;
}

inline Gender stringToGender(const QString &str) {
  if (str == "Male")
    return Gender::Male;
  if (str == "Female")
    return Gender::Female;
  return Gender::Other;
}

inline QString OutPatientStateToString(OutPatientState state) {
  switch (state) {
  case OutPatientState::REGISTERED:
    return QString("REGISTERED");
  case OutPatientState::WAITING_FOR_TREATMENT:
    return QString("WAITING FOR TREATMENT");
  case OutPatientState::TREATMENT:
    return QString("TREATMENT");
  case OutPatientState::DISCHARGED:
    return QString("DISCHARGED");
  }
}

inline OutPatientState stringToOutPatientState(const QString &str) {
  if (str == "REGISTERED")
    return OutPatientState::REGISTERED;
  if (str == "WAITING FOR TREATMENT")
    return OutPatientState::WAITING_FOR_TREATMENT;
  if (str == "TREATMENT")
    return OutPatientState::TREATMENT;
  if (str == "DISCHARGED")
    return OutPatientState::DISCHARGED;
  return OutPatientState::REGISTERED;
}

inline QString InPatientStateToString(InPatientState state) {
  switch (state) {
  case InPatientState::ADMITTED:
    return QString("ADMITTED");
  case InPatientState::DISCHARGED:
    return QString("DISCHARGED");
  case InPatientState::TRANSFERED:
    return QString("TRANSFERED");
  }
}

inline QString EmergencyPatientStateToString(EmergencyPatientState state) {
  switch (state) {
  case EmergencyPatientState::EMERGENCY:
    return QString("EMERGENCY");
  case EmergencyPatientState::DISCHARGED:
    return QString("DISCHARGED");
  case EmergencyPatientState::TRANSFERRED:
    return QString("TRANSFERED");
  }
}

inline EmergencyPatientState stringToEmergencyPatientState(const QString &str) {
  if (str == "EMERGENCY")
    return EmergencyPatientState::EMERGENCY;
  if (str == "DISCHARGED")
    return EmergencyPatientState::DISCHARGED;
  if (str == "TRANSFERED")
    return EmergencyPatientState::TRANSFERRED;
  return EmergencyPatientState::EMERGENCY;
}

enum class InvoiceStatus { UNPAID, PAID, CANCELLED };

inline QString InvoiceStatusToString(InvoiceStatus status) {
  switch (status) {
  case InvoiceStatus::UNPAID:    return QString("UNPAID");
  case InvoiceStatus::PAID:      return QString("PAID");
  case InvoiceStatus::CANCELLED: return QString("CANCELLED");
  }
}

inline InvoiceStatus stringToInvoiceStatus(const QString &str) {
  if (str == "UNPAID")    return InvoiceStatus::UNPAID;
  if (str == "PAID")      return InvoiceStatus::PAID;
  if (str == "CANCELLED") return InvoiceStatus::CANCELLED;
  return InvoiceStatus::UNPAID;
}