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
        "  patient_code TEXT,"
        "  full_name    TEXT    NOT NULL,"
        "  birth_date   TEXT,"
        "  gender       TEXT    DEFAULT 'Other',"
        "  phone_number TEXT,"
        "  address      TEXT,"
        "  blood_type   TEXT,"
        "  allergies    TEXT,"
        "  medical_history TEXT,"
        "  citizen_id   TEXT,"
        "  email        TEXT,"
        "  insurance    TEXT,"
        "  patient_type INTEGER DEFAULT 0,"
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

    // Bảng medical_records
    bool ok2 = query.exec(
        "CREATE TABLE IF NOT EXISTS medical_records ("
        "  id               INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  patient_id       INTEGER NOT NULL,"
        "  doctor_id        INTEGER,"
        "  visit_date       TEXT,"
        "  vitals           TEXT,"
        "  chief_complaint  TEXT,"
        "  clinical_notes   TEXT,"
        "  treatment        TEXT,"
        "  test_results     TEXT,"
        "  next_visit       TEXT,"
        "  FOREIGN KEY(patient_id) REFERENCES patients(id)"
        ")"
    );

    if (!ok2) {
        qCritical() << "Failed to create medical_records table:"
                    << query.lastError().text();
    } else {
        qDebug() << "Table 'medical_records' ready.";
    }

    // Migration: thêm cột state cho database đã tồn tại trước khi có state
    QSqlQuery alterQuery(m_db);
    alterQuery.exec("ALTER TABLE patients ADD COLUMN state INTEGER DEFAULT 0");
    alterQuery.exec("ALTER TABLE patients ADD COLUMN patient_type INTEGER DEFAULT 0");
    alterQuery.exec("ALTER TABLE patients ADD COLUMN patient_code TEXT");
    alterQuery.exec("ALTER TABLE patients ADD COLUMN blood_type TEXT");
    alterQuery.exec("ALTER TABLE patients ADD COLUMN allergies TEXT");
    alterQuery.exec("ALTER TABLE patients ADD COLUMN medical_history TEXT");
    alterQuery.exec("ALTER TABLE medical_records ADD COLUMN test_results TEXT");
    // Lệnh ALTER TABLE sẽ thất bại nếu cột đã tồn tại — đây là hành vi
    // mong muốn, không cần xử lý lỗi.
}
