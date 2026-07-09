# Prompt Log - Hung

---

## Prompt #1

| Field | Details |
|---|---|
| **Task** | Instruction / Rule Setup |
| **AI** | Claude (Sonnet) |
| **Prompt/Instruction** | Configure the AI to automatically log every future prompt into `prompt_cua_hung.md` in the `docs` folder with structured metadata. |
| **Student / Verification or Modification** | For every subsequent prompt given by the user, the AI must record the following fields into this file: Task (type of task e.g. GUI design, Class Design, Debugging), AI (model used), Prompt/Instruction (a summary of what the user wants), and Student/Verification or Modification (specific details of what is requested). |

---

## Prompt #2

| Field | Details |
|---|---|
| **Task** | Class Design / DTO Design |
| **AI** | Claude (Sonnet) |
| **Prompt/Instruction** | Analyze `DatabaseManager.cpp` to read the DB schema of patient tables (out, in, emergency), then write DTO structures for insert, update, and find features in `PatientDTOs.h`. |
| **Student / Verification or Modification** | - Read `patients`, `out_patients`, `in_patients`, `emergency_patients_admissions` tables from `DatabaseManager.cpp`. - Design 5 DTO sections: **InputDTO** (UI→Service), **InsertDTO** (Service→Repository), **UpdateDTO**, **ResultDTO** (DB→UI), **FindDTO** (search/filter). - All fields match actual DB column names and constraints (nullable, NOT NULL, CHECK values). |

---

## Prompt #3

| Field | Details |
|---|---|
| **Task** | Debugging / Bug Fix |
| **AI** | Claude Sonnet 4.6 (Thinking) |
| **Prompt/Instruction** | Explain and fix the compiler error: *"No member named 'toString' in 'std::optional\<QDate\>'; did you mean to use '->' instead of '.'?"* at `patientDTOs.h:L164`. |
| **Student / Verification or Modification** | - Root cause: `dischargeDate` in `InPatientInputDTO` is `std::optional<QDate>`, which does not forward member functions of `QDate`. Calling `.toString()` directly on the wrapper fails. - Fix applied: replaced `.toString("yyyy-MM-dd")` with `.value_or(QDate()).toString("yyyy-MM-dd")` so that a missing (null) date safely produces an empty `QString` instead of crashing. |

---

## Prompt #4

| Field | Details |
|---|---|
| **Task** | Class Design / Repository Layer |
| **AI** | Claude Sonnet 4.6 (Thinking) |
| **Prompt/Instruction** | Rewrite `PatientRepository.h` and `PatientRepository.cpp` after the `PatientDTOs.h` changes — only functions to insert patients into the DB tables are needed. |
| **Student / Verification or Modification** | - Removed all old `PatientDTO`/`OutPatientDTO` result-DTO references; header now only declares `insertOutPatient`, `insertInPatient`, `insertEmergencyPatient` (public) and `insertBasePatient` (private helper). - Column names corrected to match updated DB schema: `out_patients.doctor_id`, `in_patients.admitting_doctor_id`, `emergency_patients_admissions.doctor_id`. - Table name fixed: was `emergency_patients`, now `emergency_patients_admissions`. - Placeholder count corrected for emergency INSERT (8 columns → 8 `?`). - Transaction logic bug fixed: old code had `if (db.beginTransaction()) return false` (backwards); corrected to `if (!db.beginTransaction())`. - `std::optional<int>` FK fields and empty `QString` nullable fields are explicitly bound as null `QVariant` so SQLite stores `NULL` rather than `0` or `""`. |

---

## Prompt #5

| Field | Details |
|---|---|
| **Task** | Debugging / Bug Fix |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Explain what this problem is and help me fix it: No matching constructor for initialization of 'EmergencyPatientInsertDTO' @[d:\VSCODE\SmartClinicSystem\src\service\PatientService.cpp:L150] |
| **Student / Verification or Modification** | - Root cause: `EmergencyPatientInsertDTO` requires an `EmergencyPatientInputDTO` object and a `patientCode` string for initialization, but the code was incorrectly calling an empty constructor (`EmergencyPatientInsertDTO dto;`).<br>- Fix applied: Instantiated an `EmergencyPatientInputDTO` and populated it using the input parameters passed to `AddEmergencyPatient()`, utilizing helper functions like `stringToGender()`, and mapped integer fields gracefully with null checks. The populated DTO and `patientCode` were then correctly passed to the constructor. |

