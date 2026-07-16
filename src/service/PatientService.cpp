/**
 * @file    PatientService.cpp
 * @brief   Implementación của PatientService.
 *          Validation được uỷ quyền cho PatientValidation (Validation.h).
 */

#include "PatientService.h"
#include "Validation.h"
#include "dto/PatientDTOs.h"
#include "repository/PatientRepository.h"
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>

// ───────────────────────────────────────────────────────────────────────────────
// Helpers: parse allergies string & insurance string
// ───────────────────────────────────────────────────────────────────────────────

/**
 * @brief Tách chuỗi allergies (phân cách bằng dấu phẩy) thành
 * QList<AllergyInsertDTO>. Mỗi tỪn sau khi trim thành một AllergyInsertDTO với
 * severity = 'MODERATE'. Tên rỗng sau khi trim bị bỏ qua.
 */
static QList<AllergyInsertDTO> parseAllergiesString(int patientId,
                                                    const QString &raw) {
  QList<AllergyInsertDTO> result;
  if (raw.trimmed().isEmpty())
    return result;

  const QStringList parts = raw.split(',', Qt::SkipEmptyParts);
  for (const QString &part : parts) {
    const QString name = part.trimmed();
    if (name.isEmpty())
      continue;
    AllergyInsertDTO item;
    item.patientId = patientId;
    item.allergenName = name;
    item.severity = "MODERATE";
    result.append(item);
  }
  return result;
}

/**
 * @brief Xây InsuranceInsertDTO từ chuỗi insurance (số thẻ BHYT đơn giản).
 *        Nếu chuỗi rỗng thì trả về std::nullopt.
 */
static std::optional<InsuranceInsertDTO>
parseInsuranceString(int patientId, const QString &raw) {
  if (raw.trimmed().isEmpty())
    return std::nullopt;

  InsuranceInsertDTO dto;
  dto.patientId = patientId;
  dto.policyNumber = raw.trimmed();
  dto.providerName = "BHYT"; // mặc định
  dto.insuranceType = "BHYT";
  dto.coveragePercent = 80.0;
  return dto;
}

// ─────────────────────────────────────────────────────────────────────────────
// Sinh mã bệnh nhân
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::generatePatientCode(PatientType type) {
  static int outSeq = 1;
  static int inSeq = 1;
  static int emerSeq = 1;

  QString prefix;
  int *seqPtr;

  if (type == PatientType::Outpatient) {
    prefix = "OP";
    seqPtr = &outSeq;
  } else if (type == PatientType::Inpatient) {
    prefix = "IP";
    seqPtr = &inSeq;
  } else if (type == PatientType::Emergency) {
    prefix = "EP";
    seqPtr = &emerSeq;
  } else {
    prefix = "PT";
    static int defSeq = 1;
    seqPtr = &defSeq;
  }

  QString dateStr = QDateTime::currentDateTime().toString("yyyyMMdd");
  QString code = QString("%1%2%3").arg(prefix).arg(dateStr).arg(*seqPtr, 4, 10,
                                                                QChar('0'));
  (*seqPtr)++;
  return code;
}

// ─────────────────────────────────────────────────────────────────────────────
// AddOutPatient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientService::addOutPatient(OutPatientInputDTO &dto) {
  normalizePatientInput(dto);
  QString patientCode = generatePatientCode(dto.type);

  QString err = validateBaseInput(dto, patientCode);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (Base):" << err;
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  OutPatientInsertDTO insertDto(dto, patientCode);
  insertDto.allergies = parseAllergiesString(0, dto.allergies);
  insertDto.insurance = parseInsuranceString(0, dto.insurance);

  return m_patientRepository->insertOutPatient(insertDto);
}

// ─────────────────────────────────────────────────────────────────────────────
// Normalize
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Chuyển đổi chuỗi thành dạng Title Case (vd: "NGUYen VAN a" -> "Nguyen Van A")
 */
