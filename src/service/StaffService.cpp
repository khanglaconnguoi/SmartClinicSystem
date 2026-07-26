#include "StaffService.h"

#include <QDate>
#include <QPixmap>
#include <QRandomGenerator>
#include <QRegularExpression>

#include "Validation.h"
#include "ext/bcrypt.h"
#include "model/SystemUser.h"

static constexpr qsizetype NUMBER_BASE = 10;
static constexpr qsizetype LAST_TWO_DIGITS_FACTOR = 100;
static constexpr qsizetype RANDOM_PASSWORD_LENGTH = 16;
static constexpr qsizetype LEGAL_WORKING_AGE = 18;
static constexpr unsigned int PASSWORD_HASH_COST_FACTOR = 12;

// -- Field chung cho moi role ────────────────────────────────────
/**
 * @brief dateOfBirth: khong duoc tuong lai, phai it nhat 18 tuoi
 *        (Nhan vien phai du tuoi lao dong)
 */
QString StaffService::validateDateOfBirth(const QDate &dateOfBirth) {
  QString err = Validation::validateDateOfBirth(dateOfBirth);
  if (!err.isEmpty()) {
    return err;
  }

  QDate today = QDate::currentDate();
  // Kiểm tra đủ 18 tuổi lao động
  if (dateOfBirth.addYears(LEGAL_WORKING_AGE) > today) {
    return QString("Staff member must be at least 18 years old.")
        .arg(LEGAL_WORKING_AGE);
  }
  return "";
}

// /**
//  * @brief shift: phai la 1 trong 4 gia tri hop le
//  *        "MORNING" | "AFTERNOON" | "NIGHT" | "FULL_DAY"
//  */
// QString StaffService::validateShift(const QString& shift) {
//     if (shift.isEmpty()) {
//         return "Shift selection is required.";
//     }
//     if (shift != "MORNING" && shift != "AFTERNOON" && shift != "NIGHT" &&
//     shift != "FULL_DAY") {
//         return "Invalid shift value selected.";
//     }
//     return "";
// }

// -- Field dac thu Doctor ────────────────────────────────────────

/**
 * @brief licenseNumber: khong duoc rong, chi chua chu va so
 *        (Dinh dang so chung chi hanh nghe VN)
 */
QString StaffService::validateLicenseNumber(const QString &licenseNumber) {
  QString err;
  if (!(err = Validation::validateTrimmedNotEmpty(
            licenseNumber, "Medical license number is required."))
           .isEmpty()) {
    return err;
  }
  // Giới hạn chỉ chứa chữ và số (Ký tự alphanumeric)
  static const QRegularExpression alphanumericRegex("^[a-zA-Z0-9]+$");
  if (!alphanumericRegex.match(licenseNumber.trimmed()).hasMatch()) {
    return "License number must contain only letters and digits.";
  }
  return "";
}

/** @brief experienceYears: phai >= 0 */
QString StaffService::validateExperienceYears(int experienceYears) {
  if (experienceYears < 0) {
    return "Experience years cannot be negative.";
  }
  return "";
}

/** @brief consultationFee: phai >= 0 */
QString StaffService::validateConsultationFee(int consultationFee) {
  if (consultationFee < 0) {
    return "Consultation fee cannot be negative.";
  }
  return "";
}

// -- Field dac thu Nurse ─────────────────────────────────────────
/**
 * @brief nurseLevel: phai la 1 trong 3 gia tri hop le
 *        "JUNIOR" | "SENIOR" | "HEAD"
 */
QString StaffService::validateNurseLevel(const QString &nurseLevel) {
  if (nurseLevel.isEmpty()) {
    return "Nurse level selection is required.";
  }
  if (nurseLevel != "JUNIOR" && nurseLevel != "SENIOR" &&
      nurseLevel != "HEAD") {
    return "Invalid nurse level value selected.";
  }
  return "";
}

// =================================================================
// UNIQUENESS VALIDATORS — public non-static (Cần liên kết với DB)
// =================================================================

