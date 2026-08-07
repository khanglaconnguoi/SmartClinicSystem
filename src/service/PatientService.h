/**
 * @file    PatientService.h
 * @brief   Service layer cho module Patient.
 *
 *  Các hàm validate là public static — UI có thể gọi trực tiếp để kiểm tra từng
 * ô. Mỗi hàm validate trả về chuỗi rỗng "" nếu hợp lệ, hoặc chuỗi mô tả lỗi nếu
 * không hợp lệ.
 */

#pragma once

#include "dto/PatientDTOs.h"
#include "model/CommonEnums.h"
#include "repository/PatientRepository.h"
#include <QDate>
#include <QString>
#include <memory>



class PatientService {
private:
  std::shared_ptr<PatientRepository> m_patientRepository;

  // =================================================================
  // MAPPING HELPERS (private static)
  // Convert a normalized InputDTO into a repo-level InsertDTO/UpdateDTO.
  // No trimming here — assumes input is already normalized.
  // =================================================================
  static PatientInsertDTO mapPatientToInsertDTO(const PatientInputDTO &input, const QString &patientCode, const QString &patientType);
  static OutPatientInsertDTO mapOutPatientToInsertDTO(const OutPatientInputDTO &input, const QString &patientCode);
  static InPatientInsertDTO mapInPatientToInsertDTO(const InPatientInputDTO &input, const QString &patientCode);
  static EmergencyPatientInsertDTO mapEmergencyPatientToInsertDTO(const EmergencyPatientInputDTO &input, const QString &patientCode);

  static PatientUpdateDTO mapPatientToUpdateDTO(const PatientInputDTO &input, int patientId);
  static OutPatientUpdateDTO mapOutPatientToUpdateDTO(const OutPatientInputDTO &input, int patientId, const QString &status);
  static InPatientUpdateDTO mapInPatientToUpdateDTO(const InPatientInputDTO &input, int patientId, const QString &status);
  static EmergencyPatientUpdateDTO mapEmergencyPatientToUpdateDTO(const EmergencyPatientInputDTO &input, int patientId, const QString &status);

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
  QString validateBaseInput(const PatientInputDTO &dto,
                            const QString &patientCode = "");

  /**
   * @brief Kiểm tra trường đặc thù của bệnh nhân nội trú (`in_patients`).
   */
  static QString validateInPatientInput(const InPatientInputDTO &dto);

  /**
   * @brief Kiểm tra trường đặc thù của bệnh nhân cấp cứu
   * (`emergency_patients`).
   */
  static QString
  validateEmergencyPatientInput(const EmergencyPatientInputDTO &dto);

  /**
   * @brief Kiểm tra trường cơ bản khi UPDATE bảng `patients`.
   *        Bỏ qua các trường không thay đổi.
   */
  static QString validateUpdateBaseInput(const PatientInputDTO &dto,
                                         int patientId);


  /**
   * @brief Chuẩn hóa dữ liệu đầu vào từ UI:
   *        trim khoảng trắng, chuẩn hóa chữ hoa/thường, email về lowercase...
   *        LUÔN gọi trước validate — đảm bảo validate trên dữ liệu đã sạch.
   */
  static void normalizePatientInput(PatientInputDTO &dto);

  static void normalizeInPatientInput(InPatientInputDTO &dto);
  static void normalizeOutPatientInput(OutPatientInputDTO &dto);
  static void normalizeEmergencyPatientInput(EmergencyPatientInputDTO &dto);

  /**
   * @brief Chuẩn hóa một bản ghi dị ứng:
   *        trim allergenName, severity → UPPER, notes → simplified.
   */
  static void normalizeAllergyInput(AllergyInputDTO &dto);

  /**
   * @brief Chuẩn hóa toàn bộ danh sách dị ứng trong PatientInputDTO.
   */
  static void normalizeAllergyInputList(QList<AllergyInputDTO> &list);

  /**
   * @brief Chuẩn hóa bản ghi bảo hiểm:
   *        trim providerName, policyNumber → UPPER, insuranceType → UPPER,
   *        validFrom/validTo giữ nguyên định dạng yyyy-MM-dd.
   */
  static void normalizeInsuranceInput(InsuranceInputDTO &dto);

  /**
   * @brief Chuẩn hóa tiêu chí tìm kiếm trước khi lọc (vd: cắt khoảng trắng từ
   * khoá).
   */
  static void normalizeSearchCriteria(PatientSearchCriteria &criteria);

  // ── Validate các trường đơn lẻ dành cho UI gọi trực tiếp ───────────────
  static QString validateBloodType(const QString &bloodType);

  static QString
  validateInPatientDischargeDate(const QDate &admissionDate,
                                 std::optional<QDate> dischargeDate);
  static QString
  validateEmergencyDischargeDate(const QDate &admissionDate,
                                 std::optional<QDate> dischargeDate);

  // ── Allergy ──────────────────────────────────────────────────────────────

  /**
   * @brief Kiểm tra toàn bộ một bản ghi dị ứng sau khi đã normalize.
   *        allergenName bắt buộc; severity phải là MILD/MODERATE/SEVERE;
   *        notes tùy chọn, tối đa 500 ký tự.
   */
  static QString validateAllergyInput(const AllergyInputDTO &dto);

