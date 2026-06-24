/**
 * @file    RegisteredState.h
 * @brief   Concrete state: bệnh nhân vừa đăng ký.
 */
#pragma once

#include "IPatientState.h"

/**
 * @brief Trạng thái "Đã đăng ký" — trạng thái khởi tạo mặc định.
 *
 * Chỉ cho phép chuyển sang WaitingForExamination.
 */
class RegisteredState : public IPatientState {
public:
    QString name() const override;
    PatientStateType type() const override;
    std::unique_ptr<IPatientState> nextState() const override;
    bool canTransitionTo(PatientStateType target) const override;
};
