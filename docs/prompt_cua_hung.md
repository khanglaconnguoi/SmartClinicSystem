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
