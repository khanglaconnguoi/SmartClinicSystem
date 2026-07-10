#include "OutPatientInvoice.h"

OutPatientInvoice::OutPatientInvoice(int patientId, double consultationFee, double medicationFee, const QDate& issuedDate)
    : Invoice(patientId, consultationFee, medicationFee, issuedDate)
{
}

double OutPatientInvoice::calculate() const {
    return m_consultationFee + m_medicationFee; // không phụ phí
}

QString OutPatientInvoice::toSummaryString() const {
    return QString("Hóa đơn ngoại trú #%1 — %2 VNĐ")
        .arg(m_invoiceCode).arg(calculate(), 0, 'f', 0);
}
