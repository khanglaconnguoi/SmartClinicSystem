/**
 * @file    PatientService.cpp
 * @brief   Implementación của PatientService.
 *          Validation được uỷ quyền cho PatientValidation (Validation.h).
 */

#include "PatientService.h"
#include "Validation.h"
#include "dto/PatientDTOs.h"
#include "model/CommonEnums.h"
#include "repository/PatientRepository.h"
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>

// ─────────────────────────────────────────────────────────────────────────────
// Sinh mã bệnh nhân
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::generatePatientCode(PatientType type) {
  QString prefix;
  if (type == PatientType::Outpatient) {
    prefix = "OP";
  } else if (type == PatientType::Inpatient) {
    prefix = "IP";
  } else if (type == PatientType::Emergency) {
    prefix = "EP";
  } else {
    prefix = "PT";
  }

  QString dateStr = QDateTime::currentDateTime().toString("yyyyMMdd");
  QString prefixWithDate = prefix + dateStr;

  int seq = 1;
  if (m_patientRepository) {
    auto result = m_patientRepository->getLatestPatientCode(prefixWithDate);
    if (result.has_value()) {
      QString latestCode = result.value();
      QString countCode = latestCode.right(4);
      seq = countCode.toInt() + 1;
    }
  }

  QString code = QString("%1%2%3").arg(prefix).arg(dateStr).arg(seq, 4, 10, QChar('0'));
  return code;
}

// ─────────────────────────────────────────────────────────────────────────────
// Normalize
// ─────────────────────────────────────────────────────────────────────────────

void PatientService::normalizePatientInput(PatientInputDTO &dto) {
  dto.fullName = dto.fullName.simplified();
  dto.citizenId = dto.citizenId.trimmed();
  dto.phone = dto.phone.trimmed();
  dto.email = dto.email.trimmed();
  dto.address = dto.address.simplified();
  dto.bloodType = dto.bloodType.trimmed().toUpper();
//   normalizeAllergyInputList(dto.allergies);
  normalizeInsuranceInput(dto.insurance);
  dto.emergencyContactName = dto.emergencyContactName.simplified();
  dto.emergencyContactPhone = dto.emergencyContactPhone.trimmed();
}

void PatientService::normalizeInPatientInput(InPatientInputDTO &dto) {
  normalizePatientInput(dto);
  dto.reason = dto.reason.trimmed();
}

void PatientService::normalizeOutPatientInput(OutPatientInputDTO &dto) {
  normalizePatientInput(dto);
}

void PatientService::normalizeEmergencyPatientInput(EmergencyPatientInputDTO &dto) {
  normalizePatientInput(dto);
  dto.injuryCause = dto.injuryCause.trimmed();
  dto.injuryDescription = dto.injuryDescription.trimmed();
}

void PatientService::normalizeSearchCriteria(PatientSearchCriteria &criteria) {
  criteria.searchKey = criteria.searchKey.simplified();
  criteria.status = criteria.status.trimmed();
}

// ─────────────────────────────────────────────────────────────────────────────
// Normalize – Allergy & Insurance
// ─────────────────────────────────────────────────────────────────────────────

void PatientService::normalizeAllergyInput(AllergyInputDTO &dto) {
  dto.allergenName = dto.allergenName.simplified();
  dto.notes = dto.notes.trimmed();
}

void PatientService::normalizeAllergyInputList(QList<AllergyInputDTO> &list) {
  for (AllergyInputDTO &allergy : list)
    normalizeAllergyInput(allergy);
}

void PatientService::normalizeInsuranceInput(InsuranceInputDTO &dto) {
  dto.providerName = dto.providerName.simplified();
  dto.policyNumber = dto.policyNumber.trimmed().toUpper();
  dto.insuranceType = dto.insuranceType.trimmed().toUpper();
  dto.notes = dto.notes.trimmed();
}

// ─────────────────────────────────────────────────────────────────────────────
// Mapping Helpers
// ─────────────────────────────────────────────────────────────────────────────

