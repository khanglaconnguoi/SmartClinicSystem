#pragma once
#include "dto/BillingDTOs.h"
#include "model/CommonEnums.h"
#include <memory>
#include <optional>
#include <QList>

class BillingRepository;
class IInvoiceFactory;
struct PrescriptionItemDTO; // của đồng đội

class BillingService {
private:
  std::shared_ptr<BillingRepository> m_billingRepository;
  std::unique_ptr<IInvoiceFactory> selectFactory(PatientType type) const;

public:
  explicit BillingService(std::shared_ptr<BillingRepository> repo);

  bool generateInvoice(int patientId, int recordId, PatientType type,
                       double consultationFee,
                       const QList<PrescriptionItemDTO> &prescriptionItems);

  double calculateMedicationTotal(const QList<PrescriptionItemDTO> &items) const;
  std::optional<InvoiceResultDTO> getInvoiceByRecordId(int recordId);

  bool updateInvoice(const InvoiceUpdateDTO &dto);
  bool cancelInvoice(int invoiceId);
};
