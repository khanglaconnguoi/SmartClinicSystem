CREATE TABLE sqlite_sequence(name,seq);

CREATE TABLE departments (

    department_id   INTEGER PRIMARY KEY AUTOINCREMENT,

    department_code TEXT    NOT NULL UNIQUE,

    department_name TEXT    NOT NULL,

    description      TEXT,

    is_deleted        INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),

    created_at        TEXT    NOT NULL DEFAULT (datetime('now')),

    updated_at        TEXT    NOT NULL DEFAULT (datetime('now'))

);

CREATE TABLE rooms (

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

CREATE TABLE staff (

    staff_id             INTEGER PRIMARY KEY AUTOINCREMENT,

    staff_code           TEXT    NOT NULL UNIQUE,  -- login ID kiêm mã nhân viên

    password_hash        TEXT    NOT NULL,

    must_change_password INTEGER NOT NULL DEFAULT 1 CHECK (must_change_password IN (0,1)),

    full_name            TEXT    NOT NULL,

    avatar               BLOB,

    role                 TEXT    NOT NULL CHECK (role IN ('ADMIN','DOCTOR','NURSE','RECEPTIONIST')),

    gender               TEXT    NOT NULL CHECK (gender IN ('MALE','FEMALE','OTHER')),

    date_of_birth        TEXT    NOT NULL,

    citizen_id           TEXT    NOT NULL UNIQUE,

    phone_number         TEXT    NOT NULL UNIQUE,                                         

    email                TEXT    NOT NULL UNIQUE, 

    address              TEXT    NOT NULL,

    department_id        INTEGER NOT NULL,

    hire_date            TEXT    NOT NULL DEFAULT (date('now')),

    shift                TEXT    NOT NULL DEFAULT 'FULL_DAY' CHECK (shift IN ('MORNING','AFTERNOON','NIGHT','FULL_DAY')),

    is_active            INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1)),

    is_deleted           INTEGER NOT NULL DEFAULT 0 CHECK (is_deleted IN (0,1)),

    created_at           TEXT    NOT NULL DEFAULT (datetime('now')),

    updated_at           TEXT    NOT NULL DEFAULT (datetime('now')),

    FOREIGN KEY (department_id) REFERENCES departments(department_id) ON DELETE SET NULL

);

CREATE TABLE doctor_profiles (

    staff_id         INTEGER PRIMARY KEY,

    specialty        TEXT    NOT NULL,

    license_number   TEXT    NOT NULL UNIQUE,

    experience_years INTEGER NOT NULL DEFAULT 0 CHECK (experience_years >= 0),

    consultation_fee REAL    NOT NULL DEFAULT 0 CHECK (consultation_fee >= 0),

    bio              TEXT, room_id INTEGER REFERENCES rooms(room_id),

    FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE

);

CREATE TABLE nurse_profiles (

    staff_id      INTEGER PRIMARY KEY,

    nurse_level   TEXT NOT NULL DEFAULT 'JUNIOR' CHECK (nurse_level IN ('JUNIOR','SENIOR','HEAD')),

    certification TEXT,

    FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE

);

