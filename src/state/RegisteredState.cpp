/**
 * @file    RegisteredState.cpp
 * @brief   Implementation cho RegisteredState.
 */
#include "RegisteredState.h"
#include "WaitingForExaminationState.h"

QString RegisteredState::name() const {
    return QStringLiteral("Đã đăng ký");
}

PatientStateType RegisteredState::type() const {
    return PatientStateType::Registered;
}

std::unique_ptr<IPatientState> RegisteredState::nextState() const {
    return std::make_unique<WaitingForExaminationState>();
}

bool RegisteredState::canTransitionTo(PatientStateType target) const {
    return target == PatientStateType::WaitingForExamination;
}
