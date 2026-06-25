/**
 * @file    WaitingForExaminationState.cpp
 * @brief   Implementation cho WaitingForExaminationState.
 */
#include "WaitingForExaminationState.h"
#include "UnderExaminationState.h"

QString WaitingForExaminationState::name() const {
  return QStringLiteral("Chờ khám");
}

PatientStateType WaitingForExaminationState::type() const {
  return PatientStateType::WaitingForExamination;
}

std::unique_ptr<IPatientState> WaitingForExaminationState::nextState() const {
  return std::make_unique<UnderExaminationState>();
}

bool WaitingForExaminationState::canTransitionTo(
    PatientStateType target) const {
  return target == PatientStateType::UnderExamination;
}
