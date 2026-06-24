/**
 * @file    WaitingForPaymentState.h
 * @brief   Concrete state: bệnh nhân chờ thanh toán.
 */
#pragma once

#include "IPatientState.h"

/**
 * @brief Trạng thái "Chờ thanh toán" — bệnh nhân đã khám xong.
 *
 * Chỉ cho phép chuyển sang Completed.
 */
class WaitingForPaymentState : public IPatientState {
public:
    QString name() const override;
    PatientStateType type() const override;
    std::unique_ptr<IPatientState> nextState() const override;
    bool canTransitionTo(PatientStateType target) const override;
};
