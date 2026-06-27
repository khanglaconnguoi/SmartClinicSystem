#include "StaffService.h"
#include "model/SystemUser.h"
#include "ext/bcrypt.h"
#include <algorithm>
#include <QDate>
#include <QPixmap>
#include <QRegularExpression>
#include <QRandomGenerator>


static constexpr qsizetype MOBILE_PHONE_NUMBER_LENGTH = 10;
static constexpr qsizetype LANDLINE_PHONE_NUMBER_LENGTH = 11;
static constexpr qsizetype CITIZEN_ID_LENGTH = 12;
static constexpr qsizetype PASSWORD_MINIMUM_LENGTH = 12;
static constexpr qsizetype NUMBER_BASE = 10;
static constexpr qsizetype LAST_TWO_DIGITS_FACTOR = 100;
static constexpr qsizetype RANDOM_PASSWORD_LENGTH = 16;

static constexpr std::array<QStringView, 63> CITIZEN_ID_VALID_PREFIXES = {u"001", u"002", u"004",
        u"006", u"008", u"010", u"011", u"012", u"014", u"015", u"017", u"019", u"020", u"022",
        u"023", u"025", u"026", u"027", u"030", u"031", u"033", u"034", u"035", u"036", u"037",
        u"038", u"040", u"042", u"044", u"045", u"046", u"048", u"049", u"051", u"052", u"054",
        u"056", u"058", u"060", u"062", u"064", u"066", u"067", u"068", u"070", u"072", u"074",
        u"075", u"077", u"079", u"080", u"082", u"083", u"084", u"086", u"087", u"089", u"091",
        u"092", u"093", u"094", u"095", u"096"};


// -- Field chung cho moi role ────────────────────────────────────

QString StaffService::validatePlainPassword(const QString& plainPassword) {
    if (plainPassword.length() < PASSWORD_MINIMUM_LENGTH)
        return QString("Password too weak. Must have at least %1 characters.")
                .arg(PASSWORD_MINIMUM_LENGTH);

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (const QChar& c : plainPassword) {
        if (c.isUpper()) hasUpper = true;
        else if (c.isLower()) hasLower = true;
        else if (c.isDigit()) hasDigit = true;
        else if (!c.isSpace()) hasSpecial = true;
    }

    if (!hasUpper) return "Password too weak. Must have at least 1 uppercase letter.";
    if (!hasLower) return "Password too weak. Must have at least 1 lowercase letter.";
    if (!hasDigit) return "Password too weak. Must have at least 1 digit.";
    if (!hasSpecial) return "Password too weak. Must have at least 1 special character.";

    return "";
}




/** @brief fullName: khong duoc rong, toi da 100 ky tu */
QString StaffService::validateFullName(const QString& fullName) {
    if (fullName.trimmed().isEmpty()) {
        return "Full name is required.";
    }
    if (fullName.trimmed().length() > 100) {
        return "Full name must not exceed 100 characters.";
    }
    return "";
}

/**
 * @brief dateOfBirth: khong duoc tuong lai, phai it nhat 18 tuoi
 *        (Nhan vien phai du tuoi lao dong)
 */
QString StaffService::validateDateOfBirth(const QDate& dateOfBirth) {
    if (dateOfBirth.isNull()) {
        return "Date of birth is required.";
    }
    
    QDate today = QDate::currentDate();
    if (dateOfBirth > today) {
        return "Date of birth cannot be in the future.";
    }
    
    // Kiểm tra đủ 18 tuổi lao động
    if (dateOfBirth.addYears(18) > today) {
        return "Staff member must be at least 18 years old.";
    }
    return "";
}


QString StaffService::validateCitizenId(const QString& citizenId) {
    const QString msgInvalidCitizenId = "Invalid value for citizen ID.";

    if (citizenId.length() != CITIZEN_ID_LENGTH) return msgInvalidCitizenId;

    if (!std::all_of(citizenId.begin(), citizenId.end(), [](const QChar& c) { return c.isDigit(); }))
        return msgInvalidCitizenId;

    if (std::find(CITIZEN_ID_VALID_PREFIXES.begin(), CITIZEN_ID_VALID_PREFIXES.end(), 
                QStringView(citizenId).left(3)) == CITIZEN_ID_VALID_PREFIXES.end())
        return msgInvalidCitizenId;

    return "";
}


QString StaffService::validatePhoneNumber(const QString& phoneNumber) {
    const QString msgInvalidPhoneNumber = "Invalid phone number.";

    if (phoneNumber.isEmpty() || phoneNumber[0] != '0') return msgInvalidPhoneNumber;

    if (!std::all_of(
                phoneNumber.begin(), phoneNumber.end(), [](const QChar& c) { return c.isDigit(); }))
        return msgInvalidPhoneNumber;

    switch (phoneNumber.length()) {
        case MOBILE_PHONE_NUMBER_LENGTH:
            if (phoneNumber[1] == '0' || phoneNumber[1] == '1' || phoneNumber[1] == '2' ||
                    phoneNumber[1] == '6')
                return msgInvalidPhoneNumber;
            break;
        case LANDLINE_PHONE_NUMBER_LENGTH:
            if (phoneNumber[1] != '2') return msgInvalidPhoneNumber;
            break;
        default:
            return msgInvalidPhoneNumber;
    }

    return "";
}


