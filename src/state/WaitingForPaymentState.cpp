/**
 * @file    WaitingForPaymentState.cpp
 * @brief   Implementation cho WaitingForPaymentState.
 */
#include "WaitingForPaymentState.h"
#include "CompletedState.h"

QString WaitingForPaymentState::name() const {
  return QStringLiteral("Chờ thanh toán");
}

PatientStateType WaitingForPaymentState::type() const {
  return PatientStateType::WaitingForPayment;
}

std::unique_ptr<IPatientState> WaitingForPaymentState::nextState() const {
  return std::make_unique<CompletedState>();
}

bool WaitingForPaymentState::canTransitionTo(PatientStateType target) const {
  return target == PatientStateType::Completed;
}
