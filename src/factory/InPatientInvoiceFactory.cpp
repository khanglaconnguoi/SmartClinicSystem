#include "InPatientInvoiceFactory.h"
#include "model/InPatientInvoice.h"

std::unique_ptr<Invoice> InPatientInvoiceFactory::createInvoice(
    int patientId, double consultationFee, double medicationFee,
    const QDate &issuedDate) {
    return std::make_unique<InPatientInvoice>(patientId, consultationFee, medicationFee, issuedDate);
}
