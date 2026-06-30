-- =====================================================================
-- SMART HOSPITAL / CLINIC MANAGEMENT SYSTEM
-- SQLite Database Schema (Production-ready, normalized to 3NF)
-- C++17 / Qt6 SQL Module backend
--
-- IMPORTANT (Qt Integration):
--   SQLite enforces FOREIGN KEY constraints only if the pragma below
--   is executed on EVERY connection. In DatabaseManager (Singleton),
--   call this immediately after QSqlDatabase::open():
--
--       QSqlQuery("PRAGMA foreign_keys = ON;");
-- =====================================================================

PRAGMA foreign_keys = ON;

-- =====================================================================
-- SECTION 1: ORGANIZATIONAL STRUCTURE
-- =====================================================================

-- 1.1 ----------------------------------------------------------------
CREATE TABLE IF NOT EXISTS departments (
    department_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    department_code TEXT    NOT NULL UNIQUE,
    department_name TEXT    NOT NULL,
    description      TEXT,
    is_deleted        INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
    created_at        TEXT    NOT NULL DEFAULT (datetime('now')),
    updated_at        TEXT    NOT NULL DEFAULT (datetime('now'))
);

-- 1.2 ----------------------------------------------------------------
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

-- =====================================================================
-- SECTION 2: HUMAN RESOURCES (UNIFIED STAFF / RBAC)
-- =====================================================================

-- 2.1 -- Unified Staff table (maps to SystemUser abstract class) ------
-- NOTE: staff_code đóng vai trò login identifier (thay thế username riêng)
--       Nhóm đã thống nhất dùng staff_code để đăng nhập thay vì tạo username riêng.
CREATE TABLE IF NOT EXISTS staff (
    staff_id      INTEGER PRIMARY KEY AUTOINCREMENT,
    staff_code    TEXT    NOT NULL UNIQUE,  -- login ID kiêm mã nhân viên
    password_hash TEXT    NOT NULL,
    full_name     TEXT    NOT NULL,
    avatar        BLOB,
    role          TEXT    NOT NULL CHECK (role IN ('ADMIN','DOCTOR','NURSE','RECEPTIONIST')),
    gender        TEXT    NOT NULL CHECK (gender IN ('MALE','FEMALE','OTHER')),
    date_of_birth TEXT    NOT NULL,
    citizen_id   TEXT     NOT NULL UNIQUE,
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

-- 2.2 -- Role-specific extension: Doctor (1-to-1 with staff) ----------
CREATE TABLE IF NOT EXISTS doctor_profiles (
    staff_id         INTEGER PRIMARY KEY,
    specialty        TEXT    NOT NULL,
    license_number   TEXT    NOT NULL UNIQUE,
    experience_years INTEGER NOT NULL DEFAULT 0 CHECK (experience_years >= 0),
    consultation_fee REAL    NOT NULL DEFAULT 0 CHECK (consultation_fee >= 0),
    bio              TEXT,
    FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE
);

-- 2.3 -- Role-specific extension: Nurse (1-to-1 with staff) -----------
CREATE TABLE IF NOT EXISTS nurse_profiles (
    staff_id      INTEGER PRIMARY KEY,
    nurse_level   TEXT NOT NULL DEFAULT 'JUNIOR' CHECK (nurse_level IN ('JUNIOR','SENIOR','HEAD')),
    certification TEXT,
    FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE
);

-- NOTE: Receptionist has no extra attributes beyond `staff` -> no
-- profile table needed (avoids an empty 1-1 table / redundancy).

-- =====================================================================
-- SECTION 3: PATIENTS
-- =====================================================================

-- 3.1 ------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS patients (
    patient_id              INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_code            TEXT    NOT NULL UNIQUE,
    full_name               TEXT    NOT NULL,
    date_of_birth           TEXT    NOT NULL,
    gender                  TEXT    NOT NULL CHECK (gender IN ('MALE','FEMALE','OTHER')),
    citizen_id              TEXT    NOT NULL UNIQUE,
    phone                   TEXT    NOT NULL,
    email                   TEXT,
    address                 TEXT,
    blood_type              TEXT    NOT NULL DEFAULT 'UNKNOWN'
                                     CHECK (blood_type IN ('A+','A-','B+','B-','AB+','AB-','O+','O-','UNKNOWN')),
    default_patient_type    TEXT    NOT NULL DEFAULT 'OUTPATIENT'
                                     CHECK (default_patient_type IN ('OUTPATIENT','INPATIENT','EMERGENCY')),
    emergency_contact_name  TEXT,
    emergency_contact_phone TEXT,
    is_deleted              INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),
    created_at              TEXT    NOT NULL DEFAULT (datetime('now')),
    updated_at              TEXT    NOT NULL DEFAULT (datetime('now'))
);

