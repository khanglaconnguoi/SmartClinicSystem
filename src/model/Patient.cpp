/**
 * @file    Patient.cpp
 * @brief   Implementation cho Patient entity.
 */
#include "Patient.h"

Patient::Patient(const QString &fullName, const QDate &birthDate, Gender gender,
                 const QString &phoneNumber, const QString &address,
                 const QString &citizenId, const QString &email,
                 const QString &insurance)
    : m_fullName(fullName), m_birthDate(birthDate), m_gender(gender),
      m_phoneNumber(phoneNumber), m_address(address), m_citizenId(citizenId),
      m_email(email), m_insurance(insurance) {}

// --- Getters ---

int Patient::id() const { return m_id; }

QString Patient::fullName() const { return m_fullName; }

QDate Patient::birthDate() const { return m_birthDate; }

Gender Patient::gender() const { return m_gender; }

QString Patient::phoneNumber() const { return m_phoneNumber; }

QString Patient::address() const { return m_address; }

QString Patient::citizenId() const { return m_citizenId; }

QString Patient::email() const { return m_email; }

QString Patient::insurance() const { return m_insurance; }

bool Patient::isActive() const { return m_isActive; }

// --- Setters ---

void Patient::setId(int id) { m_id = id; }

void Patient::setFullName(const QString &fullName) { m_fullName = fullName; }

void Patient::setBirthDate(const QDate &birthDate) { m_birthDate = birthDate; }

void Patient::setGender(Gender gender) { m_gender = gender; }

void Patient::setPhoneNumber(const QString &phoneNumber) {
  m_phoneNumber = phoneNumber;
}

void Patient::setAddress(const QString &address) { m_address = address; }

void Patient::setCitizenId(const QString &citizenId) {
  m_citizenId = citizenId;
}

void Patient::setEmail(const QString &email) { m_email = email; }

void Patient::setInsurance(const QString &insurance) {
  m_insurance = insurance;
}

void Patient::setIsActive(bool isActive) { m_isActive = isActive; }

// --- Validation ---

bool Patient::isValid() const {
  // Tên không được rỗng và không chứa số
  if (m_fullName.trimmed().isEmpty()) {
    return false;
  }
  for (const QChar &ch : m_fullName) {
    if (ch.isDigit()) {
      return false;
    }
  }

  // SĐT phải đủ 10 số
  if (m_phoneNumber.length() != 10) {
    return false;
  }
  for (const QChar &ch : m_phoneNumber) {
    if (!ch.isDigit()) {
      return false;
    }
  }

  // CCCD phải đủ 12 số (nếu có nhập)
  if (!m_citizenId.isEmpty()) {
    if (m_citizenId.length() != 12) {
      return false;
    }
    for (const QChar &ch : m_citizenId) {
      if (!ch.isDigit()) {
        return false;
      }
    }
  }

  // Email phải đúng định dạng (nếu có nhập)
  if (!m_email.isEmpty()) {
    int atIndex = m_email.indexOf('@');
    int dotIndex = m_email.lastIndexOf('.');
    // Phải có @ và . sau @, và không ở đầu/cuối
    if (atIndex < 1 || dotIndex <= atIndex + 1
        || dotIndex >= m_email.length() - 1) {
      return false;
    }
  }

  return true;
}

