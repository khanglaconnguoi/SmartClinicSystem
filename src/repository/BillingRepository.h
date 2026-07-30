#pragma once

#include "dto/BillingDTOs.h"
#include <QList>
#include <QString>
#include <QVariantList>
#include <optional>

class BillingRepository {
private:
  /**
   * @brief Build mệnh đề WHERE động dựa trên tiêu chí tìm kiếm.
   *        Escape ký tự LIKE đặc biệt, thêm wildcard %...%.
   */
  QString buildSearchWhereClause(const InvoiceSearchCriteria &criteria,
                                 QVariantList &outParams) const;

public:
  // Helper for Service layer to generate invoice codes
  int countInvoicesByPrefix(const QString &prefix);

  bool insertInvoice(const InvoiceInsertDTO &dto);
  bool updateInvoice(const InvoiceUpdateDTO &dto);
  bool cancelInvoice(int invoiceId);
  std::optional<InvoiceResultDTO> getInvoiceByRecordId(int recordId);
  QList<InvoiceResultDTO> getInvoicesByPatientId(int patientId);

  /**
   * @brief Tìm kiếm hóa đơn theo tiêu chí linh hoạt.
   *        Hỗ trợ partial match (LIKE %...%) và không phân biệt hoa thường.
   */
  QList<InvoiceSummaryDTO>
  searchInvoices(const InvoiceSearchCriteria &criteria);

  PagedResult<InvoiceSummaryDTO>
  searchInvoicesPaged(const InvoiceSearchCriteria &criteria) const;

  /**
   * @brief Đếm tổng số kết quả khớp tiêu chí — dùng cho phân trang.
   */
  int countSearchResults(const InvoiceSearchCriteria &criteria) const;
};