-- 3.2 -- Normalized allergy list (1 patient -> many allergies) --------
CREATE TABLE IF NOT EXISTS patient_allergies (
    allergy_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id    INTEGER NOT NULL,
    allergen_name TEXT    NOT NULL,
    severity      TEXT    NOT NULL DEFAULT 'MODERATE' CHECK (severity IN ('MILD','MODERATE','SEVERE')),
    notes         TEXT,
    recorded_at   TEXT    NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
    UNIQUE (patient_id, allergen_name)
);

-- 3.3 -- Insurance providers (lookup table) ----------------------------
CREATE TABLE IF NOT EXISTS insurance_providers (
    provider_id         INTEGER PRIMARY KEY AUTOINCREMENT,
    provider_name       TEXT    NOT NULL UNIQUE,
    coverage_percentage REAL    NOT NULL CHECK (coverage_percentage BETWEEN 0 AND 100),
    contact_info        TEXT,
    is_deleted          INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1))
);

-- 3.4 -- Patient <-> Insurance (many-to-many via policy records) ------
CREATE TABLE IF NOT EXISTS patient_insurance (
    patient_insurance_id INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id           INTEGER NOT NULL,
    provider_id          INTEGER NOT NULL,
    policy_number        TEXT    NOT NULL,
    valid_from           TEXT    NOT NULL,
    valid_to             TEXT,
    is_active            INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1)),
    FOREIGN KEY (patient_id)  REFERENCES patients(patient_id) ON DELETE CASCADE,
    FOREIGN KEY (provider_id) REFERENCES insurance_providers(provider_id) ON DELETE RESTRICT,
    UNIQUE (provider_id, policy_number),
    CHECK (valid_to IS NULL OR valid_to >= valid_from)
);

-- 3.5 -- Inpatient admission tracking (maps to InPatient subclass) ----
CREATE TABLE IF NOT EXISTS inpatient_admissions (
    admission_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id          INTEGER NOT NULL,
    room_id             INTEGER,
    admitting_doctor_id INTEGER,
    admission_date      TEXT    NOT NULL DEFAULT (datetime('now')),
    discharge_date      TEXT,
    reason              TEXT,
    status              TEXT    NOT NULL DEFAULT 'ADMITTED' CHECK (status IN ('ADMITTED','DISCHARGED','TRANSFERRED')),
    created_at          TEXT    NOT NULL DEFAULT (datetime('now')),
    updated_at          TEXT    NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
    FOREIGN KEY (room_id) REFERENCES rooms(room_id) ON DELETE SET NULL,
    FOREIGN KEY (admitting_doctor_id) REFERENCES staff(staff_id) ON DELETE SET NULL,
    CHECK (discharge_date IS NULL OR discharge_date >= admission_date)
);

-- =====================================================================
-- SECTION 4: SCHEDULING & QUEUE
-- =====================================================================

-- 4.1 ------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS appointments (
    appointment_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id       INTEGER NOT NULL,
    doctor_id        TEXT    NOT NULL,
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
    FOREIGN KEY (doctor_id)  REFERENCES staff(staff_code) ON DELETE RESTRICT,
    FOREIGN KEY (room_id)    REFERENCES rooms(room_id) ON DELETE SET NULL,
    FOREIGN KEY (created_by) REFERENCES staff(staff_id) ON DELETE SET NULL,
    UNIQUE (doctor_id, appointment_date, start_time),
    CHECK (end_time IS NULL OR end_time > start_time)
);

