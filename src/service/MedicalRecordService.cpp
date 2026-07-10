#include "MedicalRecordService.h"
#include "repository/MedicalRecordRepository.h"
#include "utils/StringSanitize.h"
#include <QDebug>

MedicalRecordService::MedicalRecordService(
    std::shared_ptr<MedicalRecordRepository> recordRepo,
    std::shared_ptr<PatientService> patientService)
    : m_recordRepository(recordRepo),
      m_patientService(patientService) {}

QString MedicalRecordService::validateVitalSigns(const VitalSigns &vitals) {
  if (vitals.temperature < 30.0 || vitals.temperature > 45.0)
    return "Nhiệt độ cơ thể phải nằm trong khoảng 30.0 đến 45.0 °C.";
  if (vitals.heartRate < 30 || vitals.heartRate > 250)
    return "Nhịp tim phải nằm trong khoảng 30 đến 250 bpm.";
  if (vitals.weight <= 0 || vitals.weight > 500)
    return "Cân nặng phải lớn hơn 0 và nhỏ hơn hoặc bằng 500 kg.";
  if (vitals.height <= 0 || vitals.height > 250)
    return "Chiều cao phải lớn hơn 0 và nhỏ hơn hoặc bằng 250 cm.";
  return "";
}

QString MedicalRecordService::validateChiefComplaint(const QString &complaint) {
  if (complaint.trimmed().isEmpty())
    return "Lý do khám không được để trống.";
  return "";
}

QString MedicalRecordService::validateDiagnosisList(const QList<Diagnosis> &diagnoses) {
  if (diagnoses.isEmpty())
    return "Phải có ít nhất một chẩn đoán.";
  for (const Diagnosis &d : diagnoses) {
    if (d.description.trimmed().isEmpty())
      return "Mô tả chẩn đoán không được để trống.";
    QString severity = d.severity.toUpper();
    if (severity != "MILD" && severity != "MODERATE" && severity != "SEVERE")
      return "Mức độ chẩn đoán không hợp lệ.";
  }
  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// Normalize
// ─────────────────────────────────────────────────────────────────────────────

void MedicalRecordService::normalizeMedicalRecordInput(MedicalRecordInsertDTO &dto) {
  dto.chiefComplaint = dto.chiefComplaint.trimmed();
  dto.clinicalNotes  = dto.clinicalNotes.trimmed();
  dto.treatment      = dto.treatment.trimmed();

  for (Diagnosis &d : dto.diagnoses) {
    d.description = StringSanitize::collapseSpaces(d.description);
    d.severity    = d.severity.trimmed().toUpper();
    d.icdCode     = d.icdCode.trimmed().toUpper();
  }
}

void MedicalRecordService::normalizeMedicalRecordUpdate(MedicalRecordUpdateDTO &dto) {
  dto.chiefComplaint = dto.chiefComplaint.trimmed();
  dto.clinicalNotes  = dto.clinicalNotes.trimmed();
  dto.treatment      = dto.treatment.trimmed();

  for (Diagnosis &d : dto.diagnoses) {
    d.description = StringSanitize::collapseSpaces(d.description);
    d.severity    = d.severity.trimmed().toUpper();
    d.icdCode     = d.icdCode.trimmed().toUpper();
  }
}

int MedicalRecordService::createMedicalRecord(MedicalRecordInsertDTO &dto) {
    normalizeMedicalRecordInput(dto);

    QString errVitals = validateVitalSigns(dto.vitals);
    if (!errVitals.isEmpty()) {
        qDebug() << "Validation failed (vitals):" << errVitals;
        return -1;
    }

    QString errChief = validateChiefComplaint(dto.chiefComplaint);
    if (!errChief.isEmpty()) {
        qDebug() << "Validation failed (chief complaint):" << errChief;
        return -1;
    }

    QString errDiag = validateDiagnosisList(dto.diagnoses);
    if (!errDiag.isEmpty()) {
        qDebug() << "Validation failed (diagnoses):" << errDiag;
        return -1;
    }

    return m_recordRepository->insertMedicalRecord(dto);
}

bool MedicalRecordService::updateMedicalRecord(MedicalRecordUpdateDTO &dto) {
    normalizeMedicalRecordUpdate(dto);

    QString errVitals = validateVitalSigns(dto.vitals);
    if (!errVitals.isEmpty()) {
        qDebug() << "Validation failed (vitals):" << errVitals;
        return false;
    }

    QString errChief = validateChiefComplaint(dto.chiefComplaint);
    if (!errChief.isEmpty()) {
        qDebug() << "Validation failed (chief complaint):" << errChief;
        return false;
    }

    QString errDiag = validateDiagnosisList(dto.diagnoses);
    if (!errDiag.isEmpty()) {
        qDebug() << "Validation failed (diagnoses):" << errDiag;
        return false;
    }

    return m_recordRepository->updateMedicalRecord(dto);
}

bool MedicalRecordService::softDeleteMedicalRecord(int recordId) {
    return m_recordRepository->softDeleteMedicalRecord(recordId);
}

QList<MedicalRecordResultDTO> MedicalRecordService::getMedicalHistory(int patientId) {
    return m_recordRepository->getHistoryByPatientId(patientId);
}

QList<MedicalRecordSummaryDTO> MedicalRecordService::searchMedicalRecords(
    const MedicalRecordSearchCriteria &criteria) {
  return m_recordRepository->searchMedicalRecords(criteria);
}

int MedicalRecordService::countSearchResults(
    const MedicalRecordSearchCriteria &criteria) {
  return m_recordRepository->countSearchResults(criteria);
}

