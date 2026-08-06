/**
 * @file    PatientRepository.h
 * @brief   Repository layer cho Patient module.
 *
 *  Luồng ghi dữ liệu:
 *    Service  →  (InsertDTO / UpdateDTO)  →  PatientRepository  →  DB
 *    DB       →  (ResultDTO)              →  PatientRepository  →  Service
 *
 *  Mỗi hàm insert/update chạy trong một transaction hoàn chỉnh.
 */

#pragma once

#include "dto/PatientDTOs.h"
#include "repository/DatabaseManager.h"
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
  // ─── Insert ──────────────────────────────────────────────────────────────

  /**
   * @brief Đăng ký bệnh nhân ngoại trú.
   *        Ghi vào `patients` + `out_patients` + dị ứng + bảo hiểm (nếu có)
   *        trong cùng 1 transaction.
   * @return true nếu tất cả INSERT thành công và transaction được commit.
   */
  bool insertOutPatient(const OutPatientInsertDTO &dto);

  /**
   * @brief Nhập viện bệnh nhân nội trú.
   *        Ghi vào `patients` + `in_patients` + dị ứng + bảo hiểm (nếu có)
   *        trong cùng 1 transaction.
   * @return true nếu tất cả INSERT thành công và transaction được commit.
   */
  bool insertInPatient(const InPatientInsertDTO &dto);

  /**
   * @brief Tiếp nhận bệnh nhân cấp cứu.
   *        Ghi vào `patients` + `emergency_patients` + dị ứng + bảo hiểm
   *        (nếu có) trong cùng 1 transaction.
   * @return true nếu tất cả INSERT thành công và transaction được commit.
   */
  bool insertEmergencyPatient(const EmergencyPatientInsertDTO &dto);

  // ─── Update ──────────────────────────────────────────────────────────────

  /**
   * @brief Cập nhật thông tin cơ bản của bệnh nhân (bảng `patients`).
   * @return true nếu UPDATE thành công.
   */
  bool updatePatient(const PatientUpdateDTO &dto);

  std::optional<PatientShortDTO> getPatientByPhoneOrCitizenId(const QString &phone, const QString &citizenId) const;
  bool existsByCitizenId(const QString &citizenId, int excludePatientId = -1) const;
  bool existsByPhoneNumber(const QString &phone, int excludePatientId = -1) const;

  /**
   * @brief Cập nhật thông tin bệnh nhân ngoại trú (bảng `out_patients`).
   * @return true nếu UPDATE thành công.
   */
  bool updateOutPatient(const OutPatientUpdateDTO &dto);

  /**
   * @brief Cập nhật thông tin nhập viện bệnh nhân nội trú (bảng `in_patients`).
   * @return true nếu UPDATE thành công.
   */
  bool updateInPatient(const InPatientUpdateDTO &dto);

  /**
   * @brief Cập nhật thông tin tiếp nhận bệnh nhân cấp cứu
   *        (bảng `emergency_patients`).
   * @return true nếu UPDATE thành công.
   */
  bool updateEmergencyPatient(const EmergencyPatientUpdateDTO &dto);

  // ─── Read ────────────────────────────────────────────────────────────────

  /**
   * @brief Lấy thông tin chi tiết một bệnh nhân (flatten tất cả bảng con).
   * @param patientId ID bệnh nhân cần lấy.
   * @return std::optional<PatientDetailDTO> chứa dữ liệu nếu tìm thấy,
   *         std::nullopt nếu không tồn tại.
   */
  std::optional<PatientDetailDTO> getPatientById(int patientId);

  /**
   * @brief Tìm kiếm bệnh nhân theo tiêu chí, gộp cả 3 loại bằng UNION ALL.
   *        Nếu criteria.type được set, chỉ nhánh tương ứng được build.
   * @return Danh sách kết quả đã làm phẳng, sắp xếp theo full_name,
   *         áp dụng limit/offset để phân trang.
   */
  // QList<PatientSearchResultDTO> searchPatients(const PatientSearchCriteria &criteria);
  // int countSearchResults(const PatientSearchCriteria &criteria);

  PagedResult<PatientSearchResultDTO> searchPatientsPaged(const PatientSearchCriteria& criteria) const;


  /**
   * @brief Build mệnh đề WHERE dùng chung cho searchPatients / countSearchResults.
   * @param hasRoomColumn true nếu bảng con có cột room_id (in_patients /
   *        emergency_patients), false với out_patients.
   * @param[out] outParams Danh sách bind values được nối thêm vào.
   */
  QString buildSearchWhereClause(const PatientSearchCriteria &criteria,
                                 bool hasRoomColumn,
                                 QVariantList &outParams) const;

  // ─── Soft delete / Restore ────────────────────────────────────────────────

  /**
   * @brief Đánh dấu bệnh nhân là đã xoá (is_deleted = 1).
   * @return true nếu UPDATE thành công.
   */
  bool softDeletePatient(int patientId);

  /**
   * @brief Khôi phục bệnh nhân đã bị xoá mềm (is_deleted = 0).
   * @return true nếu UPDATE thành công.
   */
  bool restorePatient(int patientId);

  /**
   * @brief Kiểm tra xem bệnh nhân đã bị xoá mềm chưa.
   * @return true nếu is_deleted = 1.
   */
  bool isPatientSoftDeleted(int patientId);

  // ─── Allergies ────────────────────────────────────────────────────────────

  /**
   * @brief Ghi nhiều dị ứng vào `patient_allergies`.
   *        Dùng INSERT OR IGNORE để không ghi trùng (patient_id, allergen_name).
   * @return true nếu tất cả INSERT thành công.
   */
  bool insertAllergies(int patientId, const QList<AllergyInputDTO> &items);

  /**
   * @brief Xóa toàn bộ dị ứng (is_active = 0) của một bệnh nhân.
   *        Dùng trước khi re-insert khi update.
   */
  bool deactivateAllergies(int patientId);

  /**
   * @brief Lấy danh sách dị ứng đang active của bệnh nhân.
   */
  QList<AllergyResultDTO> getAllergiesByPatientId(int patientId);

  // ─── Insurance ────────────────────────────────────────────────────────────

  /**
   * @brief Ghi bản ghi bảo hiểm mới vào `patient_insurance`.
   *        Dùng khi bệnh nhân chưa có bảo hiểm (lần đầu tạo).
   * @param patientId ID bệnh nhân chủ sở hữu bảo hiểm.
   * @return true nếu INSERT thành công.
   */
  bool insertInsurance(int patientId, const InsuranceInsertDTO &dto);

  /**
   * @brief Cập nhật bản ghi bảo hiểm hiện có của bệnh nhân.
   *        Dùng khi bệnh nhân đã có bảo hiểm và muốn thay đổi thông tin.
   * @param patientId ID bệnh nhân cần cập nhật bảo hiểm.
   * @return true nếu UPDATE thành công.
   */
  bool updateInsurance(int patientId, const InsuranceInsertDTO &dto);

  /**
   * @brief Lấy thông tin bảo hiểm của bệnh nhân.
   * @return nullopt nếu bệnh nhân chưa có bảo hiểm.
   */
  std::optional<InsuranceResultDTO> getInsuranceByPatientId(int patientId);

  /**
   * @brief Lấy tỷ lệ chi trả bảo hiểm của bệnh nhân.
   * @param patientId ID của bệnh nhân.
   * @return Tỷ lệ chi trả (0.0 đến 100.0). Nếu không tìm thấy hoặc bảo hiểm không hoạt động, trả về 0.0.
   */
  double getInsuranceCoveragePercent(int patientId) const;


  /**
   * @brief Lấy mã bệnh nhân mới nhất có tiền tố chỉ định.
   */
  std::optional<QString> getLatestPatientCode(const QString &prefixWithDate) const;
};
