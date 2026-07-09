#include "MedicalRecordService.h"
#include "repository/MedicalRecordRepository.h"
#include "Validation.h"
#include <QDebug>

MedicalRecordService::MedicalRecordService(
    std::shared_ptr<MedicalRecordRepository> recordRepo,
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<PharmacyService> pharmacyService)
    : m_recordRepository(recordRepo),
      m_patientService(patientService),
      m_pharmacyService(pharmacyService) {}

int MedicalRecordService::createMedicalRecord(const MedicalRecordInsertDTO &dto) {
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

bool MedicalRecordService::updateMedicalRecord(const MedicalRecordUpdateDTO &dto) {
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
