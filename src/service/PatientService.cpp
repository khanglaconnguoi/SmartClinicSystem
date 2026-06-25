/**
 * @file    PatientService.cpp
 * @brief   Implementation cho PatientService — business logic.
 */
#include "PatientService.h"
#include "repository/PatientRepository.h"
#include "model/InPatient.h"
#include "model/EmergencyPatient.h"
#include <QDebug>

PatientService::PatientService(std::shared_ptr<PatientRepository> repo)
    : m_repo(std::move(repo)) {}

static PatientInsertDTO mapBasePatient(std::shared_ptr<Patient> patient) {
  PatientInsertDTO dto;
  dto.patientCode = patient->patientCode();
  dto.fullName = patient->fullName();
  dto.birthDate = patient->birthDate();
  dto.gender = patient->gender();
  dto.phoneNumber = patient->phoneNumber();
  dto.address = patient->address();
  dto.bloodType = patient->bloodType();
  dto.allergies = patient->allergies();
  dto.medicalHistory = patient->medicalHistory();
  dto.citizenId = patient->citizenId();
  dto.email = patient->email();
  dto.insurance = patient->insurance();
  dto.isActive = patient->isActive();
  dto.state = patient->stateType();
  dto.type = patient->getType();
  return dto;
}

bool PatientService::addPatient(std::shared_ptr<Patient> patient) {
  if (!patient || !patient->isValid()) {
    qWarning() << "Cannot add patient: invalid data";
    return false;
  }

  // Nếu chưa có mã bệnh nhân, ta có thể sinh mã. Nhưng ở đây mặc định model đã có.
  if (patient->patientCode().isEmpty()) {
      patient->setPatientCode(Patient::generatePatientCode());
  }

  bool success = false;
  PatientType type = patient->getType();
  if (type == PatientType::OutPatient) {
      OutPatientInsertDTO dto;
      static_cast<PatientInsertDTO&>(dto) = mapBasePatient(patient);
      success = m_repo->insertOutPatient(dto);
  } else if (type == PatientType::InPatient) {
      InPatientInsertDTO dto;
      static_cast<PatientInsertDTO&>(dto) = mapBasePatient(patient);
      auto inPatient = std::dynamic_pointer_cast<InPatient>(patient);
      if (inPatient) {
          dto.roomNo = inPatient->roomNo();
          dto.admitDate = inPatient->admitDate();
      }
      success = m_repo->insertInPatient(dto);
  } else if (type == PatientType::Emergency) {
      EmergencyPatientInsertDTO dto;
      static_cast<PatientInsertDTO&>(dto) = mapBasePatient(patient);
      auto emPatient = std::dynamic_pointer_cast<EmergencyPatient>(patient);
      if (emPatient) {
          dto.severity = emPatient->severity();
      }
      success = m_repo->insertEmergencyPatient(dto);
  }

  if (success) {
      auto savedOpt = m_repo->findByPatientCode(patient->patientCode());
      if (savedOpt) {
          patient->setId((*savedOpt)->id());
      }
      return true;
  }
  return false;
}

bool PatientService::updatePatient(std::shared_ptr<Patient> patient) {
  if (!patient || !patient->isValid()) {
    qWarning() << "Cannot update patient: invalid data";
    return false;
  }

  if (patient->id() < 0) {
    qWarning() << "Cannot update patient: invalid ID";
    return false;
  }

  PatientType type = patient->getType();
  if (type == PatientType::OutPatient) {
      OutPatientInsertDTO dto;
      static_cast<PatientInsertDTO&>(dto) = mapBasePatient(patient);
      return m_repo->updatePatient(dto, patient->id());
  } else if (type == PatientType::InPatient) {
      InPatientInsertDTO dto;
      static_cast<PatientInsertDTO&>(dto) = mapBasePatient(patient);
      auto inPatient = std::dynamic_pointer_cast<InPatient>(patient);
      if (inPatient) {
          dto.roomNo = inPatient->roomNo();
          dto.admitDate = inPatient->admitDate();
      }
      return m_repo->updateInPatient(dto, patient->id());
  } else if (type == PatientType::Emergency) {
      EmergencyPatientInsertDTO dto;
      static_cast<PatientInsertDTO&>(dto) = mapBasePatient(patient);
      auto emPatient = std::dynamic_pointer_cast<EmergencyPatient>(patient);
      if (emPatient) {
          dto.severity = emPatient->severity();
      }
      return m_repo->updateEmergencyPatient(dto, patient->id());
  }
  return false;
}

