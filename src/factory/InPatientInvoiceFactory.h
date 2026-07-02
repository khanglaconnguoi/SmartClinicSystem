#pragma once
#include "IInvoiceFactory.h"

class InPatientInvoiceFactory : public IInvoiceFactory {
public:
    std::unique_ptr<Invoice> createInvoice(
        int patientId, double consultationFee, double medicationFee,
        const QDate &issuedDate) override;
};
