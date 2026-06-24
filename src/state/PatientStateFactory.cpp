/**
 * @file    PatientStateFactory.cpp
 * @brief   Implementation cho factory function createPatientState().
 */
#include "PatientStateFactory.h"

#include "RegisteredState.h"
#include "WaitingForExaminationState.h"
#include "UnderExaminationState.h"
#include "WaitingForPaymentState.h"
#include "CompletedState.h"
#include "ArchivedState.h"

#include <QDebug>

std::unique_ptr<IPatientState> createPatientState(PatientStateType type) {
    switch (type) {
        case PatientStateType::Registered:
            return std::make_unique<RegisteredState>();
        case PatientStateType::WaitingForExamination:
            return std::make_unique<WaitingForExaminationState>();
        case PatientStateType::UnderExamination:
            return std::make_unique<UnderExaminationState>();
        case PatientStateType::WaitingForPayment:
            return std::make_unique<WaitingForPaymentState>();
        case PatientStateType::Completed:
            return std::make_unique<CompletedState>();
        case PatientStateType::Archived:
            return std::make_unique<ArchivedState>();
        default:
            qWarning() << "Unknown PatientStateType:"
                       << static_cast<int>(type)
                       << "— defaulting to Registered";
            return std::make_unique<RegisteredState>();
    }
}
