/**
 * @file    UnderExaminationState.h
 * @brief   Concrete state: bệnh nhân đang được khám.
 */
#pragma once

#include "IPatientState.h"

/**
 * @brief Trạng thái "Đang khám" — bác sĩ đang khám bệnh nhân.
 *
 * Chỉ cho phép chuyển sang WaitingForPayment.
 */
class UnderExaminationState : public IPatientState {
public:
    QString name() const override;
    PatientStateType type() const override;
    std::unique_ptr<IPatientState> nextState() const override;
    bool canTransitionTo(PatientStateType target) const override;
};
