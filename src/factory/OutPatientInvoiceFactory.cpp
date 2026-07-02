#include "OutPatientInvoiceFactory.h"
#include "model/OutPatientInvoice.h"

std::unique_ptr<Invoice> OutPatientInvoiceFactory::createInvoice(
    int patientId, double consultationFee, double medicationFee,
    const QDate &issuedDate) {
    return std::make_unique<OutPatientInvoice>(patientId, consultationFee, medicationFee, issuedDate);
}
