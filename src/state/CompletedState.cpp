/**
 * @file    CompletedState.cpp
 * @brief   Implementation cho CompletedState.
 */
#include "CompletedState.h"
#include "ArchivedState.h"

QString CompletedState::name() const {
    return QStringLiteral("Hoàn thành");
}

PatientStateType CompletedState::type() const {
    return PatientStateType::Completed;
}

std::unique_ptr<IPatientState> CompletedState::nextState() const {
    return std::make_unique<ArchivedState>();
}

bool CompletedState::canTransitionTo(PatientStateType target) const {
    return target == PatientStateType::Archived;
}