  /**
   * @brief Kiểm tra toàn bộ danh sách dị ứng (gọi validateAllergyInput cho
   *        từng phần tử, trả về lỗi đầu tiên tìm được kèm chỉ số 1-based).
   */
  static QString validateAllergyInputList(const QList<AllergyInputDTO> &list);

  // ── Insurance ────────────────────────────────────────────────────────────

  /**
   * @brief Kiểm tra toàn bộ bản ghi bảo hiểm sau khi đã normalize.
   *        providerName, policyNumber, insuranceType bắt buộc;
   *        coveragePercent ∈ [0, 100]; validFrom ≤ validTo (nếu cả hai có).
   */
  static QString validateInsuranceInput(const InsuranceInputDTO &dto);

  /**
   * @brief Tạo mã bệnh nhân theo loại (OUT-yyyyMMdd-NNNN / IN-… / EMER-…).
   */
  QString generatePatientCode(PatientType type);

  /**
   * @brief Đăng ký bệnh nhân ngoại trú.
   */
  QString addOutPatient(OutPatientInputDTO &dto);

  /**
   * @brief Nhập viện bệnh nhân nội trú.
   */
  QString addInPatient(InPatientInputDTO &dto);

  /**
   * @brief Tiếp nhận bệnh nhân cấp cứu.
   */
  QString addEmergencyPatient(EmergencyPatientInputDTO &dto);

  /**
   * @brief Cập nhật thông tin bệnh nhân ngoại trú.
   *        Ghi đè cả `patients` lẫn `out_patients`.
   */
  QString updateOutPatient(int patientId, OutPatientInputDTO &dto,
                           const QString &status = "REGISTERED");

  /**
   * @brief Cập nhật thông tin bệnh nhân nội trú.
   *        Ghi đè cả `patients` lẫn `in_patients`.
   */
  QString updateInPatient(int patientId, InPatientInputDTO &dto,
                          const QString &status = "ADMITTED");

  /**
   * @brief Cập nhật thông tin bệnh nhân cấp cứu.
   *        Ghi đè cả `patients` lẫn `emergency_patients`.
   */
  QString updateEmergencyPatient(int patientId, EmergencyPatientInputDTO &dto,
                                 const QString &status = "EMERGENCY");

  /**
   * @brief Tìm kiếm bệnh nhân kết hợp tất cả các tiêu chí.
   *        Gọi xuống Validation để kiểm tra ngày tháng trước.
   * @param criteria Tiêu chí tìm kiếm (từ UI).
   * @return Danh sách kết quả (đã limit/offset). Rỗng nếu lỗi validation.
   */
  // QList<PatientSearchResultDTO> searchPatients(PatientSearchCriteria criteria);
  // int countSearchResults(PatientSearchCriteria criteria);

  PagedResult<PatientSearchResultDTO> searchPatientsPaged(PatientSearchCriteria criteria) const;

  std::optional<PatientDetailDTO> getPatientById(int patientId);



  bool softDeletePatient(int patientId);
  bool restorePatient(int patientId);

  /**
   * @brief Kiểm tra bệnh nhân có dị ứng với loại thuốc chỉ định không.
   * @param patientId ID bệnh nhân cần kiểm tra.
   * @param drugName  Tên thuốc cần kiểm tra (so khớp với danh sách dị ứng).
   * @return true nếu có xung đột dị ứng, false nếu an toàn hoặc không có dữ
   * liệu.
   */
  // bool checkDrugAllergyConflict(int patientId, const QString &drugName)
  // const;

  QString updateBloodType(int patientId, const QString &bloodType);

  /**
   * @brief Thêm danh sách dị ứng cho bệnh nhân đã tồn tại.
   * @param patientId ID bệnh nhân.
   * @param allergies Danh sách thông tin dị ứng từ UI.
   * @return "" nếu thành công, hoặc chuỗi thông báo lỗi.
   */
  QString addAllergiesToPatient(int patientId, QList<AllergyInputDTO> allergies);

  /**
   * @brief Ghi đè/cập nhật toàn bộ danh sách dị ứng cho bệnh nhân.
   */
  QString updateAllergiesOfPatient(int patientId, QList<AllergyInputDTO> allergies);

  /**
   * @brief Lấy danh sách dị ứng đang active của bệnh nhân.
   */
  QList<AllergyResultDTO> getAllergies(int patientId);

  /**
   * @brief Lấy thông tin bảo hiểm của bệnh nhân.
   * @return nullopt  // Tìm kiếm thông tin bảo hiểm theo ID bệnh nhân
   */
  std::optional<InsuranceResultDTO> getInsurance(int patientId) const;

  /**
   * @brief Lấy tỷ lệ bảo hiểm chi trả của bệnh nhân theo ID.
   *        Thực hiện validate ID bệnh nhân.
   * @param patientId ID của bệnh nhân.
   * @return Tỷ lệ bảo hiểm chi trả (double từ 0.0 đến 100.0).
   */
  double getInsuranceCoveragePercent(int patientId) const;

  std::optional<PatientShortDTO> getPatientByPhoneOrCitizenId(const QString &phone, const QString &citizenId) const;
  QList<PatientShortDTO> getPatientsByPhoneOrCitizenId(const QString &phone, const QString &citizenId) const;
  QString validateCitizenIdUnique(const QString &citizenId, int excludePatientId = -1) const;
  QString validatePhoneNumberUnique(const QString &phone, int excludePatientId = -1) const;
};