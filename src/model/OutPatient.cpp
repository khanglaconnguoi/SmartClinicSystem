/**
 * @file    OutPatient.cpp
 * @brief   Implementation cho OutPatient entity (Bệnh nhân ngoại trú).
 */
#include "OutPatient.h"

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

/**
 * @brief Constructor mặc định — tạo đối tượng OutPatient rỗng.
 *
 * Hữu ích khi cần khởi tạo tạm hoặc deserialize từ database.
 */
OutPatient::OutPatient()
    : Patient(0,
              QString(),               // m_patientCode
              QString(),               // m_fullName
              QDate(),                 // m_dateOfBirth
              QString(),               // m_gender
              QString(),               // m_citizenId
              QString(),               // m_phone
              QString(),               // m_email
              QString(),               // m_address
              QString(),               // m_bloodType
              QString(),               // m_allergies
              QString(),               // m_insurance
              PatientType::OUTPATIENT, // m_type
              QString(),               // m_emergencyContactName
              QString())               // m_emergencyContactPhone
{}

// ---------------------------------------------------------------------------
// Pure Virtual Overrides
// ---------------------------------------------------------------------------

/**
 * @brief Trả về mức độ ưu tiên của bệnh nhân ngoại trú.
 *
 * Bệnh nhân ngoại trú mặc định ở mức Normal.
 *
 * @return PatientPriority::Normal
 */
PatientPriority OutPatient::getPriority() const {
  return PatientPriority::NORMAL;
}

/**
 * @brief Trả về loại hình thanh toán của bệnh nhân ngoại trú.
 *
 * @return Chuỗi "Outpatient Billing".
 */
QString OutPatient::getBillingType() const {
  return QString("Outpatient Billing");
}

/**
 * @brief Trả về nhãn trạng thái hiển thị cho bệnh nhân ngoại trú.
 *
 * @return Chuỗi "Outpatient".
 */
QString OutPatient::getStatusLabel() const { return QString("Outpatient"); }

/**
 * @brief Trả về phí khám cơ bản cho bệnh nhân ngoại trú.
 *
 * @return Phí khám cơ bản (đơn vị: VNĐ).
 */
double OutPatient::getBaseFee() const {
  return 150000.0; // 150,000 VNĐ — phí khám ngoại trú cơ bản
}