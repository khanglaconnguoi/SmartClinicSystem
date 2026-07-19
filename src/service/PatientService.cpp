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

  err = validateAllergyInputList(dto.allergies);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  err = validateInsuranceInput(dto.insurance);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  OutPatientInsertDTO insertDto(dto, patientCode);
  return m_patientRepository->insertOutPatient(insertDto);
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
  normalizeAllergyInputList(dto.allergies);
  normalizeInsuranceInput(dto.insurance);
  dto.emergencyContactName = dto.emergencyContactName.simplified();
  dto.emergencyContactPhone = dto.emergencyContactPhone.trimmed();
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
  dto.severity = dto.severity.trimmed().toUpper();
  dto.notes = dto.notes.simplified();
}

void PatientService::normalizeAllergyInputList(QList<AllergyInputDTO> &list) {
  for (AllergyInputDTO &allergy : list)
    normalizeAllergyInput(allergy);
}

void PatientService::normalizeInsuranceInput(InsuranceInputDTO &dto) {
  dto.providerName = dto.providerName.simplified();
  dto.policyNumber = dto.policyNumber.trimmed().toUpper();
  dto.insuranceType = dto.insuranceType.trimmed().toUpper();
  dto.notes = dto.notes.simplified();
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
  // allergenName: bắt buộc, tối đa 200 ký tự
  if (dto.allergenName.isEmpty())
    return "Tên chất gây dị ứng không được để trống.";
  if (dto.allergenName.length() > 200)
    return "Tên chất gây dị ứng không được vượt quá 200 ký tự.";

  // severity: phải là một trong ba giá trị cố định

  if (dto.severity.isEmpty())
    return "Mức độ dị ứng (severity) không được để trống.";
  if (!SeverityText::isValid(dto.severity))
    return "Mức độ dị ứng không hợp lệ.";

  // notes: tùy chọn, nhưng giới hạn 500 ký tự
  if (dto.notes.length() > 500)
    return "Ghi chú dị ứng không được vượt quá 500 ký tự.";

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

  if (dto.providerName.isEmpty())
    return "Tên công ty bảo hiểm không được để trống.";
  if (dto.providerName.length() > 200)
    return "Tên công ty bảo hiểm không được vượt quá 200 ký tự.";

  // policyNumber: bắt buộc, tối đa 100 ký tự
  if (dto.policyNumber.isEmpty())
    return "Số thẻ / hợp đồng bảo hiểm không được để trống.";
  if (dto.policyNumber.length() > 100)
    return "Số thẻ / hợp đồng không được vượt quá 100 ký tự.";

  // insuranceType: phải là NATIONAL / COMMERCIAL / OTHER
  if (dto.insuranceType.isEmpty())
    return "Loại bảo hiểm không được để trống.";
  if (!InsuraceTypeText::isValid(dto.insuranceType))
    return "Loại bảo hiểm không hợp lệ.";

  // coveragePercent: [0.0, 100.0]
  if (dto.coveragePercent < 0.0 || dto.coveragePercent > 100.0)
    return "Tỉ lệ chi trả phải nằm trong khoảng [0, 100].";

  if (!validateDateRange(dto.validFrom, dto.validTo).isEmpty())
    return "Ngày hiệu lực phải nhỏ hơn hoặc "
           "bằng ngày hết hạn.";

  // notes: tùy chọn, tối đa 500 ký tự
  if (dto.notes.length() > 500)
    return "Ghi chú bảo hiểm không được vượt quá 500 ký tự.";

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

  err = validateAllergyInputList(dto.allergies);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  err = validateInsuranceInput(dto.insurance);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  InPatientInsertDTO insertDto(dto, patientCode);
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

  err = validateAllergyInputList(dto.allergies);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  err = validateInsuranceInput(dto.insurance);
  if (!err.isEmpty()) {
    QMessageBox::warning(nullptr, "Validation Error", err);
    return false;
  }

  EmergencyPatientInsertDTO insertDto(dto, patientCode);
  return m_patientRepository->insertEmergencyPatient(insertDto);
}

// ─────────────────────────────────────────────────────────────────────────────
// Nhóm trường theo loại bệnh nhân
// ─────────────────────────────────────────────────────────────────────────────



QString PatientService::validateBaseInput(const PatientInputDTO &dto,
                                          const QString &patientCode) {
  QString err;
  if (!(err = Validation::validateTrimmedNotEmpty(patientCode, "Mã bệnh nhân không được để trống.")).isEmpty()) return err;
  if (!(err = Validation::validateFullName(dto.fullName)).isEmpty()) return err;
  if (dto.dateOfBirth > QDate::currentDate() || dto.dateOfBirth.year() < QDate::currentDate().year() - 150)
    return "Ngày sinh không hợp lệ (tuổi phải nhỏ hơn 150)";

  if (!(err = Validation::validateCitizenId(dto.citizenId)).isEmpty())
    return err;
  if (!(err = Validation::validatePhoneNumber(dto.phone)).isEmpty())
    return err;
  if (!(err = Validation::validateEmail(dto.email)).isEmpty())
    return err;

  if (dto.address.isEmpty())
    return "Địa chỉ không được để trống.";
  if (dto.address.length() > 255)
    return "Địa chỉ không được vượt quá 255 ký tự.";

  if (!(err = validateBloodType(dto.bloodType)).isEmpty()) return err;
  if (!(err = Validation::validateTrimmedNotEmpty(dto.emergencyContactName, "Người liên hệ khẩn cấp không được để trống.")).isEmpty()) return err;
  if (!(err = Validation::validatePhoneNumber(dto.emergencyContactPhone)).isEmpty()) return err;

  return "";
}

