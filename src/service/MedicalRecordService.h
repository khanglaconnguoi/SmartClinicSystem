#pragma once
#include "dto/MedicalRecordDTOs.h"
#include <memory>
#include <optional>
#include <QList>

class MedicalRecordRepository;
class PatientService;
class PharmacyService;   // của đồng đội — forward declare, chưa tồn tại file thật
struct PrescriptionItemDTO; // của đồng đội — forward declare

class MedicalRecordService {
private:
  std::shared_ptr<MedicalRecordRepository> m_recordRepository;
  std::shared_ptr<PatientService> m_patientService;
  std::shared_ptr<PharmacyService> m_pharmacyService;

public:
  explicit MedicalRecordService(
      std::shared_ptr<MedicalRecordRepository> recordRepo,
      std::shared_ptr<PatientService> patientService,
      std::shared_ptr<PharmacyService> pharmacyService);

  // ── Phần của bạn ──
  int createMedicalRecord(const MedicalRecordInsertDTO &dto);
  bool updateMedicalRecord(const MedicalRecordUpdateDTO &dto);
  bool softDeleteMedicalRecord(int recordId);
  QList<MedicalRecordResultDTO> getMedicalHistory(int patientId);

  // ── Phần đồng đội — CHỈ khai báo, không cài đặt ──
  bool addPrescription(int recordId, const QList<PrescriptionItemDTO> &items);
  std::optional<QList<PrescriptionItemDTO>> getPrescriptionByRecordId(int recordId);
};