-- 4.2 -- Queue ticket (State Pattern persistence) ----------------------
CREATE TABLE IF NOT EXISTS queue_tickets (
    ticket_id      INTEGER PRIMARY KEY AUTOINCREMENT,
    ticket_number  INTEGER NOT NULL CHECK (ticket_number > 0),
    patient_id     INTEGER NOT NULL,
    doctor_id      INTEGER,
    appointment_id INTEGER,
    room_id        INTEGER,
    queue_date     TEXT    NOT NULL DEFAULT (date('now')),
    priority_level INTEGER NOT NULL DEFAULT 0 CHECK (priority_level BETWEEN 0 AND 3),
    status         TEXT    NOT NULL DEFAULT 'REGISTERED'
                            CHECK (status IN ('REGISTERED','WAITING','CALLED','IN_PROGRESS','COMPLETED','CANCELLED')),
    registered_at  TEXT    NOT NULL DEFAULT (datetime('now')),
    called_at      TEXT,
    started_at     TEXT,
    completed_at   TEXT,
    cancel_reason  TEXT,
    FOREIGN KEY (patient_id)     REFERENCES patients(patient_id) ON DELETE CASCADE,
    FOREIGN KEY (doctor_id)      REFERENCES staff(staff_id) ON DELETE SET NULL,
    FOREIGN KEY (appointment_id) REFERENCES appointments(appointment_id) ON DELETE SET NULL,
    FOREIGN KEY (room_id)        REFERENCES rooms(room_id) ON DELETE SET NULL,
    UNIQUE (queue_date, ticket_number)
);

-- =====================================================================
-- SECTION 5: CLINICAL / ELECTRONIC MEDICAL RECORDS (EMR)
-- =====================================================================

-- 5.1 ------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS medical_records (
    record_id                INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id               INTEGER NOT NULL,
    doctor_id                INTEGER NOT NULL,
    appointment_id           INTEGER,
    encounter_type           TEXT    NOT NULL DEFAULT 'OUTPATIENT'
                                      CHECK (encounter_type IN ('OUTPATIENT','INPATIENT','EMERGENCY')),
    visit_datetime           TEXT    NOT NULL DEFAULT (datetime('now')),
    chief_complaint          TEXT,
    clinical_notes           TEXT,
    temperature_celsius      REAL    CHECK (temperature_celsius IS NULL OR temperature_celsius BETWEEN 30 AND 45),
    blood_pressure_systolic  INTEGER CHECK (blood_pressure_systolic  IS NULL OR blood_pressure_systolic  > 0),
    blood_pressure_diastolic INTEGER CHECK (blood_pressure_diastolic IS NULL OR blood_pressure_diastolic > 0),
    heart_rate_bpm           INTEGER CHECK (heart_rate_bpm IS NULL OR heart_rate_bpm > 0),
    weight_kg                REAL    CHECK (weight_kg IS NULL OR weight_kg > 0),
    height_cm                REAL    CHECK (height_cm IS NULL OR height_cm > 0),
    treatment_plan           TEXT,
    next_visit_date          TEXT,
    created_at               TEXT    NOT NULL DEFAULT (datetime('now')),
    updated_at               TEXT    NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (patient_id)     REFERENCES patients(patient_id) ON DELETE CASCADE,
    FOREIGN KEY (doctor_id)      REFERENCES staff(staff_id) ON DELETE RESTRICT,
    FOREIGN KEY (appointment_id) REFERENCES appointments(appointment_id) ON DELETE SET NULL
);

-- 5.2 -- Diagnoses (1 record -> many diagnoses) ------------------------
CREATE TABLE IF NOT EXISTS diagnoses (
    diagnosis_id INTEGER PRIMARY KEY AUTOINCREMENT,
    record_id    INTEGER NOT NULL,
    icd_code     TEXT,
    description  TEXT    NOT NULL,
    severity     TEXT    NOT NULL DEFAULT 'MILD' CHECK (severity IN ('MILD','MODERATE','SEVERE')),
    is_primary   INTEGER NOT NULL DEFAULT 0 CHECK (is_primary IN (0,1)),
    FOREIGN KEY (record_id) REFERENCES medical_records(record_id) ON DELETE CASCADE
);

-- =====================================================================
-- SECTION 6: PHARMACY & PRESCRIPTIONS
-- =====================================================================

-- 6.1 -- Medication categories (lookup) --------------------------------
CREATE TABLE IF NOT EXISTS medication_categories (
    category_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    category_name TEXT NOT NULL UNIQUE,
    description   TEXT
);

