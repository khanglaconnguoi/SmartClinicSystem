#pragma once
#include <QDate>
#include <QString>

namespace Validation {

inline constexpr qsizetype LANDLINE_PHONE_NUMBER_LENGTH = 11;
inline constexpr qsizetype MOBILE_PHONE_NUMBER_LENGTH = 10;
inline constexpr qsizetype CITIZEN_ID_LENGTH = 12;
inline constexpr qsizetype PASSWORD_MINIMUM_LENGTH = 12;
inline constexpr qsizetype FULL_NAME_MAX_LENGTH = 100;

QString validatePlainPassword(const QString& plainPassword);
QString validateFullName(const QString& fullName);
QString validateCitizenId(const QString& citizenId);
QString validatePhoneNumber(const QString& phoneNumber);
QString validateEmail(const QString& email);
QString validateAddress(const QString& address);
QString validateDateOfBirth(const QDate& dateOfBirth);

}  // namespace Validation
