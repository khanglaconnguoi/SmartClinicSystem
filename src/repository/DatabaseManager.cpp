#include "DatabaseManager.h"
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
    QSqlQuery query(m_db);

    if (!m_db.transaction()) {
        qDebug() << "Không thể mở transaction:" << m_db.lastError().text();
        return false; // Dừng lại luôn để bảo vệ dữ liệu
    }
// ============= TẠO CÁC BẢNG ===============

    // Bật tính năng Khóa ngoại (Foreign Keys) cho SQLite
    query.exec("PRAGMA foreign_keys = ON;");

    // Bảng Deparments
    QString createDepartments = R"(
        CREATE TABLE IF NOT EXISTS departments (
            department_id   INTEGER PRIMARY KEY AUTOINCREMENT,
            department_code TEXT    NOT NULL UNIQUE,
            department_name TEXT    NOT NULL,
            description      TEXT,
            is_deleted        INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
            created_at        TEXT    NOT NULL DEFAULT (datetime('now')),
            updated_at        TEXT    NOT NULL DEFAULT (datetime('now'))
        );
    )";
    if (!query.exec(createDepartments))
    {   
        qDebug() << "Lỗi bảng Deparments:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

// -------------------------------------------------

    // Bảng Rooms
    QString createRooms = R"(
        CREATE TABLE IF NOT EXISTS rooms (
            room_id       INTEGER PRIMARY KEY AUTOINCREMENT,
            room_number   TEXT    NOT NULL UNIQUE,
            department_id INTEGER,
            room_type     TEXT    NOT NULL CHECK (room_type IN ('EXAM','SURGERY','WARD','LAB','PHARMACY','ADMIN')),
            capacity      INTEGER NOT NULL DEFAULT 1 CHECK (capacity > 0),
            status        TEXT    NOT NULL DEFAULT 'AVAILABLE' CHECK (status IN ('AVAILABLE','OCCUPIED','CLEANING','MAINTENANCE')),
            is_deleted    INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
            created_at    TEXT    NOT NULL DEFAULT (datetime('now')),
            updated_at    TEXT    NOT NULL DEFAULT (datetime('now')),
            FOREIGN KEY (department_id) REFERENCES departments(department_id) ON DELETE SET NULL
        );
    )";
    if (!query.exec(createRooms))
    {
        qDebug() << "Lỗi bảng Rooms:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

// -------------------------------------------------

    // Bảng Patients
    // QString createPatients = R"(
    //     CREATE TABLE IF NOT EXISTS patients (
    //         patient_id              INTEGER PRIMARY KEY AUTOINCREMENT,
    //         patient_code            TEXT    NOT NULL UNIQUE,
    //         full_name               TEXT    NOT NULL,
    //         date_of_birth           TEXT    NOT NULL CHECK (date_of_birth <= date('now')),
    //         gender                  TEXT    NOT NULL CHECK (gender IN ('MALE','FEMALE','OTHER')),
    //         national_id             TEXT    UNIQUE,
    //         phone                   TEXT,
    //         email                   TEXT,
    //         address                 TEXT,
    //         blood_type              TEXT    NOT NULL DEFAULT 'UNKNOWN'
    //                                         CHECK (blood_type IN ('A+','A-','B+','B-','AB+','AB-','O+','O-','UNKNOWN')),
    //         default_patient_type    TEXT    NOT NULL DEFAULT 'OUTPATIENT'
    //                                         CHECK (default_patient_type IN ('OUTPATIENT','INPATIENT','EMERGENCY')),
    //         emergency_contact_name  TEXT,
    //         emergency_contact_phone TEXT,
    //         is_deleted              INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
    //         created_at              TEXT    NOT NULL DEFAULT (datetime('now')),
    //         updated_at              TEXT    NOT NULL DEFAULT (datetime('now'))
    //     );
    // )";
    // if (!query.exec(createPatients))
    // {
    //     qDebug() << "Lỗi bảng Patients:" << query.lastError().text();
    //     m_db.rollback();
    //     return false;
    // }

// -------------------------------------------------

    // Bảng Staff
    QString createStaff = R"(
        CREATE TABLE IF NOT EXISTS staff (
            staff_id      INTEGER PRIMARY KEY AUTOINCREMENT,
            staff_code    TEXT    NOT NULL UNIQUE,
            password_hash TEXT    NOT NULL,
            full_name     TEXT    NOT NULL,
            role          TEXT    NOT NULL CHECK (role IN ('ADMIN','DOCTOR','NURSE','RECEPTIONIST')),
            gender        TEXT    CHECK (gender IN ('MALE','FEMALE','OTHER')),
            date_of_birth TEXT    CHECK (date_of_birth IS NULL OR date_of_birth <= date('now')),
            national_id   TEXT    UNIQUE,
            phone_number         TEXT,
            email         TEXT    UNIQUE,
            address       TEXT,
            department_id INTEGER,
            hire_date     TEXT    NOT NULL DEFAULT (date('now')),
            shift         TEXT    NOT NULL DEFAULT 'FULL_DAY' CHECK (shift IN ('MORNING','AFTERNOON','NIGHT','FULL_DAY')),
            is_active     INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1)),
            is_deleted    INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
            created_at    TEXT    NOT NULL DEFAULT (datetime('now')),
            updated_at    TEXT    NOT NULL DEFAULT (datetime('now')),
            FOREIGN KEY (department_id) REFERENCES departments(department_id) ON DELETE SET NULL
        );      
    )";
    if (!query.exec(createStaff)) {
        qDebug() << "Lỗi bảng Staff:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

// -------------------------------------------------

    // Bảng Doctor Profiles
    QString createDoctorProfiles = R"(
        CREATE TABLE IF NOT EXISTS doctor_profiles (
            staff_id         INTEGER PRIMARY KEY,
            specialty        TEXT    NOT NULL,
            license_number   TEXT    NOT NULL UNIQUE,
            experience_years INTEGER NOT NULL DEFAULT 0 CHECK (experience_years >= 0),
            consultation_fee REAL    NOT NULL DEFAULT 0 CHECK (consultation_fee >= 0),
            bio              TEXT,
            FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE
        );  
    )";
    if (!query.exec(createDoctorProfiles)) {
        qDebug() << "Lỗi bảng Doctor Profiles:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

// -------------------------------------------------

    // Bảng Nurse Profiles
    QString createNurseProfiles = R"(
        CREATE TABLE IF NOT EXISTS nurse_profiles (
            staff_id      INTEGER PRIMARY KEY,
            nurse_level   TEXT NOT NULL DEFAULT 'JUNIOR' CHECK (nurse_level IN ('JUNIOR','SENIOR','HEAD')),
            certification TEXT,
            FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE
        ); 
    )";
    if (!query.exec(createNurseProfiles)) {
        qDebug() << "Lỗi bảng Nurse Profiles:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

    // Bảng Login Information
    // QString createLoginInformation = R"(
    //     CREATE TABLE login_information (
    //         user_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    //         username TEXT NOT NULL,
    //         password_hash TEXT NOT NULL,
    //         created_at TEXT NOT NULL,
    //         staff_id INTEGER NOT NULL, account_type TEXT NOT NULL,
    //         CONSTRAINT login_information_staff_FK FOREIGN KEY (staff_id) REFERENCES staff(staff_id)
    //     );
    // )";
    // if (!query.exec(createLoginInformation)) {
    //     qDebug() << "Lỗi bảng Login Information:" << query.lastError().text();
    //     m_db.rollback();
    //     return false;
    // }

// -------------------------------------------------

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


// ============= TẠO INDEX ===============
    QStringList createIndexList = { 
        R"( CREATE INDEX IF NOT EXISTS idx_staff_role               ON staff(role);             )",
        R"( CREATE INDEX IF NOT EXISTS idx_staff_department         ON staff(department_id);    )",
        R"( CREATE INDEX IF NOT EXISTS idx_patients_full_name       ON patients(full_name);     )"
    };

    for(const QString& createIndex: createIndexList) {
        QString create = createIndex.trimmed();

        if(create.isEmpty()) continue;

        if(!query.exec(create)){
            qDebug() << "Lỗi tạo index:" << query.lastError().text();
            m_db.rollback();
            return false;
        }
    }

// ============= TẠO TRIGGER ===============

    QStringList createTriggerList =  {
        R"( 
            CREATE TRIGGER IF NOT EXISTS trg_departments_updated_at
            AFTER UPDATE ON departments FOR EACH ROW
            BEGIN
                UPDATE departments SET updated_at = datetime('now') WHERE department_id = OLD.department_id;
            END;
        )",
        R"(
            CREATE TRIGGER IF NOT EXISTS trg_rooms_updated_at
            AFTER UPDATE ON rooms FOR EACH ROW
            BEGIN
                UPDATE rooms SET updated_at = datetime('now') WHERE room_id = OLD.room_id;
            END;
        )",
        R"(
            CREATE TRIGGER IF NOT EXISTS trg_staff_updated_at
            AFTER UPDATE ON staff FOR EACH ROW
            BEGIN
                UPDATE staff SET updated_at = datetime('now') WHERE staff_id = OLD.staff_id;
            END;
        )"
    };


    for(const QString& createTrigger: createTriggerList) {
        QString create = createTrigger.trimmed();

        if(create.isEmpty()) continue;

        if(!query.exec(create)){
            qDebug() << "Lỗi tạo trigger:" << query.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    if (!m_db.commit()) {
        qDebug() << "Ghi dữ liệu thất bại" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    qDebug() << "Hệ thống các bảng CSDL đã sẵn sàng!"; 
    return true;
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