#include "EmergencyInvoiceFactory.h"
#include "model/EmergencyInvoice.h"

std::unique_ptr<Invoice> EmergencyInvoiceFactory::createInvoice(
    int patientId, double consultationFee, double medicationFee,
    const QDate &issuedDate) {
    return std::make_unique<EmergencyInvoice>(patientId, consultationFee, medicationFee, issuedDate);
}
