#include "MedicalRecord.h"

/**
 * @brief   Khởi tạo một hồ sơ bệnh án mới.
 * @param   patientId  ID của bệnh nhân.
 * @param   doctorId   ID của bác sĩ.
 */
MedicalRecord::MedicalRecord(int patientId, int doctorId)
    : m_recordId(0),
      m_patientId(patientId),
      m_doctorId(doctorId),
      m_appointmentId(std::nullopt),
      m_visitDateTime(),
      m_vitals(),
      m_chiefComplaint(),
      m_clinicalNotes(),
      m_treatment(),
      m_nextVisitDate(std::nullopt),
      m_diagnoses()
{
}

/**
 * @brief   Kiểm tra hồ sơ đã hoàn thành (đầy đủ các trường bắt buộc) chưa.
 * @return  true nếu hoàn thành, ngược lại false.
 */
bool MedicalRecord::isComplete() const {
    if (m_chiefComplaint.isEmpty() || m_diagnoses.isEmpty() ||
        m_vitals.weight <= 0.0 || m_vitals.height <= 0.0) {
        return false;
    }
    return true;
}

/**
 * @brief   Tính chỉ số BMI từ cân nặng và chiều cao.
 * @return  Giá trị BMI, hoặc 0.0 nếu chiều cao <= 0.
 */
double MedicalRecord::calculateBMI() const {
    if (m_vitals.height <= 0.0) {
        return 0.0;
    }
    double heightInMeters = m_vitals.height / 100.0;
    return m_vitals.weight / (heightInMeters * heightInMeters);
}
