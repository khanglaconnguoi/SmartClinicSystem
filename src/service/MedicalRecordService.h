#pragma once
#include "dto/MedicalRecordDTOs.h"
#include "repository/MedicalRecordRepository.h"
#include "service/PatientService.h"
#include <QList>
#include <memory>

class MedicalRecordService {
private:
  std::shared_ptr<MedicalRecordRepository> m_recordRepository;
  std::shared_ptr<PatientService> m_patientService;

public:
  /**
   * @brief Chuẩn hóa dữ liệu hồ sơ khám trước khi INSERT.
   *        Trim khoảng trắng, chuẩn hóa severity chẩn đoán về IN HOA.
   *        LUÔN gọi trước validate.
   */
  static void normalizeMedicalRecordInput(MedicalRecordInsertDTO &dto);

  /**
   * @brief Chuẩn hóa dữ liệu hồ sơ khám trước khi UPDATE.
   */
  static void normalizeMedicalRecordUpdate(MedicalRecordUpdateDTO &dto);

  /**
   * @brief Chuẩn hóa tiêu chí tìm kiếm.
   */
  static void normalizeSearchCriteria(MedicalRecordSearchCriteria &criteria);

  static QString validateVitalSigns(const VitalSigns &vitals);
  static QString validateTemperature(double temperature);
  static QString validateHeartRate(int heartRate);
  static QString validateWeight(double weight);
  static QString validateHeight(double height);

  static QString validateDiagnosisList(const QList<Diagnosis> &diagnoses);
  static QString validateDiagnosisSeverity(const QString &severity);

  /**
   * @brief Validate danh sách dị ứng mới gửi kèm hồ sơ khám.
   *        Cho phép danh sách rỗng (dị ứng là optional).
   *        Kiểm tra từng item: allergenName không rỗng, severity hợp lệ.
   */
  static QString validateAllergyList(const QList<AllergyInputDTO> &allergies);

  explicit MedicalRecordService(
      std::shared_ptr<MedicalRecordRepository> recordRepo,
      std::shared_ptr<PatientService> patientService);

  QString createMedicalRecord(MedicalRecordInsertDTO &dto, int *outRecordId = nullptr);
  QString updateMedicalRecord(MedicalRecordUpdateDTO &dto);
  bool softDeleteMedicalRecord(int recordId);
  QList<MedicalRecordResultDTO> getMedicalHistory(int patientId);
  std::optional<MedicalRecordResultDTO> getMedicalRecordById(int recordId) const;
  std::optional<MedicalRecordResultDTO> getLatestRecordByAppointmentId(int appointmentId) const;

  /**
   * @brief Tìm kiếm hồ sơ khám theo tiêu chí linh hoạt.
   *        Hỗ trợ partial match (LIKE %...%) và không phân biệt hoa thường.
   */
  // QList<MedicalRecordSummaryDTO> searchMedicalRecords(MedicalRecordSearchCriteria criteria);
  // int countSearchResults(MedicalRecordSearchCriteria criteria);

  PagedResult<MedicalRecordSummaryDTO> searchMedicalRecordsPaged(MedicalRecordSearchCriteria criteria) const;

};