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
| **Prompt/Instruction** | Rewrite `main.cpp` để chương trình tạo `hospital.db`; tìm hiểu tại sao file `.db` và `.log` không xuất hiện sau khi build; sửa lỗi thiếu Qt DLLs khiến app không chạy được. |
| **Student / Verification or Modification** | - Làm rõ khái niệm **build ≠ run**: `.db` và `.log` chỉ được tạo khi `.exe` thực sự chạy, không phải lúc biên dịch. <br>- Thêm `isOpen()` vào `DatabaseManager` và cập nhật `main.cpp` để kiểm tra sau khởi động — hiện dialog lỗi mô tả chi tiết và thoát với code 1 nếu DB không mở được. <br>- Thêm `#include <QTextStream>` bị thiếu (gây lỗi compile tại dòng `QTextStream out(&logFile)`). <br>- Sửa lỗi `cmake_check_build_system Error 1` do `add_subdirectory(tests)` trỏ đến thư mục `tests/` rỗng — tạo `tests/CMakeLists.txt` tối giản. <br>- Chạy `windeployqt6` để copy tất cả Qt DLL (`Qt6Core.dll`, `Qt6Sql.dll`, `Qt6Gui.dll`, ...) vào cùng thư mục với `.exe`; cập nhật `CMakeLists.txt` để tự động chạy `windeployqt6` như POST_BUILD step. |

---

## Prompt #8

| Field | Details |
|---|---|
| **Task** | Debugging / SQL Syntax Errors in `DatabaseManager.cpp` |
| **AI** | Claude Sonnet 4.6 (Thinking) |
| **Prompt/Instruction** | Đọc `debug.log` và sửa các lỗi tạo bảng trong `hospital.db`; các bảng trong `hospital.db` khác với câu lệnh CREATE TABLE đã viết; sửa tất cả lỗi bảng hiển thị trong debug.log. |
| **Student / Verification or Modification** | - Nguyên nhân 1: Comment `//` trong C++ đặt **bên trong** raw string `R"(...)"` được gửi nguyên văn cho SQLite — SQLite không hiểu `//` và báo `"near '/': syntax error"`. Sửa: chuyển tất cả comment ra **ngoài** dấu `)"`. <br>- Nguyên nhân 2: `CREATE TABLE IF NOT EXISTS` bỏ qua tạo lại nếu bảng đã tồn tại, kể cả khi schema đã thay đổi. Sửa: xóa `hospital.db` và chạy lại app để tái tạo từ code `createTables()` hiện tại. <br>- Nguyên nhân 3: SQL bảng `Appointments` bị vỡ `ON DELETE CASCADE` qua nhiều dòng với các fragment comment lẫn vào. Sửa: viết lại SQL gọn gàng, mỗi cột một dòng. <br>- Định dạng lại SQL bảng `departments` và `rooms` từ kiểu nén một dòng sang nhiều dòng dễ đọc để tránh lỗi trong tương lai. |

---

## Prompt #9

| Field | Details |
|---|---|
| **Task** | Testing / Integration Test + DBeaver Inspection |
| **AI** | Claude Sonnet 4.6 (Thinking) |
| **Prompt/Instruction** | Test tất cả các hàm dùng để insert bệnh nhân vào database; xem kết quả trong DBeaver. |
| **Student / Verification or Modification** | - Phát hiện và sửa lỗi tên cột trong `PatientRepository.cpp`: `insertInPatient` dùng cột `admitting_doctor_id` nhưng schema DB thực tế là `doctor_id`. <br>- Tạo `tests/PatientInsertTest.cpp` với 4 bài test tích hợp: `testInsertOutPatient`, `testInsertInPatient`, `testInsertEmergencyPatient`, và `testDuplicatePatientCode` (kiểm tra ràng buộc UNIQUE từ chối chèn trùng). Mỗi test tự dọn dữ liệu sau khi chạy. <br>- Cập nhật `tests/CMakeLists.txt` để build `PatientInsertTest` thành file exe độc lập, link với `Qt6::Core` và `Qt6::Sql`. <br>- Hướng dẫn từng bước dùng DBeaver: kết nối đến `hospital.db` qua driver SQLite, duyệt bảng trong `main → Tables`, dùng SQL Editor với câu JOIN để kiểm tra dữ liệu đã insert, nhấn F5 để refresh sau mỗi lần chạy test. |
