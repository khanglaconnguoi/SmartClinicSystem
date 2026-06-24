/**
 * @file    ArchivedState.cpp
 * @brief   Implementation cho ArchivedState.
 */
#include "ArchivedState.h"

QString ArchivedState::name() const {
    return QStringLiteral("Đã lưu trữ");
}

PatientStateType ArchivedState::type() const {
    return PatientStateType::Archived;
}

std::unique_ptr<IPatientState> ArchivedState::nextState() const {
    // Trạng thái cuối cùng — không có state tiếp theo
    return nullptr;
}

bool ArchivedState::canTransitionTo(PatientStateType target) const {
    Q_UNUSED(target);
    // Trạng thái cuối cùng — không cho phép chuyển đổi
    return false;
}
