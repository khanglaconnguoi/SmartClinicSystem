#pragma once

#include "dto/MedicalRecordDTOs.h"
#include "PatientRepository.h"
#include <QList>
#include <QString>
#include <memory>
#include <optional>

// Forward declare — đồng đội sẽ #include header thật của họ khi PrescriptionDTOs.h
// tồn tại; để file này compile ngay bây giờ, tạm forward declare.
struct PrescriptionItemDTO;

class MedicalRecordRepository {
private:
  std::shared_ptr<PatientRepository> m_patientRepository;

  // Helper riêng của bạn — insert từng dòng chẩn đoán sau khi đã có recordId.
  bool insertDiagnoses(int recordId, const QList<Diagnosis> &diagnoses);

  // Helper: gán patientId vào từng AllergyInputDTO rồi ủy qua PatientRepository.
  bool insertNewAllergies(int patientId,
                          const QList<AllergyInputDTO> &items);

  /**
   * @brief Build mệnh đề WHERE động dựa trên tiêu chí tìm kiếm.
   *        Escape ký tự LIKE đặc biệt, thêm wildcard %...%.
   */
  QString buildSearchWhereClause(const MedicalRecordSearchCriteria &criteria,
                                  QVariantList &outParams) const;

public:
  /**
   * @brief Khởi tạo repository với PatientRepository để ghi allergy.
   * @param patientRepo  shared_ptr tới PatientRepository (có thể nullptr nếu
   *                     không cần ghi allergy).
   */
  explicit MedicalRecordRepository(
      std::shared_ptr<PatientRepository> patientRepo = nullptr);

  // ── Phần của bạn ──
  int insertMedicalRecord(const MedicalRecordInsertDTO &dto); // trả -1 nếu lỗi
  bool updateMedicalRecord(const MedicalRecordUpdateDTO &dto);
  bool softDeleteMedicalRecord(int recordId);
  std::optional<MedicalRecordResultDTO> findById(int recordId);
  QList<MedicalRecordResultDTO> getHistoryByPatientId(int patientId);

  /**
   * @brief Tìm kiếm hồ sơ khám theo tiêu chí linh hoạt.
   *        Hỗ trợ partial match (LIKE %...%) và không phân biệt hoa thường.
   */
  QList<MedicalRecordSummaryDTO> searchMedicalRecords(const MedicalRecordSearchCriteria &criteria);

  /**
   * @brief Đếm tổng số kết quả khớp tiêu chí — dùng cho phân trang.
   */
  int countSearchResults(const MedicalRecordSearchCriteria &criteria);

  // ── Phần đồng đội — CHỈ khai báo, KHÔNG cài đặt, để trống cho họ ──
  bool insertPrescription(int recordId, const QList<PrescriptionItemDTO> &items);
  std::optional<QList<PrescriptionItemDTO>> getPrescriptionByRecordId(int recordId);
  bool confirmPrescription(int prescriptionId);
};

