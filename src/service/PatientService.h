/**
 * @file    PatientService.h
 * @brief   Business logic layer cho quản lý bệnh nhân.
 */
#pragma once

#include "model/Patient.h"
#include "model/MedicalRecord.h"
#include <QList>
#include <QString>
#include <memory>
#include <optional>

#include "repository/PatientRepository.h"


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
   * @param repo Con trỏ thông minh đến PatientRepository.
   */
  explicit PatientService(std::shared_ptr<PatientRepository> repo);

  /**
   * @brief Thêm bệnh nhân mới sau khi validate dữ liệu.
   * @param patient Đối tượng Patient (ID sẽ được gán sau khi thêm).
   * @return true nếu thêm thành công.
   */
  bool addPatient(std::shared_ptr<Patient> patient);

  /**
   * @brief Cập nhật thông tin bệnh nhân.
   * @param patient Đối tượng Patient đã cập nhật.
   * @return true nếu cập nhật thành công.
   */
  bool updatePatient(std::shared_ptr<Patient> patient);

  /**
   * @brief Xóa mềm bệnh nhân theo ID.
   * @param patientId ID bệnh nhân cần xóa.
   * @return true nếu xóa thành công.
   */
  bool deletePatient(int patientId);

  /**
   * @brief Lấy thông tin bệnh nhân theo ID.
   * @param patientId ID cần tìm.
   * @return Patient nếu tìm thấy, nullptr nếu không.
   */
  std::shared_ptr<Patient> getPatient(int patientId);

  /**
   * @brief Lấy tất cả bệnh nhân đang active.
   * @return Danh sách Patient.
   */
  std::vector<std::shared_ptr<Patient>> getAllPatients() const;

  /**
   * @brief Tìm kiếm bệnh nhân theo tiêu chí.
   */
  std::vector<std::shared_ptr<Patient>> searchPatients(const PatientSearchCriteria& criteria) const;

  /**
   * @brief Rule engine cơ bản: Kiểm tra tương tác/dị ứng thuốc.
   * @param patientId ID của bệnh nhân.
   * @param medications Danh sách tên các loại thuốc dự định kê đơn.
   * @param outWarningMessage Biến nhận thông báo cảnh báo nếu có dị ứng.
   * @return true nếu có dị ứng (cần cảnh báo), false nếu an toàn.
   */
  bool checkAllergyWarning(int patientId, const std::vector<QString>& medications, QString& outWarningMessage) const;

  /**
   * @brief Tìm kiếm bệnh nhân theo tên.
   * @param keyword Từ khóa tìm kiếm.
   * @return Danh sách Patient phù hợp.
   */
  std::vector<std::shared_ptr<Patient>> searchPatients(const QString &keyword);

  /**
   * @brief Thêm mới hồ sơ khám bệnh.
   */
  bool addMedicalRecord(const MedicalRecord& record);

  /**
   * @brief Lấy danh sách hồ sơ khám bệnh của bệnh nhân.
   */
  std::vector<MedicalRecord> getMedicalRecords(int patientId) const;

  /**
   * @brief Chuyển trạng thái bệnh nhân sang bước kế tiếp.
   * @param patientId ID bệnh nhân.
   * @return true nếu chuyển trạng thái và lưu DB thành công.
   */
  bool advancePatientState(int patientId);

private:
  std::shared_ptr<PatientRepository> m_repo;
};
