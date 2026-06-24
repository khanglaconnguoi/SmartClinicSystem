/**
 * @file    WaitingForExaminationState.h
 * @brief   Concrete state: bệnh nhân đang chờ khám.
 */
#pragma once

#include "IPatientState.h"

/**
 * @brief Trạng thái "Chờ khám" — bệnh nhân đã xác nhận hồ sơ.
 *
 * Chỉ cho phép chuyển sang UnderExamination.
 */
class WaitingForExaminationState : public IPatientState {
public:
    QString name() const override;
    PatientStateType type() const override;
    std::unique_ptr<IPatientState> nextState() const override;
    bool canTransitionTo(PatientStateType target) const override;
};
