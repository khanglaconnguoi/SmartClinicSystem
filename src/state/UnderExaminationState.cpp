/**
 * @file    UnderExaminationState.cpp
 * @brief   Implementation cho UnderExaminationState.
 */
#include "UnderExaminationState.h"
#include "WaitingForPaymentState.h"

QString UnderExaminationState::name() const {
    return QStringLiteral("Đang khám");
}

PatientStateType UnderExaminationState::type() const {
    return PatientStateType::UnderExamination;
}

std::unique_ptr<IPatientState> UnderExaminationState::nextState() const {
    return std::make_unique<WaitingForPaymentState>();
}

bool UnderExaminationState::canTransitionTo(PatientStateType target) const {
    return target == PatientStateType::WaitingForPayment;
}
