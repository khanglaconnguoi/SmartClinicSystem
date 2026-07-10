#pragma once

#include "dto/MedicalRecordDTOs.h"
#include <QList>
#include <QString>
#include <optional>

// Forward declare — đồng đội sẽ #include header thật của họ khi PrescriptionDTOs.h
// tồn tại; để file này compile ngay bây giờ, tạm forward declare.
struct PrescriptionItemDTO;

class MedicalRecordRepository {
private:
  // Helper riêng của bạn — insert từng dòng chẩn đoán sau khi đã có recordId.
  bool insertDiagnoses(int recordId, const QList<Diagnosis> &diagnoses);

public:
  // ── Phần của bạn ──
  int insertMedicalRecord(const MedicalRecordInsertDTO &dto); // trả -1 nếu lỗi
  bool updateMedicalRecord(const MedicalRecordUpdateDTO &dto);
  bool softDeleteMedicalRecord(int recordId);
  std::optional<MedicalRecordResultDTO> findById(int recordId);
  QList<MedicalRecordResultDTO> getHistoryByPatientId(int patientId);

  // ── Phần đồng đội — CHỈ khai báo, KHÔNG cài đặt, để trống cho họ ──
  bool insertPrescription(int recordId, const QList<PrescriptionItemDTO> &items);
  std::optional<QList<PrescriptionItemDTO>> getPrescriptionByRecordId(int recordId);
  bool confirmPrescription(int prescriptionId);
};
