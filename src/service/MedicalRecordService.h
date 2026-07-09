#pragma once

#include "dto/PrescriptionDTOs.h"
#include

// service/MedicalRecordService.h — SƯỜN chung, chốt trước khi code
class MedicalRecordService {
public:
  // ── Bạn implement ──
  int createMedicalRecord(
      const MedicalRecordInsertDTO &dto); // trả về recordId, -1 nếu lỗi
  QVector<MedicalRecordResultDTO> getMedicalHistory(int patientId);

  // ── Bạn của bạn implement ──
  // bool addPrescription(int recordId, const QVector<PrescriptionItemDTO> &items);
  // std::optional<QVector<PrescriptionItemDTO>>
  // getPrescriptionByRecordId(int recordId);
};