/**
 * @file    PatientService.cpp
 * @brief   Implementation cho PatientService — business logic.
 */
#include "PatientService.h"
#include "repository/PatientRepository.h"

#include <QDebug>

PatientService::PatientService(PatientRepository* repo)
    : m_repo(repo) {
}

bool PatientService::addPatient(Patient& patient) {
    if (!patient.isValid()) {
        qWarning() << "Cannot add patient: invalid data"
                   << "(fullName or phoneNumber is empty)";
        return false;
    }

    return m_repo->insert(patient);
}

bool PatientService::updatePatient(const Patient& patient) {
    if (!patient.isValid()) {
        qWarning() << "Cannot update patient: invalid data"
                   << "(fullName or phoneNumber is empty)";
        return false;
    }

    if (patient.id() < 0) {
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

std::optional<Patient> PatientService::getPatient(int patientId) {
    return m_repo->findById(patientId);
}

QList<Patient> PatientService::getAllPatients() {
    return m_repo->findAllActive();
}

QList<Patient> PatientService::searchPatients(const QString& keyword) {
    if (keyword.trimmed().isEmpty()) {
        return getAllPatients();
    }
    return m_repo->searchByName(keyword);
}
