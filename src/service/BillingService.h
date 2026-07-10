#pragma once
#include "dto/BillingDTOs.h"
#include "model/CommonEnums.h"
#include <memory>
#include <optional>
#include <QList>

#include "dto/PrescriptionDTOs.h"

class BillingRepository;
class IInvoiceFactory;

class BillingService {
private:
  std::shared_ptr<BillingRepository> m_billingRepository;
  std::unique_ptr<IInvoiceFactory> selectFactory(PatientType type) const;

  /**
   * @brief Kiểm tra đầu vào trước khi phát sinh hóa đơn.
   * @return Chuỗi rỗng nếu hợp lệ, chuỗi lỗi nếu không hợp lệ.
   */
  static QString validateInvoiceInput(int patientId, int recordId,
                                      double consultationFee,
                                      const QList<PrescriptionItemDTO> &items);

  /**
   * @brief Kiểm tra tiêu chí tìm kiếm trước khi xuống Repository.
   * @return Chuỗi rỗng nếu hợp lệ, chuỗi lỗi nếu không hợp lệ.
   */
  static QString validateSearchCriteria(const InvoiceSearchCriteria &criteria);

public:
  explicit BillingService(std::shared_ptr<BillingRepository> repo);

  bool generateInvoice(int patientId, int recordId, PatientType type,
                       double consultationFee,
                       const QList<PrescriptionItemDTO> &prescriptionItems);

  double calculateMedicationTotal(const QList<PrescriptionItemDTO> &items) const;
  std::optional<InvoiceResultDTO> getInvoiceByRecordId(int recordId);

  bool updateInvoice(const InvoiceUpdateDTO &dto);
  bool cancelInvoice(int invoiceId);

  /**
   * @brief Tìm kiếm hóa đơn theo tiêu chí linh hoạt.
   *        Hỗ trợ partial match (LIKE %...%) và không phân biệt hoa thường.
   */
  QList<InvoiceSummaryDTO> searchInvoices(const InvoiceSearchCriteria &criteria);

  /**
   * @brief Đếm tổng số kết quả khớp tiêu chí — dùng cho phân trang.
   */
  int countSearchResults(const InvoiceSearchCriteria &criteria);
};