static QString toTitleCase(const QString &str) {
  QStringList words = str.simplified().split(' ', Qt::SkipEmptyParts);
  for (int i = 0; i < words.size(); ++i) {
    QString word = words[i].toLower();
    if (!word.isEmpty()) {
      word[0] = word[0].toUpper();
      words[i] = word;
    }
  }
  return words.join(' ');
}

static QString normalizePhoneNumber(QString phone) {
  phone = phone.replace(" ", "").replace(".", "").replace("-", "").trimmed();
  if (phone.startsWith("+84")) {
    phone.replace(0, 3, "0");
  }
  return phone;
}

void PatientService::normalizePatientInput(PatientInputDTO &dto) {
  dto.fullName = toTitleCase(dto.fullName);
  dto.citizenId = dto.citizenId.trimmed();
  dto.phone = normalizePhoneNumber(dto.phone);
  dto.email = dto.email.trimmed().toLower();
  dto.address = dto.address.simplified();
  dto.bloodType = dto.bloodType.trimmed().toUpper();
  dto.allergies = dto.allergies.trimmed();
  dto.insurance = dto.insurance.trimmed();
  dto.emergencyContactName = toTitleCase(dto.emergencyContactName);
  dto.emergencyContactPhone = normalizePhoneNumber(dto.emergencyContactPhone);
}

void PatientService::normalizeSearchCriteria(PatientSearchCriteria &criteria) {
  criteria.searchKey = criteria.searchKey.simplified();
  criteria.status = criteria.status.trimmed();
}

// ─────────────────────────────────────────────────────────────────────────────
// Validate Functions
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::validateBloodType(const QString &bloodType) {
  const QStringList valid = {"A+",  "A-", "B+", "B-",     "AB+",
                             "AB-", "O+", "O-", "UNKNOWN"};
  if (bloodType.isEmpty())
    return "Nhóm máu không được để trống. Nếu không rõ, vui lòng nhập UNKNOWN.";
  if (!valid.contains(bloodType))
    return "Nhóm máu không hợp lệ. Vui lòng nhập A+, A-, B+, B-, AB+, AB-, O+, "
           "O- hoặc UNKNOWN.";
  return "";
}

QString PatientService::validateDateRange(const QDate &fromDate,
                                          const QDate &toDate) {
  if (!fromDate.isValid() || !toDate.isValid())
    return "";

  if (fromDate > toDate)
    return "Ngày bắt đầu (Từ ngày) không được lớn hơn ngày kết thúc (Đến "
           "ngày).";

  return "";
}

bool PatientService::addInPatient(InPatientInputDTO &dto) {
  normalizePatientInput(dto);
  QString patientCode = generatePatientCode(dto.type);

  QString err = validateBaseInput(dto, patientCode);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (Base):" << err;
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }


  err = validateInPatientInput(dto);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (InPatient):" << err;
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  InPatientInsertDTO insertDto(dto, patientCode);
  insertDto.allergies = parseAllergiesString(0, dto.allergies);
  insertDto.insurance = parseInsuranceString(0, dto.insurance);
  return m_patientRepository->insertInPatient(insertDto);
}

// ─────────────────────────────────────────────────────────────────────────────
// addEmergencyPatient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientService::addEmergencyPatient(EmergencyPatientInputDTO &dto) {
  normalizePatientInput(dto);
  QString patientCode = generatePatientCode(dto.type);

  QString err = validateBaseInput(dto, patientCode);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (Base):" << err;
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }


  err = validateEmergencyPatientInput(dto);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (Emergency):" << err;
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  EmergencyPatientInsertDTO insertDto(dto, patientCode);
  insertDto.allergies = parseAllergiesString(0, dto.allergies);
  insertDto.insurance = parseInsuranceString(0, dto.insurance);
  return m_patientRepository->insertEmergencyPatient(insertDto);
}

// ─────────────────────────────────────────────────────────────────────────────
// Nhóm trường theo loại bệnh nhân
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::validatePatientCode(const QString &patientCode) {
  if (patientCode.isEmpty())
    return "Mã bệnh nhân không được để trống.";
  return "";
}

