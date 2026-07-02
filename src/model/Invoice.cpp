#include "Invoice.h"

/**
 * @brief   Khởi tạo một hóa đơn mới.
 * @param   patientId        ID của bệnh nhân.
 * @param   consultationFee  Phí khám bệnh.
 * @param   medicationFee    Phí thuốc.
 * @param   issuedDate       Ngày phát hành hóa đơn.
 */
Invoice::Invoice(int patientId, double consultationFee, double medicationFee, const QDate& issuedDate)
    : m_invoiceId(0),
      m_invoiceCode(""),
      m_patientId(patientId),
      m_recordId(std::nullopt),
      m_consultationFee(consultationFee),
      m_medicationFee(medicationFee),
      m_issuedDate(issuedDate),
      m_status("UNPAID")
{
}