PatientInsertDTO PatientService::mapPatientToInsertDTO(
    const PatientInputDTO &input, const QString &patientCode, const QString &patientType) {
  PatientInsertDTO dto;
  dto.patientCode = patientCode;
  dto.fullName = input.fullName;
  dto.dateOfBirth = input.dateOfBirth.toString("yyyy-MM-dd");
  dto.gender = input.gender;
  dto.citizenId = input.citizenId;
  dto.phone = input.phone;
  dto.email = input.email;
  dto.address = input.address;
  dto.bloodType = input.bloodType;
  dto.type = patientType;
  dto.emergencyContactName = input.emergencyContactName;
  dto.emergencyContactPhone = input.emergencyContactPhone;
//   dto.allergies = input.allergies;

  const auto &ins = input.insurance;
  dto.insurance.providerName = ins.providerName;
  dto.insurance.policyNumber = ins.policyNumber;
  dto.insurance.insuranceType = ins.insuranceType;
  dto.insurance.coveragePercent = ins.coveragePercent;
  dto.insurance.validFrom = ins.validFrom.isValid() ? ins.validFrom.toString("yyyy-MM-dd") : QString();
  dto.insurance.validTo = ins.validTo.isValid() ? ins.validTo.toString("yyyy-MM-dd") : QString();
  dto.insurance.notes = ins.notes;

  return dto;
}

OutPatientInsertDTO PatientService::mapOutPatientToInsertDTO(
    const OutPatientInputDTO &input, const QString &patientCode) {
  OutPatientInsertDTO dto;
  static_cast<PatientInsertDTO&>(dto) = mapPatientToInsertDTO(input, patientCode, patientTypeToEn(PatientType::Outpatient));
  dto.doctorId = input.doctorId;
  dto.status = outPatientStateToEn(OutPatientState::Registered);
  return dto;
}

InPatientInsertDTO PatientService::mapInPatientToInsertDTO(
    const InPatientInputDTO &input, const QString &patientCode) {
  InPatientInsertDTO dto;
  static_cast<PatientInsertDTO&>(dto) = mapPatientToInsertDTO(input, patientCode, patientTypeToEn(PatientType::Inpatient));
  dto.roomId = input.roomId;
  dto.doctorId = input.doctorId;
  dto.admissionDate = input.admissionDate.toString("yyyy-MM-dd");
  dto.dischargeDate = input.dischargeDate.value_or(QDate()).toString("yyyy-MM-dd");
  dto.reason = input.reason;
  dto.status = inPatientStateToEn(InPatientState::Admitted);
  return dto;
}

EmergencyPatientInsertDTO PatientService::mapEmergencyPatientToInsertDTO(
    const EmergencyPatientInputDTO &input, const QString &patientCode) {
  EmergencyPatientInsertDTO dto;
  static_cast<PatientInsertDTO&>(dto) = mapPatientToInsertDTO(input, patientCode, patientTypeToEn(PatientType::Emergency));
  dto.roomId = input.roomId;
  dto.doctorId = input.doctorId;
  dto.injuryCause = input.injuryCause;
  dto.injuryDescription = input.injuryDescription;
  dto.admissionDate = input.admissionDate.toString("yyyy-MM-dd");
  dto.dischargeDate = input.dischargeDate.value_or(QDate()).toString("yyyy-MM-dd");
  dto.status = emergencyPatientStateToEn(EmergencyPatientState::Emergency);
  return dto;
}

PatientUpdateDTO PatientService::mapPatientToUpdateDTO(
    const PatientInputDTO &input, int patientId) {
  PatientUpdateDTO dto;
  dto.patientId = patientId;
  dto.fullName = input.fullName;
  dto.dateOfBirth = input.dateOfBirth.toString("yyyy-MM-dd");
  dto.gender = input.gender;
  dto.citizenId = input.citizenId;
  dto.phone = input.phone;
  dto.email = input.email;
  dto.address = input.address;
  dto.bloodType = input.bloodType;
  dto.emergencyContactName = input.emergencyContactName;
  dto.emergencyContactPhone = input.emergencyContactPhone;
  return dto;
}

OutPatientUpdateDTO PatientService::mapOutPatientToUpdateDTO(
    const OutPatientInputDTO &input, int patientId, const QString &status) {
  OutPatientUpdateDTO dto;
  static_cast<PatientUpdateDTO&>(dto) = mapPatientToUpdateDTO(input, patientId);
  dto.status = status;
  dto.doctorId = input.doctorId;
  return dto;
}

InPatientUpdateDTO PatientService::mapInPatientToUpdateDTO(
    const InPatientInputDTO &input, int patientId, const QString &status) {
  InPatientUpdateDTO dto;
  static_cast<PatientUpdateDTO&>(dto) = mapPatientToUpdateDTO(input, patientId);
  dto.roomId = input.roomId;
  dto.doctorId = input.doctorId;
  dto.admissionDate = input.admissionDate.toString("yyyy-MM-dd");
  dto.dischargeDate = input.dischargeDate.value_or(QDate()).toString("yyyy-MM-dd");
  dto.reason = input.reason;
  dto.status = status;
  return dto;
}

