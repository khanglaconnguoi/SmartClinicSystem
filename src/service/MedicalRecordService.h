#pragma once
#include "dto/MedicalRecordDTOs.h"
#include <memory>
#include <QList>

class MedicalRecordRepository;
class PatientService;

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

  explicit MedicalRecordService(
      std::shared_ptr<MedicalRecordRepository> recordRepo,
      std::shared_ptr<PatientService> patientService);

  int  createMedicalRecord(MedicalRecordInsertDTO &dto);
  bool updateMedicalRecord(MedicalRecordUpdateDTO &dto);
  bool softDeleteMedicalRecord(int recordId);
  QList<MedicalRecordResultDTO> getMedicalHistory(int patientId);

  /**
   * @brief Tìm kiếm hồ sơ khám theo tiêu chí linh hoạt.
   *        Hỗ trợ partial match (LIKE %...%) và không phân biệt hoa thường.
   */
  QList<MedicalRecordSummaryDTO> searchMedicalRecords(MedicalRecordSearchCriteria criteria);

  /**
   * @brief Đếm tổng số kết quả khớp tiêu chí — dùng cho phân trang.
   */
  int countSearchResults(MedicalRecordSearchCriteria criteria);
};