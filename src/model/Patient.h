/**
 * @file    Patient.h
 * @brief   Data entity đại diện cho một bệnh nhân trong hệ thống.
 */
#pragma once

#include "state/IPatientState.h"
#include <QDate>
#include <QString>
#include <memory>

/**
 * @brief Giới tính bệnh nhân.
 */
enum class Gender { Male, Female, Other };

/**
 * @brief Mức độ ưu tiên của bệnh nhân.
 */
enum class PatientPriority { Low, Normal, High, Critical };

/**
 * @brief Loại bệnh nhân.
 */
enum class PatientType { OutPatient, InPatient, Emergency };

/**
 * @brief Data class (POCO) chứa thông tin bệnh nhân.
 *
 * Lớp này chỉ chứa dữ liệu + getter/setter, không chứa logic
 * nghiệp vụ hay SQL. Validation cơ bản qua isValid().
 */
class Patient {
protected:
  int m_patientId = -1;
  QString m_patientCode;
  QString m_fullName;
  QDate m_birthDate;
  Gender m_gender = Gender::Other;
  QString m_phoneNumber;
  QString m_address;
  QString m_bloodType;
  QString m_allergies;
  QString m_medicalHistory;
  QString m_citizenId;
  QString m_email;
  QString m_insurance;
  bool m_isActive = true;
  std::shared_ptr<IPatientState> m_state;

public:
  Patient() = default;
  virtual ~Patient() = default;
  Patient(const QString &fullName, const QDate &birthDate, Gender gender,
          const QString &phoneNumber, const QString &address,
          const QString &citizenId = {}, const QString &email = {},
          const QString &insurance = {});

  // --- Getters ---
  int id() const;
  QString patientCode() const;
  QString fullName() const;
  QDate birthDate() const;
  int getAge() const;
  Gender gender() const;
  QString phoneNumber() const;
  QString address() const;
  QString bloodType() const;
  QString allergies() const;
  QString medicalHistory() const;
  QString citizenId() const;
  QString email() const;
  QString insurance() const;
  bool isActive() const;

  // --- State Pattern ---
  /**
   * @brief Lấy enum type của trạng thái hiện tại (để lưu DB).
   */
  PatientStateType stateType() const;

  /**
   * @brief Lấy tên trạng thái hiển thị trên UI.
   */
  QString stateName() const;

  /**
   * @brief Kiểm tra có thể chuyển sang trạng thái kế tiếp không.
   */
  bool canAdvance() const;

  // --- Pure Virtual (Đa hình) ---
  virtual PatientPriority getPriority() const = 0;
  virtual QString getBillingType() const = 0;
  virtual QString getStatusLabel() const = 0;
  virtual double getBaseFee() const = 0;
  virtual PatientType getType() const = 0;

  // --- Utility ---
  bool hasAllergy(const QString &medicationName) const;
  static QString generatePatientCode();

  // --- Setters ---
  void setId(int id);
  void setPatientCode(const QString &code);
  void setFullName(const QString &fullName);
  void setBirthDate(const QDate &birthDate);
  void setGender(Gender gender);
  void setPhoneNumber(const QString &phoneNumber);
  void setAddress(const QString &address);
  void setBloodType(const QString &bloodType);
  void setAllergies(const QString &allergies);
  void setMedicalHistory(const QString &history);
  void setCitizenId(const QString &citizenId);
  void setEmail(const QString &email);
  void setInsurance(const QString &insurance);
  void setIsActive(bool isActive);

  /**
   * @brief Gán trạng thái bằng state object.
   */
  void setState(std::shared_ptr<IPatientState> state);

  /**
   * @brief Gán trạng thái bằng enum type (tiện dùng khi đọc từ DB).
   */
  void setState(PatientStateType type);

  /**
   * @brief Chuyển sang trạng thái kế tiếp trong quy trình.
   * @return true nếu chuyển thành công, false nếu đã ở trạng thái cuối.
   */
  bool advanceState();

  /**
   * @brief Kiểm tra dữ liệu bệnh nhân có hợp lệ không.
   * @return check kĩ cả tên (không được phép có số), số điện thoại(đủ 10 số),
   * cccd(đủ 12 số, là số), email(đúng định dạng, nếu có), bảo hiểm (nếu có)
   */
  bool isValid() const;
};
