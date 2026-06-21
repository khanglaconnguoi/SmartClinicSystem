/**
 * @file    PatientService.h
 * @brief   Business logic layer cho quản lý bệnh nhân.
 */
#pragma once

#include <QList>
#include <QString>
#include <optional>
#include "model/Patient.h"

class PatientRepository;

/**
 * @brief Lớp chứa logic nghiệp vụ cho module Patient.
 *
 * Đóng vai trò trung gian giữa UI và Repository.
 * Thực hiện validation trước khi gọi Repository.
 * Không chứa SQL — delegate toàn bộ sang PatientRepository.
 */
class PatientService {
public:
    /**
     * @brief Khởi tạo service với repository.
     * @param repo Con trỏ đến PatientRepository (không sở hữu).
     */
    explicit PatientService(PatientRepository* repo);

    /**
     * @brief Thêm bệnh nhân mới sau khi validate dữ liệu.
     * @param patient Đối tượng Patient (ID sẽ được gán sau khi thêm).
     * @return true nếu thêm thành công.
     */
    bool addPatient(Patient& patient);

    /**
     * @brief Cập nhật thông tin bệnh nhân.
     * @param patient Đối tượng Patient đã cập nhật.
     * @return true nếu cập nhật thành công.
     */
    bool updatePatient(const Patient& patient);

    /**
     * @brief Xóa mềm bệnh nhân theo ID.
     * @param patientId ID bệnh nhân cần xóa.
     * @return true nếu xóa thành công.
     */
    bool deletePatient(int patientId);

    /**
     * @brief Lấy thông tin bệnh nhân theo ID.
     * @param patientId ID cần tìm.
     * @return Patient nếu tìm thấy, std::nullopt nếu không.
     */
    std::optional<Patient> getPatient(int patientId);

    /**
     * @brief Lấy tất cả bệnh nhân đang active.
     * @return Danh sách Patient.
     */
    QList<Patient> getAllPatients();

    /**
     * @brief Tìm kiếm bệnh nhân theo tên.
     * @param keyword Từ khóa tìm kiếm.
     * @return Danh sách Patient phù hợp.
     */
    QList<Patient> searchPatients(const QString& keyword);

private:
    PatientRepository* m_repo;
};
