#include "EmergencyPatient.h"

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

/**
 * @brief Constructor mặc định — tạo đối tượng EmergencyPatient rỗng.
 *
 * Hữu ích khi cần khởi tạo tạm hoặc deserialize từ database.
 */
EmergencyPatient::EmergencyPatient()
    : Patient(0,
              QString(),              // m_patientCode
              QString(),              // m_fullName
              QDate(),                // m_dateOfBirth
              QString(),              // m_gender
              QString(),              // m_citizenId
              QString(),              // m_phone
              QString(),              // m_email
              QString(),              // m_address
              QString(),              // m_bloodType
              QString(),              // m_allergies
              QString(),              // m_insurance
              PatientType::Emergency, // m_type
              QString(),              // m_emergencyContactName
              QString()               // m_emergencyContactPhone
              ),
      m_emergencyRoomId(QString()), m_emergencyDoctorId(QString()),
      m_injuryCause(QString()), m_injuryDescription(QString()),
      m_admissionDate(QDate()), m_dischargeDate(QDate()),
      m_status(EmergencyPatientState::Emergency) {}

// ---------------------------------------------------------------------------
// Pure Virtual Overrides
// ---------------------------------------------------------------------------

/**
 * @brief Trả về mức độ ưu tiên của bệnh nhân cấp cứu.
 *
 * Mặc định luôn là CRITICAL vì là cấp cứu.
 *
 * @return PatientPriority::Critical
 */
PatientPriority EmergencyPatient::getPriority() const {
  return PatientPriority::Critical;
}

/**
 * @brief Trả về loại hình thanh toán cho bệnh nhân cấp cứu.
 *
 * @return Chuỗi "Emergency Billing".
 */
QString EmergencyPatient::getBillingType() const {
  return QString("Emergency Billing");
}

/**
 * @brief Trả về nhãn trạng thái hiển thị cho bệnh nhân cấp cứu.
 *
 * @return Chuỗi "Emergency".
 */
QString EmergencyPatient::getStatusLabel() const {
  return QString("Emergency");
}

/**
 * @brief Trả về phí cấp cứu cơ bản cho bệnh nhân.
 *
 * Cao hơn ngoại trú và nội trú do tính chất khẩn cấp.
 *
 * @return Phí cấp cứu (đơn vị: VNĐ).
 */
double EmergencyPatient::getBaseFee() const {
  return 800000.0; // 800,000 VNĐ — phí cấp cứu cơ bản
}

// ---------------------------------------------------------------------------
// Getters & Setters
// ---------------------------------------------------------------------------

QString EmergencyPatient::getEmergencyRoomId() const {
  return m_emergencyRoomId;
}

void EmergencyPatient::setEmergencyRoomId(const QString &emergencyRoomId) {
  m_emergencyRoomId = emergencyRoomId;
}

QString EmergencyPatient::getEmergencyDoctorId() const {
  return m_emergencyDoctorId;
}

void EmergencyPatient::setEmergencyDoctorId(const QString &emergencyDoctorId) {
  m_emergencyDoctorId = emergencyDoctorId;
}

QString EmergencyPatient::getInjuryCause() const { return m_injuryCause; }

void EmergencyPatient::setInjuryCause(const QString &injuryCause) {
  m_injuryCause = injuryCause;
}

QString EmergencyPatient::getInjuryDescription() const {
  return m_injuryDescription;
}

void EmergencyPatient::setInjuryDescription(const QString &injuryDescription) {
  m_injuryDescription = injuryDescription;
}

QDate EmergencyPatient::getAdmissionDate() const { return m_admissionDate; }

void EmergencyPatient::setAdmissionDate(const QDate &admissionDate) {
  m_admissionDate = admissionDate;
}

QDate EmergencyPatient::getDischargeDate() const { return m_dischargeDate; }

void EmergencyPatient::setDischargeDate(const QDate &dischargeDate) {
  m_dischargeDate = dischargeDate;
}

EmergencyPatientState EmergencyPatient::getStatus() const { return m_status; }

void EmergencyPatient::setStatus(EmergencyPatientState status) {
  m_status = status;
}
