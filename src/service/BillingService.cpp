#include "BillingService.h"
#include "repository/BillingRepository.h"
#include "factory/OutPatientInvoiceFactory.h"
#include "factory/InPatientInvoiceFactory.h"
#include "factory/EmergencyInvoiceFactory.h"
#include "dto/PrescriptionDTOs.h"
#include <QDate>
#include <QDebug>

BillingService::BillingService(std::shared_ptr<BillingRepository> repo)
    : m_billingRepository(repo) {}

// ─────────────────────────────────────────────────────────────────────────────
// Validate
// ─────────────────────────────────────────────────────────────────────────────

QString BillingService::validatePatientId(int patientId) {
  if (patientId <= 0) return "Định danh bệnh nhân không hợp lệ.";
  return "";
}

QString BillingService::validateRecordId(int recordId) {
  if (recordId <= 0) return "Định danh hồ sơ khám không hợp lệ.";
  return "";
}

QString BillingService::validateConsultationFee(double consultationFee) {
  if (consultationFee < 0) return "Phí khám không được âm.";
  return "";
}

QString BillingService::validatePrescriptionItems(const QList<PrescriptionItemDTO> &items) {
  for (const auto &item : items) {
    if (item.quantity <= 0)
      return QString("Thuốc '%1' có số lượng không hợp lệ (%2).")
          .arg(item.brandName).arg(item.quantity);
    if (item.unitPrice < 0)
      return QString("Thuốc '%1' có đơn giá không hợp lệ.").arg(item.brandName);
  }
  return "";
}

void BillingService::normalizeSearchCriteria(InvoiceSearchCriteria &criteria) {
  criteria.searchKey = criteria.searchKey.simplified();
  criteria.status = criteria.status.trimmed().toUpper();
}

QString BillingService::validateInvoiceInput(int patientId, int recordId,
                                             double consultationFee,
                                             const QList<PrescriptionItemDTO> &items) {
  QString err;
  if (!(err = validatePatientId(patientId)).isEmpty()) return err;
  if (!(err = validateRecordId(recordId)).isEmpty()) return err;
  if (!(err = validateConsultationFee(consultationFee)).isEmpty()) return err;
  if (!(err = validatePrescriptionItems(items)).isEmpty()) return err;
  return "";
}

std::unique_ptr<IInvoiceFactory> BillingService::selectFactory(PatientType type) const {
    if (type == PatientType::Inpatient) {
        return std::make_unique<InPatientInvoiceFactory>();
    } else if (type == PatientType::Emergency) {
        return std::make_unique<EmergencyInvoiceFactory>();
    } else {
        return std::make_unique<OutPatientInvoiceFactory>();
    }
}

double BillingService::calculateMedicationTotal(const QList<PrescriptionItemDTO> &items) const {
    double total = 0.0;
    for (const auto &item : items) {
        total += (item.quantity * item.unitPrice);
    }
    return total;
}

bool BillingService::generateInvoice(int patientId, int recordId, PatientType type,
                                     double consultationFee,
                                     const QList<PrescriptionItemDTO> &prescriptionItems) {
    QString err = validateInvoiceInput(patientId, recordId, consultationFee, prescriptionItems);
    if (!err.isEmpty()) {
        qDebug() << "BillingService::generateInvoice validation failed:" << err;
        return false;
    }

    double medicationFee = calculateMedicationTotal(prescriptionItems);
    
    // Sử dụng Factory để tạo ra đối tượng Invoice (nhằm tận dụng hàm calculate() nếu cần mở rộng)
    auto factory = selectFactory(type);
    auto invoiceModel = factory->createInvoice(patientId, consultationFee, medicationFee, QDate::currentDate());

    InvoiceInsertDTO dto;
    dto.patientId = patientId;
    dto.recordId = recordId;
    dto.patientType = type;
    dto.consultationFee = consultationFee;
    dto.medicationFee = medicationFee;
    dto.totalAmount = invoiceModel->calculate(); // tổng tiền được tính từ model
    dto.issuedDate = QDate::currentDate();

    // Build items
    InvoiceItemDTO consultationItem;
    consultationItem.itemType = "CONSULTATION";
    consultationItem.description = "Phí khám bệnh";
    consultationItem.quantity = 1;
    consultationItem.unitPrice = consultationFee;
    consultationItem.subtotal = consultationFee;
    dto.items.append(consultationItem);

    for (const auto &pItem : prescriptionItems) {
        InvoiceItemDTO mItem;
        mItem.itemType = "MEDICATION";
        mItem.description = pItem.brandName;
        mItem.quantity = pItem.quantity;
        mItem.unitPrice = pItem.unitPrice;
        mItem.subtotal = pItem.quantity * pItem.unitPrice;
        dto.items.append(mItem);
    }

    return m_billingRepository->insertInvoice(dto);
}

std::optional<InvoiceResultDTO> BillingService::getInvoiceByRecordId(int recordId) {
    return m_billingRepository->getInvoiceByRecordId(recordId);
}

bool BillingService::updateInvoice(const InvoiceUpdateDTO &dto) {
    return m_billingRepository->updateInvoice(dto);
}

bool BillingService::cancelInvoice(int invoiceId) {
    return m_billingRepository->cancelInvoice(invoiceId);
}

QString BillingService::validateSearchCriteria(const InvoiceSearchCriteria &criteria) {
  // Validate khoảng ngày nếu cả hai đầu đều hợp lệ
  if (criteria.fromDate.has_value() && criteria.toDate.has_value() &&
      criteria.fromDate->isValid() && criteria.toDate->isValid()) {
    if (criteria.fromDate.value() > criteria.toDate.value())
      return "Ngày bắt đầu (Từ ngày) không được lớn hơn ngày kết thúc (Đến ngày).";
  }
  return "";
}

QList<InvoiceSummaryDTO> BillingService::searchInvoices(
    InvoiceSearchCriteria criteria) {
  normalizeSearchCriteria(criteria);
  QString err = validateSearchCriteria(criteria);
  if (!err.isEmpty()) {
    qDebug() << "BillingService::searchInvoices validation failed:" << err;
    return {};
  }
  return m_billingRepository->searchInvoices(criteria);
}

int BillingService::countSearchResults(InvoiceSearchCriteria criteria) {
  normalizeSearchCriteria(criteria);
  QString err = validateSearchCriteria(criteria);
  if (!err.isEmpty()) {
    qDebug() << "BillingService::countSearchResults validation failed:" << err;
    return 0;
  }
  return m_billingRepository->countSearchResults(criteria);
}