---

## Prompt #6

| Field | Details |
|---|---|
| **Task** | Feature Implementation / DTO Mapping |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | do the same with in and out patient |
| **Student / Verification or Modification** | - Applied the same DTO conversion pattern to `AddOutPatient` and `AddInPatient` in `PatientService.cpp`. <br>- For OutPatient, mapped raw parameters to `OutPatientInputDTO`, setting `doctorId` to `std::nullopt` since it isn't provided at creation, before initializing `OutPatientInsertDTO`. <br>- For InPatient, mapped parameters to `InPatientInputDTO`, safely converting foreign keys (`roomId`, `admittingDoctorId`) to optional integers and mapping `dischargeDate` appropriately before initializing `InPatientInsertDTO`. |

---

## Prompt #7

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Application Entry Point |
| **AI** | Claude Sonnet 4.6 (Thinking) |
| **Prompt/Instruction** | Rewrite `main.cpp` so the program creates `hospital.db`; find out why the `.db` and `.log` files do not appear after building; fix the missing Qt DLLs error causing the app to crash on start. |
| **Student / Verification or Modification** | - Clarified the concept **build ≠ run**: `.db` and `.log` are only created when the `.exe` actually runs, not during compilation. <br>- Added `isOpen()` to `DatabaseManager` and updated `main.cpp` to verify DB connection after startup — a detailed error dialog is displayed, and the app exits with code 1 if the DB fails to open. <br>- Added missing `#include <QTextStream>` (which caused a compile error at `QTextStream out(&logFile)`). <br>- Fixed `cmake_check_build_system Error 1` caused by `add_subdirectory(tests)` pointing to an empty `tests/` folder by creating a minimal `tests/CMakeLists.txt`. <br>- Executed `windeployqt6` to copy all required Qt DLLs (`Qt6Core.dll`, `Qt6Sql.dll`, `Qt6Gui.dll`, etc.) to the `.exe` directory; updated `CMakeLists.txt` to automatically run `windeployqt6` as a POST_BUILD step. |

---

## Prompt #8

| Field | Details |
|---|---|
| **Task** | Debugging / SQL Syntax Errors in `DatabaseManager.cpp` |
| **AI** | Claude Sonnet 4.6 (Thinking) |
| **Prompt/Instruction** | Read `debug.log` and fix the table creation errors in `hospital.db`; the tables in `hospital.db` do not match the written CREATE TABLE statements; fix all table errors shown in debug.log. |
| **Student / Verification or Modification** | - Cause 1: C++ comments `//` placed **inside** the raw string `R"(...)"` were sent verbatim to SQLite — SQLite does not understand `//` and reported `"near '/': syntax error"`. Fix: moved all comments **outside** the `)"` boundary. <br>- Cause 2: `CREATE TABLE IF NOT EXISTS` skips recreation if the table already exists, even if the schema has changed. Fix: deleted `hospital.db` and reran the app to recreate from the updated `createTables()` code. <br>- Cause 3: The SQL for the `Appointments` table had its `ON DELETE CASCADE` split across multiple lines with fragment comments mixed in. Fix: rewrote the SQL cleanly, one line per column. <br>- Reformatted the SQL for `departments` and `rooms` tables from a single-line compressed format to a readable multi-line format to prevent future errors. |

---

## Prompt #9

