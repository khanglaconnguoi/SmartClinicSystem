/**
 * @file    Validation.cpp
 * @brief   Implementación của Validation.
 *          Tập hợp các hàm kiểm tra đầu vào.
 */

#include "Validation.h"
#include <QRegularExpression>
#include <QStringList>
#include <algorithm>
#include <array>


namespace Validation {

// ─────────────────────────────────────────────────────────────────────────────
// Hằng số nội bộ
// ─────────────────────────────────────────────────────────────────────────────

/// Mã tỉnh/thành đầu CCCD hợp lệ (63 tỉnh thành Việt Nam).
static constexpr std::array<QStringView, 63> CITIZEN_ID_VALID_PREFIXES = {
    u"001", u"002", u"004", u"006", u"008", u"010", u"011", u"012", u"014",
    u"015", u"017", u"019", u"020", u"022", u"023", u"025", u"026", u"027",
    u"030", u"031", u"033", u"034", u"035", u"036", u"037", u"038", u"040",
    u"042", u"044", u"045", u"046", u"048", u"049", u"051", u"052", u"054",
    u"056", u"058", u"060", u"062", u"064", u"066", u"067", u"068", u"070",
    u"072", u"074", u"075", u"077", u"079", u"080", u"082", u"083", u"084",
    u"086", u"087", u"089", u"091", u"092", u"093", u"094", u"095", u"096"};

// ─────────────────────────────────────────────────────────────────────────────
// Trường đơn lẻ
// ─────────────────────────────────────────────────────────────────────────────

QString validateTrimmedNotEmpty(const QString& str, const QString& errorMessage) {
    if (str.trimmed().isEmpty()) { return errorMessage; }
    return "";
}

QString validateValidId(int id, const QString& errorMessage) {
    if (id <= 0) { return errorMessage; }
    return "";
}

QString validatePlainPassword(const QString &plainPassword) {
  if (plainPassword.length() < PASSWORD_MINIMUM_LENGTH)
    return QString("Password too weak. Must have at least %1 characters.")
        .arg(PASSWORD_MINIMUM_LENGTH);

  bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
  for (const QChar &c : plainPassword) {
    if (c.isUpper())
      hasUpper = true;
    else if (c.isLower())
      hasLower = true;
    else if (c.isDigit())
      hasDigit = true;
    else if (!c.isSpace())
      hasSpecial = true;
  }

  if (!hasUpper)
    return "Password too weak. Must have at least 1 uppercase letter.";
  if (!hasLower)
    return "Password too weak. Must have at least 1 lowercase letter.";
  if (!hasDigit)
    return "Password too weak. Must have at least 1 digit.";
  if (!hasSpecial)
    return "Password too weak. Must have at least 1 special character.";

  return "";
}

QString validateFullName(const QString &fullName) {
  if (fullName.trimmed().isEmpty()) {
    return "Full name is required.";
  }
  if (fullName.trimmed().length() > FULL_NAME_MAX_LENGTH) {
    return QString("Full name must not exceed %1 characters.")
        .arg(FULL_NAME_MAX_LENGTH);
  }
  static const QRegularExpression nameRegex(QStringLiteral("^[\\p{L}\\s]+$"));
  if (!nameRegex.match(fullName.trimmed()).hasMatch()) {
    return "Full name must not contain digits or special characters.";
  }
  return "";
}

QString validateCitizenId(const QString &citizenId) {
  static const QString msgInvalidCitizenId = "Invalid value for citizen ID.";

  if (citizenId.length() != CITIZEN_ID_LENGTH)
    return msgInvalidCitizenId;

  if (!std::all_of(citizenId.begin(), citizenId.end(),
                   [](const QChar &c) { return c.isDigit(); }))
    return msgInvalidCitizenId;

  if (std::find(
          CITIZEN_ID_VALID_PREFIXES.begin(), CITIZEN_ID_VALID_PREFIXES.end(),
          QStringView(citizenId).left(3)) == CITIZEN_ID_VALID_PREFIXES.end())
    return msgInvalidCitizenId;

  return "";
}

QString validatePhoneNumber(const QString &phoneNumber) {
  static const QString msgInvalidPhoneNumber = "Invalid phone number.";

  if (phoneNumber.isEmpty() || phoneNumber[0] != '0')
    return msgInvalidPhoneNumber;

  if (!std::all_of(phoneNumber.begin(), phoneNumber.end(),
                   [](const QChar &c) { return c.isDigit(); }))
    return msgInvalidPhoneNumber;

  switch (phoneNumber.length()) {
  case MOBILE_PHONE_NUMBER_LENGTH:
    // Số di động: chữ số thứ 2 không được là 0, 1, 2, 6
    if (phoneNumber[1] == '0' || phoneNumber[1] == '1' ||
        phoneNumber[1] == '2' || phoneNumber[1] == '6')
      return msgInvalidPhoneNumber;
    break;
  case LANDLINE_PHONE_NUMBER_LENGTH:
    // Số bàn: chữ số thứ 2 phải là 2
    if (phoneNumber[1] != '2')
      return msgInvalidPhoneNumber;
    break;
  default:
    return msgInvalidPhoneNumber;
  }

  return "";
}

QString validateEmail(const QString &email) {
  if (email.trimmed().isEmpty()) {
    return "Email is required.";
  }

  static const QRegularExpression emailRegex(
      R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
  if (!emailRegex.match(email.trimmed()).hasMatch()) {
    return "Invalid email format (e.g., example@domain.com).";
  }
  return "";
}

QString validateAddress(const QString &address) {
  if (address.trimmed().isEmpty()) {
    return "Address is required.";
  }
  return "";
}

QString validateDateOfBirth(const QDate &dateOfBirth) {
  if (dateOfBirth.isNull()) {
    return "Date of birth is required.";
  }

  QDate today = QDate::currentDate();
  if (dateOfBirth > today) {
    return "Date of birth cannot be in the future.";
  }
  if (dateOfBirth.year() < today.year() - 150) {
    return "Date of birth is unrealistic (age > 150).";
  }

  return "";
}

} // namespace Validation
