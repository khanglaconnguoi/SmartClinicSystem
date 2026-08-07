#include "InPatientInvoice.h"

InPatientInvoice::InPatientInvoice(int patientId, double consultationFee, double medicationFee, const QDate& issuedDate)
    : Invoice(patientId, consultationFee, medicationFee, issuedDate)
{
}

QString InPatientInvoice::getInvoiceType() const {
    return "INPATIENT";
}

double InPatientInvoice::calculate() const {
    return m_consultationFee + m_medicationFee; // (chưa tính phụ phí giường bệnh theo ngày)
}

QString InPatientInvoice::toSummaryString() const {
    return QString("Hóa đơn nội trú #%1 — %2 VNĐ")
        .arg(m_invoiceCode).arg(calculate(), 0, 'f', 0);
}
