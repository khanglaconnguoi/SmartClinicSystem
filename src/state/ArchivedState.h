/**
 * @file    ArchivedState.h
 * @brief   Concrete state: bệnh nhân đã được lưu trữ.
 */
#pragma once

#include "IPatientState.h"

/**
 * @brief Trạng thái "Đã lưu trữ" — trạng thái cuối cùng.
 *
 * Không cho phép chuyển sang bất kỳ trạng thái nào.
 */
class ArchivedState : public IPatientState {
public:
    QString name() const override;
    PatientStateType type() const override;
    std::unique_ptr<IPatientState> nextState() const override;
    bool canTransitionTo(PatientStateType target) const override;
};
