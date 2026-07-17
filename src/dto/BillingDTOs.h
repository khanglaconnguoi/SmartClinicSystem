#pragma once

#include "model/CommonEnums.h"
#include <QDate>
#include <QString>
#include <QList>
#include <optional>

struct InvoiceItemDTO {
  // ── invoice_items ─────────────────────────────────────────────────────────
  QString itemType;    // item_type    NOT NULL  (vd: "CONSULTATION" | "MEDICATION" | "SERVICE")
  QString description; // description  NOT NULL
  int quantity;        // quantity     NOT NULL  DEFAULT 1
  double unitPrice;    // unit_price   NOT NULL
  double subtotal;     // subtotal     NOT NULL
};

struct InvoiceInsertDTO {
  // ── invoices ──────────────────────────────────────────────────────────────
  // invoice_code  → NOT NULL UNIQUE  (tự sinh bởi Repository/Service)
  int patientId;                    // patient_id      NOT NULL  FK → patients
  std::optional<int> recordId;      // record_id       nullable  FK → medical_records
  PatientType patientType;          // patient_type    NOT NULL  CHECK: OUTPATIENT|INPATIENT|EMERGENCY
  double consultationFee;           // consultation_fee NOT NULL  DEFAULT 0
  double medicationFee;             // medication_fee   NOT NULL  DEFAULT 0
  double totalAmount;               // total_amount     NOT NULL  DEFAULT 0
  // status  → NOT NULL DEFAULT 'UNPAID'  CHECK: UNPAID|PAID|CANCELLED  (Repository tự gán)
  QDate issuedDate;                 // issued_date      NOT NULL
  // paid_date → nullable  (chỉ set khi thanh toán)

  // ── invoice_items (chi tiết) ───────────────────────────────────────────────
  QList<InvoiceItemDTO> items;      // require ≥1 item
};

struct InvoiceResultDTO {
  // ── invoices ──────────────────────────────────────────────────────────────
  int invoiceId;                    // invoice_id       NOT NULL  PK
  QString invoiceCode;              // invoice_code     NOT NULL  UNIQUE
  int patientId;                    // patient_id       NOT NULL
  std::optional<int> recordId;      // record_id        nullable
  PatientType patientType;          // patient_type     NOT NULL  CHECK: OUTPATIENT|INPATIENT|EMERGENCY
  double consultationFee;           // consultation_fee NOT NULL
  double medicationFee;             // medication_fee   NOT NULL
  double totalAmount;               // total_amount     NOT NULL
  QString status;                   // status           NOT NULL  CHECK: UNPAID|PAID|CANCELLED
  QDate issuedDate;                 // issued_date      NOT NULL
  std::optional<QDate> paidDate;    // paid_date        nullable

  // ── invoice_items (JOIN) ───────────────────────────────────────────────────
  QList<InvoiceItemDTO> items;
};

struct InvoiceUpdateDTO {
  // ── invoices ──────────────────────────────────────────────────────────────
  int invoiceId;          // invoice_id       NOT NULL  PK  (bắt buộc để UPDATE)
  double consultationFee; // consultation_fee NOT NULL
  double medicationFee;   // medication_fee   NOT NULL
  double totalAmount;     // total_amount     NOT NULL

  // ── invoice_items (replace toàn bộ khi update) ────────────────────────────
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
  int patientId = -1;                     // -1 = tất cả bệnh nhân
  QString status;                         // "" = tất cả  CHECK: UNPAID|PAID|CANCELLED
  std::optional<PatientType> patientType; // nullopt = tất cả loại  CHECK: OUTPATIENT|INPATIENT|EMERGENCY

  // Khoảng ngày phát hành (issued_date)
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
  int invoiceId;                 // invoice_id   NOT NULL  PK
  QString invoiceCode;           // invoice_code NOT NULL  UNIQUE
  int patientId;                 // patient_id   NOT NULL
  std::optional<int> recordId;   // record_id    nullable
  PatientType patientType;       // patient_type NOT NULL  CHECK: OUTPATIENT|INPATIENT|EMERGENCY
  double totalAmount;            // total_amount NOT NULL
  QString status;                // status       NOT NULL  CHECK: UNPAID|PAID|CANCELLED
  QDate issuedDate;              // issued_date  NOT NULL
  std::optional<QDate> paidDate; // paid_date    nullable
};