| Field | Details |
|---|---|
| **Task** | Testing / Integration Test + DBeaver Inspection |
| **AI** | Claude Sonnet 4.6 (Thinking) |
| **Prompt/Instruction** | Test all functions used to insert patients into the database; view the results in DBeaver. |
| **Student / Verification or Modification** | - Discovered and fixed a column name error in `PatientRepository.cpp`: `insertInPatient` was using `admitting_doctor_id` but the actual DB schema is `doctor_id`. <br>- Created `tests/PatientInsertTest.cpp` with 4 integration tests: `testInsertOutPatient`, `testInsertInPatient`, `testInsertEmergencyPatient`, and `testDuplicatePatientCode` (verified the UNIQUE constraint rejects duplicate inserts). Each test cleans up data after running. <br>- Updated `tests/CMakeLists.txt` to build `PatientInsertTest` as a standalone executable linked with `Qt6::Core` and `Qt6::Sql`. <br>- Provided step-by-step instructions on using DBeaver: connecting to `hospital.db` via SQLite driver, browsing tables in `main → Tables`, using the SQL Editor with JOIN queries to inspect inserted data, and pressing F5 to refresh after each test run. |

---

## Prompt #10

| Field | Details |
|---|---|
| **Task** | Debugging / Bug Fix |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Explain what this problem is and help me fix it: Member access into incomplete type 'PatientRepository' (fix available) @[PatientService.h:L203] |
| **Student / Verification or Modification** | - Root cause: `PatientRepository` was only forward-declared (`class PatientRepository;`) in `PatientService.h`, so the compiler lacked the full class definition needed to resolve inline method calls.<br>- Fix applied: Moved the inline definitions of `softDeletePatient` and `restorePatient` from the header `.h` file to the source `.cpp` file, where `PatientRepository.h` is fully included. |

---

## Prompt #11

| Field | Details |
|---|---|
| **Task** | Testing / Integration Test |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Add test code to check if deletion works in main.cpp |
| **Student / Verification or Modification** | - Added a test case to verify soft deletion and restoration functionality within the `runIntegrationTests()` function in `main.cpp`.<br>- The test retrieves a patient (`TEST-MAIN-OUT`), executes `softDeletePatient`, logs the success, subsequently calls `restorePatient`, and logs the final result. |

---

## Prompt #12

| Field | Details |
|---|---|
| **Task** | Code Analysis / Explanation |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Which keywords can my search function use and how does it work? |
| **Student / Verification or Modification** | - Analyzed the `buildSearchWhereClause` function in `PatientRepository.cpp`.<br>- Explained the `LIKE` logic: The keyword performs a partial match across 4 fields (`full_name`, `patient_code`, `citizen_id`, `phone`).<br>- Clarified how the function uses SQL escaping for special characters, and how other conditions (patient type, room, status, date) are dynamically combined into a `UNION ALL` query. |

---

## Prompt #13

| Field | Details |
|---|---|
| **Task** | Code Review / QA |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Please thoroughly analyze my project to see if there are any errors and run it again to check if anything is wrong. |
| **Student / Verification or Modification** | - Performed a complete rebuild of the project (`cmake --build build`) and test-ran the executable.<br>- Confirmed the system runs stably and all integration tests (Insert, Update, Search, Delete) pass successfully.<br>- Detected and suggested an improvement: The `emergency_patients` table in `DatabaseManager.cpp` was missing the `CHECK (discharge_date IS NULL OR discharge_date >= admission_date)` constraint, which existed in the inpatient schema. |

---

## Prompt #14

| Field | Details |
|---|---|
| **Task** | Feature Implementation / CRUD Completion |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | So have I completed the patient CRUD? & Please write these support functions |
| **Student / Verification or Modification** | - Analyzed CRUD completion: Identified the system was missing the Read Detail (`getPatientById`) function to fetch detailed data for UI editing forms.<br>- Introduced a new `PatientDetailDTO` (a flattened structure containing all comprehensive info) in `PatientDTOs.h` instead of splitting it into 3 separate structs.<br>- Implemented the `getPatientById` function in `PatientRepository.cpp` using a `UNION ALL` query to aggregate data from all tables, and exposed it through `PatientService.cpp` to achieve 100% CRUD flow completion. |

---

## Prompt #15

