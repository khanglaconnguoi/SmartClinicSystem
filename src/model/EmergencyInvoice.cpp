#include "EmergencyInvoice.h"

EmergencyInvoice::EmergencyInvoice(int patientId, double consultationFee, double medicationFee, const QDate& issuedDate)
    : Invoice(patientId, consultationFee, medicationFee, issuedDate)
{
}

QString EmergencyInvoice::getInvoiceType() const {
    return "EMERGENCY";
}

double EmergencyInvoice::calculate() const {
    return m_consultationFee + m_medicationFee; // Tính cơ bản, có thể thêm m_emergencySurcharge nếu cần
}

QString EmergencyInvoice::toSummaryString() const {
    return QString("Hóa đơn cấp cứu #%1 — %2 VNĐ")
        .arg(m_invoiceCode).arg(calculate(), 0, 'f', 0);
}
