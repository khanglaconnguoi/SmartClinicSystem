/**
 * @file    Patient.cpp
 * @brief   Implementation cho Patient entity.
 */
#include "Patient.h"
#include <QDate>
#include <QString>
#include <QStringList>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

Patient::Patient(int patientId, const QString &patientCode,
                 const QString &fullName, const QDate &dateOfBirth,
                 const QString &gender, const QString &citizenId,
                 const QString &phone, const QString &email,
                 const QString &address, const QString &bloodType,
                 const QString &allergies, const QString &insurance,
                 PatientType type, const QString &emergencyContactName,
                 const QString &emergencyContactPhone, bool isDeleted)
    : m_patientId(patientId), m_patientCode(patientCode), m_fullName(fullName),
      m_dateOfBirth(dateOfBirth), m_gender(gender), m_citizenId(citizenId),
      m_phone(phone), m_email(email), m_address(address),
      m_bloodType(bloodType), m_allergies(allergies), m_insurance(insurance),
      m_type(type), m_emergencyContactName(emergencyContactName),
      m_emergencyContactPhone(emergencyContactPhone), is_deleted(isDeleted) {}

// ---------------------------------------------------------------------------
// Getters (non-inline implementations)
// ---------------------------------------------------------------------------

int Patient::getAge() const {
  if (!m_dateOfBirth.isValid())
    return 0;

  const QDate today = QDate::currentDate();
  int age = today.year() - m_dateOfBirth.year();

  // Chưa qua sinh nhật trong năm nay → trừ 1
  if (m_dateOfBirth.month() > today.month() ||
      (m_dateOfBirth.month() == today.month() &&
       m_dateOfBirth.day() > today.day())) {
    age--;
  }

  return age;
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

/**
 * @brief Kiểm tra bệnh nhân có dị ứng với thuốc/chất cho trước không.
 *
 * Danh sách dị ứng được lưu trong m_allergies, phân cách bởi dấu phẩy.
 * So sánh không phân biệt hoa/thường và bỏ qua khoảng trắng thừa.
 *
 * @param medicationName Tên thuốc / chất cần kiểm tra.
 * @return true nếu tên xuất hiện trong danh sách dị ứng, ngược lại false.
 */
bool Patient::hasAllergy(const QString &medicationName) const {
  if (m_allergies.trimmed().isEmpty())
    return false;

  const QStringList allergyList = m_allergies.split(',', Qt::SkipEmptyParts);

  const QString target = medicationName.trimmed().toLower();

  for (const QString &entry : allergyList) {
    if (entry.trimmed().toLower() == target)
      return true;
  }

  return false;
}

/**
 * @brief Sinh mã bệnh nhân theo định dạng BN-YYYY-XXXX.
 *
 * Ví dụ: id = 1 → "BN-2024-0001"
 *
 * @param id  ID số nguyên của bệnh nhân.
 * @return    Chuỗi mã bệnh nhân đã được định dạng.
 */
QString Patient::generatePatientCode(int id) {
  const QString year = QString::number(QDate::currentDate().year());
  const QString serial = QString("%1").arg(id, 4, 10, QChar('0'));
  return QString("BN-%1-%2").arg(year, serial);
}