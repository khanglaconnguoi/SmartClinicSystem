/**
 * @file    DatabaseManager.cpp
 * @brief   Implementation cho DatabaseManager singleton.
 */
#include "DatabaseManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager() {
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // Đặt file database cùng thư mục với executable
    QString dbPath = QCoreApplication::applicationDirPath()
                     + QDir::separator() + "clinic.db";
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Cannot open database:" << m_db.lastError().text();
        return;
    }

    qDebug() << "Database opened:" << dbPath;

    // Bật foreign keys cho SQLite
    QSqlQuery query(m_db);
    query.exec("PRAGMA foreign_keys = ON");

    createTables();
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
        qDebug() << "Database closed.";
    }
}

QSqlDatabase& DatabaseManager::database() {
    return m_db;
}

bool DatabaseManager::isOpen() const {
    return m_db.isOpen();
}

void DatabaseManager::createTables() {
    QSqlQuery query(m_db);

    // Bảng patients
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS patients ("
        "  id           INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  full_name    TEXT    NOT NULL,"
        "  birth_date   TEXT,"
        "  gender       TEXT    DEFAULT 'Other',"
        "  phone_number TEXT,"
        "  address      TEXT,"
        "  citizen_id   TEXT,"
        "  email        TEXT,"
        "  insurance    TEXT,"
        "  is_active    INTEGER DEFAULT 1,"
        "  state        INTEGER DEFAULT 0"
        ")"
    );

    if (!ok) {
        qCritical() << "Failed to create patients table:"
                    << query.lastError().text();
    } else {
        qDebug() << "Table 'patients' ready.";
    }

    // Migration: thêm cột state cho database đã tồn tại trước khi có state
    QSqlQuery alterQuery(m_db);
    alterQuery.exec("ALTER TABLE patients ADD COLUMN state INTEGER DEFAULT 0");
    // Lệnh ALTER TABLE sẽ thất bại nếu cột đã tồn tại — đây là hành vi
    // mong muốn, không cần xử lý lỗi.
}