| Field | Details |
|---|---|
| **Task** | Debugging / Bug Fix |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | tại sao lại tạo thêm hàm QSqlDatabase &database ở DatabaseManager.h / fix lỗi FOREIGN KEY constraint failed khi AddOutPatient |
| **Student / Verification or Modification** | - Root cause 1: `QSqlQuery lastId = DatabaseManager::getInstance().selectQuery("SELECT last_insert_rowid()")` created a new query object, causing SQLite to lose track of the `last_insert_rowid()` within the transaction and returning `0`.<br>- Root cause 2: `AddOutPatient` passed `doctorId = 0` directly into an `std::optional<int>`, which resulted in inserting `0` instead of `NULL` into the DB, violating the FOREIGN KEY constraint.<br>- Fix applied: Exposed `m_db` via `DatabaseManager::getInstance().database()`, enabling `PatientRepository::insertBasePatient` to use a single `QSqlQuery` to `exec()` the insert and immediately call `query.lastInsertId()` safely. Also added checks in `PatientService` to map `doctorId = 0` or `roomId = 0` to `std::nullopt`. |

---

## Prompt #16

| Field | Details |
|---|---|
| **Task** | Class Design / Model |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Write model/MedicalRecord.h and model/MedicalRecord.cpp with VitalSigns, Diagnosis structures and MedicalRecord class according to project standards. |
| **Student / Verification or Modification** | - Created `MedicalRecord.h` and `MedicalRecord.cpp`.<br>- Defined `VitalSigns`, `Diagnosis` (checking severity `MILD`/`MODERATE`/`SEVERE`).<br>- Declared `m_` properties using `std::optional`.<br>- Implemented `isComplete()` to validate required fields and `calculateBMI()`. |

---

## Prompt #17

| Field | Details |
|---|---|
| **Task** | Class Design / Model |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Write model/Invoice.h and model/Invoice.cpp — abstract base class. |
| **Student / Verification or Modification** | - Created `Invoice` abstract base class.<br>- Added protected attributes.<br>- Declared 2 pure virtual methods: `calculate()` and `toSummaryString()`. |

---

## Prompt #18

| Field | Details |
|---|---|
| **Task** | Class Design / Model |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Write 3 pairs of classes inheriting from Invoice: model/OutPatientInvoice.h/.cpp, model/InPatientInvoice.h/.cpp, and model/EmergencyInvoice.h/.cpp. |
| **Student / Verification or Modification** | - Created `OutPatientInvoice` inheriting from `Invoice`, calculating total consultation + medication fee without additional charges.<br>- Created `InPatientInvoice` inheriting from `Invoice`, calculating total consultation + medication fee excluding bed charges.<br>- Created `EmergencyInvoice` inheriting from `Invoice`, calculating total consultation + medication fee.<br>- Implemented constructors calling base class, overridden `calculate` and `toSummaryString`. |

---

## Prompt #19

| Field | Details |
|---|---|
| **Task** | Class Design / DTO Design |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Write `dto/BillingDTOs.h` with `InvoiceItemDTO`, `InvoiceInsertDTO`, and `InvoiceResultDTO` structures. Include `model/CommonEnums.h` for `PatientType`. After writing the code, log the prompt into `prompt_cua_hung.md` in English. |
| **Student / Verification or Modification** | - Created `src/dto/BillingDTOs.h`.<br>- Defined `InvoiceItemDTO`, `InvoiceInsertDTO`, and `InvoiceResultDTO` exactly as requested with all specified fields and types (including `QVector`, `QDate`, `QString`, and `std::optional`).<br>- Logged Prompt #19 in English as requested. |

---

## Prompt #20

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Enums |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Open `model/CommonEnums.h` and append the `InvoiceStatus` enum and its string conversion helper functions to the end without modifying existing enums. Log this prompt to `prompt_cua_hung.md`. |
| **Student / Verification or Modification** | - Appended `InvoiceStatus` enum, `InvoiceStatusToString()`, and `stringToInvoiceStatus()` to `CommonEnums.h`.<br>- Existing contents of the file were kept intact.<br>- Logged Prompt #20 in English. |

