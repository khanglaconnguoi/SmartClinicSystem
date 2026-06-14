#include "database_manager.h"
#include <QCoreApplication>
#include <QDir>
#include <QSqlError>
#include <QDebug>
#include <QFile>



DatabaseManager::DatabaseManager() {
    // Khai báo sử dụng driver SQLite của Qt
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        m_db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
    }

    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);
    if(!dir.exists("database")) dir.mkdir("database");
    QString dbPath = appDir + "/database/hospital.db";

    // Tên file database. Nó sẽ tự động được tạo ra ở thư mục 'build' khi chạy
    m_db.setDatabaseName(dbPath);
    initializeDatabase();
}


bool DatabaseManager::initializeDatabase() {
    if (!m_db.open()) {
        qDebug() << "Lỗi mở Database:" << m_db.lastError().text();
        return false;
    }
    qDebug() << "Khởi tạo Database SQLite thành công!";
    return createTables();
}

bool DatabaseManager::createTables() {
    QSqlQuery query;
    bool success = true;

    // Bật tính năng Khóa ngoại (Foreign Keys) cho SQLite
    query.exec("PRAGMA foreign_keys = ON;");

    // 1. Bảng Patients
    // QString createPatients = R"(
    //     CREATE TABLE IF NOT EXISTS Patients (
    //         patient_id INTEGER PRIMARY KEY AUTOINCREMENT,
    //         full_name TEXT NOT NULL,
    //         dob TEXT NOT NULL,
    //         gender TEXT,
    //         phone TEXT,
    //         address TEXT
    //     );
    // )";
    // if (!query.exec(createPatients))
    // {
    //     qDebug() << "Lỗi bảng Patients:" << query.lastError().text();
    //     success = false;
    // }

    // 2. Bảng Staff
    QString createStaff = R"(
        CREATE TABLE IF NOT EXISTS staffs (
            staff_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
            full_name TEXT NOT NULL,
            date_of_birth TEXT NOT NULL,
            citizen_id_number TEXT NOT NULL,
            specialty TEXT NOT NULL,
            phone_number TEXT NOT NULL, 
            role TEXT NOT NULL
        );
    )";
    if (!query.exec(createStaff)) {
        qDebug() << "Lỗi bảng Staff:" << query.lastError().text();
        success = false;
    }

    // 3. Bảng Login Information
    QString createLoginInformation = R"(
        CREATE TABLE IF NOT EXISTS login_information (
            staff_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            passwordHash TEXT NOT NULL,
            createdAT TEXT NOT NULL
        );
    )";
    if (!query.exec(createLoginInformation)) {
        qDebug() << "Lỗi bảng Login Information:" << query.lastError().text();
        success = false;
    }

    // 4. Bảng Appointments
    // QString createAppointments = R"(
    //     CREATE TABLE IF NOT EXISTS Appointments (
    //         appointment_id INTEGER PRIMARY KEY AUTOINCREMENT,
    //         patient_id INTEGER NOT NULL,
    //         doctor_id INTEGER NOT NULL,
    //         appointment_date TEXT NOT NULL,
    //         status TEXT NOT NULL DEFAULT 'Chờ khám',
    //         symptoms TEXT,
    //         FOREIGN KEY (patient_id) REFERENCES Patients(patient_id) ON DELETE CASCADE,
    //         FOREIGN KEY (doctor_id) REFERENCES Doctors(doctor_id) ON DELETE CASCADE
    //     );
    // )";
    // if (!query.exec(createAppointments))
    // {
    //     qDebug() << "Lỗi bảng Appointments:" << query.lastError().text();
    //     success = false;
    // }

    if (success) qDebug() << "Hệ thống các bảng CSDL đã sẵn sàng!"; 
    return success;
}


bool DatabaseManager::executeQuery(const QString& sql, const QVariantList& params) {
    QSqlQuery query(m_db);
 
    if (!query.prepare(sql)) {
        qDebug() << "Lỗi prepare query:" << query.lastError().text() << "| SQL:" << sql;
        return false;
    }
 
    for (const QVariant& param : params) {
        query.addBindValue(param);
    }
 
    if (!query.exec()) {
        qDebug() << "Lỗi exec query:" << query.lastError().text() << "| SQL:" << sql;
        return false;
    }
 
    return true;
}
 
QSqlQuery DatabaseManager::selectQuery(const QString& sql, const QVariantList& params) {
    QSqlQuery query(m_db);
 
    if (!query.prepare(sql)) {
        qDebug() << "Lỗi prepare query:" << query.lastError().text() << "| SQL:" << sql;
        return query;
    }
 
    for (const QVariant& param : params) {
        query.addBindValue(param);
    }
 
    if (!query.exec()) {
        qDebug() << "Lỗi exec select query:" << query.lastError().text() << "| SQL:" << sql;
    }
 
    return query;
}