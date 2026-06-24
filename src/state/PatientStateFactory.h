/**
 * @file    PatientStateFactory.h
 * @brief   Factory function tạo IPatientState từ enum PatientStateType.
 *
 * Dùng khi đọc giá trị state từ database (số nguyên) và cần khôi phục
 * lại đúng concrete state object tương ứng.
 */
#pragma once

#include "IPatientState.h"
#include <memory>

/**
 * @brief Tạo concrete IPatientState từ giá trị enum.
 * @param type Loại trạng thái (thường đọc từ database).
 * @return unique_ptr đến concrete state tương ứng.
 *         Trả về RegisteredState nếu giá trị không hợp lệ.
 */
std::unique_ptr<IPatientState> createPatientState(PatientStateType type);
