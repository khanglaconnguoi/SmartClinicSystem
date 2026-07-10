#pragma once
#include "model/Invoice.h"
#include <memory>

class IInvoiceFactory {
public:
  virtual ~IInvoiceFactory() = default;
  virtual std::unique_ptr<Invoice> createInvoice(
      int patientId, double consultationFee, double medicationFee,
      const QDate &issuedDate) = 0;
};
