/**
 * @file    Patient.h
 * @brief   Data entity đại diện cho một bệnh nhân trong hệ thống.
 */
#pragma once

#include <QDate>
#include <QString>
// #include <optional>

/**
 * @brief Giới tính bệnh nhân.
 */
enum class Gender { Male, Female, Other };

/**
 * @brief Data class (POCO) chứa thông tin bệnh nhân.
 *
 * Lớp này chỉ chứa dữ liệu + getter/setter, không chứa logic
 * nghiệp vụ hay SQL. Validation cơ bản qua isValid().
 */
class Patient {
public:
  Patient() = default;
  Patient(const QString &fullName, const QDate &birthDate, Gender gender,
          const QString &phoneNumber, const QString &address,
          const QString &citizenId = {}, const QString &email = {},
          const QString &insurance = {});

  // --- Getters ---
  int id() const;
  QString fullName() const;
  QDate birthDate() const;
  Gender gender() const;
  QString phoneNumber() const;
  QString address() const;
  QString citizenId() const;
  QString email() const;
  QString insurance() const;
  bool isActive() const;

  // --- Setters ---
  void setId(int id);
  void setFullName(const QString &fullName);
  void setBirthDate(const QDate &birthDate);
  void setGender(Gender gender);
  void setPhoneNumber(const QString &phoneNumber);
  void setAddress(const QString &address);
  void setCitizenId(const QString &citizenId);
  void setEmail(const QString &email);
  void setInsurance(const QString &insurance);
  void setIsActive(bool isActive);

  /**
   * @brief Kiểm tra dữ liệu bệnh nhân có hợp lệ không.
   * @return check kĩ cả tên (không được phép có số), số điện thoại(đủ 10 số),
   * cccd(đủ 12 số, là số), email(đúng định dạng, nếu có), bảo hiểm (nếu có)
   */
  bool isValid() const;

private:
  int m_id = -1;
  QString m_fullName;
  QDate m_birthDate;
  Gender m_gender = Gender::Other;
  QString m_phoneNumber;
  QString m_address;
  QString m_citizenId;
  QString m_email;
  QString m_insurance;
  bool m_isActive = true;
};