-- 6.2 -- Medications / Pharmacy Inventory ------------------------------
CREATE TABLE IF NOT EXISTS medications (
    medication_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    medication_code  TEXT    NOT NULL UNIQUE,
    medication_name  TEXT    NOT NULL,
    category_id      INTEGER,
    unit             TEXT    NOT NULL,
    unit_price       REAL    NOT NULL CHECK (unit_price >= 0),
    stock_quantity   INTEGER NOT NULL DEFAULT 0 CHECK (stock_quantity >= 0),
    reorder_threshold INTEGER NOT NULL DEFAULT 10 CHECK (reorder_threshold >= 0),
    expiry_date      TEXT,
    manufacturer     TEXT,
    description      TEXT,
    is_active        INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1)),
    created_at       TEXT    NOT NULL DEFAULT (datetime('now')),
    updated_at       TEXT    NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (category_id) REFERENCES medication_categories(category_id) ON DELETE SET NULL
);

-- 6.3 -- Prescription header (1 medical record -> many prescriptions) -
CREATE TABLE IF NOT EXISTS prescriptions (
    prescription_id INTEGER PRIMARY KEY AUTOINCREMENT,
    record_id       INTEGER NOT NULL,
    doctor_id       INTEGER NOT NULL,
    prescribed_at   TEXT    NOT NULL DEFAULT (datetime('now')),
    status          TEXT    NOT NULL DEFAULT 'PENDING' CHECK (status IN ('PENDING','DISPENSED','CANCELLED')),
    notes           TEXT,
    FOREIGN KEY (record_id) REFERENCES medical_records(record_id) ON DELETE CASCADE,
    FOREIGN KEY (doctor_id) REFERENCES staff(staff_id) ON DELETE RESTRICT
);

-- 6.4 -- Prescription line items (many-to-many: prescription <-> medication)
CREATE TABLE IF NOT EXISTS prescription_items (
    item_id             INTEGER PRIMARY KEY AUTOINCREMENT,
    prescription_id     INTEGER NOT NULL,
    medication_id       INTEGER NOT NULL,
    quantity            INTEGER NOT NULL CHECK (quantity > 0),
    dosage_instruction  TEXT    NOT NULL,
    duration_days       INTEGER CHECK (duration_days IS NULL OR duration_days > 0),
    unit_price_snapshot REAL    NOT NULL CHECK (unit_price_snapshot >= 0),
    FOREIGN KEY (prescription_id) REFERENCES prescriptions(prescription_id) ON DELETE CASCADE,
    FOREIGN KEY (medication_id)   REFERENCES medications(medication_id) ON DELETE RESTRICT
);

-- =====================================================================
-- SECTION 7: BILLING / INVOICES
-- =====================================================================

-- 7.1 ------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS invoices (
    invoice_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    invoice_code     TEXT    NOT NULL UNIQUE,
    patient_id       INTEGER NOT NULL,
    record_id        INTEGER,
    invoice_type     TEXT    NOT NULL CHECK (invoice_type IN ('OUTPATIENT','INPATIENT')),
    pricing_strategy TEXT    NOT NULL DEFAULT 'STANDARD'
                              CHECK (pricing_strategy IN ('STANDARD','INSURANCE','SENIOR_DISCOUNT')),
    subtotal         REAL    NOT NULL DEFAULT 0 CHECK (subtotal >= 0),
    discount_amount  REAL    NOT NULL DEFAULT 0 CHECK (discount_amount >= 0),
    tax_amount       REAL    NOT NULL DEFAULT 0 CHECK (tax_amount >= 0),
    total_amount     REAL    NOT NULL DEFAULT 0 CHECK (total_amount >= 0),
    payment_status   TEXT    NOT NULL DEFAULT 'UNPAID'
                              CHECK (payment_status IN ('UNPAID','PARTIAL','PAID','REFUNDED')),
    payment_method   TEXT    CHECK (payment_method IS NULL OR payment_method IN ('CASH','CARD','BANK_TRANSFER','INSURANCE','E_WALLET')),
    issued_by        INTEGER,
    issued_at        TEXT    NOT NULL DEFAULT (datetime('now')),
    created_at       TEXT    NOT NULL DEFAULT (datetime('now')),
    updated_at       TEXT    NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE RESTRICT,
    FOREIGN KEY (record_id)  REFERENCES medical_records(record_id) ON DELETE SET NULL,
    FOREIGN KEY (issued_by)  REFERENCES staff(staff_id) ON DELETE SET NULL
);

