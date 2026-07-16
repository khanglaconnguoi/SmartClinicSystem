#pragma once

#include "model/CommonEnums.h"
#include <QDate>
#include <QString>
#include <QList>
#include <optional>

struct InvoiceItemDTO {
    QString itemType;    // "CONSULTATION" / "MEDICATION" / "SERVICE"
    QString description;
    int quantity;
    double unitPrice;
    double subtotal;
};

struct InvoiceInsertDTO {
    int patientId;
    std::optional<int> recordId;
    PatientType patientType;
    double consultationFee;
    double medicationFee;
    double totalAmount;
    QDate issuedDate;
    QList<InvoiceItemDTO> items;
};

struct InvoiceResultDTO {
    int invoiceId;
    QString invoiceCode;
    int patientId;
    std::optional<int> recordId;
    PatientType patientType;
    double consultationFee;
    double medicationFee;
    double totalAmount;
    QString status;
    QDate issuedDate;
    std::optional<QDate> paidDate;
    QList<InvoiceItemDTO> items;
};

struct InvoiceUpdateDTO {
    int invoiceId;
    double consultationFee;
    double medicationFee;
    double totalAmount;
    QList<InvoiceItemDTO> items;
};

// ═══════════════════════════════════════════════════════════════════════════
// SEARCH DTOs
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Tiêu chí tìm kiếm hóa đơn.
 *        Tất cả trường để rỗng / -1 / nullopt = không lọc theo trường đó.
 */
struct InvoiceSearchCriteria {
  // TEXT search: LIKE trên invoice_code
  QString searchKey;

  // Bộ lọc
  int patientId = -1;                         // -1 = tất cả bệnh nhân
  QString status;                             // "" = tất cả (UNPAID/PAID/CANCELLED)
  std::optional<PatientType> patientType;     // nullopt = tất cả loại

  // Khoảng ngày phát hành
  std::optional<QDate> fromDate;
  std::optional<QDate> toDate;

  // Phân trang
  int limit  = 50;
  int offset = 0;
};

/**
 * @brief Kết quả gọn khi tìm kiếm hóa đơn.
 *        Không load danh sách invoice_items — chỉ dùng để hiển thị list.
 */
struct InvoiceSummaryDTO {
    int                  invoiceId;
    QString              invoiceCode;
    int                  patientId;
    std::optional<int>   recordId;
    PatientType          patientType;
    double               totalAmount;
    QString              status;
    QDate                issuedDate;
    std::optional<QDate> paidDate;
};

