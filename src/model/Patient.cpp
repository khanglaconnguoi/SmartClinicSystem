/**
 * @file    Patient.cpp
 * @brief   Implementation cho Patient entity.
 */
#include "Patient.h"
#include "state/PatientStateFactory.h"
#include "state/RegisteredState.h"
#include <QDate>
#include <QStringList>
#include <QUuid>

Patient::Patient(const QString &fullName, const QDate &birthDate, Gender gender,
                 const QString &phoneNumber, const QString &address,
                 const QString &citizenId, const QString &email,
                 const QString &insurance)
    : m_fullName(fullName), m_birthDate(birthDate), m_gender(gender),
      m_phoneNumber(phoneNumber), m_address(address), m_citizenId(citizenId),
      m_email(email), m_insurance(insurance),
      m_state(std::make_shared<RegisteredState>()) {}

// --- Getters ---

int Patient::id() const { return m_patientId; }

QString Patient::patientCode() const { return m_patientCode; }

QString Patient::fullName() const { return m_fullName; }

QDate Patient::birthDate() const { return m_birthDate; }

int Patient::getAge() const {
  if (!m_birthDate.isValid())
    return 0;
  QDate current = QDate::currentDate();
  int age = current.year() - m_birthDate.year();
  if (m_birthDate.month() > current.month() ||
      (m_birthDate.month() == current.month() &&
       m_birthDate.day() > current.day())) {
    age--;
  }
  return age;
}

Gender Patient::gender() const { return m_gender; }

QString Patient::phoneNumber() const { return m_phoneNumber; }

QString Patient::address() const { return m_address; }

QString Patient::bloodType() const { return m_bloodType; }

QString Patient::allergies() const { return m_allergies; }

QString Patient::medicalHistory() const { return m_medicalHistory; }

QString Patient::citizenId() const { return m_citizenId; }

QString Patient::email() const { return m_email; }

QString Patient::insurance() const { return m_insurance; }

bool Patient::isActive() const { return m_isActive; }

// --- Setters ---

void Patient::setId(int id) {
  m_patientId = id;
  if (m_patientCode.isEmpty()) {
    m_patientCode = generatePatientCode();
  }
}

void Patient::setPatientCode(const QString &code) { m_patientCode = code; }

void Patient::setFullName(const QString &fullName) { m_fullName = fullName; }

void Patient::setBirthDate(const QDate &birthDate) { m_birthDate = birthDate; }

void Patient::setGender(Gender gender) { m_gender = gender; }

void Patient::setPhoneNumber(const QString &phoneNumber) {
  m_phoneNumber = phoneNumber;
}

void Patient::setAddress(const QString &address) { m_address = address; }

void Patient::setBloodType(const QString &bloodType) {
  m_bloodType = bloodType;
}

void Patient::setAllergies(const QString &allergies) {
  m_allergies = allergies;
}

void Patient::setMedicalHistory(const QString &history) {
  m_medicalHistory = history;
}

void Patient::setCitizenId(const QString &citizenId) {
  m_citizenId = citizenId;
}

void Patient::setEmail(const QString &email) { m_email = email; }

void Patient::setInsurance(const QString &insurance) {
  m_insurance = insurance;
}

void Patient::setIsActive(bool isActive) { m_isActive = isActive; }

// --- State Pattern ---

PatientStateType Patient::stateType() const {
  if (!m_state) {
    return PatientStateType::Registered;
  }
  return m_state->type();
}

QString Patient::stateName() const {
  if (!m_state) {
    return QStringLiteral("Đã đăng ký");
  }
  return m_state->name();
}

bool Patient::canAdvance() const {
  if (!m_state) {
    return false;
  }
  return m_state->nextState() != nullptr;
}

void Patient::setState(std::shared_ptr<IPatientState> state) {
  m_state = std::move(state);
}

void Patient::setState(PatientStateType type) {
  m_state = std::shared_ptr<IPatientState>(createPatientState(type).release());
}

bool Patient::advanceState() {
  if (!m_state) {
    return false;
  }
  auto next = m_state->nextState();
  if (!next) {
    return false;
  }
  m_state = std::shared_ptr<IPatientState>(next.release());
  return true;
}

// --- Utility ---

bool Patient::hasAllergy(const QString &medicationName) const {
  if (m_allergies.isEmpty() || medicationName.isEmpty())
    return false;

  QStringList list = m_allergies.split(",", Qt::SkipEmptyParts);
  for (const QString &item : list) {
    if (item.trimmed().compare(medicationName.trimmed(), Qt::CaseInsensitive) ==
        0) {
      return true;
    }
  }
  return false;
}

QString Patient::generatePatientCode() {
  QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
  return QString("BN-%1").arg(uuid.left(8).toUpper());
}

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
    if (atIndex < 1 || dotIndex <= atIndex + 1 ||
        dotIndex >= m_email.length() - 1) {
      return false;
    }
  }

  return true;
}