EmergencyPatientUpdateDTO PatientService::mapEmergencyPatientToUpdateDTO(
    const EmergencyPatientInputDTO &input, int patientId, const QString &status) {
  EmergencyPatientUpdateDTO dto;
  static_cast<PatientUpdateDTO&>(dto) = mapPatientToUpdateDTO(input, patientId);
  dto.roomId = input.roomId;
  dto.doctorId = input.doctorId;
  dto.injuryCause = input.injuryCause;
  dto.injuryDescription = input.injuryDescription;
  dto.admissionDate = input.admissionDate.toString("yyyy-MM-dd");
  dto.dischargeDate = input.dischargeDate.value_or(QDate()).toString("yyyy-MM-dd");
  dto.status = status;
  return dto;
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

// ─────────────────────────────────────────────────────────────────────────────
// Validate – Allergy
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::validateAllergyInput(const AllergyInputDTO &dto) {
  // allergenName: bắt buộc
  if (dto.allergenName.isEmpty())
    return "Tên chất gây dị ứng không được để trống.";

  return "";
}

QString
PatientService::validateAllergyInputList(const QList<AllergyInputDTO> &list) {
  for (int i = 0; i < list.size(); ++i) {
    QString err = validateAllergyInput(list[i]);
    if (!err.isEmpty())
      return QString("Dị ứng #%1: %2").arg(i + 1).arg(err);
  }
  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// Validate – Insurance
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::validateInsuranceInput(const InsuranceInputDTO &dto) {
  // providerName: bắt buộc nếu bất kỳ trường nào khác được điền
  const bool hasAnyData = !dto.providerName.isEmpty() ||
                          !dto.policyNumber.isEmpty() ||
                          !dto.insuranceType.isEmpty();

  if (!hasAnyData)
    return ""; // toàn bộ rỗng → không có bảo hiểm, bỏ qua

  QString err;
  if (!(err = Validation::validateTrimmedNotEmpty(dto.providerName, "Tên công ty bảo hiểm không được để trống.")).isEmpty())
    return err;

  // policyNumber: bắt buộc
  if (!(err = Validation::validateTrimmedNotEmpty(dto.policyNumber, "Số thẻ / hợp đồng bảo hiểm không được để trống.")).isEmpty())
    return err;

  // insuranceType: phải là NATIONAL / COMMERCIAL / OTHER
  if (!(err = Validation::validateTrimmedNotEmpty(dto.insuranceType, "Loại bảo hiểm không được để trống.")).isEmpty())
    return err;
  if (!InsuranceTypeText::isValid(dto.insuranceType))
    return "Loại bảo hiểm không hợp lệ.";

  // coveragePercent: [0.0, 100.0]
  if (dto.coveragePercent < 0.0 || dto.coveragePercent > 100.0)
    return "Tỉ lệ chi trả phải nằm trong khoảng [0, 100].";

  if (!Validation::validateDateRange(dto.validFrom, dto.validTo).isEmpty())
    return "Ngày hiệu lực phải nhỏ hơn hoặc "
           "bằng ngày hết hạn.";

  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// AddOutPatient
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::addOutPatient(OutPatientInputDTO &dto) {
  normalizeOutPatientInput(dto);
  QString patientCode = generatePatientCode(dto.type);

  QString err = validateBaseInput(dto, patientCode);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (Base):" << err;
    return err;
  }

  OutPatientInsertDTO insertDto = mapOutPatientToInsertDTO(dto, patientCode);
  if (!m_patientRepository->insertOutPatient(insertDto)) {
    return "Không thể lưu bệnh nhân ngoại trú vào cơ sở dữ liệu.";
  }
  return "";
}

QString PatientService::addInPatient(InPatientInputDTO &dto) {
  normalizeInPatientInput(dto);
  QString patientCode = generatePatientCode(dto.type);

  QString err = validateBaseInput(dto, patientCode);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (Base):" << err;
    return err;
  }

  err = validateInPatientInput(dto);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (InPatient):" << err;
    return err;
  }

  InPatientInsertDTO insertDto = mapInPatientToInsertDTO(dto, patientCode);
  if (!m_patientRepository->insertInPatient(insertDto)) {
    return "Không thể lưu bệnh nhân nội trú vào cơ sở dữ liệu.";
  }
  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// addEmergencyPatient
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::addEmergencyPatient(EmergencyPatientInputDTO &dto) {
  normalizeEmergencyPatientInput(dto);
  QString patientCode = generatePatientCode(dto.type);

  QString err = validateBaseInput(dto, patientCode);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (Base):" << err;
    return err;
  }

  err = validateEmergencyPatientInput(dto);
  if (!err.isEmpty()) {
    qDebug() << "Validation Error (Emergency):" << err;
    return err;
  }

  EmergencyPatientInsertDTO insertDto = mapEmergencyPatientToInsertDTO(dto, patientCode);
  if (!m_patientRepository->insertEmergencyPatient(insertDto)) {
    return "Không thể lưu bệnh nhân cấp cứu vào cơ sở dữ liệu.";
  }
  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// Nhóm trường theo loại bệnh nhân
// ─────────────────────────────────────────────────────────────────────────────



QString PatientService::validateBaseInput(const PatientInputDTO &dto,
                                          const QString &patientCode) {
  QString err;
  if (!(err = Validation::validateTrimmedNotEmpty(patientCode, "Mã bệnh nhân không được để trống.")).isEmpty()) return err;
  if (!(err = Validation::validateFullName(dto.fullName)).isEmpty()) return err;
  if (!(err = Validation::validateDateOfBirth(dto.dateOfBirth)).isEmpty()) return err;

  if (!(err = Validation::validateCitizenId(dto.citizenId)).isEmpty())
    return err;
  if (!(err = Validation::validatePhoneNumber(dto.phone)).isEmpty())
    return err;
  if (!(err = Validation::validateEmail(dto.email)).isEmpty())
    return err;
  if (!(err = Validation::validateTrimmedNotEmpty(dto.address, "Address is required.")).isEmpty())
    return err;

  if (!(err = validateCitizenIdUnique(dto.citizenId)).isEmpty()) return err;
  if (!(err = validatePhoneNumberUnique(dto.phone)).isEmpty()) return err;

  if (!(err = validateBloodType(dto.bloodType)).isEmpty()) return err;
  if (!(err = Validation::validateTrimmedNotEmpty(dto.emergencyContactName, "Người liên hệ khẩn cấp không được để trống.")).isEmpty()) return err;
  if (!(err = Validation::validatePhoneNumber(dto.emergencyContactPhone)).isEmpty()) return err;

  if (!(err = validateInsuranceInput(dto.insurance)).isEmpty()) return err;

  return "";
}

QString PatientService::validateInPatientInput(const InPatientInputDTO &dto) {
  QString err;
  if (!(err = Validation::validateValidId(dto.roomId, "Mã phòng không hợp lệ.")).isEmpty())
    return err;
  if (!(err = Validation::validateValidId(dto.doctorId, "Bác sĩ phụ trách không hợp lệ.")).isEmpty())
    return err;
  if (!(err = validateInPatientDischargeDate(dto.admissionDate,
                                             dto.dischargeDate))
           .isEmpty())
    return err;
  if (!(err = Validation::validateTrimmedNotEmpty(dto.reason, "Lý do nhập viện không được để trống.")).isEmpty())
    return err;

  return "";
}

QString PatientService::validateEmergencyPatientInput(
    const EmergencyPatientInputDTO &dto) {
  QString err;
  if (!(err = Validation::validateValidId(dto.roomId, "Mã phòng không hợp lệ.")).isEmpty())
    return err;
  if (!(err = Validation::validateValidId(dto.doctorId, "Bác sĩ phụ trách không hợp lệ.")).isEmpty())
    return err;
  if (!(err = Validation::validateTrimmedNotEmpty(dto.injuryCause, "Nguyên nhân chấn thương không được để trống.")).isEmpty())
    return err;
  if (!(err = Validation::validateTrimmedNotEmpty(dto.injuryDescription, "Mô tả chấn thương không được để trống.")).isEmpty())
    return err;
  if (!(err = validateEmergencyDischargeDate(dto.admissionDate,
                                             dto.dischargeDate))
           .isEmpty())
    return err;

  return "";
}

QString PatientService::validateInPatientDischargeDate(
    const QDate &admissionDate, std::optional<QDate> dischargeDate) {
  if (dischargeDate.has_value() && admissionDate > dischargeDate.value())
    return "Ngày nhập viện phải nhỏ hơn hoặc bằng ngày xuất viện.";
  return "";
}

QString PatientService::validateEmergencyDischargeDate(
    const QDate &admissionDate, std::optional<QDate> dischargeDate) {
  if (dischargeDate.has_value() && admissionDate > dischargeDate.value())
    return "Ngày nhập viện phải nhỏ hơn hoặc bằng ngày xuất viện.";
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
  if (!(err = Validation::validateFullName(dto.fullName)).isEmpty())
    return err;
  if (!(err = Validation::validateDateOfBirth(dto.dateOfBirth)).isEmpty())
    return err;

  if (!(err = Validation::validateCitizenId(dto.citizenId)).isEmpty())
    return err;
  if (!(err = Validation::validatePhoneNumber(dto.phone)).isEmpty())
    return err;
  if (!(err = Validation::validateEmail(dto.email)).isEmpty())
    return err;
  if (!(err = validateBloodType(dto.bloodType)).isEmpty())
    return err;

  if (dto.address.isEmpty())
    return "Địa chỉ không được để trống.";

  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// update patient
// ─────────────────────────────────────────────────────────────────────────────

QString PatientService::updateOutPatient(int patientId, OutPatientInputDTO &dto,
                                         const QString &status) {
  normalizeOutPatientInput(dto);
  QString err = validateUpdateBaseInput(dto, patientId);
  if (!err.isEmpty()) {
    return err;
  }
  
  OutPatientUpdateDTO updateDto = mapOutPatientToUpdateDTO(dto, patientId, status);

  if (!m_patientRepository->updateOutPatient(updateDto)) {
    return "Không thể cập nhật thông tin bệnh nhân ngoại trú.";
  }

  return "";
}

QString PatientService::updateInPatient(int patientId, InPatientInputDTO &dto,
                                        const QString &status) {
  normalizeInPatientInput(dto);
  QString err = validateUpdateBaseInput(dto, patientId);
  if (!err.isEmpty()) {
    return err;
  }

  err = validateInPatientInput(dto);
  if (!err.isEmpty()) {
    return err;
  }

  InPatientUpdateDTO updateDto = mapInPatientToUpdateDTO(dto, patientId, status);

  if (!m_patientRepository->updateInPatient(updateDto)) {
    return "Không thể cập nhật thông tin bệnh nhân nội trú.";
  }

  return "";
}

QString PatientService::updateEmergencyPatient(int patientId,
                                               EmergencyPatientInputDTO &dto,
                                               const QString &status) {
  normalizeEmergencyPatientInput(dto);
  QString err = validateUpdateBaseInput(dto, patientId);
  if (!err.isEmpty()) {
    return err;
  }

  err = validateEmergencyPatientInput(dto);
  if (!err.isEmpty()) {
    return err;
  }

  EmergencyPatientUpdateDTO updateDto = mapEmergencyPatientToUpdateDTO(dto, patientId, status);

  if (!m_patientRepository->updateEmergencyPatient(updateDto)) {
    return "Không thể cập nhật thông tin bệnh nhân cấp cứu.";
  }

  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// Search / Lọc bệnh nhân
// ─────────────────────────────────────────────────────────────────────────────

PagedResult<PatientSearchResultDTO>
PatientService::searchPatientsPaged(PatientSearchCriteria criteria) const {
  normalizeSearchCriteria(criteria);
  criteria.searchKey = criteria.searchKey.simplified();
  criteria.status = criteria.status.trimmed();
  criteria.page = qMax(1, criteria.page);
  criteria.pageSize = qBound(0, criteria.pageSize, 200);

  QString err = Validation::validateDateRange(criteria.fromDate.value_or(QDate()),
                                              criteria.toDate.value_or(QDate()));
  if (!err.isEmpty()) {
    PagedResult<PatientSearchResultDTO> emptyResult;
    emptyResult.page = criteria.page;
    emptyResult.pageSize = criteria.pageSize;
    emptyResult.totalCount = 0;
    return emptyResult;
  }

  return m_patientRepository->searchPatientsPaged(criteria);
}

std::optional<PatientDetailDTO> PatientService::getPatientById(int patientId) {
  if (patientId <= 0) {
    qWarning() << "Lỗi: ID bệnh nhân không hợp lệ:" << patientId;
    return std::nullopt;
  }
  return m_patientRepository->getPatientById(patientId);
}

/*
QList<PatientSearchResultDTO> PatientService::searchPatients(PatientSearchCriteria criteria) { ... }
int PatientService::countSearchResults(PatientSearchCriteria criteria) { ... }
*/



// ─────────────────────────────────────────────────────────────────────────────
// Xóa / Khôi phục bệnh nhân
// ─────────────────────────────────────────────────────────────────────────────

bool PatientService::softDeletePatient(int patientId) {
  return m_patientRepository->softDeletePatient(patientId);
}

bool PatientService::restorePatient(int patientId) {
  return m_patientRepository->restorePatient(patientId);
}

QString PatientService::addAllergiesToPatient(int patientId, QList<AllergyInputDTO> allergies) {
  QString err;
  if (!(err = Validation::validateValidId(patientId, "ID bệnh nhân không hợp lệ.")).isEmpty())
    return err;

  if (!m_patientRepository->getPatientById(patientId).has_value())
    return "Không tìm thấy bệnh nhân trong hệ thống.";

  if (allergies.isEmpty())
    return "Danh sách dị ứng không được để trống.";

  normalizeAllergyInputList(allergies);

  if (!(err = validateAllergyInputList(allergies)).isEmpty())
    return err;

  if (!m_patientRepository->insertAllergies(patientId, allergies))
    return "Lỗi hệ thống khi lưu thông tin dị ứng. Vui lòng thử lại.";

  return "";
}

QList<AllergyResultDTO> PatientService::getAllergies(int patientId) {
  if (patientId <= 0)
    return {};
  return m_patientRepository->getAllergiesByPatientId(patientId);
}

std::optional<InsuranceResultDTO> PatientService::getInsurance(int patientId) const {
    return m_patientRepository->getInsuranceByPatientId(patientId);
}

double PatientService::getInsuranceCoveragePercent(int patientId) const {
  QString err = Validation::validateValidId(patientId, "ID bệnh nhân không hợp lệ.");
  if (!err.isEmpty()) {
    qWarning() << "PatientService::getInsuranceCoveragePercent validation failed:" << err;
    return 0.0;
  }
  return m_patientRepository->getInsuranceCoveragePercent(patientId);
}

std::optional<PatientShortDTO> PatientService::getPatientByPhoneOrCitizenId(const QString &phone, const QString &citizenId) const {
    return m_patientRepository->getPatientByPhoneOrCitizenId(phone, citizenId);
}

QString PatientService::validateCitizenIdUnique(const QString &citizenId, int excludePatientId) const {
  if (citizenId.trimmed().isEmpty()) return "";
  if (m_patientRepository && m_patientRepository->existsByCitizenId(citizenId.trimmed(), excludePatientId)) {
    auto exist = m_patientRepository->getPatientByPhoneOrCitizenId("", citizenId.trimmed());
    if (exist.has_value()) {
      return QString("Số CCCD [%1] đã tồn tại trên hệ thống (Bệnh nhân: %2 - Mã: %3).")
          .arg(citizenId.trimmed(), exist->fullName, exist->patientCode);
    }
    return QString("Số CCCD [%1] đã tồn tại trên hệ thống.").arg(citizenId.trimmed());
  }
  return "";
}

QString PatientService::validatePhoneNumberUnique(const QString &phone, int excludePatientId) const {
  if (phone.trimmed().isEmpty()) return "";
  if (m_patientRepository && m_patientRepository->existsByPhoneNumber(phone.trimmed(), excludePatientId)) {
    auto exist = m_patientRepository->getPatientByPhoneOrCitizenId(phone.trimmed(), "");
    if (exist.has_value()) {
      return QString("Số điện thoại cá nhân [%1] đã tồn tại trên hệ thống (Bệnh nhân: %2 - Mã: %3).")
          .arg(phone.trimmed(), exist->fullName, exist->patientCode);
    }
    return QString("Số điện thoại cá nhân [%1] đã tồn tại trên hệ thống.").arg(phone.trimmed());
  }
  return "";
}

// std::optional<DatabaseManager::PatientRecord>
// PatientService::getPatientByPhoneOrCitizenId(const QString &phone,
//                                              const QString &citizenId) const
//                                              {
//   return m_patientRepository->getPatientByPhoneOrCitizenId(phone, citizenId);
// }

// bool PatientService::checkDrugAllergyConflict(int patientId,
//                                               const QString &drugName) const
//                                               {
//   QString allergies =
//       m_patientRepository->getAllergiesStringByPatientId(patientId);
//   if (allergies.trimmed().isEmpty())
//     return false;
//   const QStringList allergyList = allergies.split(',', Qt::SkipEmptyParts);
//   const QString target = drugName.trimmed().toLower();
//   for (const QString &entry : allergyList) {
//     if (entry.trimmed().toLower() == target)
//       return true;
//   }
//   return false;
// }