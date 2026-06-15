#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariantList>

class DatabaseManager {
private:
    // 3. Đưa constructor và destructor vào private
    DatabaseManager();
    ~DatabaseManager() { if (m_db.isOpen()) m_db.close(); }

    // Hàm khởi tạo database và tạo bảng
    bool initializeDatabase();
    bool createTables();

    QSqlDatabase m_db;

public:
    // 1. Điểm truy cập toàn cục duy nhất (Dấu hiệu của Singleton Pattern)
    static DatabaseManager& getInstance(){
        static DatabaseManager instance;
        return instance;
    }

    // 2. Xóa tính năng copy để đảm bảo chỉ có 1 Object duy nhất tồn tại
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    
    bool        executeQuery(const QString& sql, const QVariantList& params = {});
    QSqlQuery   selectQuery(const QString& sql, const QVariantList& params = {});
    bool        beginTransaction() { return m_db.transaction(); }
    bool        commitTransaction() { return m_db.commit(); }
    bool        rollbackTransaction() { return m_db.rollback(); }
};
