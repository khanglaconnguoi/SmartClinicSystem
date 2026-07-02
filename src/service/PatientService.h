/**
 * @file    PatientService.h
 * @brief   Service layer cho module Patient.
 *
 *  Các hàm validate là private — chỉ dùng nội bộ trong class này.
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

class PatientService {
private:
  std::shared_ptr<PatientRepository> m_patientRepository;

  // ── Validate nhóm trường theo loại bệnh nhân ────────────────────────────

  /**
   * @brief Kiểm tra toàn bộ trường cơ bản của bảng `patients`.
   *        Bao gồm: patientId, patientCode, fullName, dateOfBirth, gender,
   *                 citizenId, phone, email, address, bloodType, allergies,
   *                 insurance, type, emergencyContactName,
   * emergencyContactPhone.
   */
  static QString
  validateBaseInput(int patientId, const QString &patientCode,
                    const QString &fullName, const QDate &dateOfBirth,
                    const QString &gender, const QString &citizenId,
                    const QString &phone, const QString &email,
                    const QString &address, const QString &bloodType,
                    const QString &allergies, const QString &insurance,
                    const QString &type, const QString &emergencyContactName,
                    const QString &emergencyContactPhone);

  /**
   * @brief Kiểm tra trường đặc thù của bệnh nhân nội trú (`in_patients`).
   *        Bao gồm: roomId, doctorId,
   *                 admissionDate, dischargeDate, reason.
   */
  static QString validateInPatientInput(const QString &roomId,
                                        const QString &doctorId,
                                        const QDate &admissionDate,
                                        const QDate &dischargeDate,
                                        const QString &reason);

  /**
   * @brief Kiểm tra trường đặc thù của bệnh nhân cấp cứu
   *        (`emergency_patients`).
   *        Bao gồm: roomId, doctorId,
   *                 injuryCause, injuryDescription,
   *                 admissionDate, dischargeDate.
   */
  static QString validateEmergencyPatientInput(const QString &roomId,
                                               const QString &doctorId,
                                               const QString &injuryCause,
                                               const QString &injuryDescription,
                                               const QDate &admissionDate,
                                               const QDate &dischargeDate);

  /**
   * @brief Kiểm tra trường cơ bản khi UPDATE bảng `patients`.
   *        Bỏ qua patientCode, allergies, insurance, type
   *        vì những trường đó không thay đổi trong luồng cập nhật.
   */
  static QString
  validateUpdateBaseInput(int patientId, const QString &fullName,
                          const QDate &dateOfBirth, const QString &gender,
                          const QString &citizenId, const QString &phone,
                          const QString &email, const QString &bloodType,
                          const QString &allergies, const QString &insurance);

