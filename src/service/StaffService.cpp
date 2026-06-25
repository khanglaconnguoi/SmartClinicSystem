#include "StaffService.h"

#include <algorithm>

#include "model/SystemUser.h"

static constexpr qsizetype MOBILE_PHONE_NUMBER_LENGTH = 10;
static constexpr qsizetype LANDLINE_PHONE_NUMBER_LENGTH = 11;
static constexpr qsizetype NATIONAL_ID_LENGTH = 12;
static constexpr qsizetype PASSWORD_MINIMUM_LENGTH = 12;
static constexpr qsizetype NUMBER_BASE = 10;
static constexpr qsizetype LAST_TWO_DIGITS_FACTOR = 100;

static constexpr std::array<QStringView, 63> NATIONAL_ID_VALID_PREFIXES = {u"001", u"002", u"004",
        u"006", u"008", u"010", u"011", u"012", u"014", u"015", u"017", u"019", u"020", u"022",
        u"023", u"025", u"026", u"027", u"030", u"031", u"033", u"034", u"035", u"036", u"037",
        u"038", u"040", u"042", u"044", u"045", u"046", u"048", u"049", u"051", u"052", u"054",
        u"056", u"058", u"060", u"062", u"064", u"066", u"067", u"068", u"070", u"072", u"074",
        u"075", u"077", u"079", u"080", u"082", u"083", u"084", u"086", u"087", u"089", u"091",
        u"092", u"093", u"094", u"095", u"096"};

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

QString StaffService::validateNationalId(const QString& nationalId) {
    const QString msgInvalidNationalId = "Invalid value for national ID.";

    if (nationalId.length() != NATIONAL_ID_LENGTH) return msgInvalidNationalId;

    if (!std::all_of(
                nationalId.begin(), nationalId.end(), [](const QChar& c) { return c.isDigit(); }))
        return msgInvalidNationalId;

    if (std::find(NATIONAL_ID_VALID_PREFIXES.begin(), NATIONAL_ID_VALID_PREFIXES.end(),
                QStringView(nationalId).left(3)) == NATIONAL_ID_VALID_PREFIXES.end())
        return msgInvalidNationalId;

    return "";
}

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

QString StaffService::validateBaseInput(const QString& staffCode,
        const QString& plainPassword,
        const QString& fullName,
        const QDate& dateOfBirth,
        const QString& nationalId,
        const QString& phoneNumber,
        const QString& email,
        const QString& address,
        int departmentId,
        const QDate& hireDate,
        const QString& shift,
        const QString& specialty,
        const QString& licenseNumber,
        int experienceYears,
        int consultationFee,
        const QString& bio) {
    if (staffCode.isEmpty()) return "Staff code is required.";
    if (fullName.isEmpty()) return "Full name is required.";
    if (experienceYears < 0) return "Experience years must not be negative.";
    if (consultationFee < 0) return "Consultation Fee must not be negative.";
    if (departmentId <= 0) return "Invalid value for department ID.";
    if (hireDate <= dateOfBirth) return "Date of birth must be strictly before hire date.";
    if (email.isEmpty()) return "Email is required";
    if (address.isEmpty()) return "Address is required";
    if (shift.isEmpty()) return "Shift is required";
    if (specialty.isEmpty()) return "Specialty is required";
    if (licenseNumber.isEmpty()) return "License number is required";
    if (bio.isEmpty()) return "Bio is required";

    if (QString errorMsg = validateNationalId(nationalId); !errorMsg.isEmpty()) return errorMsg;
    if (QString errorMsg = validatePhoneNumber(phoneNumber); !errorMsg.isEmpty()) return errorMsg;
    if (QString errorMsg = validatePlainPassword(plainPassword); !errorMsg.isEmpty())
        return errorMsg;

    return "";
}

QString StaffService::generateStaffCode(int year, UserRole role) {
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
    std::optional<QString> result = m_staffRepository->getLatestIdByYear(year);
    if (result.has_value()) {
        QString countCode = result.value().right(2);
        count = countCode.toInt() + 1;
    }

    QString newCountCode = QString("%1").arg(count, 2, NUMBER_BASE, QChar('0'));

    QString staffCode = roleCode + yearCode + newCountCode;
    return staffCode;
}

bool StaffService::hireNewDoctor(const QString& staffCode,
        const QString& plainPassword,
        const QString& fullName,
        Gender gender,
        const QDate& dateOfBirth,
        const QString& nationalId,
        const QString& phoneNumber,
        const QString& email,
        const QString& address,
        int departmentId,
        const QDate& hireDate,
        const QString& shift,
        const QString& specialty,
        const QString& licenseNumber,
        int experienceYears,
        int consultationFee,
        const QString& bio) {
    if (!validateBaseInput(staffCode, plainPassword, fullName, dateOfBirth, nationalId, phoneNumber,
                email, address, departmentId, hireDate, shift, specialty, licenseNumber,
                experienceYears, consultationFee, bio)
                    .isEmpty()) {
        return false;
    }

    DoctorInsertDTO dto = {
            {staffCode, plainPassword, fullName, UserRole::Doctor, gender,
                    dateOfBirth.toString("yyyy-MM-dd"), nationalId, phoneNumber, email, address,
                    departmentId, hireDate.toString("yyyy-MM-dd"), shift},
            specialty, licenseNumber, experienceYears, consultationFee, bio};

    return this->m_staffRepository->insertDoctor(dto);
}