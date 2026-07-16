#include "MedicalRecordService.h"
#include "model/MedicalRecord.h"
#include "repository/MedicalRecordRepository.h"
#include <QDebug>

MedicalRecordService::MedicalRecordService(
    std::shared_ptr<MedicalRecordRepository> recordRepo,
    std::shared_ptr<PatientService> patientService)
    : m_recordRepository(recordRepo), m_patientService(patientService) {}

QString MedicalRecordService::validateTemperature(double temperature) {
  if (temperature < 30.0 || temperature > 45.0)
    return "Nhiệt độ cơ thể phải nằm trong khoảng 30.0 đến 45.0 °C.";
  return "";
}

QString MedicalRecordService::validateHeartRate(int heartRate) {
  if (heartRate < 30 || heartRate > 250)
    return "Nhịp tim phải nằm trong khoảng 30 đến 250 bpm.";
  return "";
}

QString MedicalRecordService::validateWeight(double weight) {
  if (weight <= 0 || weight > 500)
    return "Cân nặng phải lớn hơn 0 và nhỏ hơn hoặc bằng 500 kg.";
  return "";
}

QString MedicalRecordService::validateHeight(double height) {
  if (height <= 0 || height > 250)
    return "Chiều cao phải lớn hơn 0 và nhỏ hơn hoặc bằng 250 cm.";
  return "";
}

QString MedicalRecordService::validateVitalSigns(const VitalSigns &vitals) {
  QString err;
  if (!(err = validateTemperature(vitals.temperature)).isEmpty()) return err;
  if (!(err = validateHeartRate(vitals.heartRate)).isEmpty()) return err;
  if (!(err = validateWeight(vitals.weight)).isEmpty()) return err;
  if (!(err = validateHeight(vitals.height)).isEmpty()) return err;
  return "";
}

QString MedicalRecordService::validateChiefComplaint(const QString &complaint) {
  if (complaint.trimmed().isEmpty())
    return "Lý do khám không được để trống.";
  return "";
}

QString MedicalRecordService::validateDiagnosisDescription(const QString &desc) {
  if (desc.trimmed().isEmpty())
    return "Mô tả chẩn đoán không được để trống.";
  return "";
}

QString MedicalRecordService::validateDiagnosisSeverity(const QString &severity) {
  QString upperSev = severity.toUpper();
  if (upperSev != "MILD" && upperSev != "MODERATE" && upperSev != "SEVERE")
    return "Mức độ chẩn đoán không hợp lệ.";
  return "";
}

QString
MedicalRecordService::validateDiagnosisList(const QList<Diagnosis> &diagnoses) {
  if (diagnoses.isEmpty())
    return "Phải có ít nhất một chẩn đoán.";
  for (const Diagnosis &d : diagnoses) {
    QString err;
    if (!(err = validateDiagnosisDescription(d.description)).isEmpty()) return err;
    if (!(err = validateDiagnosisSeverity(d.severity)).isEmpty()) return err;
  }
  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// Normalize
// ─────────────────────────────────────────────────────────────────────────────

void MedicalRecordService::normalizeMedicalRecordInput(
    MedicalRecordInsertDTO &dto) {
  dto.chiefComplaint = dto.chiefComplaint.trimmed();
  dto.clinicalNotes = dto.clinicalNotes.trimmed();
  dto.treatment = dto.treatment.trimmed();

  for (Diagnosis &d : dto.diagnoses) {
    d.description = d.description.simplified();
    d.severity = d.severity.trimmed().toUpper();
    d.icdCode = d.icdCode.trimmed().toUpper();
  }
}

void MedicalRecordService::normalizeMedicalRecordUpdate(
    MedicalRecordUpdateDTO &dto) {
  dto.chiefComplaint = dto.chiefComplaint.trimmed();
  dto.clinicalNotes = dto.clinicalNotes.trimmed();
  dto.treatment = dto.treatment.trimmed();

  for (Diagnosis &d : dto.diagnoses) {
    d.description = d.description.simplified();
    d.severity = d.severity.trimmed().toUpper();
    d.icdCode = d.icdCode.trimmed().toUpper();
  }
}

void MedicalRecordService::normalizeSearchCriteria(
    MedicalRecordSearchCriteria &criteria) {
  criteria.searchKey = criteria.searchKey.simplified();
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

QList<MedicalRecordResultDTO>
MedicalRecordService::getMedicalHistory(int patientId) {
  return m_recordRepository->getHistoryByPatientId(patientId);
}

QList<MedicalRecordSummaryDTO> MedicalRecordService::searchMedicalRecords(
    MedicalRecordSearchCriteria criteria) {
  normalizeSearchCriteria(criteria);
  return m_recordRepository->searchMedicalRecords(criteria);
}

int MedicalRecordService::countSearchResults(
    MedicalRecordSearchCriteria criteria) {
  normalizeSearchCriteria(criteria);
  return m_recordRepository->countSearchResults(criteria);
}
