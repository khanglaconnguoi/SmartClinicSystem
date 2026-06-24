/**
 * @file    CompletedState.h
 * @brief   Concrete state: bệnh nhân đã hoàn thành quy trình khám.
 */
#pragma once

#include "IPatientState.h"

/**
 * @brief Trạng thái "Hoàn thành" — bệnh nhân đã thanh toán xong.
 *
 * Chỉ cho phép chuyển sang Archived.
 */
class CompletedState : public IPatientState {
public:
    QString name() const override;
    PatientStateType type() const override;
    std::unique_ptr<IPatientState> nextState() const override;
    bool canTransitionTo(PatientStateType target) const override;
};