QString PatientService::validateEmergencyContactName(const QString &name) {
  if (name.trimmed().isEmpty())
    return "Người liên hệ khẩn cấp không được để trống.";
  return "";
}

QString PatientService::validateBaseInput(const PatientInputDTO &dto,
                                          const QString &patientCode) {
  QString err;
  if (!(err = validatePatientCode(patientCode)).isEmpty()) return err;
  if (!(err = Validation::validateFullName(dto.fullName)).isEmpty()) return err;
  if (dto.dateOfBirth > QDate::currentDate() || dto.dateOfBirth.year() < QDate::currentDate().year() - 150)
    return "Ngày sinh không hợp lệ (tuổi phải nhỏ hơn 150)";

  if (!(err = Validation::validateCitizenId(dto.citizenId)).isEmpty()) return err;
  if (!(err = Validation::validatePhoneNumber(dto.phone)).isEmpty()) return err;
  if (!(err = Validation::validateEmail(dto.email)).isEmpty()) return err;

  if (dto.address.isEmpty()) return "Địa chỉ không được để trống.";
  if (dto.address.length() > 255) return "Địa chỉ không được vượt quá 255 ký tự.";

  if (!(err = validateBloodType(dto.bloodType)).isEmpty()) return err;
  if (!(err = validateEmergencyContactName(dto.emergencyContactName)).isEmpty()) return err;
  if (!(err = Validation::validatePhoneNumber(dto.emergencyContactPhone)).isEmpty()) return err;

  return "";
}

QString PatientService::validateInPatientRoomId(std::optional<int> roomId) {
  if (!roomId.has_value())
    return "Mã phòng không được để trống.";
  return "";
}

QString PatientService::validateInPatientDoctorId(std::optional<int> doctorId) {
  if (!doctorId.has_value())
    return "Bác sĩ phụ trách không được để trống.";
  return "";
}

QString PatientService::validateInPatientDischargeDate(const QDate &admissionDate, std::optional<QDate> dischargeDate) {
  if (dischargeDate.has_value() && admissionDate > dischargeDate.value())
    return "Ngày nhập viện phải nhỏ hơn hoặc bằng ngày xuất viện.";
  return "";
}

QString PatientService::validateInPatientReason(const QString &reason) {
  if (reason.trimmed().isEmpty())
    return "Lý do nhập viện không được để trống.";
  if (reason.trimmed().length() > 1000)
    return "Lý do nhập viện không được vượt quá 1000 ký tự.";
  return "";
}

QString PatientService::validateInPatientInput(const InPatientInputDTO &dto) {
  QString err;
  if (!(err = validateInPatientRoomId(dto.roomId)).isEmpty()) return err;
  if (!(err = validateInPatientDoctorId(dto.doctorId)).isEmpty()) return err;
  if (!(err = validateInPatientDischargeDate(dto.admissionDate, dto.dischargeDate)).isEmpty()) return err;
  if (!(err = validateInPatientReason(dto.reason)).isEmpty()) return err;

  return "";
}

QString PatientService::validateEmergencyRoomId(std::optional<int> roomId) {
  if (!roomId.has_value())
    return "Phòng cấp cứu không được để trống.";
  return "";
}

QString PatientService::validateEmergencyDoctorId(std::optional<int> doctorId) {
  if (!doctorId.has_value())
    return "Bác sĩ trực cấp cứu không được để trống.";
  return "";
}

QString PatientService::validateEmergencyDischargeDate(const QDate &admissionDate, std::optional<QDate> dischargeDate) {
  if (dischargeDate.has_value() && admissionDate > dischargeDate.value())
    return "Ngày nhập viện phải nhỏ hơn hoặc bằng ngày xuất viện.";
  return "";
}

