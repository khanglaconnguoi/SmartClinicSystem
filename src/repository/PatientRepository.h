/**
 * @file    PatientRepository.h
 * @brief   Data access layer cho bảng patients (SQLite).
 */
#pragma once

#include <QList>
#include <QString>
#include <optional>
#include "model/Patient.h"

class QSqlDatabase;
class QSqlQuery;

/**
 * @brief Repository đóng gói toàn bộ SQL cho bảng patients.
 *
 * Tuân thủ convention: tất cả truy vấn dùng prepared statement,
 * không nối chuỗi SQL, lỗi được log qua qCritical().
 */
class PatientRepository {
public:
    /**
     * @brief Khởi tạo repository với kết nối database.
     * @param db Tham chiếu đến QSqlDatabase đã mở.
     */
    explicit PatientRepository(QSqlDatabase& db);

    /**
     * @brief Thêm bệnh nhân mới. ID sẽ được gán tự động.
     * @param patient Đối tượng Patient cần lưu (m_id sẽ được cập nhật).
     * @return true nếu thêm thành công.
     */
    bool insert(Patient& patient);

    /**
     * @brief Cập nhật thông tin bệnh nhân theo ID.
     * @param patient Đối tượng Patient đã cập nhật thông tin.
     * @return true nếu cập nhật thành công.
     */
    bool update(const Patient& patient);

    /**
     * @brief Xóa mềm (soft-delete) — đặt is_active = 0.
     * @param patientId ID bệnh nhân cần xóa.
     * @return true nếu xóa thành công.
     */
    bool softDelete(int patientId);

    /**
     * @brief Lấy bệnh nhân theo ID.
     * @param patientId ID cần tìm.
     * @return Patient nếu tìm thấy, std::nullopt nếu không.
     */
    std::optional<Patient> findById(int patientId);

    /**
     * @brief Lấy tất cả bệnh nhân đang active.
     * @return Danh sách Patient có is_active = 1.
     */
    QList<Patient> findAllActive();

    /**
     * @brief Tìm kiếm bệnh nhân theo tên (LIKE %keyword%).
     * @param keyword Từ khóa tìm kiếm.
     * @return Danh sách Patient phù hợp.
     */
    QList<Patient> searchByName(const QString& keyword);

private:
    /**
     * @brief Map một row kết quả SQL thành đối tượng Patient.
     * @param query QSqlQuery đã trỏ đến row hiện tại.
     * @return Đối tượng Patient.
     */
    Patient mapRowToPatient(const QSqlQuery& query);

    QSqlDatabase& m_db;
};