public:
  explicit PatientService(std::shared_ptr<PatientRepository> patientRepository)
      : m_patientRepository(patientRepository) {}
  ~PatientService() {}

  /**
   * @brief Tạo mã bệnh nhân theo loại (OUT-yyyyMMdd-NNNN / IN-… / EMER-…).
   */
  QString generatePatientCode(PatientType type);

  /**
   * @brief Đăng ký bệnh nhân ngoại trú.
   */
  bool AddOutPatient(int patientId, int doctorId, const QString &fullName,
                     const QDate &dateOfBirth, const QString &gender,
                     const QString &citizenId, const QString &phone,
                     const QString &email, const QString &address,
                     const QString &bloodType, const QString &allergies,
                     const QString &insurance, PatientType type,
                     const QString &emergencyContactName,
                     const QString &emergencyContactPhone);

  /**
   * @brief Nhập viện bệnh nhân nội trú.
   */
  bool AddInPatient(int patientId, const QString &fullName,
                    const QDate &dateOfBirth, const QString &gender,
                    const QString &citizenId, const QString &phone,
                    const QString &email, const QString &address,
                    const QString &bloodType, const QString &allergies,
                    const QString &insurance, PatientType type,
                    const QString &emergencyContactName,
                    const QString &emergencyContactPhone, const QString &roomId,
                    const QString &doctorId, const QDate &admissionDate,
                    const QDate &dischargeDate, const QString &reason);

  /**
   * @brief Tiếp nhận bệnh nhân cấp cứu.
   */
  bool AddEmergencyPatient(
      int patientId, const QString &fullName, const QDate &dateOfBirth,
      const QString &gender, const QString &citizenId, const QString &phone,
      const QString &email, const QString &address, const QString &bloodType,
      const QString &allergies, const QString &insurance, PatientType type,
      const QString &emergencyContactName, const QString &emergencyContactPhone,
      const QString &roomId, const QString &doctorId,
      const QString &injuryCause, const QString &injuryDescription,
      const QDate &admissionDate, const QDate &dischargeDate);

  /**
   * @brief Cập nhật thông tin cơ bản của bệnh nhân trong bảng `patients`.
   */
  bool UpdatePatient(int patientId, const QString &fullName,
                     const QDate &dateOfBirth, const QString &gender,
                     const QString &citizenId, const QString &phone,
                     const QString &email, const QString &address,
                     const QString &bloodType, const QString &allergies,
                     const QString &insurance,
                     const QString &emergencyContactName,
                     const QString &emergencyContactPhone);

  /**
   * @brief Cập nhật thông tin bệnh nhân ngoại trú.
   *        Ghi đè cả `patients` lẫn `out_patients`.
   */
  bool UpdateOutPatient(int patientId, int doctorId, const QString &fullName,
                        const QDate &dateOfBirth, const QString &gender,
                        const QString &citizenId, const QString &phone,
                        const QString &email, const QString &address,
                        const QString &bloodType, const QString &allergies,
                        const QString &insurance,
                        const QString &emergencyContactName,
                        const QString &emergencyContactPhone,
                        const QString &status);

  /**
   * @brief Cập nhật thông tin bệnh nhân nội trú.
   *        Ghi đè cả `patients` lẫn `in_patients`.
   */
  bool UpdateInPatient(int patientId, const QString &fullName,
                       const QDate &dateOfBirth, const QString &gender,
                       const QString &citizenId, const QString &phone,
                       const QString &email, const QString &address,
                       const QString &bloodType, const QString &allergies,
                       const QString &insurance,
                       const QString &emergencyContactName,
                       const QString &emergencyContactPhone,
                       const QString &roomId, const QString &doctorId,
                       const QDate &admissionDate, const QDate &dischargeDate,
                       const QString &reason, const QString &status);

  /**
   * @brief Cập nhật thông tin bệnh nhân cấp cứu.
   *        Ghi đè cả `patients` lẫn `emergency_patients`.
   */
  bool UpdateEmergencyPatient(
      int patientId, const QString &fullName, const QDate &dateOfBirth,
      const QString &gender, const QString &citizenId, const QString &phone,
      const QString &email, const QString &address, const QString &bloodType,
      const QString &allergies, const QString &insurance,
      const QString &emergencyContactName, const QString &emergencyContactPhone,
      const QString &roomId, const QString &doctorId,
      const QString &injuryCause, const QString &injuryDescription,
      const QDate &admissionDate, const QDate &dischargeDate,
      const QString &status);

  /**
   * @brief Tìm kiếm bệnh nhân kết hợp tất cả các tiêu chí.
   *        Gọi xuống Validation để kiểm tra ngày tháng trước.
   * @param criteria Tiêu chí tìm kiếm (từ UI).
   * @return Danh sách kết quả (đã limit/offset). Rỗng nếu lỗi validation.
   */
  QVector<PatientSearchResultDTO>
  searchPatients(const PatientSearchCriteria &criteria);

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
  int countSearchResults(const PatientSearchCriteria &criteria);

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
   * @return nullopt nếu bệnh nhân chưa có bảo hiểm.
   */
  std::optional<InsuranceResultDTO> getInsurance(int patientId);
};