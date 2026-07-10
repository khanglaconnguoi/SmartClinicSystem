#pragma once

#include "model/MedicalRecord.h"
#include <QDateTime>
#include <QList>
#include <QString>
#include <optional>

struct MedicalRecordInsertDTO {
  int patientId;
  int doctorId;
  std::optional<int> appointmentId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QString treatment;
  std::optional<QDate> nextVisitDate;
  QList<Diagnosis> diagnoses;
};

struct MedicalRecordResultDTO {
  int recordId;
  int patientId;
  int doctorId;
  std::optional<int> appointmentId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QString treatment;
  std::optional<QDate> nextVisitDate;
  QList<Diagnosis> diagnoses;
};

struct MedicalRecordUpdateDTO {
  int recordId;
  int doctorId;
  std::optional<int> appointmentId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QString treatment;
  std::optional<QDate> nextVisitDate;
  QList<Diagnosis> diagnoses;
};

// ═══════════════════════════════════════════════════════════════════════════
// SEARCH DTOs
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Tiêu chí tìm kiếm hồ sơ khám.
 *        Tất cả trường để rỗng / -1 / nullopt = không lọc theo trường đó.
 */
struct MedicalRecordSearchCriteria {
  // TEXT search: LIKE trên chief_complaint, clinical_notes
  QString searchKey;

  // Bộ lọc
  int patientId = -1;              // -1 = tất cả bệnh nhân
  int doctorId  = -1;              // -1 = tất cả bác sĩ
  bool includeDeleted = false;     // mặc định bỏ qua bản ghi đã xoá

  // Khoảng ngày khám
  std::optional<QDate> fromDate;
  std::optional<QDate> toDate;

  // Phân trang
  int limit  = 50;
  int offset = 0;
};

/**
 * @brief Kết quả gọn khi tìm kiếm hồ sơ khám.
 *        Không load danh sách diagnoses đầy đủ — chỉ dùng để hiển thị list.
 */
struct MedicalRecordSummaryDTO {
  int       recordId;
  int       patientId;
  int       doctorId;
  QDateTime visitDateTime;
  QString   chiefComplaint;
  bool      isDeleted;
};