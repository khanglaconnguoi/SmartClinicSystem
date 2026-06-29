#include "StaffService.h"
#include "Validation.h"
#include "model/SystemUser.h"
#include "ext/bcrypt.h"
#include <QDate>
#include <QPixmap>
#include <QRegularExpression>
#include <QRandomGenerator>


static constexpr qsizetype NUMBER_BASE = 10;
static constexpr qsizetype LAST_TWO_DIGITS_FACTOR = 100;
static constexpr qsizetype RANDOM_PASSWORD_LENGTH = 16;
static constexpr qsizetype LEGAL_WORKING_AGE = 18;



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

/** @brief departmentId: phai > 0 */
QString StaffService::validateDepartmentId(int departmentId) {
    if (departmentId <= 0) {
        return "Please select a valid department.";
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
/** @brief specialty: khong duoc rong */
QString StaffService::validateSpecialty(const QString& specialty) {
    if (specialty.trimmed().isEmpty()) {
        return "Specialty field is required for doctors.";
    }
    return "";
}

/**
 * @brief licenseNumber: khong duoc rong, chi chua chu va so
 *        (Dinh dang so chung chi hanh nghe VN)
 */
QString StaffService::validateLicenseNumber(const QString& licenseNumber) {
    if (licenseNumber.trimmed().isEmpty()) {
        return "Medical license number is required.";
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
    if (!(err = validateDepartmentId(staff.departmentId)).isEmpty()) return err;
    //if (!(err = validateShift(staff.shift)).isEmpty()) return err;

    // Chạy tiếp bộ check trùng lặp (Mặc định cho trường hợp INSERT, không loại trừ ID nào)
    // Lưu ý: Nếu làm tính năng UPDATE, bạn cần truyền staffId vào aggregate validator này.
    if (!(err = validateCitizenIdUnique(staff.citizenId, staffId)).isEmpty()) return err;
    if (!(err = validatePhoneNumberUnique(staff.phoneNumber, staffId)).isEmpty()) return err;
    if (!(err = validateEmailUnique(staff.email, staffId)).isEmpty()) return err;

    return "";
}

QString StaffService::validateDoctorInput(const DoctorInputDTO& doctor, int staffId) {
    // Check các trường cơ bản trước
    QString err = validateStaffBaseInput(doctor, staffId);
    if (!err.isEmpty()) return err;
    
    // Check các trường đặc thù của Doctor
    if (!(err = validateSpecialty(doctor.specialty)).isEmpty()) return err;
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
    static QString validChars =
            R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)";
    QRandomGenerator* generator = QRandomGenerator::global();

    QString password;
    password.reserve(RANDOM_PASSWORD_LENGTH);

    for (int i = 0; i < RANDOM_PASSWORD_LENGTH; ++i) {
        auto index = generator->bounded(validChars.length());
        password.append(validChars[index]);
    }

    return password;
}



bool StaffService::hireNewDoctor(const DoctorInputDTO& doctor) {
    if (!validateDoctorInput(doctor).isEmpty()) {
        return false;
    }

    return this->m_staffRepository->insertDoctor({
        doctor, 
        generateStaffCode(UserRole::Doctor), 
        QString::fromStdString(bcrypt::generateHash(generateRandomPassword().toStdString(), 12))
    });
}

bool StaffService::hireNewNurse(const NurseInputDTO& nurse) {
    if (!validateNurseInput(nurse).isEmpty()) {
        return false;
    }

    return this->m_staffRepository->insertNurse({
        nurse,
        generateStaffCode(UserRole::Nurse), 
        QString::fromStdString(bcrypt::generateHash(generateRandomPassword().toStdString(), 12))
    });
}


bool StaffService::editStaffBaseInformation(const StaffInputDTO& staffInformation, int staffId) {
    if (!validateStaffBaseInput(staffInformation, staffId).isEmpty()) return false;
    return m_staffRepository->updateStaff(StaffUpdateDTO(staffInformation, staffId));
}

bool StaffService::editDoctorInformation(const DoctorInputDTO& doctorInformation, int staffId) {
    if (!validateDoctorInput(doctorInformation, staffId).isEmpty()) return false;
    return m_staffRepository->updateDoctor(DoctorUpdateDTO(doctorInformation, staffId));
}

bool StaffService::editNurseInformation(const NurseInputDTO& nurseInformation, int staffId) {
    if (!validateNurseInput(nurseInformation, staffId).isEmpty()) return false;
    return m_staffRepository->updateNurse(NurseUpdateDTO(nurseInformation, staffId));
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