QString PatientService::validateEmergencyInjuryCause(const QString &cause) {
  if (cause.trimmed().isEmpty())
    return "Nguyên nhân chấn thương không được để trống.";
  if (cause.trimmed().length() > 255)
    return "Nguyên nhân chấn thương không được vượt quá 255 ký tự.";
  return "";
}

QString PatientService::validateEmergencyInjuryDescription(const QString &desc) {
  if (desc.trimmed().isEmpty())
    return "Mô tả chấn thương không được để trống.";
  if (desc.trimmed().length() > 1000)
    return "Mô tả chấn thương không được vượt quá 1000 ký tự.";
  return "";
}

QString PatientService::validateEmergencyPatientInput(
    const EmergencyPatientInputDTO &dto) {
  QString err;
  if (!(err = validateEmergencyRoomId(dto.roomId)).isEmpty()) return err;
  if (!(err = validateEmergencyDoctorId(dto.doctorId)).isEmpty()) return err;
  if (!(err = validateEmergencyInjuryCause(dto.injuryCause)).isEmpty()) return err;
  if (!(err = validateEmergencyInjuryDescription(dto.injuryDescription)).isEmpty()) return err;
  if (!(err = validateEmergencyDischargeDate(dto.admissionDate, dto.dischargeDate)).isEmpty()) return err;

  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// validateUpdateBaseInput
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::validateUpdateBaseInput(const PatientInputDTO &dto,
                                                int patientId) {
  if (patientId <= 0)
    return "Mã số bệnh nhân không hợp lệ.";

  QString err;
  if (!(err = Validation::validateFullName(dto.fullName)).isEmpty()) return err;
  if (!dto.dateOfBirth.isValid() || dto.dateOfBirth > QDate::currentDate() || dto.dateOfBirth.year() < QDate::currentDate().year() - 150)
    return "Ngày sinh không hợp lệ (tuổi phải nhỏ hơn 150).";

  if (!(err = Validation::validateCitizenId(dto.citizenId)).isEmpty()) return err;
  if (!(err = Validation::validatePhoneNumber(dto.phone)).isEmpty()) return err;
  if (!(err = Validation::validateEmail(dto.email)).isEmpty()) return err;
  if (!(err = validateBloodType(dto.bloodType)).isEmpty()) return err;

  if (dto.address.isEmpty()) return "Địa chỉ không được để trống.";
  if (dto.address.length() > 255) return "Địa chỉ không được vượt quá 255 ký tự.";

  return "";
}

  // ─────────────────────────────────────────────────────────────────────────────
  // UpdatePatient
  // ─────────────────────────────────────────────────────────────────────────────

  bool PatientService::updatePatient(int patientId, PatientInputDTO &dto) {
    normalizePatientInput(dto);
    QString err = validateUpdateBaseInput(dto, patientId);
    if (!err.isEmpty()) {
      QMessageBox::warning(nullptr, "Validation Error", err);
      return false;
    }


    PatientUpdateDTO updateDto(dto, patientId);
    return m_patientRepository->updatePatient(updateDto);
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // updateOutPatient
  // ─────────────────────────────────────────────────────────────────────────────

  bool PatientService::updateOutPatient(int patientId, OutPatientInputDTO &dto,
                                        const QString &status) {
    normalizePatientInput(dto);
    QString err = validateUpdateBaseInput(dto, patientId);
    if (!err.isEmpty()) {
      QMessageBox::warning(nullptr, "Validation Error", err);
      return false;
    }


    OutPatientUpdateDTO updateDto(dto, patientId, status);

    if (!m_patientRepository->updatePatient(updateDto))
      return false;

    return m_patientRepository->updateOutPatient(updateDto);
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // updateInPatient
  // ─────────────────────────────────────────────────────────────────────────────

  bool PatientService::updateInPatient(int patientId, InPatientInputDTO &dto,
                                       const QString &status) {
    normalizePatientInput(dto);
    QString err = validateUpdateBaseInput(dto, patientId);
    if (!err.isEmpty()) {
      QMessageBox::warning(nullptr, "Validation Error", err);
      return false;
    }


    err = validateInPatientInput(dto);
    if (!err.isEmpty()) {
      QMessageBox::warning(nullptr, "Validation Error", err);
      return false;
    }

    InPatientUpdateDTO updateDto(dto, patientId, status);

    if (!m_patientRepository->updatePatient(updateDto))
      return false;

    return m_patientRepository->updateInPatient(updateDto);
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // updateEmergencyPatient
  // ─────────────────────────────────────────────────────────────────────────────

  bool PatientService::updateEmergencyPatient(
      int patientId, EmergencyPatientInputDTO &dto, const QString &status) {
    normalizePatientInput(dto);
    QString err = validateUpdateBaseInput(dto, patientId);
    if (!err.isEmpty()) {
      QMessageBox::warning(nullptr, "Validation Error", err);
      return false;
    }


    err = validateEmergencyPatientInput(dto);
    if (!err.isEmpty()) {
      QMessageBox::warning(nullptr, "Validation Error", err);
      return false;
    }

    EmergencyPatientUpdateDTO updateDto(dto, patientId, status);

    if (!m_patientRepository->updatePatient(updateDto))
      return false;

    return m_patientRepository->updateEmergencyPatient(updateDto);
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Search / Lọc bệnh nhân
  // ─────────────────────────────────────────────────────────────────────────────

  QList<PatientSearchResultDTO> PatientService::searchPatients(
      PatientSearchCriteria criteria) {
    normalizeSearchCriteria(criteria);
    QString err = validateDateRange(
        criteria.fromDate.value_or(QDate()), criteria.toDate.value_or(QDate()));
    if (!err.isEmpty()) {
      // In service layer, returning empty list or throw exception, but let's
      // keep previous logic
      return QList<PatientSearchResultDTO>();
    }

    return m_patientRepository->searchPatients(criteria);
  }

  std::optional<PatientDetailDTO> PatientService::getPatientById(
      int patientId) {
    if (patientId <= 0) {
      QMessageBox::warning(nullptr, "Lỗi", "ID bệnh nhân không hợp lệ.");
      return std::nullopt;
    }
    return m_patientRepository->getPatientById(patientId);
  }

  int PatientService::countSearchResults(
      PatientSearchCriteria criteria) {
    normalizeSearchCriteria(criteria);
    QString err = validateDateRange(
        criteria.fromDate.value_or(QDate()), criteria.toDate.value_or(QDate()));
    if (!err.isEmpty()) {
      return 0; // Trả về 0 nếu có lỗi validate ngày tháng
    }

    return m_patientRepository->countSearchResults(criteria);
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Xóa / Khôi phục bệnh nhân
  // ─────────────────────────────────────────────────────────────────────────────

  bool PatientService::softDeletePatient(int patientId) {
    return m_patientRepository->softDeletePatient(patientId);
  }

  bool PatientService::restorePatient(int patientId) {
    return m_patientRepository->restorePatient(patientId);
  }

  // ───────────────────────────────────────────────────────────────────────────────
  // Allergies & Insurance
  // ───────────────────────────────────────────────────────────────────────────────

  QList<AllergyResultDTO> PatientService::getAllergies(int patientId) {
    return m_patientRepository->getAllergiesByPatientId(patientId);
  }

  std::optional<InsuranceResultDTO> PatientService::getInsurance(
      int patientId) {
    return m_patientRepository->getInsuranceByPatientId(patientId);
  }

  bool PatientService::checkDrugAllergyConflict(int patientId,
                                                const QString &drugName) const {
    QString allergies =
        m_patientRepository->getAllergiesStringByPatientId(patientId);
    if (allergies.trimmed().isEmpty())
      return false;
    const QStringList allergyList = allergies.split(',', Qt::SkipEmptyParts);
    const QString target = drugName.trimmed().toLower();
    for (const QString &entry : allergyList) {
      if (entry.trimmed().toLower() == target)
        return true;
    }
    return false;
  }
