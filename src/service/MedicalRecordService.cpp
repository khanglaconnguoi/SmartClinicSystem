#include "MedicalRecordService.h"
#include <QDebug>
#include "Validation.h"
#include "model/MedicalRecord.h"
#include "repository/MedicalRecordRepository.h"
#include "service/PatientService.h"


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
  if (!(err = validateTemperature(vitals.temperature)).isEmpty())
    return err;
  if (!(err = validateHeartRate(vitals.heartRate)).isEmpty())
    return err;
  if (!(err = validateWeight(vitals.weight)).isEmpty())
    return err;
  if (!(err = validateHeight(vitals.height)).isEmpty())
    return err;
  return "";
}



QString
MedicalRecordService::validateDiagnosisSeverity(const QString &severity) {
  if (severity != "MILD" && severity != "MODERATE" && severity != "SEVERE")
    return "Mức độ chẩn đoán không hợp lệ.";
  return "";
}

QString
MedicalRecordService::validateDiagnosisList(const QList<Diagnosis> &diagnoses) {
  if (diagnoses.isEmpty())
    return "Phải có ít nhất một chẩn đoán.";
  for (const Diagnosis &d : diagnoses) {
    QString err;
    if (!(err = Validation::validateTrimmedNotEmpty(d.description, "Mô tả chẩn đoán không được để trống.")).isEmpty())
      return err;
    if (!(err = validateDiagnosisSeverity(d.severity)).isEmpty())
      return err;
  }
  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// Validate: Allergy list
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Kiểm tra từng dị ứng trong danh sách.
 *        Danh sách rỗng là hợp lệ (dị ứng là optional).
 *        Mỗi item phải có: allergenName không rỗng, severity
 * âm2MILD/MODERATE/SEVERE.
 */
QString MedicalRecordService::validateAllergyList(
    const QList<AllergyInputDTO> &allergies) {
  if (allergies.isEmpty())
    return ""; // optional — cho phép rỗng

  for (int i = 0; i < allergies.size(); ++i) {
    const AllergyInputDTO &a = allergies.at(i);
    if (a.allergenName.isEmpty())
      return QString("Dị ứng #%1: Tên chất gây dị ứng không được để trống.")
          .arg(i + 1);
    if (a.allergenName.length() > 200)
      return QString("Dị ứng #%1: Tên quá dài (tối đa 200 ký tự).").arg(i + 1);
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

  // Normalize allergy list
  for (AllergyInputDTO &a : dto.newAllergies) {
    a.allergenName = a.allergenName.trimmed();
    a.notes = a.notes.trimmed();
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

QString MedicalRecordService::createMedicalRecord(MedicalRecordInsertDTO &dto, int *outRecordId) {
  normalizeMedicalRecordInput(dto);

  QString err;

  if (!(err = validateVitalSigns(dto.vitals)).isEmpty()) {
    qDebug() << "Validation failed (vitals):" << err;
    return err;
  }

  if (!(err = Validation::validateTrimmedNotEmpty(dto.chiefComplaint, "Lý do khám không được để trống.")).isEmpty()) {
    qDebug() << "Validation failed (chief complaint):" << err;
    return err;
  }

  if (!(err = validateDiagnosisList(dto.diagnoses)).isEmpty()) {
    qDebug() << "Validation failed (diagnoses):" << err;
    return err;
  }

  if (!(err = validateAllergyList(dto.newAllergies)).isEmpty()) {
    qDebug() << "Validation failed (allergies):" << err;
    return err;
  }

  int recId = m_recordRepository->insertMedicalRecord(dto);
  if (recId <= 0) {
    return "Lỗi hệ thống khi lưu hồ sơ khám. Vui lòng thử lại.";
  }

  if (outRecordId) {
    *outRecordId = recId;
  }

  if (!dto.newAllergies.isEmpty()) {
    if (!(err = m_patientService->addAllergiesToPatient(dto.patientId, dto.newAllergies)).isEmpty()) {
      qDebug() << "Add allergies failed:" << err;
      return err;
    }
  }

  return "";
}

QString MedicalRecordService::updateMedicalRecord(MedicalRecordUpdateDTO &dto) {
  normalizeMedicalRecordUpdate(dto);

  QString err;

  if (!(err = validateVitalSigns(dto.vitals)).isEmpty()) {
    qDebug() << "Validation failed (vitals):" << err;
    return err;
  }

  if (!(err = Validation::validateTrimmedNotEmpty(dto.chiefComplaint, "Lý do khám không được để trống.")).isEmpty()) {
    qDebug() << "Validation failed (chief complaint):" << err;
    return err;
  }

  if (!(err = validateDiagnosisList(dto.diagnoses)).isEmpty()) {
    qDebug() << "Validation failed (diagnoses):" << err;
    return err;
  }

  if (!m_recordRepository->updateMedicalRecord(dto)) {
    return "Lỗi CSDL khi cập nhật hồ sơ bệnh án.";
  }
  return "";
}

bool MedicalRecordService::softDeleteMedicalRecord(int recordId) {
  return m_recordRepository->softDeleteMedicalRecord(recordId);
}

QList<MedicalRecordResultDTO>
MedicalRecordService::getMedicalHistory(int patientId) {
  return m_recordRepository->getHistoryByPatientId(patientId);
}

std::optional<MedicalRecordResultDTO>
MedicalRecordService::getMedicalRecordById(int recordId) const {
  return m_recordRepository->findById(recordId);
}

std::optional<MedicalRecordResultDTO>
MedicalRecordService::getLatestRecordByAppointmentId(int appointmentId) const {
  return m_recordRepository->findLatestByAppointmentId(appointmentId);
}

PagedResult<MedicalRecordSummaryDTO>
MedicalRecordService::searchMedicalRecordsPaged(MedicalRecordSearchCriteria criteria) const {
  normalizeSearchCriteria(criteria);
  criteria.searchKey = criteria.searchKey.simplified();
  criteria.page = qMax(1, criteria.page);
  criteria.pageSize = qBound(0, criteria.pageSize, 200);

  return m_recordRepository->searchMedicalRecordsPaged(criteria);
}

/*
QList<MedicalRecordSummaryDTO> MedicalRecordService::searchMedicalRecords(MedicalRecordSearchCriteria criteria) { ... }
int MedicalRecordService::countSearchResults(MedicalRecordSearchCriteria criteria) { ... }
*/

