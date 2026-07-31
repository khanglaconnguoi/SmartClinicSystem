#pragma once
#include "dto/BillingDTOs.h"
#include "model/CommonEnums.h"
#include "repository/BillingRepository.h"
#include "factory/IInvoiceFactory.h"
#include "dto/PrescriptionDTOs.h"
#include <memory>
#include <optional>
#include <QList>

class BillingService {
private:
  std::shared_ptr<BillingRepository> m_billingRepository;
  std::unique_ptr<IInvoiceFactory> selectFactory(PatientType type) const;

  /**
   * @brief Kiểm tra tiêu chí tìm kiếm trước khi xuống Repository.
   * @return Chuỗi rỗng nếu hợp lệ, chuỗi lỗi nếu không hợp lệ.
   */
  static QString validateSearchCriteria(const InvoiceSearchCriteria &criteria);

private:
  QString generateInvoiceCode();

public:
  static QString validateConsultationFee(double consultationFee);
  static QString validatePrescriptionItems(const QList<PrescriptionItemDTO> &items);
  static void normalizeSearchCriteria(InvoiceSearchCriteria &criteria);

  /**
   * @brief Kiểm tra đầu vào trước khi phát sinh hóa đơn.
   * @return Chuỗi rỗng nếu hợp lệ, chuỗi lỗi nếu không hợp lệ.
   */
  static QString validateInvoiceInput(int patientId, int recordId,
                                      double consultationFee,
                                      const QList<PrescriptionItemDTO> &items);

  explicit BillingService(std::shared_ptr<BillingRepository> repo);

  bool createInvoice(int patientId, int recordId, PatientType type,
                     double consultationFee,
                     const QList<PrescriptionItemDTO> &prescriptionItems);

  double calculateMedicationTotal(const QList<PrescriptionItemDTO> &items) const;
  std::optional<InvoiceResultDTO> getInvoiceByRecordId(int recordId);

  bool updateInvoice(const InvoiceUpdateDTO &dto);
  bool cancelInvoice(int invoiceId);
  bool collectPayment(int invoiceId);

  /**
   * @brief Tìm kiếm hóa đơn theo tiêu chí linh hoạt.
   *        Hỗ trợ partial match (LIKE %...%) và không phân biệt hoa thường.
   */
  QList<InvoiceSummaryDTO> searchInvoices(InvoiceSearchCriteria criteria);
  PagedResult<InvoiceSummaryDTO> searchInvoicesPaged(InvoiceSearchCriteria criteria) const;

  /**
   * @brief Đếm tổng số kết quả khớp tiêu chí — dùng cho phân trang.
   */
  int countSearchResults(InvoiceSearchCriteria criteria);
};