-- 7.2 -- Invoice line items (polymorphic: consultation/medication/...) -
CREATE TABLE IF NOT EXISTS invoice_items (
    invoice_item_id INTEGER PRIMARY KEY AUTOINCREMENT,
    invoice_id      INTEGER NOT NULL,
    item_type       TEXT    NOT NULL CHECK (item_type IN ('CONSULTATION','MEDICATION','ROOM_FEE','SERVICE','PROCEDURE','OTHER')),
    reference_id    INTEGER,
    description     TEXT    NOT NULL,
    quantity        INTEGER NOT NULL DEFAULT 1 CHECK (quantity > 0),
    unit_price      REAL    NOT NULL CHECK (unit_price >= 0),
    line_total      REAL    NOT NULL CHECK (line_total >= 0 AND line_total = quantity * unit_price),
    FOREIGN KEY (invoice_id) REFERENCES invoices(invoice_id) ON DELETE CASCADE
);

-- =====================================================================
-- SECTION 8: SYSTEM / AUDIT
-- =====================================================================

-- 8.1 ------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS audit_logs (
    log_id             INTEGER PRIMARY KEY AUTOINCREMENT,
    staff_id           INTEGER,
    username_attempted TEXT,    -- ✅ Thêm mới: lưu username khi login thất bại (staff_id lúc đó là NULL)
    action             TEXT NOT NULL,
    module_code        TEXT,
    details            TEXT,
    ip_address         TEXT,
    created_at         TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE SET NULL
);

-- 8.2 -- Attendance records (Smart Attendance Board) -------------------
-- ✅ Bảng mới: lưu dữ liệu chấm công thực tế theo từng ngày
-- (staff.shift chỉ là ca phân công cố định — không phải chấm công thực tế)
CREATE TABLE IF NOT EXISTS attendance_records (
    attendance_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    staff_id         INTEGER NOT NULL,
    work_date        TEXT    NOT NULL DEFAULT (date('now')),
    scheduled_shift  TEXT    NOT NULL CHECK (scheduled_shift IN ('MORNING','AFTERNOON','NIGHT','FULL_DAY')),
    check_in_time    TEXT,
    check_out_time   TEXT,
    status           TEXT    NOT NULL DEFAULT 'PRESENT'
                              CHECK (status IN ('PRESENT','LATE','ABSENT','ON_LEAVE')),
    notes            TEXT,
    FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE,
    UNIQUE (staff_id, work_date),
    CHECK (check_out_time IS NULL OR check_out_time > check_in_time)
);

-- =====================================================================
-- SECTION 9: INDEXES (Performance Optimization)
-- =====================================================================

CREATE INDEX IF NOT EXISTS idx_staff_role               ON staff(role);
CREATE INDEX IF NOT EXISTS idx_staff_department         ON staff(department_id);
CREATE INDEX IF NOT EXISTS idx_patients_full_name       ON patients(full_name);
CREATE INDEX IF NOT EXISTS idx_patients_phone           ON patients(phone_number);    -- ✅ Cập nhật: phone → phone_number
CREATE INDEX IF NOT EXISTS idx_appointments_doctor_date ON appointments(doctor_id, appointment_date);
CREATE INDEX IF NOT EXISTS idx_appointments_patient     ON appointments(patient_id);
CREATE INDEX IF NOT EXISTS idx_queue_date_status        ON queue_tickets(queue_date, status);
CREATE INDEX IF NOT EXISTS idx_records_patient          ON medical_records(patient_id);
CREATE INDEX IF NOT EXISTS idx_records_doctor           ON medical_records(doctor_id);
CREATE INDEX IF NOT EXISTS idx_prescriptions_record     ON prescriptions(record_id);
CREATE INDEX IF NOT EXISTS idx_invoices_patient         ON invoices(patient_id);
CREATE INDEX IF NOT EXISTS idx_invoices_status          ON invoices(payment_status);
CREATE INDEX IF NOT EXISTS idx_medications_name         ON medications(medication_name);
CREATE INDEX IF NOT EXISTS idx_medications_low_stock    ON medications(stock_quantity);
CREATE INDEX IF NOT EXISTS idx_attendance_staff_date    ON attendance_records(staff_id, work_date); -- ✅ Mới: index chấm công

-- =====================================================================
-- SECTION 10: TRIGGERS (auto-maintain updated_at timestamps)
-- =====================================================================