CREATE TABLE patients (

    patient_id              INTEGER PRIMARY KEY AUTOINCREMENT,

    patient_code            TEXT    NOT NULL UNIQUE,

    full_name               TEXT    NOT NULL,

    date_of_birth           TEXT    NOT NULL,

    gender                  TEXT    NOT NULL CHECK (gender IN ('MALE','FEMALE','OTHER')),

    citizen_id              TEXT    NOT NULL UNIQUE,

    phone_number            TEXT    NOT NULL,

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

CREATE TABLE patient_allergies (

    allergy_id    INTEGER PRIMARY KEY AUTOINCREMENT,

    patient_id    INTEGER NOT NULL,

    allergen_name TEXT    NOT NULL,

    severity      TEXT    NOT NULL DEFAULT 'MODERATE' CHECK (severity IN ('MILD','MODERATE','SEVERE')),

    notes         TEXT,

    recorded_at   TEXT    NOT NULL DEFAULT (datetime('now')),

    FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,

    UNIQUE (patient_id, allergen_name)

);

CREATE TABLE appointments (

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

    updated_at       TEXT    NOT NULL DEFAULT (datetime('now')), ticket_number INTEGER NOT NULL DEFAULT 1, checked_in_at TEXT, started_at TEXT, completed_at TEXT,

    FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,

    FOREIGN KEY (doctor_id)  REFERENCES staff(staff_id) ON DELETE RESTRICT,

    FOREIGN KEY (room_id)    REFERENCES rooms(room_id) ON DELETE SET NULL,

    FOREIGN KEY (created_by) REFERENCES staff(staff_id) ON DELETE SET NULL,

    UNIQUE (doctor_id, appointment_date, start_time),

    CHECK (end_time IS NULL OR end_time > start_time)

);

CREATE TABLE medical_records (

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

CREATE TABLE diagnoses (

    diagnosis_id INTEGER PRIMARY KEY AUTOINCREMENT,

    record_id    INTEGER NOT NULL,

    icd_code     TEXT,

    description  TEXT    NOT NULL,

    severity     TEXT    NOT NULL DEFAULT 'MILD' CHECK (severity IN ('MILD','MODERATE','SEVERE')),

    is_primary   INTEGER NOT NULL DEFAULT 0 CHECK (is_primary IN (0,1)),

    FOREIGN KEY (record_id) REFERENCES medical_records(record_id) ON DELETE CASCADE

);

CREATE TABLE medication_categories (

    category_id   INTEGER PRIMARY KEY AUTOINCREMENT,

    category_name TEXT NOT NULL UNIQUE,

    description   TEXT

);

CREATE TABLE medications (

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

CREATE TABLE prescriptions (

    prescription_id INTEGER PRIMARY KEY AUTOINCREMENT,

    record_id       INTEGER NOT NULL,

    doctor_id       INTEGER NOT NULL,

    prescribed_at   TEXT    NOT NULL DEFAULT (datetime('now')),

    status          TEXT    NOT NULL DEFAULT 'PENDING' CHECK (status IN ('PENDING','DISPENSED','CANCELLED')),

    notes           TEXT,

    FOREIGN KEY (record_id) REFERENCES medical_records(record_id) ON DELETE CASCADE,

    FOREIGN KEY (doctor_id) REFERENCES staff(staff_id) ON DELETE RESTRICT

);

CREATE TABLE prescription_items (

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

CREATE TABLE invoices (

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

CREATE TABLE invoice_items (

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

CREATE TABLE audit_logs (

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

CREATE TABLE attendance_records (

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

CREATE TABLE out_patients (
          patient_id INTEGER PRIMARY KEY,
          doctor_id           INTEGER,
          status              TEXT    NOT NULL DEFAULT 'REGISTERED' CHECK (status IN ('REGISTERED','WAITING FOR TREATMENT','TREATMENT','DISCHARGED')),
          created_at          TEXT    NOT NULL DEFAULT (datetime('now')),
          updated_at          TEXT    NOT NULL DEFAULT (datetime('now')),
          FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE,
          FOREIGN KEY (doctor_id) REFERENCES staff(staff_id) ON DELETE SET NULL
      );

CREATE TABLE emergency_patients (
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

CREATE TABLE in_patients (
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
  );

CREATE TABLE active_ingredients (
        ingredient_id   INTEGER PRIMARY KEY AUTOINCREMENT,
        ingredient_name TEXT NOT NULL,
        description     TEXT
    );

CREATE TABLE medication_ingredients (
      medication_id   INTEGER NOT NULL,
      ingredient_id   INTEGER NOT NULL,
      strength        TEXT NOT NULL,            -- Hàm lượng (VD: "500mg", "65mg")
      
      PRIMARY KEY (medication_id, ingredient_id),
      FOREIGN KEY (medication_id) REFERENCES medications(medication_id) ON DELETE CASCADE,
      FOREIGN KEY (ingredient_id) REFERENCES active_ingredients(ingredient_id) ON DELETE RESTRICT
    );

CREATE TABLE pharmacist_profiles (
        staff_id          INTEGER PRIMARY KEY,
        license_number    TEXT NOT NULL UNIQUE,
        pharmacy_section  TEXT,
        experience_years  INTEGER NOT NULL DEFAULT 0 CHECK (experience_years >= 0),
        FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE
    );

CREATE TABLE patient_insurance (
          insurance_id     INTEGER PRIMARY KEY AUTOINCREMENT,
          patient_id       INTEGER NOT NULL UNIQUE,
          provider_name    TEXT    NOT NULL,
          policy_number    TEXT    NOT NULL,
          insurance_type   TEXT    NOT NULL DEFAULT 'NATIONAL' CHECK (insurance_type IN ('NATIONAL','COMMERCIAL','OTHER')),
          coverage_percent REAL    NOT NULL DEFAULT 80,
          valid_from       TEXT,
          valid_to         TEXT,
          notes            TEXT,
          is_active        INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1)),
          created_at       TEXT    NOT NULL DEFAULT (datetime('now')),
          updated_at       TEXT    NOT NULL DEFAULT (datetime('now')),
          FOREIGN KEY (patient_id) REFERENCES patients(patient_id) ON DELETE CASCADE
      );

CREATE TABLE leave_balances (
          staff_id   INTEGER NOT NULL,
          year       INTEGER NOT NULL,
          total_days INTEGER NOT NULL DEFAULT 12,
          used_days  INTEGER NOT NULL DEFAULT 0,
          PRIMARY KEY (staff_id, year),
          FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE
      );

CREATE TABLE leave_requests (
          request_id INTEGER PRIMARY KEY AUTOINCREMENT,
          staff_id   INTEGER NOT NULL,
          start_date TEXT    NOT NULL,
          end_date   TEXT    NOT NULL,
          reason     TEXT,
          status     TEXT    NOT NULL DEFAULT 'APPROVED',
          created_at TEXT    NOT NULL DEFAULT (datetime('now')),
          updated_at TEXT    NOT NULL DEFAULT (datetime('now')),
          FOREIGN KEY (staff_id) REFERENCES staff(staff_id) ON DELETE CASCADE
      );
