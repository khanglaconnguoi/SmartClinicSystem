/**
 * @file    InPatient.cpp
 * @brief   Implementation cho InPatient entity (Bệnh nhân nội trú).
 */
#include "InPatient.h"

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

/**
 * @brief Constructor mặc định — tạo đối tượng InPatient rỗng.
 */
InPatient::InPatient()
    : Patient(0, QString(), QString(), QDate(), QString(), QString(), QString(),
              QString(), QString(), QString(), QString(), QString(),
              PatientType::Inpatient, QString(), QString()),
      m_admissionId(QString()), m_roomId(QString()),
      m_admittingDoctorId(QString()), m_admissionDate(QDate()),
      m_dischargeDate(QDate()), m_reason(QString()), m_status(QString()) {}
// ---------------------------------------------------------------------------
// Pure Virtual Overrides
// ---------------------------------------------------------------------------

/**
 * @brief Trả về mức độ ưu tiên của bệnh nhân nội trú.
 *
 * @return PatientPriority::High
 */
PatientPriority InPatient::getPriority() const { return PatientPriority::High; }

/**
 * @brief Trả về loại hình thanh toán của bệnh nhân nội trú.
 *
 * @return Chuỗi "Inpatient Billing".
 */
QString InPatient::getBillingType() const {
  return QString("Inpatient Billing");
}

/**
 * @brief Trả về nhãn trạng thái hiển thị cho bệnh nhân nội trú.
 *
 * @return Giá trị của m_status nếu có, ngược lại trả về "Inpatient".
 */
QString InPatient::getStatusLabel() const {
  return m_status.isEmpty() ? QString("Inpatient") : m_status;
}

/**
 * @brief Trả về phí cơ bản cho bệnh nhân nội trú mỗi ngày.
 *
 * @return Phí cơ bản (đơn vị: VNĐ).
 */
double InPatient::getBaseFee() const {
  return 500000.0; // 500,000 VNĐ — phí nội trú cơ bản mỗi ngày
}
