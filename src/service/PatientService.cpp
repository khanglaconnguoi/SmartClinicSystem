/**
 * @file    PatientService.cpp
 * @brief   Implementation cho PatientService — business logic.
 */
#include "PatientService.h"
#include "repository/PatientRepository.h"

#include <QDebug>

PatientService::PatientService(std::shared_ptr<PatientRepository> repo)
    : m_repo(std::move(repo)) {}

bool PatientService::addPatient(std::shared_ptr<Patient> patient) {
  if (!patient || !patient->isValid()) {
    qWarning() << "Cannot add patient: invalid data"
               << "(fullName or phoneNumber is empty)";
    return false;
  }

  return m_repo->insert(patient);
}

bool PatientService::updatePatient(std::shared_ptr<Patient> patient) {
  if (!patient || !patient->isValid()) {
    qWarning() << "Cannot update patient: invalid data"
               << "(fullName or phoneNumber is empty)";
    return false;
  }

  if (patient->id() < 0) {
    qWarning() << "Cannot update patient: invalid ID";
    return false;
  }

  return m_repo->update(patient);
}

bool PatientService::deletePatient(int patientId) {
  if (patientId < 0) {
    qWarning() << "Cannot delete patient: invalid ID";
    return false;
  }

  return m_repo->softDelete(patientId);
}

std::shared_ptr<Patient> PatientService::getPatient(int patientId) {
  return m_repo->findById(patientId);
}

std::vector<std::shared_ptr<Patient>> PatientService::getAllPatients() const {
  auto list = m_repo->findAllActive();
  return std::vector<std::shared_ptr<Patient>>(list.begin(), list.end());
}

std::vector<std::shared_ptr<Patient>>
PatientService::searchPatients(const QString &keyword) {
  if (keyword.trimmed().isEmpty()) {
    return getAllPatients();
  }
  auto list = m_repo->searchByName(keyword);
  return std::vector<std::shared_ptr<Patient>>(list.begin(), list.end());
}

std::vector<std::shared_ptr<Patient>>
PatientService::searchPatients(const PatientSearchCriteria &criteria) const {
  return m_repo->searchPatients(criteria);
}

bool PatientService::checkAllergyWarning(
    int patientId, const std::vector<QString> &medications,
    QString &outWarningMessage) const {
  auto patient = m_repo->findById(patientId);
  if (!patient) {
      outWarningMessage = "Không tìm thấy bệnh nhân để kiểm tra dị ứng.";
      return true; // Return true as a generic warning if patient not found during drug prescribing
  }
  
  QStringList warnings;

  for (const QString &med : medications) {
    if (patient->hasAllergy(med)) {
      warnings.append(
          QString("- Bệnh nhân có tiền sử dị ứng với thuốc: %1").arg(med));
    }
  }

  if (!warnings.isEmpty()) {
    outWarningMessage =
        QString("CẢNH BÁO TƯƠNG TÁC/DỊ ỨNG THUỐC:\n") + warnings.join("\n");
    return true;
  }

  outWarningMessage =
      "Bệnh nhân không có tiền sử dị ứng với các loại thuốc này.";
  return false;
}

bool PatientService::addMedicalRecord(const MedicalRecord& record) {
  return m_repo->addMedicalRecord(record);
}

std::vector<MedicalRecord> PatientService::getMedicalRecords(int patientId) const {
  return m_repo->getRecordsByPatientId(patientId);
}

bool PatientService::advancePatientState(int patientId) {
  if (patientId < 0) {
    qWarning() << "Cannot advance state: invalid patient ID";
    return false;
  }

  auto patient = m_repo->findById(patientId);
  if (!patient) {
    qWarning() << "Cannot advance state: patient not found (ID:" << patientId
               << ")";
    return false;
  }

  if (!patient->advanceState()) {
    qWarning() << "Cannot advance state for patient (ID:" << patientId
               << "): already at final state";
    return false;
  }

  qDebug() << "Patient (ID:" << patientId
           << ") state advanced to:" << patient->stateName();

  return m_repo->update(patient);
}