-- =====================================================================
-- GIẢI THÍCH CÁCH FIX INFINITY LOOP:
-- Thêm mệnh đề WHEN OLD.updated_at = NEW.updated_at vào mỗi trigger.
-- Khi trigger tự chạy UPDATE ... SET updated_at = datetime('now'),
-- lần kích hoạt kế tiếp sẽ thấy NEW.updated_at != OLD.updated_at
-- → điều kiện WHEN sai → trigger KHÔNG chạy → vòng lặp dừng lại.
-- =====================================================================

CREATE TRIGGER IF NOT EXISTS validate_staff_dob_insert
BEFORE INSERT ON staff
FOR EACH ROW
WHEN NEW.date_of_birth > date('now') -- Bỏ phần kiểm tra IS NOT NULL đi vì đã có NOT NULL ở định nghĩa bảng
BEGIN
    SELECT RAISE(ABORT, 'LỖI_SQLITE: Ngày sinh không được lớn hơn ngày hiện tại!');
END;

CREATE TRIGGER IF NOT EXISTS validate_staff_dob_update
BEFORE UPDATE ON staff
FOR EACH ROW
WHEN NEW.date_of_birth > date('now')
BEGIN
    SELECT RAISE(ABORT, 'LỖI_SQLITE: Ngày sinh không được lớn hơn ngày hiện tại!');
END;

CREATE TRIGGER IF NOT EXISTS validate_staff_dob_insert
BEFORE INSERT ON patients
FOR EACH ROW
WHEN NEW.date_of_birth > date('now') -- Bỏ phần kiểm tra IS NOT NULL đi vì đã có NOT NULL ở định nghĩa bảng
BEGIN
    SELECT RAISE(ABORT, 'LỖI_SQLITE: Ngày sinh không được lớn hơn ngày hiện tại!');
END;

CREATE TRIGGER IF NOT EXISTS validate_staff_dob_update
BEFORE UPDATE ON patients
FOR EACH ROW
WHEN NEW.date_of_birth > date('now')
BEGIN
    SELECT RAISE(ABORT, 'LỖI_SQLITE: Ngày sinh không được lớn hơn ngày hiện tại!');
END;

CREATE TRIGGER IF NOT EXISTS trg_departments_updated_at
AFTER UPDATE ON departments FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at
BEGIN
    UPDATE departments SET updated_at = datetime('now') WHERE department_id = OLD.department_id;
END;

CREATE TRIGGER IF NOT EXISTS trg_rooms_updated_at
AFTER UPDATE ON rooms FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at
BEGIN
    UPDATE rooms SET updated_at = datetime('now') WHERE room_id = OLD.room_id;
END;

CREATE TRIGGER IF NOT EXISTS trg_staff_updated_at
AFTER UPDATE ON staff FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at
BEGIN
    UPDATE staff SET updated_at = datetime('now') WHERE staff_id = OLD.staff_id;
END;

CREATE TRIGGER IF NOT EXISTS trg_patients_updated_at
AFTER UPDATE ON patients FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at
BEGIN
    UPDATE patients SET updated_at = datetime('now') WHERE patient_id = OLD.patient_id;
END;

CREATE TRIGGER IF NOT EXISTS trg_appointments_updated_at
AFTER UPDATE ON appointments FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at
BEGIN
    UPDATE appointments SET updated_at = datetime('now') WHERE appointment_id = OLD.appointment_id;
END;

CREATE TRIGGER IF NOT EXISTS trg_medical_records_updated_at
AFTER UPDATE ON medical_records FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at
BEGIN
    UPDATE medical_records SET updated_at = datetime('now') WHERE record_id = OLD.record_id;
END;

CREATE TRIGGER IF NOT EXISTS trg_medications_updated_at
AFTER UPDATE ON medications FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at
BEGIN
    UPDATE medications SET updated_at = datetime('now') WHERE medication_id = OLD.medication_id;
END;

CREATE TRIGGER IF NOT EXISTS trg_invoices_updated_at
AFTER UPDATE ON invoices FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at
BEGIN
    UPDATE invoices SET updated_at = datetime('now') WHERE invoice_id = OLD.invoice_id;
END;

CREATE TRIGGER IF NOT EXISTS trg_inpatient_admissions_updated_at
AFTER UPDATE ON inpatient_admissions FOR EACH ROW
WHEN OLD.updated_at = NEW.updated_at
BEGIN
    UPDATE inpatient_admissions SET updated_at = datetime('now') WHERE admission_id = OLD.admission_id;
END;