---

## Prompt #21

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Validation |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Open `Validation.h` and `Validation.cpp`, include `model/MedicalRecord.h`, and append three new validation functions (`validateVitalSigns`, `validateChiefComplaint`, `validateDiagnosisList`) without modifying existing functions. After writing the code, log the prompt into `prompt_cua_hung.md` in English. |
| **Student / Verification or Modification** | - Modified `src/service/Validation.h` and `src/service/Validation.cpp`.<br>- Included `model/MedicalRecord.h` and defined `validateVitalSigns` (checking temperature, heartRate, weight, height), `validateChiefComplaint` (checking for empty string), and `validateDiagnosisList` (checking if list is empty and validating description and severity `MILD`/`MODERATE`/`SEVERE`).<br>- Used `QList<Diagnosis>` to match the project's updated DTO structures.<br>- Logged Prompt #21 in English. |

---

## Prompt #22

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Repository |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Create `MedicalRecordRepository.h` and `MedicalRecordRepository.cpp` with CRUD operations for medical records and diagnoses, and include forward declarations/stubs for future Prescription features. Ensure proper transactions and nullable bindings. After writing, log this prompt into `prompt_cua_hung.md` in English. |
| **Student / Verification or Modification** | - Created `src/repository/MedicalRecordRepository.h` with `insertMedicalRecord`, `findById`, and `getHistoryByPatientId` as well as stubs for `insertPrescription`, `getPrescriptionByRecordId`, and `confirmPrescription`.<br>- Created `src/repository/MedicalRecordRepository.cpp` implementing the 4 required functions including the private `insertDiagnoses`.<br>- Used `QList` consistently with the project structure and strictly adhered to transaction procedures, `lastInsertId` retrieval via direct `QSqlQuery`, and nullable `QVariant` bindings as requested.<br>- Logged Prompt #22. |

---

## Prompt #23

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Repository |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Create `BillingRepository.h` and `BillingRepository.cpp` to handle `insertInvoice`, `getInvoiceByRecordId`, and `getInvoicesByPatientId` using SQLite. Generate `INV-yyyyMMdd-NNNN` formatted invoice codes dynamically. Adhere to global transaction patterns and change `QVector` to `QList`. After writing, log this prompt into `prompt_cua_hung.md` in English. |
| **Student / Verification or Modification** | - Created `src/repository/BillingRepository.h` and `BillingRepository.cpp`.<br>- Used `QList` for items collections instead of `QVector` as instructed.<br>- Implemented `generateInvoiceCode()` counting existing records using `LIKE` pattern.<br>- Wrapped `insertInvoice` in a transaction: inserted the base invoice, grabbed `lastInsertId`, then iterated to insert all `invoice_items`.<br>- Implemented `getInvoiceByRecordId` and `getInvoicesByPatientId` fetching nested items collections safely.<br>- Logged Prompt #23. |

---

## Prompt #24

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Repository and DTO |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Add Update and Soft Delete functionality to both MedicalRecord and Billing models. Modify the DTO headers to include Update structures. Implement `update` and `softDelete` (or `cancel` for Invoice) in the Repositories safely using transactions. |
| **Student / Verification or Modification** | - Modified `src/dto/MedicalRecordDTOs.h` to add `MedicalRecordUpdateDTO` and `src/dto/BillingDTOs.h` to add `InvoiceUpdateDTO`.<br>- Updated `MedicalRecordRepository` to include `updateMedicalRecord` (replaces old diagnoses) and `softDeleteMedicalRecord` (`is_deleted` flag).<br>- Updated `BillingRepository` to include `updateInvoice` (replaces items) and `cancelInvoice` (sets status to 'CANCELLED').<br>- Replaced remaining `QVector` with `QList` in Billing DTOs. |

---

