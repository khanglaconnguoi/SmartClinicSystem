/**
 * @file    PatientService.h
 * @brief   Service layer cho module Patient.
 *
 *  Các hàm validate là public static — UI có thể gọi trực tiếp để kiểm tra từng ô.
 *  Mỗi hàm validate trả về chuỗi rỗng "" nếu hợp lệ,
 *  hoặc chuỗi mô tả lỗi nếu không hợp lệ.
 */

#pragma once

#include "dto/PatientDTOs.h"
#include "model/CommonEnums.h"
#include <QDate>
#include <QString>
#include <memory>

class PatientRepository;

#include "repository/DatabaseManager.h"

class PatientService {
private:
  std::shared_ptr<PatientRepository> m_patientRepository;

  // ── (Các private member cũ đã chuyển xuống dưới public hoặc xoá)
public:
  explicit PatientService(std::shared_ptr<PatientRepository> patientRepository)
      : m_patientRepository(patientRepository) {}
  ~PatientService() {}

  // ── Validate nhóm trường theo loại bệnh nhân ────────────────────────────
  // ── Đã được đưa ra public để UI có thể gọi kiểm tra ──────────────────────

  /**
   * @brief Kiểm tra toàn bộ trường cơ bản của bảng `patients`.
   *        Dùng thông tin từ PatientInputDTO.
   */
  static QString validateBaseInput(const PatientInputDTO &dto, const QString &patientCode = "");

  /**
   * @brief Kiểm tra trường đặc thù của bệnh nhân nội trú (`in_patients`).
   */
  static QString validateInPatientInput(const InPatientInputDTO &dto);

  /**
   * @brief Kiểm tra trường đặc thù của bệnh nhân cấp cứu (`emergency_patients`).
   */
  static QString validateEmergencyPatientInput(const EmergencyPatientInputDTO &dto);

  /**
   * @brief Kiểm tra trường cơ bản khi UPDATE bảng `patients`.
   *        Bỏ qua các trường không thay đổi.
   */
  static QString validateUpdateBaseInput(const PatientInputDTO &dto, int patientId);

  /**
   * @brief Chuẩn hóa dữ liệu đầu vào từ UI:
   *        trim khoảng trắng, chuẩn hóa chữ hoa/thường, email về lowercase...
   *        LUÔN gọi trước validate — đảm bảo validate trên dữ liệu đã sạch.
   */
  static void normalizePatientInput(PatientInputDTO &dto);

  /**
   * @brief Chuẩn hóa tiêu chí tìm kiếm trước khi lọc (vd: cắt khoảng trắng từ khoá).
   */
  static void normalizeSearchCriteria(PatientSearchCriteria &criteria);

  static QString validateDateRange(const QDate &fromDate, const QDate &toDate);

  // ── Validate các trường đơn lẻ dành cho UI gọi trực tiếp ───────────────
  static QString validateBloodType(const QString &bloodType);

  static QString validateInPatientRoomId(std::optional<int> roomId);
  static QString validateInPatientDoctorId(std::optional<int> doctorId);
  static QString validateInPatientDischargeDate(const QDate &admissionDate, std::optional<QDate> dischargeDate);
  static QString validateInPatientReason(const QString &reason);

  static QString validateEmergencyRoomId(std::optional<int> roomId);
  static QString validateEmergencyDoctorId(std::optional<int> doctorId);
  static QString validateEmergencyDischargeDate(const QDate &admissionDate, std::optional<QDate> dischargeDate);
  static QString validateEmergencyInjuryCause(const QString &cause);
  static QString validateEmergencyInjuryDescription(const QString &desc);



  /**
   * @brief Tạo mã bệnh nhân theo loại (OUT-yyyyMMdd-NNNN / IN-… / EMER-…).
   */
  QString generatePatientCode(PatientType type);

  /**
   * @brief Đăng ký bệnh nhân ngoại trú.
   */
  bool addOutPatient(OutPatientInputDTO &dto);

  /**
   * @brief Nhập viện bệnh nhân nội trú.
   */
  bool addInPatient(InPatientInputDTO &dto);

  /**
   * @brief Tiếp nhận bệnh nhân cấp cứu.
   */
  bool addEmergencyPatient(EmergencyPatientInputDTO &dto);

  /**
   * @brief Cập nhật thông tin cơ bản của bệnh nhân trong bảng `patients`.
   */
  bool updatePatient(int patientId, PatientInputDTO &dto);

  /**
   * @brief Cập nhật thông tin bệnh nhân ngoại trú.
   *        Ghi đè cả `patients` lẫn `out_patients`.
   */
  bool updateOutPatient(int patientId, OutPatientInputDTO &dto, const QString &status = "REGISTERED");

  /**
   * @brief Cập nhật thông tin bệnh nhân nội trú.
   *        Ghi đè cả `patients` lẫn `in_patients`.
   */
  bool updateInPatient(int patientId, InPatientInputDTO &dto, const QString &status = "ADMITTED");

  /**
   * @brief Cập nhật thông tin bệnh nhân cấp cứu.
   *        Ghi đè cả `patients` lẫn `emergency_patients`.
   */
  bool updateEmergencyPatient(int patientId, EmergencyPatientInputDTO &dto, const QString &status = "EMERGENCY");

  /**
   * @brief Tìm kiếm bệnh nhân kết hợp tất cả các tiêu chí.
   *        Gọi xuống Validation để kiểm tra ngày tháng trước.
   * @param criteria Tiêu chí tìm kiếm (từ UI).
   * @return Danh sách kết quả (đã limit/offset). Rỗng nếu lỗi validation.
   */
  QList<PatientSearchResultDTO>
  searchPatients(PatientSearchCriteria criteria);

  /**
   * @brief Lấy thông tin chi tiết một bệnh nhân.
   * @param patientId ID bệnh nhân cần lấy
   * @return std::optional<PatientDetailDTO> chứa dữ liệu nếu tìm thấy, std::nullopt nếu không.
   */
  std::optional<PatientDetailDTO> getPatientById(int patientId);

  /**
   * @brief Đếm tổng số lượng kết quả thỏa tiêu chí tìm kiếm.
   * @param criteria Tiêu chí tìm kiếm (từ UI).
   * @return Tổng số lượng. 0 nếu có lỗi validation.
   */
  int countSearchResults(PatientSearchCriteria criteria);

  bool softDeletePatient(int patientId);
  bool restorePatient(int patientId);

  /**
   * @brief Kiểm tra bệnh nhân có dị ứng với loại thuốc chỉ định không.
   * @param patientId ID bệnh nhân cần kiểm tra.
   * @param drugName  Tên thuốc cần kiểm tra (so khớp với danh sách dị ứng).
   * @return true nếu có xung đột dị ứng, false nếu an toàn hoặc không có dữ liệu.
   */
  bool checkDrugAllergyConflict(int patientId, const QString &drugName) const;

  /**
   * @brief Lấy danh sách dị ứng đang active của bệnh nhân.
   */
  QList<AllergyResultDTO> getAllergies(int patientId);

  /**
   * @brief Lấy thông tin bảo hiểm của bệnh nhân.
   * @return nullopt  // Tìm kiếm thông tin bảo hiểm theo ID bệnh nhân
   */
  std::optional<InsuranceResultDTO> getInsurance(int patientId) const;

  std::optional<DatabaseManager::PatientRecord> getPatientByPhoneOrCitizenId(const QString &phone, const QString &citizenId) const;

};