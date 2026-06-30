/**
 * @file    Validation.h
 * @brief   Tập hợp các hàm kiểm tra đầu vào cho module Patient.
 *
 *  Mỗi hàm trả về:
 *    - Chuỗi rỗng ""     → đầu vào hợp lệ
 *    - Chuỗi lỗi khác   → mô tả lý do không hợp lệ
 *
 *  Tất cả hàm đều là static — không cần khởi tạo đối tượng.
 */

#pragma once

#include <QDate>
#include <QString>

// ── Trường đơn lẻ ─────────────────────────────────────────────────────────

/**
 * @brief Kiểm tra số điện thoại Việt Nam (10 hoặc 11 chữ số, bắt đầu bằng 0).
 */
QString validatePhoneNumber(const QString &phoneNumber);

/**
 * @brief Kiểm tra CCCD/CMND 12 chữ số với tiền tố tỉnh/thành hợp lệ.
 */
QString validateCitizenId(const QString &citizenId);

/**
 * @brief Kiểm tra định dạng địa chỉ email.
 */
QString validateEmail(const QString &email);

/**
 * @brief Kiểm tra nhóm máu (A+/A-/B+/B-/AB+/AB-/O+/O-/UNKNOWN).
 */
QString validateBloodType(const QString &bloodType);

/**
 * @brief Kiểm tra khoảng ngày hợp lệ, dùng cho bộ lọc tìm kiếm theo ngày.
 *
 *  Hợp lệ khi:
 *    - Cả hai ngày đều rỗng (không lọc theo ngày), hoặc
 *    - Chỉ một trong hai ngày được cung cấp, hoặc
 *    - Cả hai ngày hợp lệ và fromDate <= toDate.
 *
 * @param fromDate Ngày bắt đầu khoảng lọc (có thể rỗng — QDate không hợp lệ).
 * @param toDate   Ngày kết thúc khoảng lọc (có thể rỗng — QDate không hợp lệ).
 */
QString validateDateRange(const QDate &fromDate, const QDate &toDate);