QString PatientService::validateInPatientRoomId(int roomId) {
  if (roomId <= 0)
    return "Mã phòng không hợp lệ.";
  return "";
}

QString PatientService::validateInPatientDoctorId(int doctorId) {
  if (doctorId <= 0)
    return "Bác sĩ phụ trách không hợp lệ.";
  return "";
}

QString PatientService::validateInPatientDischargeDate(
    const QDate &admissionDate, std::optional<QDate> dischargeDate) {
  if (dischargeDate.has_value() && admissionDate > dischargeDate.value())
    return "Ngày nhập viện phải nhỏ hơn hoặc bằng ngày xuất viện.";
  return "";
}

QString PatientService::validateInPatientReason(const QString &reason) {
  QString err;
  if (!(err = Validation::validateTrimmedNotEmpty(reason, "Lý do nhập viện không được để trống.")).isEmpty())
    return err;
  if (reason.trimmed().length() > 1000)
    return "Lý do nhập viện không được vượt quá 1000 ký tự.";
  return "";
}

QString PatientService::validateInPatientInput(const InPatientInputDTO &dto) {
  QString err;
  if (!(err = validateInPatientRoomId(dto.roomId)).isEmpty())
    return err;
  if (!(err = validateInPatientDoctorId(dto.doctorId)).isEmpty())
    return err;
  if (!(err = validateInPatientDischargeDate(dto.admissionDate,
                                             dto.dischargeDate))
           .isEmpty())
    return err;
  if (!(err = validateInPatientReason(dto.reason)).isEmpty())
    return err;

  return "";
}

QString PatientService::validateEmergencyRoomId(int roomId) {
  if (roomId < 0)
    return "Id Phòng cấp cứu không hợp lệ.";
  return "";
}

QString PatientService::validateEmergencyDoctorId(int doctorId) {
  if (doctorId < 0)
    return "ID bác sĩ không hợp lệ";
  return "";
}

QString PatientService::validateEmergencyDischargeDate(
    const QDate &admissionDate, std::optional<QDate> dischargeDate) {
  if (dischargeDate.has_value() && admissionDate > dischargeDate.value())
    return "Ngày nhập viện phải nhỏ hơn hoặc bằng ngày xuất viện.";
  return "";
}

QString PatientService::validateEmergencyInjuryCause(const QString &cause) {
  QString err;
  if (!(err = Validation::validateTrimmedNotEmpty(cause, "Nguyên nhân chấn thương không được để trống.")).isEmpty())
    return err;
  if (cause.trimmed().length() > 255)
    return "Nguyên nhân chấn thương không được vượt quá 255 ký tự.";
  return "";
}

QString PatientService::validateEmergencyInjuryDescription(const QString &desc) {
  QString err;
  if (!(err = Validation::validateTrimmedNotEmpty(desc, "Mô tả chấn thương không được để trống.")).isEmpty())
    return err;
  if (desc.trimmed().length() > 1000)
    return "Mô tả chấn thương không được vượt quá 1000 ký tự.";
  return "";
}

QString PatientService::validateEmergencyPatientInput(
    const EmergencyPatientInputDTO &dto) {
  QString err;
  if (!(err = validateEmergencyRoomId(dto.roomId)).isEmpty())
    return err;
  if (!(err = validateEmergencyDoctorId(dto.doctorId)).isEmpty())
    return err;
  if (!(err = validateEmergencyInjuryCause(dto.injuryCause)).isEmpty())
    return err;
  if (!(err = validateEmergencyInjuryDescription(dto.injuryDescription))
           .isEmpty())
    return err;
  if (!(err = validateEmergencyDischargeDate(dto.admissionDate,
                                             dto.dischargeDate))
           .isEmpty())
    return err;

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
  if (!dto.dateOfBirth.isValid() || dto.dateOfBirth > QDate::currentDate() ||
      dto.dateOfBirth.year() < QDate::currentDate().year() - 150)
    return "Ngày sinh không hợp lệ (tuổi phải nhỏ hơn 150).";

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
  if (dto.address.length() > 255)
    return "Địa chỉ không được vượt quá 255 ký tự.";

  return "";
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

bool PatientService::updateEmergencyPatient(int patientId,
                                            EmergencyPatientInputDTO &dto,
                                            const QString &status) {
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

QList<PatientSearchResultDTO>
PatientService::searchPatients(PatientSearchCriteria criteria) {
  normalizeSearchCriteria(criteria);
  QString err = validateDateRange(criteria.fromDate.value_or(QDate()),
                                  criteria.toDate.value_or(QDate()));
  if (!err.isEmpty()) {
    // In service layer, returning empty list or throw exception, but let's
    // keep previous logic
    return QList<PatientSearchResultDTO>();
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

int PatientService::countSearchResults(PatientSearchCriteria criteria) {
  normalizeSearchCriteria(criteria);
  QString err = validateDateRange(criteria.fromDate.value_or(QDate()),
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

std::optional<InsuranceResultDTO> PatientService::getInsurance(int patientId) const {
    return m_patientRepository->getInsuranceByPatientId(patientId);
}

std::optional<DatabaseManager::PatientRecord> PatientService::getPatientByPhoneOrCitizenId(const QString &phone, const QString &citizenId) const {
    return m_patientRepository->getPatientByPhoneOrCitizenId(phone, citizenId);
}



std::optional<InsuranceResultDTO>
PatientService::getInsurance(int patientId) const {
  return m_patientRepository->getInsuranceByPatientId(patientId);
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