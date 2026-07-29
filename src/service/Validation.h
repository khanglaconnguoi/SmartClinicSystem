/**
 * @file    Validation.h
 * @brief   Tập hợp các hàm kiểm tra đầu vào cho module Patient.
 *
 *  Mỗi hàm trả về:
 *    - Chuỗi rỗng ""     → đầu vào hợp lệ
 *    - Chuỗi lỗi khác   → mô tả lý do không hợp lệ
 *
 *  Tất cả hàm đều được đặt trong namespace Validation — không cần khởi tạo đối
 * tượng.
 */
#pragma once
#include <QDate>
#include <QList>
#include <QString>

namespace Validation {

// ── Hằng số ─────────────────────────────────────────────────────────────
inline constexpr qsizetype LANDLINE_PHONE_NUMBER_LENGTH = 11;
inline constexpr qsizetype MOBILE_PHONE_NUMBER_LENGTH = 10;
inline constexpr qsizetype CITIZEN_ID_LENGTH = 12;
inline constexpr qsizetype PASSWORD_MINIMUM_LENGTH = 12;
inline constexpr qsizetype FULL_NAME_MAX_LENGTH = 100;

// ── Trường đơn lẻ ─────────────────────────────────────────────────────────

/**
 * @brief Validates if a string is not empty after trimming whitespace.
 * @return The error message if invalid; otherwise an empty string.
 */
QString validateTrimmedNotEmpty(const QString &str, const QString &errorMessage);

/**
 * @brief Validates if a database ID is valid (strictly greater than 0).
 * @return The error message if invalid; otherwise an empty string.
 */
QString validateValidId(int id, const QString &errorMessage);

QString validatePlainPassword(const QString &plainPassword);
QString validateFullName(const QString &fullName);

/**
 * @brief Kiểm tra CCCD/CMND 12 chữ số với tiền tố tỉnh/thành hợp lệ.
 */
QString validateCitizenId(const QString &citizenId);

/**
 * @brief Kiểm tra số điện thoại Việt Nam (10 hoặc 11 chữ số, bắt đầu bằng 0).
 */
QString validatePhoneNumber(const QString &phoneNumber);

/**
 * @brief Kiểm tra định dạng địa chỉ email.
 */
QString validateEmail(const QString &email);

/**
 * @brief Kiểm tra ngày sinh hợp lệ (không lớn hơn ngày hiện tại).
 */
QString validateDateOfBirth(const QDate &dateOfBirth);

/**
 * @brief Kiểm tra khoảng ngày hợp lệ (fromDate <= toDate).
 */
QString validateDateRange(const QDate &fromDate, const QDate &toDate);

} // namespace Validation