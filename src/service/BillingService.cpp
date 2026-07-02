#include "BillingService.h"
#include "repository/BillingRepository.h"
#include "factory/OutPatientInvoiceFactory.h"
#include "factory/InPatientInvoiceFactory.h"
#include "factory/EmergencyInvoiceFactory.h"
#include <QDate>

// Mock struct của đồng đội, dùng tạm để code độc lập
struct PrescriptionItemDTO {
    int medicationId;
    QString medicationName;
    int quantity;
    double unitPrice;
    QString dosage;
    QString frequency;
    int durationDays;
    QString note;
};

BillingService::BillingService(std::shared_ptr<BillingRepository> repo)
    : m_billingRepository(repo) {}

std::unique_ptr<IInvoiceFactory> BillingService::selectFactory(PatientType type) const {
    if (type == PatientType::INPATIENT) {
        return std::make_unique<InPatientInvoiceFactory>();
    } else if (type == PatientType::EMERGENCY) {
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
        mItem.description = pItem.medicationName;
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