/** @brief email: dinh dang co ban (co @, co domain, co dot sau @) */
QString StaffService::validateEmail(const QString& email) {
    if (email.trimmed().isEmpty()) {
        return "Email is required.";
    }
    // Regex chuẩn hóa kiểm tra định dạng email cơ bản
    static const QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(email).hasMatch()) {
        return "Invalid email format (e.g., example@domain.com).";
    }
    return "";
}

/** @brief address: khong duoc rong */
QString StaffService::validateAddress(const QString& address) {
    if (address.trimmed().isEmpty()) {
        return "Address is required.";
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
    if (!alphanumericRegex.match(licenseNumber).hasMatch()) {
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
    if (m_staffRepository->existsByCitizenId(citizenId, excludeStaffId)) {
        return "This Citizen ID is already registered in the system.";
    }
    return "";
}

/**
 * @brief Kiem tra phoneNumber co bi trung trong bang staff khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validatePhoneNumberUnique(const QString& phoneNumber, int excludeStaffId) const {
    if (phoneNumber.length() == LANDLINE_PHONE_NUMBER_LENGTH) {
        return "";
    }
    if (m_staffRepository->existsByPhoneNumber(phoneNumber, excludeStaffId)) {
        return "This Phone number is already registered in the system.";
    }
    return "";
}

/**
 * @brief Kiem tra email co bi trung trong bang staff khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validateEmailUnique(const QString& email, int excludeStaffId) const {
    if (m_staffRepository->existsByEmail(email, excludeStaffId)) {
        return "This Email address is already in use.";
    }
    return "";
}

/**
 * @brief Kiem tra licenseNumber co bi trung trong bang doctor_profiles khong
 * @param excludeStaffId Bo qua staff_id nay khi kiem tra (dung cho update)
 */
QString StaffService::validateLicenseNumberUnique(const QString& licenseNumber, int excludeStaffId) const {
    if (m_staffRepository->existsByLicenseNumber(licenseNumber, excludeStaffId)) {
        return "This License number is already registered.";
    }
    return "";
}


// =================================================================
// AGGREGATE VALIDATORS (private) - Checklist tổng trước khi Insert/Update
// =================================================================

QString StaffService::validateStaffBaseInput(const StaffInputDTO& staff, int staffId) {
    QString err;
    
    if (!(err = validateFullName(staff.fullName)).isEmpty()) return err;
    if (!(err = validateDateOfBirth(staff.dateOfBirth)).isEmpty()) return err;
    if (!(err = validateCitizenId(staff.citizenId)).isEmpty()) return err;
    if (!(err = validatePhoneNumber(staff.phoneNumber)).isEmpty()) return err;
    if (!(err = validateEmail(staff.email)).isEmpty()) return err;
    if (!(err = validateAddress(staff.address)).isEmpty()) return err;
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




QString StaffService::generateStaffCode(int year, UserRole role) const {
    // int year = QDate::currentDate().year();
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

    DoctorInsertDTO dto = {
        generateStaffCode(QDate::currentDate().year(), UserRole::Doctor), 
        QString::fromStdString(bcrypt::generateHash(generateRandomPassword().toStdString(), 12)),
        doctor.fullName, 
        doctor.avatar, 
        UserRole::Doctor, 
        doctor.gender,
        doctor.dateOfBirth.toString("yyyy-MM-dd"),
        doctor.citizenId, 
        doctor.phoneNumber, 
        doctor.email, 
        doctor.address,     
        doctor.departmentId, 
        QDate::currentDate().toString("yyyy-MM-dd"), 
        doctor.shift,

        doctor.specialty, 
        doctor.licenseNumber, 
        doctor.experienceYears, 
        doctor.consultationFee, 
        doctor.bio
    };

    return this->m_staffRepository->insertDoctor(dto);
}

bool StaffService::hireNewNurse(const NurseInputDTO& nurse) {
    if (!validateNurseInput(nurse).isEmpty()) {
        return false;
    }

    NurseInsertDTO dto = {
        generateStaffCode(QDate::currentDate().year(), UserRole::Nurse), 
        QString::fromStdString(bcrypt::generateHash(generateRandomPassword().toStdString(), 12)),
        nurse.fullName, 
        nurse.avatar, 
        UserRole::Nurse, 
        nurse.gender,
        nurse.dateOfBirth.toString("yyyy-MM-dd"),
        nurse.citizenId, 
        nurse.phoneNumber, 
        nurse.email, 
        nurse.address,     
        nurse.departmentId, 
        QDate::currentDate().toString("yyyy-MM-dd"), 
        nurse.shift,

        nurse.nurseLevel,
        nurse.certification
    };

    return this->m_staffRepository->insertNurse(dto);
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