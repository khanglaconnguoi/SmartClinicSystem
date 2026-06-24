/**
 * @file    IPatientState.h
 * @brief   Abstract base class (interface) cho State Pattern của Patient.
 *
 * Định nghĩa enum PatientStateType và interface IPatientState mà tất cả
 * concrete state phải implement. Mỗi state tự quyết định các chuyển đổi
 * hợp lệ, tuân thủ Open/Closed Principle.
 */
#pragma once

#include <QString>
#include <memory>

/**
 * @brief Enum đại diện cho các trạng thái trong quy trình khám bệnh.
 *
 * Giá trị số nguyên được lưu trực tiếp vào cột `state` trong database.
 */
enum class PatientStateType {
    Registered = 0,            ///< Đã đăng ký
    WaitingForExamination = 1, ///< Chờ khám
    UnderExamination = 2,      ///< Đang khám
    WaitingForPayment = 3,     ///< Chờ thanh toán
    Completed = 4,             ///< Hoàn thành
    Archived = 5               ///< Đã lưu trữ
};

/**
 * @brief Interface cho State Pattern — mỗi trạng thái bệnh nhân
 *        implement class này.
 *
 * Tuân thủ GoF State Pattern: context (Patient) giữ con trỏ đến
 * IPatientState, delegate hành vi chuyển trạng thái cho state object.
 */
class IPatientState {
public:
    virtual ~IPatientState() = default;

    /**
     * @brief Tên trạng thái để hiển thị trên UI.
     * @return Chuỗi tên tiếng Việt (vd: "Đang chờ khám").
     */
    virtual QString name() const = 0;

    /**
     * @brief Giá trị enum để lưu vào database.
     * @return PatientStateType tương ứng.
     */
    virtual PatientStateType type() const = 0;

    /**
     * @brief Tạo state object của trạng thái kế tiếp trong quy trình.
     * @return unique_ptr đến state mới, nullptr nếu đã là trạng thái cuối.
     */
    virtual std::unique_ptr<IPatientState> nextState() const = 0;

    /**
     * @brief Kiểm tra có thể chuyển sang trạng thái chỉ định không.
     * @param target Trạng thái đích cần kiểm tra.
     * @return true nếu chuyển đổi hợp lệ.
     */
    virtual bool canTransitionTo(PatientStateType target) const = 0;
};
