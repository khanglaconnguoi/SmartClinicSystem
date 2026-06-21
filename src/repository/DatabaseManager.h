/**
 * @file    DatabaseManager.h
 * @brief   Singleton class quản lý kết nối SQLite cho toàn bộ ứng dụng.
 */
#pragma once

#include <QSqlDatabase>
#include <QString>

/**
 * @brief Singleton quản lý kết nối SQLite.
 *
 * Cung cấp QSqlDatabase& cho các Repository. Tự động tạo file
 * database nếu chưa tồn tại.
 */
class DatabaseManager {
public:
    /**
     * @brief Lấy instance duy nhất của DatabaseManager.
     * @return Tham chiếu đến singleton instance.
     */
    static DatabaseManager& instance();

    /**
     * @brief Lấy kết nối database hiện tại.
     * @return Tham chiếu đến QSqlDatabase.
     */
    QSqlDatabase& database();

    /**
     * @brief Kiểm tra database đã mở thành công chưa.
     * @return true nếu database đang mở.
     */
    bool isOpen() const;

    // Không cho copy hay assign
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    DatabaseManager();
    ~DatabaseManager();

    /**
     * @brief Tạo tất cả các bảng cần thiết nếu chưa tồn tại.
     */
    void createTables();

    QSqlDatabase m_db;
};
