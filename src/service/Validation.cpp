/**
 * @file    Validation.cpp
 * @brief   Implementación của PatientValidation.
 *          Chuyển từ PatientService — không đổi logic, chỉ đổi class owner.
 */

#include "Validation.h"
#include <QDate>
#include <QRegularExpression>
#include <QStringList>
#include <algorithm>
#include <array>

// ─────────────────────────────────────────────────────────────────────────────
// Hằng số nội bộ
// ─────────────────────────────────────────────────────────────────────────────

static constexpr qsizetype MOBILE_PHONE_NUMBER_LENGTH = 10;
static constexpr qsizetype LANDLINE_PHONE_NUMBER_LENGTH = 11;
static constexpr qsizetype CITIZEN_ID_LENGTH = 12;

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

QString validatePhoneNumber(const QString &phoneNumber) {
  const QString err = "Invalid phone number.";

  if (phoneNumber.isEmpty() || phoneNumber[0] != '0')
    return err;

  if (!std::all_of(phoneNumber.begin(), phoneNumber.end(),
                   [](const QChar &c) { return c.isDigit(); }))
    return err;

  switch (phoneNumber.length()) {
  case MOBILE_PHONE_NUMBER_LENGTH:
    // Số di động: chữ số thứ 2 không được là 0, 1, 2, 6
    if (phoneNumber[1] == '0' || phoneNumber[1] == '1' ||
        phoneNumber[1] == '2' || phoneNumber[1] == '6')
      return err;
    break;
  case LANDLINE_PHONE_NUMBER_LENGTH:
    // Số bàn: chữ số thứ 2 phải là 2
    if (phoneNumber[1] != '2')
      return err;
    break;
  default:
    return err;
  }

  return "";
}

QString validateCitizenId(const QString &citizenId) {
  const QString err = "Invalid value for citizen ID.";

  if (citizenId.length() != CITIZEN_ID_LENGTH)
    return err;

  if (!std::all_of(citizenId.begin(), citizenId.end(),
                   [](const QChar &c) { return c.isDigit(); }))
    return err;

  if (std::find(
          CITIZEN_ID_VALID_PREFIXES.begin(), CITIZEN_ID_VALID_PREFIXES.end(),
          QStringView(citizenId).left(3)) == CITIZEN_ID_VALID_PREFIXES.end())
    return err;

  return "";
}

QString validateEmail(const QString &email) {
  QRegularExpression rx(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
  if (!rx.match(email).hasMatch())
    return "Invalid email address.";
  return "";
}

QString validateBloodType(const QString &bloodType) {
  const QStringList valid = {"A+",  "A-", "B+", "B-",     "AB+",
                             "AB-", "O+", "O-", "UNKNOWN"};
  if (bloodType.isEmpty())
    return "Blood type is required. If unknown, please enter UNKNOWN.";
  if (!valid.contains(bloodType))
    return "Invalid blood type. If unknown, please enter UNKNOWN.";
  return "";
}

QString validateDateRange(const QDate &fromDate, const QDate &toDate) {
  // Không lọc theo ngày, hoặc chỉ lọc một đầu mốc → luôn hợp lệ.
  if (!fromDate.isValid() || !toDate.isValid())
    return "";

  if (fromDate > toDate)
    return "Ngày bắt đầu (fromDate) không được sau ngày kết thúc (toDate).";

  return "";
}