bool PatientService::deletePatient(int patientId) {
  if (patientId < 0) {
    qWarning() << "Cannot delete patient: invalid ID";
    return false;
  }

  return m_repo->deactivate(patientId);
}

std::shared_ptr<Patient> PatientService::getPatient(int patientId) {
  return m_repo->findById(patientId).value_or(nullptr);
}

std::vector<std::shared_ptr<Patient>> PatientService::getAllPatients() const {
  PatientSearchCriteria criteria;
  criteria.onlyActive = true;
  criteria.includeDeleted = false;
  auto list = m_repo->search(criteria);
  return std::vector<std::shared_ptr<Patient>>(list.begin(), list.end());
}

std::vector<std::shared_ptr<Patient>>
PatientService::searchPatients(const QString &keyword) {
  if (keyword.trimmed().isEmpty()) {
    return getAllPatients();
  }
  PatientSearchCriteria criteria;
  criteria.nameKeyword = keyword;
  criteria.onlyActive = true;
  auto list = m_repo->search(criteria);
  return std::vector<std::shared_ptr<Patient>>(list.begin(), list.end());
}

std::vector<std::shared_ptr<Patient>>
PatientService::searchPatients(const PatientSearchCriteria &criteria) const {
  auto list = m_repo->search(criteria);
  return std::vector<std::shared_ptr<Patient>>(list.begin(), list.end());
}

bool PatientService::checkAllergyWarning(
    int patientId, const std::vector<QString> &medications,
    QString &outWarningMessage) const {
  auto patientOpt = m_repo->findById(patientId);
  if (!patientOpt) {
      outWarningMessage = "Không tìm thấy bệnh nhân để kiểm tra dị ứng.";
      return true; // Return true as a generic warning if patient not found
  }
  auto patient = *patientOpt;
  
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
  MedicalRecordInsertDTO dto;
  dto.patientId = record.getPatientId();
  dto.doctorId = record.getDoctorId();
  dto.visitDateTime = record.getVisitDateTime();
  dto.vitals = record.getVitals();
  dto.chiefComplaint = record.getChiefComplaint();
  dto.clinicalNotes = record.getClinicalNotes();
  dto.treatment = record.getTreatment();
  dto.testResults = record.getTestResults();
  dto.nextVisitDate = record.getNextVisitDate();
  
  return m_repo->addMedicalRecord(dto);
}

std::vector<MedicalRecord> PatientService::getMedicalRecords(int patientId) const {
  auto list = m_repo->getRecordsByPatientId(patientId);
  return std::vector<MedicalRecord>(list.begin(), list.end());
}

bool PatientService::advancePatientState(int patientId) {
  if (patientId < 0) {
    qWarning() << "Cannot advance state: invalid patient ID";
    return false;
  }

  auto patientOpt = m_repo->findById(patientId);
  if (!patientOpt) {
    qWarning() << "Cannot advance state: patient not found (ID:" << patientId
               << ")";
    return false;
  }
  auto patient = *patientOpt;

  if (!patient->advanceState()) {
    qWarning() << "Cannot advance state for patient (ID:" << patientId
               << "): already at final state";
    return false;
  }

  qDebug() << "Patient (ID:" << patientId
           << ") state advanced to:" << patient->stateName();

  return updatePatient(patient);
}