/**
 * @brief Kiem tra citizenId co bi trung trong bang staff khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validateCitizenIdUnique(const QString &citizenId,
                                              int excludeStaffId) const {
  if (m_staffRepository->existsByCitizenId(citizenId.trimmed(),
                                           excludeStaffId)) {
    return "This Citizen ID is already registered in the system.";
  }
  return "";
}

/**
 * @brief Kiem tra phoneNumber co bi trung trong bang staff khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validatePhoneNumberUnique(const QString &phoneNumber,
                                                int excludeStaffId) const {
  if (phoneNumber.length() == Validation::LANDLINE_PHONE_NUMBER_LENGTH) {
    return "";
  }
  if (m_staffRepository->existsByPhoneNumber(phoneNumber.trimmed(),
                                             excludeStaffId)) {
    return "This Phone number is already registered in the system.";
  }
  return "";
}

/**
 * @brief Kiem tra email co bi trung trong bang staff khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validateEmailUnique(const QString &email,
                                          int excludeStaffId) const {
  if (m_staffRepository->existsByEmail(email.trimmed(), excludeStaffId)) {
    return "This Email address is already in use.";
  }
  return "";
}

/**
 * @brief Kiem tra licenseNumber co bi trung trong bang doctor_profiles khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validateLicenseNumberUnique(const QString &licenseNumber,
                                                  int excludeStaffId) const {
  if (m_staffRepository->existsByLicenseNumber(licenseNumber.trimmed(),
                                               excludeStaffId)) {
    return "This License number is already registered.";
  }
  return "";
}

// =================================================================
// AGGREGATE VALIDATORS (private) - Checklist tổng trước khi Insert/Update
// =================================================================

QString StaffService::validateStaffBaseInput(const StaffInputDTO &staff,
                                             int staffId) {
  QString err;

  if (!(err = Validation::validateFullName(staff.fullName)).isEmpty())
    return err;
  if (!(err = Validation::validateDateOfBirth(staff.dateOfBirth)).isEmpty())
    return err;
  if (!(err = Validation::validateCitizenId(staff.citizenId)).isEmpty())
    return err;
  if (!(err = Validation::validatePhoneNumber(staff.phoneNumber)).isEmpty())
    return err;
  if (!(err = Validation::validateEmail(staff.email)).isEmpty())
    return err;
  if (!(err = Validation::validateTrimmedNotEmpty(staff.address,
                                                  "Address is required."))
           .isEmpty())
    return err;
  if (!(err = Validation::validateValidId(staff.departmentId,
                                          "Please select a valid department."))
           .isEmpty())
    return err;
  // if (!(err = validateShift(staff.shift)).isEmpty()) return err;

  // Chạy tiếp bộ check trùng lặp (Mặc định cho trường hợp INSERT, không loại
  // trừ ID nào) Lưu ý: Nếu làm tính năng UPDATE, bạn cần truyền staffId vào
  // aggregate validator này.
  if (!(err = validateCitizenIdUnique(staff.citizenId, staffId)).isEmpty())
    return err;
  if (!(err = validatePhoneNumberUnique(staff.phoneNumber, staffId)).isEmpty())
    return err;
  if (!(err = validateEmailUnique(staff.email, staffId)).isEmpty())
    return err;

  if (!(err = validateEmailUnique(staff.email, staffId)).isEmpty())
    return err;

  return "";
}

QString StaffService::validateDoctorInput(const DoctorInputDTO &doctor,
                                          int staffId) {
  // Check các trường cơ bản trước
  QString err = validateStaffBaseInput(doctor, staffId);
  if (!err.isEmpty())
    return err;

  // Check các trường đặc thù của Doctor
  if (!(err = Validation::validateTrimmedNotEmpty(
            doctor.specialty, "Specialty field is required for doctors."))
           .isEmpty())
    return err;
  if (!(err = validateLicenseNumber(doctor.licenseNumber)).isEmpty())
    return err;
  if (!(err = validateExperienceYears(doctor.experienceYears)).isEmpty())
    return err;
  if (!(err = validateConsultationFee(doctor.consultationFee)).isEmpty())
    return err;

  // Check trùng số chứng chỉ hành nghề của Doctor dưới DB
  if (!(err = validateLicenseNumberUnique(doctor.licenseNumber, staffId))
           .isEmpty())
    return err;

  return "";
}

QString StaffService::validateNurseInput(const NurseInputDTO &nurse,
                                         int staffId) {
  // Check các trường cơ bản trước
  QString err = validateStaffBaseInput(nurse, staffId);
  if (!err.isEmpty())
    return err;

  // Check các trường đặc thù của Nurse
  if (!(err = validateNurseLevel(nurse.nurseLevel)).isEmpty())
    return err;

  return "";
}

QString
StaffService::validatePharmacistInput(const PharmacistInputDTO &pharmacist,
                                      int staffId) {
  // Check các trường cơ bản trước
  QString err = validateStaffBaseInput(pharmacist, staffId);
  if (!err.isEmpty())
    return err;

  // Check các trường đặc thù của Pharmacist
  if (!(err = validateLicenseNumber(pharmacist.licenseNumber)).isEmpty())
    return err;
  // if (!(err = validatePharmacySection(pharmacist.pharmacySection)).isEmpty())
  // return err;
  if (!(err = validateExperienceYears(pharmacist.experienceYears)).isEmpty())
    return err;

  // Check trùng số chứng chỉ hành nghề của Pharmacist dưới DB
  if (!(err = validateLicenseNumberUnique(pharmacist.licenseNumber, staffId))
           .isEmpty())
    return err;

  return "";
}

// =================================================================
// NORMALIZE — trim all QString fields in-place before validation
// =================================================================

void StaffService::normalizeStaffInput(StaffInputDTO &dto) {
  dto.fullName = dto.fullName.simplified();
  dto.citizenId = dto.citizenId.trimmed();
  dto.phoneNumber = dto.phoneNumber.trimmed();
  dto.email = dto.email.trimmed();
  dto.address = dto.address.simplified();
  dto.shift = dto.shift.trimmed();
}

void StaffService::normalizeDoctorInput(DoctorInputDTO &dto) {
  normalizeStaffInput(dto); // implicit base-ref binding, no cast needed
  dto.specialty = dto.specialty.trimmed();
  dto.licenseNumber = dto.licenseNumber.trimmed();
  dto.bio = dto.bio.trimmed();
}

void StaffService::normalizeNurseInput(NurseInputDTO &dto) {
  normalizeStaffInput(dto);
  dto.nurseLevel = dto.nurseLevel.trimmed();
  dto.certification = dto.certification.trimmed();
}

void StaffService::normalizePharmacistInput(PharmacistInputDTO &dto) {
  normalizeStaffInput(dto);
  dto.licenseNumber = dto.licenseNumber.trimmed();
  dto.pharmacySection = dto.pharmacySection.trimmed();
}

// =================================================================
// MAPPING HELPERS — InputDTO → repo-level InsertDTO (no trimming)
// =================================================================

StaffInsertDTO StaffService::mapStaffToInsertDTO(const StaffInputDTO &input,
                                                 const QString &staffCode,
                                                 const QString &passwordHash,
                                                 UserRole role) {
  StaffInsertDTO dto;
  dto.staffCode = staffCode;
  dto.passwordHash = passwordHash;
  dto.fullName = input.fullName;
  dto.role = userRoleToEn(role);
  dto.gender = input.gender;
  dto.dateOfBirth = input.dateOfBirth.toString("yyyy-MM-dd");
  dto.citizenId = input.citizenId;
  dto.phoneNumber = input.phoneNumber;
  dto.email = input.email;
  dto.address = input.address;
  dto.departmentId = input.departmentId;
  dto.hireDate = QDate::currentDate().toString("yyyy-MM-dd");
  dto.shift = input.shift;
  if (!input.avatar.isNull()) {
    QBuffer buffer(&dto.avatarBytes);
    buffer.open(QIODevice::WriteOnly);
    input.avatar.save(&buffer, "PNG");
  }
  return dto;
}

DoctorInsertDTO
StaffService::mapDoctorToInsertDTO(const DoctorInputDTO &input,
                                   const QString &staffCode,
                                   const QString &passwordHash) {
  DoctorInsertDTO dto;
  // Populate base fields via the Staff helper
  static_cast<StaffInsertDTO &>(dto) =
      mapStaffToInsertDTO(input, staffCode, passwordHash, UserRole::Doctor);
  dto.specialty = input.specialty;
  dto.licenseNumber = input.licenseNumber;
  dto.experienceYears = input.experienceYears;
  dto.consultationFee = input.consultationFee;
  dto.bio = input.bio;
  return dto;
}

NurseInsertDTO StaffService::mapNurseToInsertDTO(const NurseInputDTO &input,
                                                 const QString &staffCode,
                                                 const QString &passwordHash) {
  NurseInsertDTO dto;
  static_cast<StaffInsertDTO &>(dto) =
      mapStaffToInsertDTO(input, staffCode, passwordHash, UserRole::Nurse);
  dto.nurseLevel = input.nurseLevel;
  dto.certification = input.certification;
  return dto;
}

PharmacistInsertDTO
StaffService::mapPharmacistToInsertDTO(const PharmacistInputDTO &input,
                                       const QString &staffCode,
                                       const QString &passwordHash) {
  PharmacistInsertDTO dto;
  static_cast<StaffInsertDTO &>(dto) =
      mapStaffToInsertDTO(input, staffCode, passwordHash, UserRole::Pharmacist);
  dto.licenseNumber = input.licenseNumber;
  dto.pharmacySection = input.pharmacySection;
  dto.experienceYears = input.experienceYears;
  return dto;
}

StaffUpdateDTO StaffService::mapStaffToUpdateDTO(const StaffInputDTO &dto,
                                                 int staffId) {
  StaffUpdateDTO updateDto;
  updateDto.staffId = staffId;
  updateDto.fullName = dto.fullName;
  if (!dto.avatar.isNull()) {
    QBuffer buffer(&updateDto.avatarBytes);
    buffer.open(QIODevice::WriteOnly);
    dto.avatar.save(&buffer, "PNG");
  }
  updateDto.gender = dto.gender;
  updateDto.dateOfBirth = dto.dateOfBirth.toString("yyyy-MM-dd");
  updateDto.citizenId = dto.citizenId;
  updateDto.phoneNumber = dto.phoneNumber;
  updateDto.email = dto.email;
  updateDto.address = dto.address;
  updateDto.departmentId = dto.departmentId;
  updateDto.shift = dto.shift;
  return updateDto;
}

DoctorUpdateDTO StaffService::mapDoctorToUpdateDTO(const DoctorInputDTO &dto,
                                                   int staffId) {
  DoctorUpdateDTO updateDto;
  static_cast<StaffUpdateDTO &>(updateDto) = mapStaffToUpdateDTO(dto, staffId);
  updateDto.specialty = dto.specialty;
  updateDto.licenseNumber = dto.licenseNumber;
  updateDto.experienceYears = dto.experienceYears;
  updateDto.consultationFee = dto.consultationFee;
  updateDto.bio = dto.bio;
  return updateDto;
}

NurseUpdateDTO StaffService::mapNurseToUpdateDTO(const NurseInputDTO &dto,
                                                 int staffId) {
  NurseUpdateDTO updateDto;
  static_cast<StaffUpdateDTO &>(updateDto) = mapStaffToUpdateDTO(dto, staffId);
  updateDto.nurseLevel = dto.nurseLevel;
  updateDto.certification = dto.certification;
  return updateDto;
}

PharmacistUpdateDTO
StaffService::mapPharmacistToUpdateDTO(const PharmacistInputDTO &dto,
                                       int staffId) {
  PharmacistUpdateDTO updateDto;
  static_cast<StaffUpdateDTO &>(updateDto) = mapStaffToUpdateDTO(dto, staffId);
  updateDto.licenseNumber = dto.licenseNumber;
  updateDto.pharmacySection = dto.pharmacySection;
  updateDto.experienceYears = dto.experienceYears;
  return updateDto;
}

QString StaffService::generateStaffCode(UserRole role) const {
  int year = QDate::currentDate().year();
  QString yearCode = QString("%1").arg(year % LAST_TWO_DIGITS_FACTOR, 2,
                                       NUMBER_BASE, QChar('0'));
  QString roleCode;
  switch (role) {
  case UserRole::Admin:
    roleCode = 'A';
    break;
  case UserRole::Doctor:
    roleCode = 'D';
    break;
  case UserRole::Nurse:
    roleCode = 'N';
    break;
  case UserRole::Receptionist:
    roleCode = 'R';
    break;
  case UserRole::Pharmacist:
    roleCode = 'P';
    break;
  }

  int count = 1;
  std::optional<QString> result =
      m_staffRepository->getLatestStaffCodeByYear(year);
  if (result.has_value()) {
    QString countCode = result.value().right(2);
    count = countCode.toInt() + 1;
  }

  QString newCountCode = QString("%1").arg(count, 2, NUMBER_BASE, QChar('0'));

  QString staffCode = roleCode + yearCode + newCountCode;
  return staffCode;
}

QString StaffService::generateRandomPassword() const {
  static QString chars =
      R"(0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz)";
  static QString specialChars = R"(#@)";
  QRandomGenerator *generator = QRandomGenerator::global();

  QString password(RANDOM_PASSWORD_LENGTH, ' ');

  for (int count = 0; count < 2;) {
    auto i = generator->bounded(RANDOM_PASSWORD_LENGTH);
    if (password[i] != ' ')
      continue;
    auto charIndex = generator->bounded(specialChars.length());
    password[i] = specialChars[charIndex];
    ++count;
  }

  for (int i = 0; i < RANDOM_PASSWORD_LENGTH; ++i) {
    if (password[i] != ' ')
      continue;
    auto charIndex = generator->bounded(chars.length());
    password[i] = chars[charIndex];
  }

  return password;
}

StaffHireResult StaffService::hireNewDoctor(DoctorInputDTO doctor) {
  normalizeDoctorInput(doctor);
  QString validationError = validateDoctorInput(doctor);
  if (!validationError.isEmpty()) {
    return StaffHireResult{validationError, "", ""};
  }

  const QString staffCode = generateStaffCode(UserRole::Doctor);
  const QString plainPassword = generateRandomPassword();
  const QString passwordHash = QString::fromStdString(bcrypt::generateHash(
      plainPassword.toStdString(), PASSWORD_HASH_COST_FACTOR));

  DoctorInsertDTO insertDto =
      mapDoctorToInsertDTO(doctor, staffCode, passwordHash);
  if (!this->m_staffRepository->insertDoctor(insertDto)) {
    return StaffHireResult{"Failed to insert doctor into the database.", "",
                           ""};
  }
  return StaffHireResult{"", staffCode, plainPassword};
}

StaffHireResult StaffService::hireNewNurse(NurseInputDTO nurse) {
  normalizeNurseInput(nurse);
  QString validationError = validateNurseInput(nurse);
  if (!validationError.isEmpty()) {
    return StaffHireResult{validationError, "", ""};
  }

  const QString staffCode = generateStaffCode(UserRole::Nurse);
  const QString plainPassword = generateRandomPassword();
  const QString passwordHash = QString::fromStdString(bcrypt::generateHash(
      plainPassword.toStdString(), PASSWORD_HASH_COST_FACTOR));

  NurseInsertDTO insertDto =
      mapNurseToInsertDTO(nurse, staffCode, passwordHash);
  if (!this->m_staffRepository->insertNurse(insertDto)) {
    return StaffHireResult{"Failed to insert nurse into the database.", "", ""};
  }
  return StaffHireResult{"", staffCode, plainPassword};
}

// StaffHireResult StaffService::hireNewReceptionist(StaffInputDTO receptionist)
// {
//     normalizeStaffInput(receptionist);
//     QString validationError = validateStaffBaseInput(receptionist);
//     if (!validationError.isEmpty()) { return StaffHireResult{validationError,
//     "", ""}; }

//     const QString staffCode = generateStaffCode(UserRole::Receptionist);
//     const QString plainPassword = generateRandomPassword();
//     const QString passwordHash = QString::fromStdString(bcrypt::generateHash(
//             plainPassword.toStdString(), PASSWORD_HASH_COST_FACTOR));

//     StaffInsertDTO insertDto =
//             mapStaffToInsertDTO(receptionist, staffCode, passwordHash,
//             UserRole::Receptionist);
//     if (!this->m_staffRepository->insertStaff(insertDto)) {
//         return StaffHireResult{"Failed to insert receptionist into the
//         database.", "", ""};
//     }
//     return StaffHireResult{"", staffCode, plainPassword};
// }

StaffHireResult StaffService::hireNewPharmacist(PharmacistInputDTO pharmacist) {
  normalizePharmacistInput(pharmacist);
  QString validationError = validatePharmacistInput(pharmacist);
  if (!validationError.isEmpty()) {
    return StaffHireResult{validationError, "", ""};
  }

  const QString staffCode = generateStaffCode(UserRole::Pharmacist);
  const QString plainPassword = generateRandomPassword();
  const QString passwordHash = QString::fromStdString(bcrypt::generateHash(
      plainPassword.toStdString(), PASSWORD_HASH_COST_FACTOR));

  PharmacistInsertDTO insertDto =
      mapPharmacistToInsertDTO(pharmacist, staffCode, passwordHash);
  if (!this->m_staffRepository->insertPharmacist(insertDto)) {
    return StaffHireResult{"Failed to insert pharmacist into the database.", "",
                           ""};
  }
  return StaffHireResult{"", staffCode, plainPassword};
}

QString StaffService::editStaffBaseInformation(StaffInputDTO staffInformation,
                                               int staffId) {
  normalizeStaffInput(staffInformation);
  QString err = validateStaffBaseInput(staffInformation, staffId);
  if (!err.isEmpty())
    return err;
  StaffUpdateDTO updateDto = mapStaffToUpdateDTO(staffInformation, staffId);
  if (!m_staffRepository->updateStaff(updateDto))
    return "Repository update failed (DB error).";
  return "";
}

QString StaffService::editDoctorInformation(DoctorInputDTO doctorInformation,
                                            int staffId) {
  normalizeDoctorInput(doctorInformation);
  QString err = validateDoctorInput(doctorInformation, staffId);
  if (!err.isEmpty())
    return err;
  DoctorUpdateDTO updateDto = mapDoctorToUpdateDTO(doctorInformation, staffId);
  if (!m_staffRepository->updateDoctor(updateDto))
    return "Repository update failed (DB error).";
  return "";
}

QString StaffService::editNurseInformation(NurseInputDTO nurseInformation,
                                           int staffId) {
  normalizeNurseInput(nurseInformation);
  QString err = validateNurseInput(nurseInformation, staffId);
  if (!err.isEmpty())
    return err;
  NurseUpdateDTO updateDto = mapNurseToUpdateDTO(nurseInformation, staffId);
  if (!m_staffRepository->updateNurse(updateDto))
    return "Repository update failed (DB error).";
  return "";
}

QString StaffService::editPharmacistInformation(
    PharmacistInputDTO pharmacistInformation, int staffId) {
  normalizePharmacistInput(pharmacistInformation);
  QString err = validatePharmacistInput(pharmacistInformation, staffId);
  if (!err.isEmpty())
    return err;
  PharmacistUpdateDTO updateDto =
      mapPharmacistToUpdateDTO(pharmacistInformation, staffId);
  if (!m_staffRepository->updatePharmacist(updateDto))
    return "Repository update failed (DB error).";
  return "";
}

QList<std::shared_ptr<SystemUser>>
StaffService::searchDoctors(QString searchKey, QString specialty,
                            int departmentId, QString shift, bool onlyActive,
                            bool includeDeleted) const {
  StaffSearchCriteria criteria;
  criteria.searchKey = searchKey.trimmed();
  criteria.role = UserRole::Doctor;
  criteria.specialty = specialty.trimmed();
  criteria.departmentId = departmentId;
  criteria.shift = shift.trimmed();
  criteria.onlyActive = onlyActive;
  criteria.includeDeleted = includeDeleted;

  return m_staffRepository->search(criteria);
}

QList<std::shared_ptr<SystemUser>>
StaffService::searchNurses(QString searchKey, int departmentId, bool onlyActive,
                           bool includeDeleted) const {
  StaffSearchCriteria criteria;
  criteria.searchKey = searchKey.trimmed();
  criteria.role = UserRole::Nurse;
  criteria.departmentId = departmentId;
  criteria.onlyActive = onlyActive;
  criteria.includeDeleted = includeDeleted;

  return m_staffRepository->search(criteria);
}

QList<std::shared_ptr<SystemUser>>
StaffService::searchReceptionists(QString searchKey, bool onlyActive,
                                  bool includeDeleted) const {
  StaffSearchCriteria criteria;
  criteria.searchKey = searchKey.trimmed();
  criteria.role = UserRole::Receptionist;
  criteria.onlyActive = onlyActive;
  criteria.includeDeleted = includeDeleted;

  return m_staffRepository->search(criteria);
}

bool StaffService::changePassword(int staffId, const QString &plainPassword) {
  if (staffId <= 0)
    return false;
  if (!Validation::validatePlainPassword(plainPassword).isEmpty())
    return false;

  auto user = m_staffRepository->findById(staffId);
  if (!user)
    return false;
  if (user->verifyPassword(plainPassword))
    return false;

  QString passwordHash = QString::fromStdString(bcrypt::generateHash(
      plainPassword.toStdString(), PASSWORD_HASH_COST_FACTOR));
  return m_staffRepository->updatePasswordInformation(staffId, passwordHash);
}

ResetPasswordResult StaffService::resetPassword(int staffId) {
  if (staffId <= 0 || !m_staffRepository->existsByStaffId(staffId))
    return ResetPasswordResult{false, ""};

  QString password = generateRandomPassword();
  QString passwordHash = QString::fromStdString(
      bcrypt::generateHash(password.toStdString(), PASSWORD_HASH_COST_FACTOR));

  bool result =
      m_staffRepository->updatePasswordInformation(staffId, passwordHash, true);
  if (!result)
    return ResetPasswordResult{false, ""};
  return ResetPasswordResult{true, password};
}