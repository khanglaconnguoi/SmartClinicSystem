#pragma once

#include "dto/PatientDTOs.h"
#include "model/MedicalRecord.h"
#include <QDateTime>
#include <QList>
#include <QString>
#include <optional>

struct MedicalRecordInsertDTO {
  // ── medical_records ──────────────────────────────────────────────────────
  int patientId;           // patient_id     NOT NULL  FK → patients
  int doctorId;            // doctor_id      NOT NULL  FK → staff
  int appointmentId;       // appointment_id NOT NULL  FK → appointments
  QDateTime visitDateTime; // visit_datetime NOT NULL

  // ── vital signs (bảng medical_records, tất cả nullable trong DB) ────────
  VitalSigns vitals;
  //   vitals.temperature    → temperature   nullable  REAL
  //   vitals.bloodPressure  → blood_pressure nullable  TEXT  (vd: "120/80")
  //   vitals.heartRate      → heart_rate    nullable  INTEGER
  //   vitals.weight         → weight        nullable  REAL  (kg)
  //   vitals.height         → height        nullable  REAL  (cm)

  // ── nội dung khám (nullable trong DB, nhưng Service validate bắt buộc) ──
  QString chiefComplaint; // chief_complaint nullable  (Service: require)
  QString clinicalNotes;  // clinical_notes  nullable  (Service: require)
  QString treatment;      // treatment       nullable  (Service: require)
  std::optional<QDate> nextVisitDate; // next_visit_date nullable

  // ── chẩn đoán (bảng diagnoses) ───────────────────────────────────────────
  QList<Diagnosis> diagnoses; // require ≥1 item  (Service validate)
  //   Diagnosis.icdCode      → icd_code    nullable
  //   Diagnosis.description  → description NOT NULL
  //   Diagnosis.severity     → severity    NOT NULL  DEFAULT 'MILD'  CHECK:
  //   MILD|MODERATE|SEVERE

  // ── dị ứng mới phát hiện trong lần khám này ──────────────────────────────
  QList<AllergyInputDTO> newAllergies; // optional  (rỗng = không ghi gì)
};

struct MedicalRecordResultDTO {
  // ── medical_records ──────────────────────────────────────────────────────
  int recordId;                       // record_id      NOT NULL  PK
  int patientId;                      // patient_id     NOT NULL
  int doctorId;                       // doctor_id      NOT NULL
  QString doctorName;                 // JOIN từ staff
  QString doctorCode;                 // JOIN từ staff
  int appointmentId;                  // appointment_id NOT NULL
  QDateTime visitDateTime;            // visit_datetime NOT NULL
  VitalSigns vitals;                  // tất cả nullable trong DB
  QString chiefComplaint;             // chief_complaint nullable
  QString clinicalNotes;              // clinical_notes  nullable
  QString treatment;                  // treatment       nullable
  std::optional<QDate> nextVisitDate; // next_visit_date nullable
  QList<Diagnosis> diagnoses;         // từ bảng diagnoses (JOIN)
};

// struct MedicalRecordUpdateDTO {
//   // ── medical_records ──────────────────────────────────────────────────────
//   int recordId;            // record_id      NOT NULL  PK  (bắt buộc để UPDATE)
//   int doctorId;            // doctor_id      NOT NULL  FK → staff
//   int appointmentId;       // appointment_id NOT NULL  FK → appointments
//   QDateTime visitDateTime; // visit_datetime NOT NULL
//   VitalSigns vitals;       // tất cả nullable trong DB
//   QString chiefComplaint;  // chief_complaint nullable  (Service: require)
//   QString clinicalNotes;   // clinical_notes  nullable  (Service: require)
//   QString treatment;       // treatment       nullable  (Service: require)
//   std::optional<QDate> nextVisitDate; // next_visit_date nullable
//   QList<Diagnosis> diagnoses;         // require ≥1 item  (Service validate)
// };

// ═══════════════════════════════════════════════════════════════════════════
// SEARCH DTOs
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Tiêu chí tìm kiếm hồ sơ khám.
 *        Tất cả trường để rỗng / -1 / nullopt = không lọc theo trường đó.
 */
#include "dto/Pagination.h"

struct MedicalRecordSearchCriteria {
  // TEXT search: LIKE trên chief_complaint, clinical_notes
  QString searchKey;

  // Bộ lọc
  int patientId = -1;          // -1 = tất cả bệnh nhân
  int doctorId = -1;           // -1 = tất cả bác sĩ
  bool includeDeleted = false; // mặc định bỏ qua bản ghi đã xoá

  // Khoảng ngày khám
  std::optional<QDate> fromDate;
  std::optional<QDate> toDate;

  // Phân trang (1-indexed)
  int page = 1;       ///< Trang hiện tại (1-indexed)
  int pageSize = 20;  ///< Số bản ghi / trang. 0 = không phân trang (trả về tất cả)
};


/**
 * @brief Kết quả gọn khi tìm kiếm hồ sơ khám.
 *        Không load danh sách diagnoses đầy đủ — chỉ dùng để hiển thị list.
 */
struct MedicalRecordSummaryDTO {
  int recordId;
  int patientId;
  int doctorId;
  QDateTime visitDateTime;
  QString chiefComplaint;
  bool isDeleted;
};