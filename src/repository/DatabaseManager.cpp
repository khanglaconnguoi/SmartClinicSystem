#include "DatabaseManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <qcoreapplication.h>

DatabaseManager::DatabaseManager() {
  // Khai báo sử dụng driver SQLite của Qt
  if (QSqlDatabase::contains("qt_sql_default_connection")) {
    m_db = QSqlDatabase::database("qt_sql_default_connection");
  } else {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
  }

  QString projectRoot = QString::fromUtf8(PROJECT_ROOT_DIR);

  QDir databaseDir(projectRoot + "/database");

  if (!databaseDir.exists()) {
    if (!databaseDir.mkpath(".")) {
      qWarning() << "Không thể tạo thư mục database tại:"
                 << databaseDir.absolutePath();
    }
  }

  QString dbPath = databaseDir.filePath("hospital.db");
  qDebug() << "Đường dẫn Database:" << dbPath;

  m_db.setDatabaseName(dbPath);
  initializeDatabase();
}

bool DatabaseManager::initializeDatabase() {
  if (!m_db.open()) {
    qDebug() << "Lỗi mở Database:" << m_db.lastError().text();
    return false;
  }
  qDebug() << "Khởi tạo Database SQLite thành công!";

  // Bật tính năng Khóa ngoại (Foreign Keys) cho SQLite (Phải gọi ngoài
  // transaction)
  QSqlQuery query(m_db);
  query.exec("PRAGMA foreign_keys = ON;");

  return createTables();
}

