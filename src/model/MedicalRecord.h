#pragma once

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QString>
#include <optional>

/**
 * @brief   Lưu trữ thông tin dấu hiệu sinh tồn.
 */
struct VitalSigns {
  double temperature = 0.0; // °C
  QString bloodPressure;    // dạng "120/80"
  int heartRate = 0;        // bpm
  double weight = 0.0;      // kg
  double height = 0.0;      // cm
};

/**
 * @brief   Lưu trữ thông tin chẩn đoán.
 */
struct Diagnosis {
  QString icdCode;     // có thể rỗng
  QString description; // bắt buộc
  QString severity;    // PHẢI là "MILD" / "MODERATE" / "SEVERE"
};

/**
 * @brief   Lớp quản lý thông tin hồ sơ bệnh án.
 */
class MedicalRecord {
private:
  int m_recordId;
  int m_patientId;
  int m_doctorId;
  std::optional<int> m_appointmentId;
  QDateTime m_visitDateTime;
  VitalSigns m_vitals;
  QString m_chiefComplaint;
  QString m_clinicalNotes;
  QString m_treatment;
  std::optional<QDate> m_nextVisitDate;
  QList<Diagnosis> m_diagnoses;

public:
  /**
   * @brief   Khởi tạo một hồ sơ bệnh án mới.
   * @param   patientId  ID của bệnh nhân.
   * @param   doctorId   ID của bác sĩ.
   */
  explicit MedicalRecord(int patientId, int doctorId);

  int getRecordId() const { return m_recordId; }
  int getPatientId() const { return m_patientId; }
  int getDoctorId() const { return m_doctorId; }
  std::optional<int> getAppointmentId() const { return m_appointmentId; }
  QDateTime getVisitDateTime() const { return m_visitDateTime; }
  VitalSigns getVitals() const { return m_vitals; }
  QString getChiefComplaint() const { return m_chiefComplaint; }
  QString getClinicalNotes() const { return m_clinicalNotes; }
  QString getTreatment() const { return m_treatment; }
  std::optional<QDate> getNextVisitDate() const { return m_nextVisitDate; }
  QList<Diagnosis> getDiagnoses() const { return m_diagnoses; }

  /**
   * @brief   Thêm một chẩn đoán vào hồ sơ bệnh án.
   * @param   d  Chẩn đoán cần thêm.
   */
  void addDiagnosis(const Diagnosis &d) { m_diagnoses.append(d); }

  /**
   * @brief   Kiểm tra hồ sơ đã hoàn thành (đầy đủ các trường bắt buộc) chưa.
   * @return  true nếu hoàn thành, ngược lại false.
   */
  bool isComplete() const;

  /**
   * @brief   Tính chỉ số BMI từ cân nặng và chiều cao.
   * @return  Giá trị BMI, hoặc 0.0 nếu chiều cao <= 0.
   */
  double calculateBMI() const;
};