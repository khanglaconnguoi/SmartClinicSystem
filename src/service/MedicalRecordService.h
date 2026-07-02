#pragma once

#include "dto/MedicalRecordDTOs.h"
#include "dto/PrescriptionDTOs.h"
#include <QList>
#include <optional>

// service/MedicalRecordService.h — SƯỜN chung, chốt trước khi code
class MedicalRecordService {
public:
  // ── Bạn implement ──
  int createMedicalRecord(
      const MedicalRecordInsertDTO &dto); // trả về recordId, -1 nếu lỗi
  QList<MedicalRecordResultDTO> getMedicalHistory(int patientId);

  // ── Bạn của bạn implement ──
  bool addPrescription(int recordId, const QList<PrescriptionItemDTO> &items);
  std::optional<QList<PrescriptionItemDTO>>
  getPrescriptionByRecordId(int recordId);
};