/**
 * @file    PatientRepository.h
 * @brief   Repository chỉ chứa các hàm INSERT bệnh nhân.
 *
 *  Luồng ghi dữ liệu:
 *    Service  →  (InsertDTO)  →  PatientRepository  →  DB
 *
 *  Mỗi hàm insert chạy trong một transaction:
 *    1. insertBasePatient()  → INSERT INTO patients
 *    2. INSERT INTO out_patients / in_patients / emergency_patients_admissions
 */

#pragma once

#include "dto/PatientDTOs.h"
#include <QString>

class PatientRepository {
private:
  /**
   * @brief Ghi bản ghi vào bảng `patients`.
   *        Được gọi bên trong mỗi insertXxxPatient() TRƯỚC khi ghi bảng con.
   * @param[in]  dto       Dữ liệu bệnh nhân cơ bản (PatientInsertDTO hoặc dẫn
   * xuất).
   * @param[out] patientId Nhận về ROWID vừa được sinh bởi SQLite
   * (last_insert_rowid).
   * @return true nếu INSERT thành công.
   */
  bool insertBasePatient(const PatientInsertDTO &dto, int &patientId);

public:
  /**
   * @brief Đăng ký bệnh nhân ngoại trú.
   *        Ghi vào `patients` + `out_patients` trong cùng 1 transaction.
   * @return true nếu cả hai INSERT thành công và transaction được commit.
   */
  bool insertOutPatient(const OutPatientInsertDTO &dto);

  /**
   * @brief Nhập viện bệnh nhân nội trú.
   *        Ghi vào `patients` + `in_patients` trong cùng 1 transaction.
   * @return true nếu cả hai INSERT thành công và transaction được commit.
   */
  bool insertInPatient(const InPatientInsertDTO &dto);

  /**
   * @brief Tiếp nhận bệnh nhân cấp cứu.
   *        Ghi vào `patients` + `emergency_patients_admissions` trong cùng 1
   * transaction.
   * @return true nếu cả hai INSERT thành công và transaction được commit.
   */
  bool insertEmergencyPatient(const EmergencyPatientInsertDTO &dto);

  /**
   * @brief Cập nhật thông tin cơ bản của bệnh nhân.
   *        Chỉ ghi đè dữ liệu vào bảng `patients`.
   * @return true nếu UPDATE thành công.
   */
  bool updatePatient(const PatientUpdateDTO &dto);

  /**
   * @brief Cập nhật thông tin của bệnh nhân ngoại trú.
   *        Chỉ ghi đè dữ liệu vào bảng `out_patients`.
   * @return true nếu UPDATE thành công.
   */
  bool updateOutPatient(const OutPatientUpdateDTO &dto);

  /**
   * @brief Cập nhật thông tin nhập viện của bệnh nhân nội trú.
   *        Chỉ ghi đè dữ liệu vào bảng `in_patients`.
   * @return true nếu UPDATE thành công.
   */
  bool updateInPatient(const InPatientUpdateDTO &dto);

  /**
   * @brief Cập nhật thông tin tiếp nhận của bệnh nhân cấp cứu.
   *        Chỉ ghi đè dữ liệu vào bảng `emergency_patients`.
   * @return true nếu UPDATE thành công.
   */
  bool updateEmergencyPatient(const EmergencyPatientUpdateDTO &dto);
  /**
   * @brief Lấy thông tin chi tiết một bệnh nhân.
   * @param patientId ID bệnh nhân cần lấy
   * @return std::optional<PatientDetailDTO> chứa dữ liệu nếu tìm thấy, std::nullopt nếu không.
   */
  std::optional<PatientDetailDTO> getPatientById(int patientId);

  /**
   * @brief Tìm kiếm bệnh nhân theo tiêu chí, gộp cả 3 loại
   *        (OutPatient/InPatient/Emergency) bằng UNION ALL.
   *
   *  Nếu criteria.type được set, chỉ nhánh UNION tương ứng được build
   *  (tránh quét cả 3 bảng khi không cần thiết).
   *
   * @return Danh sách kết quả đã được làm phẳng thành PatientSearchResultDTO,
   *         sắp xếp theo full_name, áp dụng limit/offset để phân trang.
   */
  QVector<PatientSearchResultDTO>
  searchPatients(const PatientSearchCriteria &criteria);

  /**
   * @brief Đếm tổng số kết quả khớp tiêu chí (không áp dụng limit/offset).
   *        Dùng để tính số trang cho UI phân trang.
   */
  int countSearchResults(const PatientSearchCriteria &criteria);

  QString buildSearchWhereClause(const PatientSearchCriteria &criteria,
                                 bool hasRoomColumn,
                                 QVariantList &outParams) const;

  /**
   * @brief Soft delete bệnh nhân.
   *        .
   * @return true nếu xoá mềm thành công.
   */
  bool softDeletePatient(int patientId);
  /**
   * @brief Khôi phục bệnh nhân.
   *        .
   * @return true nếu khôi phục thành công.
   */
  bool restorePatient(int patientId);

  /**
   * @brief Kiểm tra xem bệnh nhân đã bị xoá mềm chưa.
   *        .
   * @return true nếu bệnh nhân đã bị xoá mềm.
   */
  bool isPatientSoftDeleted(int patientId);
};