## Prompt #25

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Factory |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Create `factory/IInvoiceFactory.h`, `OutPatientInvoiceFactory.h/.cpp`, `InPatientInvoiceFactory.h/.cpp`, and `EmergencyInvoiceFactory.h/.cpp` overriding `createInvoice` to return specific derived `Invoice` smart pointers using `std::make_unique`. Log prompt to `prompt_cua_hung.md` in English. |
| **Student / Verification or Modification** | - Created `IInvoiceFactory.h` interface with virtual destructor and pure virtual `createInvoice` method.<br>- Created concrete factory classes (`OutPatientInvoiceFactory`, `InPatientInvoiceFactory`, `EmergencyInvoiceFactory`) implementing `createInvoice` by including their respective model headers and using `std::make_unique`.<br>- Wrote implementations strictly following the provided signatures without any additional generic template logic, abiding by the simplicity constraints. Logged Prompt #25. |

---

## Prompt #26

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Service |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Create `MedicalRecordService.h` and `.cpp` with `createMedicalRecord` and `getMedicalHistory`. Replace `QVector` with `QList`. Validate vitals, chief complaint, and diagnoses sequentially before inserting, returning -1 on failure. Follow the shared-file safety rules by only defining signatures for Prescription methods and providing no implementation. Include previously implemented update and delete bindings. Log to `prompt_cua_hung.md` in English. |
| **Student / Verification or Modification** | - Created `src/service/MedicalRecordService.h` and `MedicalRecordService.cpp`.<br>- Used `QList` for collections instead of `QVector` as instructed.<br>- Safely added forward declarations and constructor dependencies for `PatientService`, `PharmacyService`, and `PrescriptionItemDTO`.<br>- Implemented `createMedicalRecord` chaining `validateVitalSigns`, `validateChiefComplaint`, and `validateDiagnosisList`, halting and returning `-1` on the first failure. Integrated the previously built `updateMedicalRecord` and `softDeleteMedicalRecord` methods seamlessly. Logged Prompt #26. |

---

## Prompt #27

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Patient Module Enhancement |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Open `PatientRepository.h/.cpp` and `PatientService.h/.cpp` and add a new method to get allergies as a raw string `getAllergiesByPatientId` (renamed to `getAllergiesStringByPatientId` to fix C++ overload constraints). Then add `checkDrugAllergyConflict` in `PatientService` using that string to safely verify allergy conflicts. Do not alter existing methods. Log to `prompt_cua_hung.md` in English. |
| **Student / Verification or Modification** | - Added `getAllergiesStringByPatientId` to `PatientRepository.h/.cpp` executing the direct SQL requested.<br>- Note: Renamed from `getAllergiesByPatientId` because an existing method already held that signature returning `QList`, violating C++ overload constraints.<br>- Added `checkDrugAllergyConflict` in `PatientService.h/.cpp` executing the comma separation and case-insensitive check. Logged Prompt #27. |

---

## Prompt #28

| Field | Details |
|---|---|
| **Task** | Feature Implementation / Billing Service |
| **AI** | Gemini 3.1 Pro (High) |
| **Prompt/Instruction** | Create `BillingService.h` and `.cpp` (full ownership). Use a mock `PrescriptionItemDTO` to allow compiling independently. Implement `selectFactory` returning the appropriate factory based on `PatientType`, `calculateMedicationTotal`, `generateInvoice` using the factory and inserting consultation + medication items, and `getInvoiceByRecordId`. Add `QList` conversions and existing update/cancel bindings. Log to `prompt_cua_hung.md` in English. |
| **Student / Verification or Modification** | - Created `src/service/BillingService.h` and `BillingService.cpp`.<br>- Included a temporary struct definition for `PrescriptionItemDTO` to enable standalone compilation before the Pharmacy module is finished.<br>- Replaced all `QVector` references with `QList`.<br>- Implemented `selectFactory` to safely instantiate `OutPatientInvoiceFactory`, `InPatientInvoiceFactory`, or `EmergencyInvoiceFactory`.<br>- Implemented `generateInvoice` calculating medication sums, generating the invoice via the factory to determine total amount, and building the `CONSULTATION` and `MEDICATION` line items respectively.<br>- Bound `updateInvoice` and `cancelInvoice` to the repository. Logged Prompt #28. |
