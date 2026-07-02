#pragma once

#include "dto/BillingDTOs.h"
#include <QList>
#include <optional>
#include <QString>

class BillingRepository {
private:
    QString generateInvoiceCode(); // helper sinh mã "INV-yyyyMMdd-NNNN"

public:
    bool insertInvoice(const InvoiceInsertDTO &dto);
    bool updateInvoice(const InvoiceUpdateDTO &dto);
    bool cancelInvoice(int invoiceId);
    std::optional<InvoiceResultDTO> getInvoiceByRecordId(int recordId);
    QList<InvoiceResultDTO> getInvoicesByPatientId(int patientId);
};
