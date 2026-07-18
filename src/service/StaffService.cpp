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
QString StaffService::validateDateOfBirth(const QDate& dateOfBirth) {
    QString err = Validation::validateDateOfBirth(dateOfBirth);
    if (!err.isEmpty()) {
        return err;
    }
    
    QDate today = QDate::currentDate();
    // Kiểm tra đủ 18 tuổi lao động
    if (dateOfBirth.addYears(LEGAL_WORKING_AGE) > today) {
        return QString("Staff member must be at least 18 years old.").arg(LEGAL_WORKING_AGE);
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
//     if (shift != "MORNING" && shift != "AFTERNOON" && shift != "NIGHT" && shift != "FULL_DAY") {
//         return "Invalid shift value selected.";
//     }
//     return "";
// }


// -- Field dac thu Doctor ────────────────────────────────────────

/**
 * @brief licenseNumber: khong duoc rong, chi chua chu va so
 *        (Dinh dang so chung chi hanh nghe VN)
 */
QString StaffService::validateLicenseNumber(const QString& licenseNumber) {
    QString err;
    if (!(err = Validation::validateTrimmedNotEmpty(licenseNumber, "Medical license number is required.")).isEmpty()) {
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
QString StaffService::validateNurseLevel(const QString& nurseLevel) {
    if (nurseLevel.isEmpty()) {
        return "Nurse level selection is required.";
    }
    if (nurseLevel != "JUNIOR" && nurseLevel != "SENIOR" && nurseLevel != "HEAD") {
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
QString StaffService::validateCitizenIdUnique(const QString& citizenId, int excludeStaffId) const {
    if (m_staffRepository->existsByCitizenId(citizenId.trimmed(), excludeStaffId)) {
        return "This Citizen ID is already registered in the system.";
    }
    return "";
}

/**
 * @brief Kiem tra phoneNumber co bi trung trong bang staff khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validatePhoneNumberUnique(const QString& phoneNumber, int excludeStaffId) const {
    if (phoneNumber.length() == Validation::LANDLINE_PHONE_NUMBER_LENGTH) {
        return "";
    }
    if (m_staffRepository->existsByPhoneNumber(phoneNumber.trimmed(), excludeStaffId)) {
        return "This Phone number is already registered in the system.";
    }
    return "";
}

/**
 * @brief Kiem tra email co bi trung trong bang staff khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validateEmailUnique(const QString& email, int excludeStaffId) const {
    if (m_staffRepository->existsByEmail(email.trimmed(), excludeStaffId)) {
        return "This Email address is already in use.";
    }
    return "";
}

/**
 * @brief Kiem tra licenseNumber co bi trung trong bang doctor_profiles khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validateLicenseNumberUnique(const QString& licenseNumber, int excludeStaffId) const {
    if (m_staffRepository->existsByLicenseNumber(licenseNumber.trimmed(), excludeStaffId)) {
        return "This License number is already registered.";
    }
    return "";
}


// =================================================================
// AGGREGATE VALIDATORS (private) - Checklist tổng trước khi Insert/Update
// =================================================================

QString StaffService::validateStaffBaseInput(const StaffInputDTO& staff, int staffId) {
    QString err;
    
    if (!(err = Validation::validateFullName(staff.fullName)).isEmpty()) return err;
    if (!(err = Validation::validateDateOfBirth(staff.dateOfBirth)).isEmpty()) return err;
    if (!(err = Validation::validateCitizenId(staff.citizenId)).isEmpty()) return err;
    if (!(err = Validation::validatePhoneNumber(staff.phoneNumber)).isEmpty()) return err;
    if (!(err = Validation::validateEmail(staff.email)).isEmpty()) return err;
    if (!(err = Validation::validateAddress(staff.address)).isEmpty()) return err;
    if (!(err = Validation::validateValidId(staff.departmentId, "Please select a valid department.")).isEmpty()) return err;
    //if (!(err = validateShift(staff.shift)).isEmpty()) return err;

    // Chạy tiếp bộ check trùng lặp (Mặc định cho trường hợp INSERT, không loại trừ ID nào)
    // Lưu ý: Nếu làm tính năng UPDATE, bạn cần truyền staffId vào aggregate validator này.
    if (!(err = validateCitizenIdUnique(staff.citizenId, staffId)).isEmpty()) return err;
    if (!(err = validatePhoneNumberUnique(staff.phoneNumber, staffId)).isEmpty()) return err;
    if (!(err = validateEmailUnique(staff.email, staffId)).isEmpty()) return err;

    if (!(err = validateEmailUnique(staff.email, staffId)).isEmpty()) return err;

    return "";
}

QString StaffService::validateStaffUpdate(const StaffUpdateDTO& staff) {
    QString err;
    
    if (!(err = Validation::validateFullName(staff.fullName)).isEmpty()) return err;
    if (!(err = Validation::validateDateOfBirth(QDate::fromString(staff.dateOfBirth, "yyyy-MM-dd"))).isEmpty()) return err;
    if (!(err = Validation::validateCitizenId(staff.citizenId)).isEmpty()) return err;
    if (!(err = Validation::validatePhoneNumber(staff.phoneNumber)).isEmpty()) return err;
    if (!(err = Validation::validateEmail(staff.email)).isEmpty()) return err;
    if (!(err = Validation::validateAddress(staff.address)).isEmpty()) return err;
    if (!(err = Validation::validateValidId(staff.departmentId, "Please select a valid department.")).isEmpty()) return err;

    if (!(err = validateCitizenIdUnique(staff.citizenId, staff.staffId)).isEmpty()) return err;
    if (!(err = validatePhoneNumberUnique(staff.phoneNumber, staff.staffId)).isEmpty()) return err;
    if (!(err = validateEmailUnique(staff.email, staff.staffId)).isEmpty()) return err;

    return "";
}

QString StaffService::validateDoctorUpdate(const DoctorUpdateDTO& doctor) {
    QString err = validateStaffUpdate(doctor);
    if (!err.isEmpty()) return err;
    
    if (!(err = Validation::validateTrimmedNotEmpty(doctor.specialty, "Specialty field is required for doctors.")).isEmpty()) return err;
    if (!(err = validateLicenseNumber(doctor.licenseNumber)).isEmpty()) return err;
    if (!(err = validateExperienceYears(doctor.experienceYears)).isEmpty()) return err;
    if (!(err = validateConsultationFee(doctor.consultationFee)).isEmpty()) return err;
    
    if (!(err = validateLicenseNumberUnique(doctor.licenseNumber, doctor.staffId)).isEmpty()) return err;

    return "";
}

QString StaffService::validateDoctorInput(const DoctorInputDTO& doctor, int staffId) {
    // Check các trường cơ bản trước
    QString err = validateStaffBaseInput(doctor, staffId);
    if (!err.isEmpty()) return err;
    
    // Check các trường đặc thù của Doctor
    if (!(err = Validation::validateTrimmedNotEmpty(doctor.specialty, "Specialty field is required for doctors.")).isEmpty()) return err;
    if (!(err = validateLicenseNumber(doctor.licenseNumber)).isEmpty()) return err;
    if (!(err = validateExperienceYears(doctor.experienceYears)).isEmpty()) return err;
    if (!(err = validateConsultationFee(doctor.consultationFee)).isEmpty()) return err;
    
    // Check trùng số chứng chỉ hành nghề của Doctor dưới DB
    if (!(err = validateLicenseNumberUnique(doctor.licenseNumber, staffId)).isEmpty()) return err;

    return "";
}

QString StaffService::validateNurseInput(const NurseInputDTO& nurse, int staffId) {
    // Check các trường cơ bản trước
    QString err = validateStaffBaseInput(nurse, staffId);
    if (!err.isEmpty()) return err;
    
    // Check các trường đặc thù của Nurse
    if (!(err = validateNurseLevel(nurse.nurseLevel)).isEmpty()) return err;

    return "";
}

QString StaffService::validateNurseUpdate(const NurseUpdateDTO& nurse) {
    QString err = validateStaffUpdate(nurse);
    if (!err.isEmpty()) return err;
    
    if (!(err = validateNurseLevel(nurse.nurseLevel)).isEmpty()) return err;

    return "";
}




QString StaffService::generateStaffCode(UserRole role) const {
    int year = QDate::currentDate().year();
    QString yearCode = QString("%1").arg(year % LAST_TWO_DIGITS_FACTOR, 2, NUMBER_BASE, QChar('0'));
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
    }

    int count = 1;
    std::optional<QString> result = m_staffRepository->getLatestStaffCodeByYear(year);
    if (result.has_value()) {
        QString countCode = result.value().right(2);
        count = countCode.toInt() + 1;
    }

    QString newCountCode = QString("%1").arg(count, 2, NUMBER_BASE, QChar('0'));

    QString staffCode = roleCode + yearCode + newCountCode;
    return staffCode;
}

QString StaffService::generateRandomPassword() const {
    static QString chars = R"(0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz)";
    static QString specialChars = R"(#@)";
    QRandomGenerator* generator = QRandomGenerator::global();

    QString password(RANDOM_PASSWORD_LENGTH, ' ');

    for (int count = 0; count < 2;) {
        auto i = generator->bounded(RANDOM_PASSWORD_LENGTH);
        if (password[i] != ' ') continue;
        auto charIndex = generator->bounded(specialChars.length());
        password[i] = specialChars[charIndex];
        ++count;
    }

    for (int i = 0; i < RANDOM_PASSWORD_LENGTH; ++i) {
        if (password[i] != ' ') continue;
        auto charIndex = generator->bounded(chars.length());
        password[i] = chars[charIndex];
    }

    return password;
}

QString StaffService::hireNewDoctor(const DoctorInputDTO& doctor) {
    QString validationError = validateDoctorInput(doctor);
    if (!validationError.isEmpty()) { return validationError; }

    bool success =
            this->m_staffRepository->insertDoctor({doctor, generateStaffCode(UserRole::Doctor),
                    QString::fromStdString(bcrypt::generateHash(
                            generateRandomPassword().toStdString(), PASSWORD_HASH_COST_FACTOR))});

    if (!success) { return "Failed to insert doctor into the database."; }

    return "";
}

QString StaffService::hireNewNurse(const NurseInputDTO& nurse) {
    QString validationError = validateNurseInput(nurse);
    if (!validationError.isEmpty()) { return validationError; }

    bool success = this->m_staffRepository->insertNurse({nurse, generateStaffCode(UserRole::Nurse),
            QString::fromStdString(bcrypt::generateHash(
                    generateRandomPassword().toStdString(), PASSWORD_HASH_COST_FACTOR))});

    if (!success) { return "Failed to insert nurse into the database."; }

    return "";
}

QString StaffService::hireNewReceptionist(const ReceptionistInputDTO& receptionist) {
    QString validationError = validateStaffBaseInput(receptionist);
    if (!validationError.isEmpty()) { return validationError; }

    StaffInsertDTO insertDto(receptionist, generateStaffCode(UserRole::Receptionist),
         QString::fromStdString(bcrypt::generateHash(
                 generateRandomPassword().toStdString(), PASSWORD_HASH_COST_FACTOR)), UserRole::Receptionist);

    bool success = this->m_staffRepository->insertStaff(insertDto);

    if (!success) { return "Failed to insert receptionist into the database."; }

    return "";
}

QString StaffService::editStaffBaseInformation(const StaffUpdateDTO& staffInformation) {
    QString err = validateStaffUpdate(staffInformation);
    if (!err.isEmpty()) return err;
    if (!m_staffRepository->updateStaff(staffInformation))
        return "Repository update failed (DB error).";
    return "";
}

QString StaffService::editDoctorInformation(const DoctorUpdateDTO& doctorInformation) {
    QString err = validateDoctorUpdate(doctorInformation);
    if (!err.isEmpty()) return err;
    if (!m_staffRepository->updateDoctor(doctorInformation))
        return "Repository update failed (DB error).";
    return "";
}

QString StaffService::editNurseInformation(const NurseUpdateDTO& nurseInformation) {
    QString err = validateNurseUpdate(nurseInformation);
    if (!err.isEmpty()) return err;
    if (!m_staffRepository->updateNurse(nurseInformation))
        return "Repository update failed (DB error).";
    return "";
}


QList<std::shared_ptr<SystemUser>> StaffService::searchDoctors(
    QString searchKey,    
    QString specialty,                  
    int     departmentId,          
    QString shift,                     
    bool    onlyActive,
    bool    includeDeleted
) const {
    StaffSearchCriteria criteria;
    criteria.searchKey      = searchKey.trimmed();
    criteria.role           = UserRole::Doctor;
    criteria.specialty      = specialty.trimmed();
    criteria.departmentId   = departmentId;
    criteria.shift          = shift.trimmed();
    criteria.onlyActive     = onlyActive;
    criteria.includeDeleted = includeDeleted;

    return m_staffRepository->search(criteria);
}

QList<std::shared_ptr<SystemUser>> StaffService::searchNurses(
    QString searchKey,
    int departmentId,
    bool onlyActive,
    bool includeDeleted
) const {
    StaffSearchCriteria criteria;
    criteria.searchKey      = searchKey.trimmed();
    criteria.role           = UserRole::Nurse;
    criteria.departmentId   = departmentId;
    criteria.onlyActive     = onlyActive;
    criteria.includeDeleted = includeDeleted;

    return m_staffRepository->search(criteria);
}

QList<std::shared_ptr<SystemUser>> StaffService::searchReceptionists(
    QString searchKey,
    bool onlyActive,
    bool includeDeleted
) const {
    StaffSearchCriteria criteria;
    criteria.searchKey      = searchKey.trimmed();
    criteria.role           = UserRole::Receptionist;
    criteria.onlyActive     = onlyActive;
    criteria.includeDeleted = includeDeleted;

    return m_staffRepository->search(criteria);
}

bool StaffService::changePassword(int staffId, const QString& plainPassword) {
    if (staffId <= 0) return false;
    if (!Validation::validatePlainPassword(plainPassword).isEmpty()) return false;

    auto user = m_staffRepository->findById(staffId);
    if (!user) return false;
    if (user->verifyPassword(plainPassword)) return false;

    QString passwordHash = QString::fromStdString(
            bcrypt::generateHash(plainPassword.toStdString(), PASSWORD_HASH_COST_FACTOR));
    return m_staffRepository->updatePasswordInformation(staffId, passwordHash);
}

ResetPasswordResult StaffService::resetPassword(int staffId) {
    if (staffId <= 0 || !m_staffRepository->existsByStaffId(staffId))
        return ResetPasswordResult{false, ""};

    QString password = generateRandomPassword();
    QString passwordHash = QString::fromStdString(
            bcrypt::generateHash(password.toStdString(), PASSWORD_HASH_COST_FACTOR));

    bool result = m_staffRepository->updatePasswordInformation(staffId, passwordHash, true);
    if (!result) return ResetPasswordResult{false, ""};
    return ResetPasswordResult{true, password};
}