bool DatabaseManager::createTables() {
  QSqlQuery query(m_db);

  if (!m_db.transaction()) {
    qDebug() << "Không thể mở transaction:" << m_db.lastError().text();
    return false; // Dừng lại luôn để bảo vệ dữ liệu
  }
  // ============= TẠO CÁC BẢNG ===============

  // Bảng Departments
  QString createDepartments = R"(
      CREATE TABLE IF NOT EXISTS departments (
          department_id   INTEGER PRIMARY KEY AUTOINCREMENT,
          department_code TEXT    NOT NULL UNIQUE,
          department_name TEXT    NOT NULL,
          description     TEXT,
          is_deleted      INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
          created_at      TEXT    NOT NULL DEFAULT (datetime('now')),
          updated_at      TEXT    NOT NULL DEFAULT (datetime('now'))
      )
  )";
  if (!query.exec(createDepartments)) {
    qDebug() << "Lỗi bảng Departments:" << query.lastError().text();
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
      )
  )";
  if (!query.exec(createRooms)) {
    qDebug() << "Lỗi bảng Rooms:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // -------------------------------------------------

  // Bảng Patients
  QString createPatients = R"(
      CREATE TABLE IF NOT EXISTS patients (
          patient_id              INTEGER PRIMARY KEY AUTOINCREMENT,
          patient_code            TEXT    NOT NULL UNIQUE,
          full_name               TEXT    NOT NULL,
          date_of_birth           TEXT    NOT NULL,
          gender                  TEXT    NOT NULL CHECK (gender IN ('MALE','FEMALE','OTHER')),
          citizen_id              TEXT    UNIQUE,
          phone                   TEXT    NOT NULL,
          email                   TEXT    NOT NULL,
          address                 TEXT    NOT NULL,
          blood_type              TEXT    NOT NULL DEFAULT 'UNKNOWN' CHECK (blood_type IN ('A+','A-','B+','B-','AB+','AB-','O+','O-','UNKNOWN')),
          default_patient_type    TEXT    NOT NULL DEFAULT 'OUTPATIENT' CHECK (default_patient_type IN ('OUTPATIENT','INPATIENT','EMERGENCY')),
          emergency_contact_name  TEXT    NOT NULL,
          emergency_contact_phone TEXT    NOT NULL,
          is_deleted              INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
          created_at              TEXT    NOT NULL DEFAULT (datetime('now')),
          updated_at              TEXT    NOT NULL DEFAULT (datetime('now'))
      )
  )";
  if (!query.exec(createPatients)) {
    qDebug() << "Lỗi bảng Patients:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // Bảng Out Patient

  QString createOutPatientTable = R"(
      CREATE TABLE IF NOT EXISTS out_patients (
          patient_id INTEGER PRIMARY KEY,
          doctor_id           INTEGER,
          status              TEXT    NOT NULL DEFAULT 'REGISTERED' CHECK (status IN ('REGISTERED','WAITING FOR TREATMENT','TREATMENT','DISCHARGED')),
          created_at          TEXT    NOT NULL DEFAULT (datetime('now')),
          updated_at          TEXT    NOT NULL DEFAULT (datetime('now')),
          FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
          FOREIGN KEY (doctor_id) REFERENCES staff(staff_id) ON DELETE SET NULL
      );
  )";
  if (!query.exec(createOutPatientTable)) {
    qDebug() << "Lỗi bảng Out Patient:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // Bảng Emergency Patient
  QString createEmergencyPatientsTable = R"(
      CREATE TABLE IF NOT EXISTS emergency_patients (
          patient_id INTEGER PRIMARY KEY,
          room_id             INTEGER,
          doctor_id INTEGER,
          injury_cause        TEXT,
          injury_description  TEXT,
          admission_date      TEXT    NOT NULL DEFAULT (datetime('now')),
          discharge_date      TEXT,
          status              TEXT    NOT NULL DEFAULT 'EMERGENCY' CHECK (status IN ('EMERGENCY','DISCHARGED','TRANSFERRED')),
          created_at          TEXT    NOT NULL DEFAULT (datetime('now')),
          updated_at          TEXT    NOT NULL DEFAULT (datetime('now')),
          FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
          FOREIGN KEY (room_id) REFERENCES rooms(room_id) ON DELETE SET NULL,
          FOREIGN KEY (doctor_id) REFERENCES staff(staff_id) ON DELETE SET NULL
      );
  )";
  if (!query.exec(createEmergencyPatientsTable)) {
    qDebug() << "Lỗi bảng Emergency Patients:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // Bảng In Patient
  QString createInPatientTable = R"(
    CREATE TABLE IF NOT EXISTS in_patients (
    patient_id INTEGER PRIMARY KEY,
    room_id             INTEGER,
    doctor_id INTEGER,
    admission_date      TEXT    NOT NULL DEFAULT (datetime('now')),
    discharge_date      TEXT,
    reason              TEXT,
    status              TEXT    NOT NULL DEFAULT 'ADMITTED' CHECK (status IN ('ADMITTED','DISCHARGED','TRANSFERRED')),
    created_at          TEXT    NOT NULL DEFAULT (datetime('now')),
    updated_at          TEXT    NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
    FOREIGN KEY (room_id) REFERENCES rooms(room_id) ON DELETE SET NULL,
    FOREIGN KEY (doctor_id) REFERENCES staff(staff_id) ON DELETE SET NULL,
    CHECK (discharge_date IS NULL OR discharge_date >= admission_date)
  )
  )";

  if (!query.exec(createInPatientTable)) {
    qDebug() << "Lỗi bảng InPatientAdmissions:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // Bảng Patient Allergies
  QString createPatientAllergiesTable = R"(
      CREATE TABLE IF NOT EXISTS patient_allergies (
          allergy_id    INTEGER PRIMARY KEY AUTOINCREMENT,
          patient_id    INTEGER NOT NULL,
          allergen_name TEXT    NOT NULL COLLATE NOCASE,
          severity      TEXT    NOT NULL DEFAULT 'MODERATE' CHECK (severity IN ('MILD', 'MODERATE', 'SEVERE')),
          notes         TEXT,
          is_active     INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1)),
          recorded_at   TEXT    NOT NULL DEFAULT (datetime('now')),
          updated_at    TEXT    NOT NULL DEFAULT (datetime('now')),
          FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
          UNIQUE (patient_id, allergen_name)
      );
  )";
  if (!query.exec(createPatientAllergiesTable)) {
    qDebug() << "Lỗi bảng Patient Allergies:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // Bảng Patient Insurance (1 bản ghi / bệnh nhân)
  QString createPatientInsuranceTable = R"(
      CREATE TABLE IF NOT EXISTS patient_insurance (
          insurance_id     INTEGER PRIMARY KEY AUTOINCREMENT,
          patient_id       INTEGER NOT NULL UNIQUE,
          provider_name    TEXT    NOT NULL,
          policy_number    TEXT    NOT NULL,
          insurance_type   TEXT    NOT NULL DEFAULT 'BHYT' CHECK (insurance_type IN ('BHYT','PRIVATE','OTHER')),
          coverage_percent REAL    NOT NULL DEFAULT 80,
          valid_from       TEXT,
          valid_to         TEXT,
          notes            TEXT,
          is_active        INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1)),
          created_at       TEXT    NOT NULL DEFAULT (datetime('now')),
          updated_at       TEXT    NOT NULL DEFAULT (datetime('now')),
          FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE
      );
  )";
  if (!query.exec(createPatientInsuranceTable)) {
    qDebug() << "Lỗi bảng Patient Insurance:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  //   -- =====================================================================
  // -- SECTION 5: CLINICAL / ELECTRONIC MEDICAL RECORDS (EMR)
  // -- =====================================================================

  // -- 5.1 ------------------------------------------------------------------
  QString createMedicalRecords = R"(
      CREATE TABLE IF NOT EXISTS medical_records (
          record_id       INTEGER PRIMARY KEY AUTOINCREMENT,
          patient_id      INTEGER NOT NULL,
          doctor_id       INTEGER,
          appointment_id  INTEGER,
          visit_datetime  TEXT NOT NULL,
          temperature     REAL,
          blood_pressure  TEXT,
          heart_rate      INTEGER,
          weight          REAL,
          height          REAL,
          chief_complaint TEXT,
          clinical_notes  TEXT,
          treatment       TEXT,
          next_visit_date TEXT,
          is_deleted      INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
          created_at      TEXT DEFAULT (datetime('now')),
          FOREIGN KEY (patient_id) REFERENCES patients(patient_id),
          FOREIGN KEY (doctor_id) REFERENCES staff(staff_id),
          FOREIGN KEY (appointment_id) REFERENCES appointments(appointment_id)
      );
  )";

  if (!query.exec(createMedicalRecords)) {
    qDebug() << "Lỗi bảng Medical Records:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // -- 5.2 -- Diagnoses (1 record -> many diagnoses) ------------------------
  QString createDiagnoses = R"(
      CREATE TABLE IF NOT EXISTS diagnoses (
          diagnosis_id INTEGER PRIMARY KEY AUTOINCREMENT,
          record_id    INTEGER NOT NULL,
          icd_code     TEXT,
          description  TEXT    NOT NULL,
          severity     TEXT    NOT NULL DEFAULT 'MILD' CHECK (severity IN ('MILD','MODERATE','SEVERE')),
          FOREIGN KEY (record_id) REFERENCES medical_records(record_id) ON DELETE CASCADE
      );
  )";

  if (!query.exec(createDiagnoses)) {
    qDebug() << "Lỗi bảng Diagnoses:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // -- 5.3 -- Invoices (header) ---------------------------------------------
  const QString sqlInvoices = R"(
      CREATE TABLE IF NOT EXISTS invoices (
          invoice_id       INTEGER PRIMARY KEY AUTOINCREMENT,
          invoice_code     TEXT    NOT NULL UNIQUE,
          patient_id       INTEGER NOT NULL,
          record_id        INTEGER,
          patient_type     TEXT    NOT NULL CHECK (patient_type IN ('OUTPATIENT','INPATIENT','EMERGENCY')),
          consultation_fee REAL    NOT NULL DEFAULT 0,
          medication_fee   REAL    NOT NULL DEFAULT 0,
          total_amount     REAL    NOT NULL DEFAULT 0,
          status           TEXT    NOT NULL DEFAULT 'UNPAID' CHECK (status IN ('UNPAID','PAID','CANCELLED')),
          issued_date      TEXT    NOT NULL,
          paid_date        TEXT,
          created_at       TEXT DEFAULT (datetime('now')),
          FOREIGN KEY (patient_id) REFERENCES patients(patient_id),
          FOREIGN KEY (record_id)  REFERENCES medical_records(record_id)
      )
  )";
  if (!query.exec(sqlInvoices)) {
    qDebug() << "Lỗi bảng Invoices:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // -- 5.4 -- Invoice Items (detail) ----------------------------------------
  const QString sqlInvoiceItems = R"(
      CREATE TABLE IF NOT EXISTS invoice_items (
          item_id       INTEGER PRIMARY KEY AUTOINCREMENT,
          invoice_id    INTEGER NOT NULL,
          item_type     TEXT NOT NULL,
          description   TEXT NOT NULL,
          quantity      INTEGER NOT NULL DEFAULT 1,
          unit_price    REAL NOT NULL,
          subtotal      REAL NOT NULL,
          FOREIGN KEY (invoice_id) REFERENCES invoices(invoice_id)
      )
  )";
  if (!query.exec(sqlInvoiceItems)) {
    qDebug() << "Lỗi bảng Invoice Items:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // ── Phần bạn của bạn: Prescription + Medication ──

  const QString sqlActiveIngredients = R"(
    CREATE TABLE IF NOT EXISTS active_ingredients (
        ingredient_id   INTEGER PRIMARY KEY AUTOINCREMENT,
        ingredient_name TEXT NOT NULL,
        description     TEXT
    );
  )";
  if (!query.exec(sqlActiveIngredients)) {
    qDebug() << "Lỗi bảng Active Ingredients:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  const QString sqlMedications = R"(
    CREATE TABLE IF NOT EXISTS medications (
        medication_id       INTEGER PRIMARY KEY AUTOINCREMENT,
        brand_name          TEXT NOT NULL,
        category            TEXT,
        unit                TEXT NOT NULL,
        unit_price          REAL NOT NULL DEFAULT 0,
        stock_quantity      INTEGER NOT NULL DEFAULT 0,
        minimum_stock       INTEGER NOT NULL DEFAULT 10,
        reorder_threshold   INTEGER NOT NULL DEFAULT 20,
        expiry_date         TEXT,
        manufacturer        TEXT,
        description         TEXT,
        is_active           INTEGER NOT NULL DEFAULT 1,
        created_at          TEXT DEFAULT (datetime('now'))
    )
  )";
  if (!query.exec(sqlMedications)) {
    qDebug() << "Lỗi bảng Medications:" << query.lastError().text();
    m_db.rollback();
    return false;
  }


  const QString sqlMedicationIngredients = R"(
    CREATE TABLE IF NOT EXISTS medication_ingredients (
      medication_id   INTEGER NOT NULL,
      ingredient_id   INTEGER NOT NULL,
      strength        TEXT NOT NULL,            -- Hàm lượng (VD: "500mg", "65mg")
      
      PRIMARY KEY (medication_id, ingredient_id),
      FOREIGN KEY (medication_id) REFERENCES medications(medication_id) ON DELETE CASCADE,
      FOREIGN KEY (ingredient_id) REFERENCES active_ingredients(ingredient_id) ON DELETE RESTRICT
    );
  )";
  if (!query.exec(sqlMedicationIngredients)) {
    qDebug() << "Lỗi bảng Medication Ingredients:" << query.lastError().text();
    m_db.rollback();
    return false;
  }



  const QString sqlPrescriptions = R"(
    CREATE TABLE IF NOT EXISTS prescriptions (
      prescription_id   INTEGER PRIMARY KEY AUTOINCREMENT,
      doctor_id         INTEGER NOT NULL,
      record_id         INTEGER NOT NULL UNIQUE,
      notes             TEXT,
      prescribed_at     TEXT    NOT NULL DEFAULT (datetime('now')),
      status            TEXT    NOT NULL DEFAULT 'PENDING'
                                CHECK (status IN ('PENDING','DISPENSED','CANCELLED')),
      dispensed_by      INTEGER,
      dispensed_at      TEXT,

      cancelled_by      INTEGER,
      cancelled_at      TEXT,
      cancel_reason     TEXT,

      FOREIGN KEY (record_id)    REFERENCES medical_records(record_id) ON DELETE CASCADE,
      FOREIGN KEY (doctor_id)    REFERENCES staff(staff_id),
      FOREIGN KEY (dispensed_by) REFERENCES staff(staff_id) ON DELETE SET NULL,
      FOREIGN KEY (cancelled_by) REFERENCES staff(staff_id) ON DELETE SET NULL,
      
      CHECK (NOT (dispensed_by IS NOT NULL AND cancelled_by IS NOT NULL))
    );
  )";
  if (!query.exec(sqlPrescriptions)) {
    qDebug() << "Lỗi bảng Prescriptions:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  const QString sqlPrescriptionItems = R"(
    CREATE TABLE IF NOT EXISTS prescription_items (
      prescription_id   INTEGER NOT NULL,
      medication_id     INTEGER NOT NULL,
      brand_name        TEXT NOT NULL,
      unit_price        REAL NOT NULL CHECK (unit_price >= 0),
      quantity          INTEGER NOT NULL CHECK (quantity > 0),
      dosage            TEXT,
      frequency         TEXT,
      duration_days     INTEGER NOT NULL CHECK (duration_days > 0),
      note              TEXT,
      PRIMARY KEY (prescription_id, medication_id),
      FOREIGN KEY (prescription_id) REFERENCES prescriptions(prescription_id) ON DELETE CASCADE,
      FOREIGN KEY (medication_id) REFERENCES medications(medication_id) ON DELETE RESTRICT
    );
  )";
  if (!query.exec(sqlPrescriptionItems)) {
    qDebug() << "Lỗi bảng Prescription Items:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  

  // Bảng Staff
  QString createStaff = R"(
      CREATE TABLE IF NOT EXISTS staff (
          staff_id      INTEGER PRIMARY KEY AUTOINCREMENT,
          staff_code    TEXT    NOT NULL UNIQUE,
          password_hash TEXT    NOT NULL,
          full_name     TEXT    NOT NULL,
          avatar        BLOB,
          role          TEXT    NOT NULL CHECK (role IN ('ADMIN','DOCTOR','NURSE','RECEPTIONIST')),
          gender        TEXT    NOT NULL CHECK (gender IN ('MALE','FEMALE','OTHER')),
          date_of_birth TEXT    NOT NULL,
          citizen_id    TEXT    NOT NULL UNIQUE,
          phone_number  TEXT    NOT NULL,
          email         TEXT    NOT NULL UNIQUE,
          address       TEXT    NOT NULL,
          department_id INTEGER NOT NULL,
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
  QString createLoginInformation = R"(
      CREATE TABLE IF NOT EXISTS login_information (
          user_id       INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
          username      TEXT NOT NULL,
          password_hash TEXT NOT NULL,
          created_at    TEXT NOT NULL,
          staff_id      INTEGER NOT NULL,
          account_type  TEXT NOT NULL,
          CONSTRAINT login_information_staff_FK FOREIGN KEY (staff_id) REFERENCES staff(staff_id)
      );
  )";
  if (!query.exec(createLoginInformation)) {
    qDebug() << "Lỗi bảng Login Information:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // -------------------------------------------------

  // 4. Bảng Appointments
  QString createAppointments = R"(
    CREATE TABLE IF NOT EXISTS appointments (
        appointment_id   INTEGER PRIMARY KEY AUTOINCREMENT,
        patient_id       INTEGER NOT NULL,
        doctor_id        INTEGER NOT NULL,
        room_id          INTEGER,
        created_by       INTEGER,
        appointment_date TEXT    NOT NULL,
        start_time       TEXT    NOT NULL,
        end_time         TEXT,
        status           TEXT    NOT NULL DEFAULT 'SCHEDULED'
                              CHECK (status IN ('SCHEDULED','CONFIRMED','CHECKED_IN','COMPLETED','CANCELLED','NO_SHOW')),
        reason           TEXT,
        notes            TEXT,
        created_at       TEXT    NOT NULL DEFAULT (datetime('now')),
        updated_at       TEXT    NOT NULL DEFAULT (datetime('now')),
        FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
        FOREIGN KEY (doctor_id)  REFERENCES staff(staff_id)    ON DELETE RESTRICT,
        FOREIGN KEY (room_id)    REFERENCES rooms(room_id) ON DELETE SET NULL,
        FOREIGN KEY (created_by) REFERENCES staff(staff_id) ON DELETE SET NULL,
        UNIQUE (doctor_id, appointment_date, start_time),
        CHECK (end_time IS NULL OR end_time > start_time)
    )
  )";
  if (!query.exec(createAppointments)) {
    qDebug() << "Lỗi bảng Appointments:" << query.lastError().text();
    m_db.rollback();
    return false;
  }

  // ============= TẠO INDEX ===============
  QStringList createIndexList = {
      R"( CREATE INDEX IF NOT EXISTS idx_staff_role               ON staff(role);             )",
      R"( CREATE INDEX IF NOT EXISTS idx_staff_department         ON staff(department_id);    )",
      // R"( CREATE INDEX IF NOT EXISTS idx_patients_full_name       ON
      // patients(full_name);     )"
  };

  for (const QString &createIndex : createIndexList) {
    QString create = createIndex.trimmed();

    if (create.isEmpty())
      continue;

    if (!query.exec(create)) {
      qDebug() << "Lỗi tạo index:" << query.lastError().text();
      m_db.rollback();
      return false;
    }
  }

  // ============= TẠO TRIGGER ===============

  QStringList createTriggerList = {
      R"( 
            CREATE TRIGGER IF NOT EXISTS trg_departments_updated_at
            AFTER UPDATE ON departments FOR EACH ROW
            WHEN OLD.updated_at = NEW.updated_at
            BEGIN
                UPDATE departments SET updated_at = datetime('now') WHERE department_id = OLD.department_id;
            END;
        )",
      R"(
            CREATE TRIGGER IF NOT EXISTS trg_rooms_updated_at
            AFTER UPDATE ON rooms FOR EACH ROW
            WHEN OLD.updated_at = NEW.updated_at
            BEGIN
                UPDATE rooms SET updated_at = datetime('now') WHERE room_id = OLD.room_id;
            END;
        )",
      R"(
            CREATE TRIGGER IF NOT EXISTS trg_staff_updated_at
            AFTER UPDATE ON staff FOR EACH ROW
            WHEN OLD.updated_at = NEW.updated_at
            BEGIN
                UPDATE staff SET updated_at = datetime('now') WHERE staff_id = OLD.staff_id;
            END;
        )",
      R"(
            CREATE TRIGGER IF NOT EXISTS validate_staff_dob_insert
            BEFORE INSERT ON staff
            FOR EACH ROW
            WHEN NEW.date_of_birth > date('now') -- Bỏ phần kiểm tra IS NOT NULL đi vì đã có NOT NULL ở định nghĩa bảng
            BEGIN
                SELECT RAISE(ABORT, 'LỖI_SQLITE: Ngày sinh không được lớn hơn ngày hiện tại!');
            END;
        )",
      R"(
            CREATE TRIGGER IF NOT EXISTS validate_staff_dob_update
            BEFORE UPDATE ON staff
            FOR EACH ROW
            WHEN NEW.date_of_birth > date('now')
            BEGIN
                SELECT RAISE(ABORT, 'LỖI_SQLITE: Ngày sinh không được lớn hơn ngày hiện tại!');
            END;
        )"

  };

  for (const QString &createTrigger : createTriggerList) {
    QString create = createTrigger.trimmed();

    if (create.isEmpty())
      continue;

    if (!query.exec(create)) {
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

QSqlQuery DatabaseManager::executeQuery(const QString &sql,
                                        const QVariantList &params) {
    QSqlQuery query(m_db);

    if (!query.prepare(sql)) {
        qDebug() << "Lỗi prepare query:" << query.lastError().text()
                 << "| SQL:" << sql;
        return query; // Trả về đối tượng query lỗi
    }

    for (const QVariant &param : params) {
        query.addBindValue(param);
    }

    if (!query.exec()) {
        qDebug() << "Lỗi exec query:" << query.lastError().text()
                 << "| SQL:" << sql;
    }

    return query;
}

QSqlQuery DatabaseManager::selectQuery(const QString &sql,
                                       const QVariantList &params) {
  QSqlQuery query(m_db);

  if (!query.prepare(sql)) {
    qDebug() << "Lỗi prepare query:" << query.lastError().text()
             << "| SQL:" << sql;
    return query;
  }

  for (const QVariant &param : params) {
    query.addBindValue(param);
  }

  if (!query.exec()) {
    qDebug() << "Lỗi exec select query:" << query.lastError().text()
             << "| SQL:" << sql;
  }

  return query;
}

QList<DatabaseManager::AppointmentRecord> DatabaseManager::getDoctorAppointments(const QString &doctorId, const QString &date) {
    QList<AppointmentRecord> list;

    QString sql = R"(
        SELECT a.appointment_id, a.patient_id, a.doctor_id, a.appointment_date, a.start_time, a.end_time, a.status, a.reason, a.notes, p.full_name, p.patient_code, r.room_number
        FROM appointments a
        JOIN patients p ON a.patient_id = p.patient_id
        LEFT JOIN rooms r ON a.room_id = r.room_id
        JOIN staff s ON (a.doctor_id = s.staff_id OR a.doctor_id = s.staff_code)
        WHERE s.staff_id = ?
    )";
    QVariantList params = {doctorId};
    if (!date.isEmpty()) {
        sql += " AND a.appointment_date = ?";
        params << date;
    }
    sql += " ORDER BY a.appointment_date DESC, a.start_time ASC";

    qDebug() << "getDoctorAppointments - Executing query for doctorId:" << doctorId << "| Date:" << (date.isEmpty() ? "All" : date);
    QSqlQuery query = selectQuery(sql, params);

    int rowCount = 0;
    while (query.next()) {
        rowCount++;
        AppointmentRecord rec;
        rec.appointmentId = query.value(0).toInt();
        rec.patientId = query.value(1).toInt();
        rec.doctorId = query.value(2).toString();
        rec.appointmentDate = query.value(3).toString();
        rec.startTime = query.value(4).toString();
        rec.endTime = query.value(5).toString();
        rec.status = query.value(6).toString();
        rec.reason = query.value(7).toString();
        rec.notes = query.value(8).toString();
        rec.patientName = query.value(9).toString();
        rec.patientCode = query.value(10).toString();
        rec.roomNumber = query.value(11).toString();
        if (rec.roomNumber.isEmpty()) rec.roomNumber = "N/A";

        qDebug() << "  -> Row" << rowCount << ":" << rec.startTime << "|" << rec.patientName << "|" << rec.status;
        list.append(rec);
    }
    qDebug() << "getDoctorAppointments - Loaded" << rowCount << "records.";
    return list;
}

bool DatabaseManager::updateAppointmentStatus(int appointmentId, const QString &status) {
    QSqlQuery query = executeQuery(
        "UPDATE appointments SET status = ? WHERE appointment_id = ?",
        {status, appointmentId});
    return !query.lastError().isValid();
}

std::optional<DatabaseManager::PatientRecord> DatabaseManager::getPatientByPhoneOrCitizenId(const QString &phone, const QString &citizenId) {
    QString sql = "SELECT patient_id, patient_code, full_name, phone FROM patients WHERE is_deleted = 0 AND (";
    QVariantList params;
    QStringList conditions;
    if (!phone.isEmpty()) {
        conditions << "phone = ?";
        params << phone;
    }
    if (!citizenId.isEmpty()) {
        conditions << "citizen_id = ?";
        params << citizenId;
    }
    if (conditions.isEmpty()) return std::nullopt;
    sql += conditions.join(" OR ") + ")";
    
    QSqlQuery query = selectQuery(sql, params);
    if (query.next()) {
        PatientRecord rec;
        rec.patientId = query.value(0).toInt();
        rec.patientCode = query.value(1).toString();
        rec.fullName = query.value(2).toString();
        rec.phone = query.value(3).toString();
        return rec;
    }
    return std::nullopt;
}

bool DatabaseManager::createAppointment(int patientId, const QString &doctorCode, int createdBy, const QString &date, const QString &startTime, const QString &reason) {
    QString sql = R"(
        INSERT INTO appointments (patient_id, doctor_id, created_by, appointment_date, start_time, status, reason)
        VALUES (?, ?, ?, ?, ?, 'SCHEDULED', ?)
    )";
    QSqlQuery query = executeQuery(sql, {patientId, doctorCode, createdBy, date, startTime, reason});
    return !query.lastError().isValid();
}