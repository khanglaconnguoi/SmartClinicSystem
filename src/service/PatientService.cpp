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
#include <QMessageBox>

// ───────────────────────────────────────────────────────────────────────────────
// Helpers: parse allergies string & insurance string
// ───────────────────────────────────────────────────────────────────────────────

/**
 * @brief Tách chuỗi allergies (phân cách bằng dấu phẩy) thành QList<AllergyInsertDTO>.
 *        Mỗi tỪn sau khi trim thành một AllergyInsertDTO với severity = 'MODERATE'.
 *        Tên rỗng sau khi trim bị bỏ qua.
 */
static QList<AllergyInsertDTO> parseAllergiesString(int patientId,
                                                    const QString &raw) {
  QList<AllergyInsertDTO> result;
  if (raw.trimmed().isEmpty()) return result;

  const QStringList parts = raw.split(',', Qt::SkipEmptyParts);
  for (const QString &part : parts) {
    const QString name = part.trimmed();
    if (name.isEmpty()) continue;
    AllergyInsertDTO item;
    item.patientId   = patientId;
    item.allergenName = name;
    item.severity    = "MODERATE";
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
  if (raw.trimmed().isEmpty()) return std::nullopt;

  InsuranceInsertDTO dto;
  dto.patientId     = patientId;
  dto.policyNumber  = raw.trimmed();
  dto.providerName  = "BHYT"; // mặc định
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

  if (type == PatientType::OUTPATIENT) {
    prefix = "OP";
    seqPtr = &outSeq;
  } else if (type == PatientType::INPATIENT) {
    prefix = "IP";
    seqPtr = &inSeq;
  } else if (type == PatientType::EMERGENCY) {
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

bool PatientService::AddOutPatient(
    int patientId, int doctorId, const QString &fullName,
    const QDate &dateOfBirth, const QString &gender, const QString &citizenId,
    const QString &phone, const QString &email, const QString &address,
    const QString &bloodType, const QString &allergies,
    const QString &insurance, PatientType type,
    const QString &emergencyContactName, const QString &emergencyContactPhone) {

  QString patientCode = generatePatientCode(type);

  QString err = PatientService::validateBaseInput(
      patientId, patientCode, fullName, dateOfBirth, gender, citizenId, phone,
      email, address, bloodType, allergies, insurance,
      PatientTypeToString(type), emergencyContactName, emergencyContactPhone);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  OutPatientInputDTO inputInformation;
  inputInformation.fullName = fullName;
  inputInformation.dateOfBirth = dateOfBirth;
  inputInformation.gender = stringToGender(gender);
  inputInformation.citizenId = citizenId;
  inputInformation.phone = phone;
  inputInformation.email = email;
  inputInformation.address = address;
  inputInformation.bloodType = bloodType;
  inputInformation.allergies = allergies;
  inputInformation.insurance = insurance;
  inputInformation.type = type;
  inputInformation.emergencyContactName = emergencyContactName;
  inputInformation.emergencyContactPhone = emergencyContactPhone;
  // doctorId <= 0 nghĩa là "chưa gán bác sĩ" → NULL trong DB
  inputInformation.doctorId = (doctorId > 0) ? std::optional<int>(doctorId) : std::nullopt;

  OutPatientInsertDTO dto(inputInformation, patientCode);

  // Giả định patientId tạm thời = 0, repository sẽ set lại sau insertBasePatient.
  // Parse allergies và insurance → gán vào dto để repository xử lý trong 1 transaction.
  dto.allergies = parseAllergiesString(0, allergies); // patientId sẽ được gán trong repo
  dto.insurance = parseInsuranceString(0, insurance);

  return m_patientRepository->insertOutPatient(dto);
}

// ─────────────────────────────────────────────────────────────────────────────
// AddInPatient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientService::AddInPatient(
    int patientId, const QString &fullName, const QDate &dateOfBirth,
    const QString &gender, const QString &citizenId, const QString &phone,
    const QString &email, const QString &address, const QString &bloodType,
    const QString &allergies, const QString &insurance, PatientType type,
    const QString &emergencyContactName, const QString &emergencyContactPhone,
    const QString &roomId, const QString &admittingDoctorId,
    const QDate &admissionDate, const QDate &dischargeDate,
    const QString &reason) {

  QString patientCode = generatePatientCode(type);

  QString err = PatientService::validateBaseInput(
      patientId, patientCode, fullName, dateOfBirth, gender, citizenId, phone,
      email, address, bloodType, allergies, insurance,
      PatientTypeToString(type), emergencyContactName, emergencyContactPhone);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  err = PatientService::validateInPatientInput(
      roomId, admittingDoctorId, admissionDate, dischargeDate, reason);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  InPatientInputDTO inputInformation;
  inputInformation.fullName = fullName;
  inputInformation.dateOfBirth = dateOfBirth;
  inputInformation.gender = stringToGender(gender);
  inputInformation.citizenId = citizenId;
  inputInformation.phone = phone;
  inputInformation.email = email;
  inputInformation.address = address;
  inputInformation.bloodType = bloodType;
  inputInformation.allergies = allergies;
  inputInformation.insurance = insurance;
  inputInformation.type = type;
  inputInformation.emergencyContactName = emergencyContactName;
  inputInformation.emergencyContactPhone = emergencyContactPhone;

  // roomId/doctorId rỗng hoặc "0" → NULL trong DB (tránh FK violation)
  {
    bool ok = false;
    int rid = roomId.toInt(&ok);
    inputInformation.roomId = (ok && rid > 0) ? std::optional<int>(rid) : std::nullopt;
  }
  {
    bool ok = false;
    int did = admittingDoctorId.toInt(&ok);
    inputInformation.doctorId = (ok && did > 0) ? std::optional<int>(did) : std::nullopt;
  }

  inputInformation.admissionDate = admissionDate;
  if (dischargeDate.isValid()) {
    inputInformation.dischargeDate = dischargeDate;
  } else {
    inputInformation.dischargeDate = std::nullopt;
  }

  inputInformation.reason = reason;

  InPatientInsertDTO dto(inputInformation, patientCode);
  dto.allergies = parseAllergiesString(0, allergies);
  dto.insurance = parseInsuranceString(0, insurance);
  return m_patientRepository->insertInPatient(dto);
}

// ─────────────────────────────────────────────────────────────────────────────
// AddEmergencyPatient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientService::AddEmergencyPatient(
    int patientId, const QString &fullName, const QDate &dateOfBirth,
    const QString &gender, const QString &citizenId, const QString &phone,
    const QString &email, const QString &address, const QString &bloodType,
    const QString &allergies, const QString &insurance, PatientType type,
    const QString &emergencyContactName, const QString &emergencyContactPhone,
    const QString &emergencyRoomId, const QString &emergencyDoctorId,
    const QString &injuryCause, const QString &injuryDescription,
    const QDate &admissionDate, const QDate &dischargeDate) {

  QString patientCode = generatePatientCode(type);

  QString err = PatientService::validateBaseInput(
      patientId, patientCode, fullName, dateOfBirth, gender, citizenId, phone,
      email, address, bloodType, allergies, insurance,
      PatientTypeToString(type), emergencyContactName, emergencyContactPhone);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  err = PatientService::validateEmergencyPatientInput(
      emergencyRoomId, emergencyDoctorId, injuryCause, injuryDescription,
      admissionDate, dischargeDate);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  EmergencyPatientInputDTO inputInformation;
  inputInformation.fullName = fullName;
  inputInformation.dateOfBirth = dateOfBirth;
  inputInformation.gender = stringToGender(gender);
  inputInformation.citizenId = citizenId;
  inputInformation.phone = phone;
  inputInformation.email = email;
  inputInformation.address = address;
  inputInformation.bloodType = bloodType;
  inputInformation.allergies = allergies;
  inputInformation.insurance = insurance;
  inputInformation.type = type;
  inputInformation.emergencyContactName = emergencyContactName;
  inputInformation.emergencyContactPhone = emergencyContactPhone;

  // roomId/doctorId rỗng hoặc "0" → NULL trong DB (tránh FK violation)
  {
    bool ok = false;
    int rid = emergencyRoomId.toInt(&ok);
    inputInformation.roomId = (ok && rid > 0) ? std::optional<int>(rid) : std::nullopt;
  }
  {
    bool ok = false;
    int did = emergencyDoctorId.toInt(&ok);
    inputInformation.doctorId = (ok && did > 0) ? std::optional<int>(did) : std::nullopt;
  }

  inputInformation.injuryCause = injuryCause;
  inputInformation.injuryDescription = injuryDescription;
  inputInformation.admissionDate = admissionDate;
  if (dischargeDate.isValid()) {
    inputInformation.dischargeDate = dischargeDate;
  } else {
    inputInformation.dischargeDate = std::nullopt;
  }

  EmergencyPatientInsertDTO dto(inputInformation, patientCode);
  dto.allergies = parseAllergiesString(0, allergies);
  dto.insurance = parseInsuranceString(0, insurance);
  return m_patientRepository->insertEmergencyPatient(dto);
}

// ─────────────────────────────────────────────────────────────────────────────
// Nhóm trường theo loại bệnh nhân
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::validateBaseInput(
    int patientId, const QString &patientCode, const QString &fullName,
    const QDate &dateOfBirth, const QString &gender, const QString &citizenId,
    const QString &phone, const QString &email, const QString &address,
    const QString &bloodType, const QString &/*allergies*/,
    const QString &/*insurance*/, const QString &type,
    const QString &emergencyContactName, const QString &emergencyContactPhone) {

  if (patientId < 0)
    return "Invalid patient ID.";
  if (patientCode.isEmpty())
    return "Patient code is required.";
  if (fullName.isEmpty())
    return "Full name is required.";
  if (dateOfBirth > QDate::currentDate())
    return "Date of birth is invalid.";
  if (gender.isEmpty())
    return "Gender is required.";

  QString err;
  err = Validation::validateCitizenId(citizenId);
  if (!err.isEmpty())
    return err;

  err = Validation::validatePhoneNumber(phone);
  if (!err.isEmpty())
    return err;

  err = Validation::validateEmail(email);
  if (!err.isEmpty())
    return err;

  if (address.isEmpty())
    return "Address is required.";

  err = Validation::validateBloodType(bloodType);
  if (!err.isEmpty())
    return err;

  // allergies và insurance là tùy chọn, không bắt buộc
  if (type.isEmpty())
    return "Patient type is required.";
  if (emergencyContactName.isEmpty())
    return "Emergency contact name is required.";

  err = Validation::validatePhoneNumber(emergencyContactPhone);
  if (!err.isEmpty())
    return err;

  return "";
}

QString PatientService::validateInPatientInput(const QString &roomId,
                                               const QString &doctorId,
                                               const QDate &admissionDate,
                                               const QDate &dischargeDate,
                                               const QString &reason) {

  if (roomId.isEmpty())
    return "Room ID is required.";
  if (doctorId.isEmpty())
    return "Doctor ID is required.";
  if (admissionDate > dischargeDate)
    return "Admission date must be before discharge date.";
  if (reason.isEmpty())
    return "Reason is required.";

  return "";
}

QString PatientService::validateEmergencyPatientInput(
    const QString &roomId, const QString &doctorId, const QString &injuryCause,
    const QString &injuryDescription, const QDate &admissionDate,
    const QDate &dischargeDate) {

  if (roomId.isEmpty())
    return "Emergency room ID is required.";
  if (doctorId.isEmpty())
    return "Emergency doctor ID is required.";
  if (injuryCause.isEmpty())
    return "Injury cause is required.";
  if (injuryDescription.isEmpty())
    return "Injury description is required.";
  if (admissionDate > dischargeDate)
    return "Admission date must be before discharge date.";

  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// validateUpdateBaseInput
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::validateUpdateBaseInput(
    int patientId, const QString &fullName, const QDate &dateOfBirth,
    const QString &gender, const QString &citizenId, const QString &phone,
    const QString &email, const QString &bloodType, const QString &/*allergies*/,
    const QString &/*insurance*/) {
  if (patientId <= 0)
    return "Invalid patient ID.";
  if (fullName.isEmpty())
    return "Full name is required.";
  if (!dateOfBirth.isValid() || dateOfBirth > QDate::currentDate())
    return "Date of birth is invalid.";
  if (gender.isEmpty())
    return "Gender is required.";
  // allergies và insurance là tùy chọn, không bắt buộc

  QString err;
  err = Validation::validateCitizenId(citizenId);
  if (!err.isEmpty())
    return err;

  err = Validation::validatePhoneNumber(phone);
  if (!err.isEmpty())
    return err;

  err = Validation::validateEmail(email);
  if (!err.isEmpty())
    return err;

  err = Validation::validateBloodType(bloodType);
  if (!err.isEmpty())
    return err;

  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdatePatient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientService::UpdatePatient(
    int patientId, const QString &fullName, const QDate &dateOfBirth,
    const QString &gender, const QString &citizenId, const QString &phone,
    const QString &email, const QString &address, const QString &bloodType,
    const QString &allergies, const QString &insurance,
    const QString &emergencyContactName, const QString &emergencyContactPhone) {
  QString err = validateUpdateBaseInput(patientId, fullName, dateOfBirth,
                                        gender, citizenId, phone, email,
                                        bloodType, allergies, insurance);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  PatientInputDTO inputInformation;
  inputInformation.fullName = fullName;
  inputInformation.dateOfBirth = dateOfBirth;
  inputInformation.gender = stringToGender(gender);
  inputInformation.citizenId = citizenId;
  inputInformation.phone = phone;
  inputInformation.email = email;
  inputInformation.address = address;
  inputInformation.bloodType = bloodType;
  inputInformation.emergencyContactName = emergencyContactName;
  inputInformation.emergencyContactPhone = emergencyContactPhone;

  PatientUpdateDTO dto(inputInformation, patientId);
  return m_patientRepository->updatePatient(dto);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateOutPatient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientService::UpdateOutPatient(
    int patientId, int doctorId, const QString &fullName,
    const QDate &dateOfBirth, const QString &gender, const QString &citizenId,
    const QString &phone, const QString &email, const QString &address,
    const QString &bloodType, const QString &allergies,
    const QString &insurance, const QString &emergencyContactName,
    const QString &emergencyContactPhone, const QString &status) {
  QString err = validateUpdateBaseInput(patientId, fullName, dateOfBirth,
                                        gender, citizenId, phone, email,
                                        bloodType, allergies, insurance);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  OutPatientInputDTO inputInformation;
  inputInformation.fullName = fullName;
  inputInformation.dateOfBirth = dateOfBirth;
  inputInformation.gender = stringToGender(gender);
  inputInformation.citizenId = citizenId;
  inputInformation.phone = phone;
  inputInformation.email = email;
  inputInformation.address = address;
  inputInformation.bloodType = bloodType;
  inputInformation.allergies = allergies;
  inputInformation.insurance = insurance;
  inputInformation.emergencyContactName = emergencyContactName;
  inputInformation.emergencyContactPhone = emergencyContactPhone;
  inputInformation.doctorId =
      doctorId > 0 ? std::optional<int>(doctorId) : std::nullopt;

  OutPatientUpdateDTO dto(inputInformation, patientId, status);

  if (!m_patientRepository->updatePatient(dto))
    return false;

  return m_patientRepository->updateOutPatient(dto);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateInPatient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientService::UpdateInPatient(
    int patientId, const QString &fullName, const QDate &dateOfBirth,
    const QString &gender, const QString &citizenId, const QString &phone,
    const QString &email, const QString &address, const QString &bloodType,
    const QString &allergies, const QString &insurance,
    const QString &emergencyContactName, const QString &emergencyContactPhone,
    const QString &roomId, const QString &doctorId, const QDate &admissionDate,
    const QDate &dischargeDate, const QString &reason, const QString &status) {
  QString err = validateUpdateBaseInput(patientId, fullName, dateOfBirth,
                                        gender, citizenId, phone, email,
                                        bloodType, allergies, insurance);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  err = validateInPatientInput(roomId, doctorId, admissionDate, dischargeDate,
                               reason);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  InPatientInputDTO inputInformation;
  inputInformation.fullName = fullName;
  inputInformation.dateOfBirth = dateOfBirth;
  inputInformation.gender = stringToGender(gender);
  inputInformation.citizenId = citizenId;
  inputInformation.phone = phone;
  inputInformation.email = email;
  inputInformation.address = address;
  inputInformation.bloodType = bloodType;
  inputInformation.allergies = allergies;
  inputInformation.insurance = insurance;
  inputInformation.emergencyContactName = emergencyContactName;
  inputInformation.emergencyContactPhone = emergencyContactPhone;

  inputInformation.roomId =
      roomId.isEmpty() ? std::nullopt : std::optional<int>(roomId.toInt());
  inputInformation.doctorId =
      doctorId.isEmpty() ? std::nullopt : std::optional<int>(doctorId.toInt());
  inputInformation.admissionDate = admissionDate;
  inputInformation.dischargeDate = dischargeDate.isValid()
                                       ? std::optional<QDate>(dischargeDate)
                                       : std::nullopt;
  inputInformation.reason = reason;

  InPatientUpdateDTO dto(inputInformation, patientId, status);

  if (!m_patientRepository->updatePatient(dto))
    return false;

  return m_patientRepository->updateInPatient(dto);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateEmergencyPatient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientService::UpdateEmergencyPatient(
    int patientId, const QString &fullName, const QDate &dateOfBirth,
    const QString &gender, const QString &citizenId, const QString &phone,
    const QString &email, const QString &address, const QString &bloodType,
    const QString &allergies, const QString &insurance,
    const QString &emergencyContactName, const QString &emergencyContactPhone,
    const QString &roomId, const QString &doctorId, const QString &injuryCause,
    const QString &injuryDescription, const QDate &admissionDate,
    const QDate &dischargeDate, const QString &status) {

  QString err = validateUpdateBaseInput(patientId, fullName, dateOfBirth,
                                        gender, citizenId, phone, email,
                                        bloodType, allergies, insurance);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  err = validateEmergencyPatientInput(roomId, doctorId, injuryCause,
                                      injuryDescription, admissionDate,
                                      dischargeDate);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  EmergencyPatientInputDTO inputInformation;
  inputInformation.fullName = fullName;
  inputInformation.dateOfBirth = dateOfBirth;
  inputInformation.gender = stringToGender(gender);
  inputInformation.citizenId = citizenId;
  inputInformation.phone = phone;
  inputInformation.email = email;
  inputInformation.address = address;
  inputInformation.bloodType = bloodType;
  inputInformation.allergies = allergies;
  inputInformation.insurance = insurance;
  inputInformation.emergencyContactName = emergencyContactName;
  inputInformation.emergencyContactPhone = emergencyContactPhone;

  inputInformation.roomId =
      roomId.isEmpty() ? std::nullopt : std::optional<int>(roomId.toInt());
  inputInformation.doctorId =
      doctorId.isEmpty() ? std::nullopt : std::optional<int>(doctorId.toInt());
  inputInformation.injuryCause = injuryCause;
  inputInformation.injuryDescription = injuryDescription;
  inputInformation.admissionDate = admissionDate;
  inputInformation.dischargeDate = dischargeDate.isValid()
                                       ? std::optional<QDate>(dischargeDate)
                                       : std::nullopt;

  EmergencyPatientUpdateDTO dto(inputInformation, patientId, status);

  if (!m_patientRepository->updatePatient(dto))
    return false;

  return m_patientRepository->updateEmergencyPatient(dto);
}

// ─────────────────────────────────────────────────────────────────────────────
// Search / Lọc bệnh nhân
// ─────────────────────────────────────────────────────────────────────────────

QVector<PatientSearchResultDTO>
PatientService::searchPatients(const PatientSearchCriteria &criteria) {
  QString err = Validation::validateDateRange(criteria.fromDate.value_or(QDate()),
                                  criteria.toDate.value_or(QDate()));
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return QVector<PatientSearchResultDTO>();
  }

  return m_patientRepository->searchPatients(criteria);
}

std::optional<PatientDetailDTO> PatientService::getPatientById(int patientId) {
  if (patientId <= 0) {
    QMessageBox::warning(nullptr, "Lỗi", "ID bệnh nhân không hợp lệ.");
    return std::nullopt;
  }
  return m_patientRepository->getPatientById(patientId);
}

int PatientService::countSearchResults(const PatientSearchCriteria &criteria) {
  QString err = Validation::validateDateRange(criteria.fromDate.value_or(QDate()),
                                  criteria.toDate.value_or(QDate()));
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

std::optional<InsuranceResultDTO> PatientService::getInsurance(int patientId) const {
    return m_patientRepository->getInsuranceByPatientId(patientId);
}

std::optional<DatabaseManager::PatientRecord> PatientService::getPatientByPhoneOrCitizenId(const QString &phone, const QString &citizenId) const {
    return m_patientRepository->getPatientByPhoneOrCitizenId(phone, citizenId);
}



bool PatientService::checkDrugAllergyConflict(int patientId, const QString &drugName) const {
  QString allergies = m_patientRepository->getAllergiesStringByPatientId(patientId);
  if (allergies.trimmed().isEmpty()) return false;
  const QStringList allergyList = allergies.split(',', Qt::SkipEmptyParts);
  const QString target = drugName.trimmed().toLower();
  for (const QString &entry : allergyList) {
    if (entry.trimmed().toLower() == target) return true;
  }
  return false;
}
