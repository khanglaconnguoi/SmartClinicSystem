#include "StaffService.h"

#include <QDate>
#include <QRegularExpression>
#include <algorithm>

#include "model/SystemUser.h"
#include "repository/StaffRepository.h"

const qsizetype MOBILE_PHONE_NUMER_LENGTH = 10;
const qsizetype LANDLINE_PHONE_NUMER_LENGTH = 11;
const QString VALID_DIGITS = "0123456789";
const QString INVALID_PHONE_NUMBER_MSG = "Invalid phone number.";

QString StaffService::validateBaseInput(const QString& fullName,
    const QString& phone,
    const QString& username,
    int experienceYears,
    int consultationFee,
    int departmentId) const {
    if (fullName.isEmpty()) return "Full name must not be empty.";
    if (username.isEmpty()) return "Username must not be empty.";
    if (experienceYears < 0) return "Invalid value for experience years.";
    if (consultationFee < 0) return "Invalid value for consultation fee.";
    if (departmentId > 0) return "Department ID must not be empty.";

    if (!std::all_of(
            phone.begin(), phone.end(), [](const QChar& c) { return VALID_DIGITS.contains(c); })) {
        return INVALID_PHONE_NUMBER_MSG;
    }

    switch (phone.length()) {
        case MOBILE_PHONE_NUMER_LENGTH:
            if (phone[1] == '0' || phone[1] == '1' || phone[1] == '2' || phone[1] == '6')
                return INVALID_PHONE_NUMBER_MSG;
            break;
        case LANDLINE_PHONE_NUMER_LENGTH:
            if (phone[1] != '2') return INVALID_PHONE_NUMBER_MSG;
            break;
        default:
            return INVALID_PHONE_NUMBER_MSG;
    }

    return "";
}

bool StaffService::hireNewDoctor(const QString& username,
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
    if (!this
            ->validateBaseInput(
                fullName, phoneNumber, username, experienceYears, consultationFee, departmentId)
            .isEmpty()) {
        return false;
    }

    DoctorInsertDTO dto = {{"1", username, plainPassword, fullName, UserRole::Doctor, gender,
                               dateOfBirth.toString("yyyy-MM-dd"), nationalId, phoneNumber, email,
                               address, departmentId, hireDate.toString("yyyy-MM-dd"), shift},
        specialty, licenseNumber, experienceYears, consultationFee, bio};

    return this->m_staffRepository->insertDoctor(dto);
}