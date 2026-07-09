# Kế hoạch công việc — Module MedicalRecord + BillingService

> Phạm vi: chỉ phần việc của **bạn** (MedicalRecord + BillingService).
> Phần Prescription + Medication + PharmacyService thuộc về đồng đội, không nằm trong tài liệu này.

---

## 0. Cách dùng tài liệu này

1. Copy **Phần 1 — Ngữ cảnh dùng chung** vào đầu **MỌI** prompt bạn đưa cho Agent, bất kể nhiệm vụ nào.
2. Nối thêm **Prompt riêng** của nhiệm vụ tương ứng (lấy từ Phần 3) vào ngay sau đó.
3. Thực hiện theo đúng **Đợt (Wave)** ở Phần 2 — các nhiệm vụ cùng Đợt không phụ thuộc nhau, có thể giao cho nhiều Agent chạy song song. Đợt sau chỉ bắt đầu khi Đợt trước xong.
4. `MR-00` (Database) đã hoàn thành ở bước trước — liệt kê lại để tham chiếu, không cần giao cho Agent nữa.

---

## 1. Ngữ cảnh dùng chung (dán vào đầu mọi prompt)

```
Dự án: Ứng dụng Quản lý Bệnh viện (SmartClinicSystem) — C++17, Qt Widgets, SQLite.
Kiến trúc theo lớp, dữ liệu đi theo chiều: Model → DTO → Validation → Repository → Service → UI.

Cấu trúc thư mục:
  model/       — domain class thuần C++ (không SQL, không Qt Widget)
  dto/         — struct dữ liệu (Insert/Update/Result DTO)
  repository/  — tầng duy nhất được phép viết SQL, thao tác qua DatabaseManager
  service/     — validate + điều phối business logic, gọi xuống repository
  ui/          — QWidget/QDialog, chỉ gọi xuống Service, không gọi thẳng Repository

QUY ƯỚC BẮT BUỘC theo đúng code hiện có trong dự án:
- Comment Doxygen bằng tiếng Việt, dạng:
  /**
   * @brief   Mô tả ngắn.
   * @param   ten_tham_so  Mô tả.
   * @return  Mô tả giá trị trả về.
   */
- Hàm validate (trong Validation.h/.cpp) LUÔN là hàm tự do (không phải method),
  trả về QString: "" nếu hợp lệ, chuỗi mô tả lỗi nếu không hợp lệ.
- Repository dùng DatabaseManager::getInstance().executeQuery(sql, params) cho
  INSERT/UPDATE, .selectQuery(sql, params) cho SELECT (trả QSqlQuery).
  Insert nhiều bảng liên quan PHẢI bọc trong transaction:
  beginTransaction() → các executeQuery() → commitTransaction() hoặc
  rollbackTransaction() nếu có bước lỗi.
- Giá trị nullable khi bind vào QVariantList theo đúng pattern có sẵn:
    dto.field.has_value() ? QVariant(dto.field.value()) : QVariant(QVariant::Int)
    dto.field.isEmpty()   ? QVariant(QVariant::String)  : dto.field
- Class model: thuộc tính private tiền tố m_, getter public inline trong .h,
  logic phức tạp hơn cài đặt trong .cpp.
- Không dùng namespace, không dùng exception — trả bool/QString/-1/std::optional
  để báo lỗi, đúng như PatientService/PatientRepository hiện có.

QUY TẮC AN TOÀN KHI SỬA FILE DÙNG CHUNG:
MedicalRecordRepository.h/.cpp và MedicalRecordService.h/.cpp là file CHUNG với
đồng đội (họ phụ trách Prescription). CHỈ thêm/sửa phần thuộc nhiệm vụ được giao
bên dưới. TUYỆT ĐỐI KHÔNG xóa, đổi chữ ký, hay viết đè lên các hàm/khai báo đã
tồn tại sẵn trong file (kể cả các hàm còn trống thân hàm dành cho đồng đội).

Database (đã tạo sẵn, không cần tạo lại) gồm các cột chính liên quan:
  medical_records(record_id, patient_id, doctor_id, appointment_id,
    visit_datetime, temperature, blood_pressure, heart_rate, weight, height,
    chief_complaint, clinical_notes, treatment, next_visit_date, is_deleted)
  diagnoses(diagnosis_id, record_id, icd_code, description, severity)
    -- CHÚ Ý: cột severity có CHECK (severity IN ('MILD','MODERATE','SEVERE'))
    -- bắt buộc chữ HOA, phải chuẩn hóa .toUpper() trước khi validate/insert.
  invoices(invoice_id, invoice_code, patient_id, record_id, patient_type,
    consultation_fee, medication_fee, total_amount, status, issued_date, paid_date)
  invoice_items(item_id, invoice_id, item_type, description, quantity,
    unit_price, subtotal)

Nếu Agent có quyền đọc file trong repo, hãy đọc thêm các file sau để khớp style
trước khi viết code: PatientService.h/.cpp, PatientRepository.h/.cpp,
Validation.h/.cpp, DatabaseManager.h/.cpp, CommonEnums.h.
Sau khi viết code xong, hãy ghi lại prompt vào file ../docs/prompt_cua_hung.md theo format trong file và ghi bằng tiếng anh
```

---

## 2. Bảng tổng quan nhiệm vụ

| ID | Nhiệm vụ | File cần tạo/sửa | Phụ thuộc | Đợt |
|----|----------|-------------------|-----------|-----|
| MR-00 | ✅ Database schema (đã xong) | `DatabaseManager.cpp` | — | — |
| MR-01 | Model `MedicalRecord` + `VitalSigns` + `Diagnosis` | `model/MedicalRecord.h/.cpp` | — | 1 |
| MR-02 | Model `Invoice` (abstract base) | `model/Invoice.h/.cpp` | — | 1 |
| MR-05 | DTO Billing | `dto/BillingDTOs.h` | — | 1 |
| MR-05B | (Tuỳ chọn) `InvoiceStatus` enum | `model/CommonEnums.h` | — | 1 |
| MR-10 | `PatientService::checkDrugAllergyConflict()` | `PatientService.h/.cpp`, `PatientRepository.h/.cpp` | — | 1 |
| MR-03 | Model `OutPatientInvoice` + `InPatientInvoice` | `model/OutPatientInvoice.h/.cpp`, `model/InPatientInvoice.h/.cpp` | MR-02 | 2 |
| MR-04 | DTO MedicalRecord | `dto/MedicalRecordDTOs.h` | MR-01 | 2 |
| MR-06 | Validation bổ sung | `Validation.h/.cpp` | MR-01 | 2 |
| MR-07 | Repository `MedicalRecordRepository` (phần của bạn) | `repository/MedicalRecordRepository.h/.cpp` | MR-04, MR-06 | 3 |
| MR-08 | Repository `BillingRepository` | `repository/BillingRepository.h/.cpp` | MR-05 | 3 |
| MR-12 | Factory `IInvoiceFactory` + 2 concrete factory | `factory/IInvoiceFactory.h`, `factory/OutPatientInvoiceFactory.h/.cpp`, `factory/InPatientInvoiceFactory.h/.cpp` | MR-02, MR-03 | 3 |
| MR-09 | Service `MedicalRecordService` (phần của bạn) | `service/MedicalRecordService.h/.cpp` | MR-07, MR-06 | 4 |
| MR-11 | Service `BillingService` | `service/BillingService.h/.cpp` | MR-08, MR-12, **DTO thuốc từ đồng đội** | 5 |
| MR-13 | UI `MedicalRecordDialog` | `ui/dialogs/MedicalRecordDialog.h/.cpp` | MR-09 | 6 |
| MR-14 | UI `InvoiceDialog` | `ui/dialogs/InvoiceDialog.h/.cpp` | MR-11 | 6 |
| MR-15 | (Tuỳ chọn) `MedicalTimelineWidget` | `ui/widgets/MedicalTimelineWidget.h/.cpp` | MR-07 | 6 |

**Đợt 1 và 2** không phụ thuộc đồng đội — có thể giao Agent làm ngay.
**MR-10 nên ưu tiên làm sớm nhất trong Đợt 1** vì đồng đội đang chờ hàm này để code `addPrescription()`.
**MR-11 có rủi ro chờ đồng đội** — xem Phần 4.

---

## 3. Chi tiết từng nhiệm vụ

### MR-01 — Model `MedicalRecord`

**Mục tiêu:** struct `VitalSigns`, struct `Diagnosis`, class `MedicalRecord` đại diện 1 lần khám.

**Prompt riêng:**

```
Nhiệm vụ: Viết model/MedicalRecord.h và model/MedicalRecord.cpp.

1. struct VitalSigns (không cần .cpp riêng, định nghĩa ngay trong .h):
   double temperature = 0.0;   // °C
   QString bloodPressure;      // dạng "120/80"
   int heartRate = 0;          // bpm
   double weight = 0.0;        // kg
   double height = 0.0;        // cm

2. struct Diagnosis (định nghĩa ngay trong .h):
   QString icdCode;      // có thể rỗng
   QString description;  // bắt buộc
   QString severity;     // PHẢI là "MILD" / "MODERATE" / "SEVERE" (chữ HOA,
                          // khớp CHECK constraint cột diagnoses.severity trong DB)

3. class MedicalRecord (private field tiền tố m_, giống Patient.h):
   int m_recordId;
   int m_patientId;
   int m_doctorId;
   std::optional<int> m_appointmentId;
   QDateTime m_visitDateTime;
   VitalSigns m_vitals;
   QString m_chiefComplaint;
   QString m_clinicalNotes;
   QString m_treatment;
   std::optional<QDate> m_nextVisitDate;
   QVector<Diagnosis> m_diagnoses;

   Constructor: nhận patientId, doctorId (2 tham số bắt buộc), các trường còn
   lại khởi tạo giá trị mặc định/rỗng.

   Getter public inline cho mọi field (theo đúng style Patient.h).
   void addDiagnosis(const Diagnosis &d) { m_diagnoses.append(d); }

   Hai hàm cài đặt trong .cpp:
   - bool isComplete() const;
     Trả false nếu: chiefComplaint rỗng, HOẶC diagnoses rỗng, HOẶC
     vitals.weight <= 0, HOẶC vitals.height <= 0.
   - double calculateBMI() const;
     Công thức: weight / (height/100)^2. Trả 0.0 nếu height <= 0 (tránh chia 0).

Include cần có: "CommonEnums.h", <QDate>, <QDateTime>, <QString>, <QVector>, <optional>.
```

**Tiêu chí hoàn thành:**

- Compile không lỗi khi include độc lập.
- `calculateBMI()` không crash khi `height = 0`.
- `isComplete()` có ít nhất 1 test case true và 1 test case false khi thử nhanh trong `main.cpp`.

---

### MR-02 — Model `Invoice` (abstract)

**Prompt riêng:**

```
Nhiệm vụ: Viết model/Invoice.h và model/Invoice.cpp — abstract base class.

protected field (tiền tố m_):
  int m_invoiceId;
  QString m_invoiceCode;
  int m_patientId;
  std::optional<int> m_recordId;
  double m_consultationFee;
  double m_medicationFee;
  QDate m_issuedDate;
  QString m_status; // UNPAID / PAID / CANCELLED

Constructor: nhận patientId, consultationFee, medicationFee, issuedDate.
  m_status mặc định "UNPAID". m_invoiceId mặc định 0 (chưa lưu DB).

virtual ~Invoice() = default;

Getter public inline cho mọi field.

Pure virtual (bắt buộc lớp con override), khai báo trong .h, KHÔNG cài đặt ở đây:
  virtual double calculate() const = 0;
    // Lớp con định nghĩa cách cộng tiền (khám + thuốc, có thể có phụ phí riêng)
  virtual QString toSummaryString() const = 0;
    // Lớp con trả về 1 dòng tóm tắt hiển thị UI, vd:
    // "Hóa đơn ngoại trú #INV-20260702-0001 — 350,000 VNĐ"

Không cần .cpp implement gì thêm ngoài constructor (vì 2 hàm chính là pure virtual).
Include: "CommonEnums.h", <QDate>, <QString>, <optional>.
```

**Tiêu chí hoàn thành:** compile độc lập được (dù chưa có lớp con), không có hàm nào thiếu `= 0`.

---

### MR-03 — Model `OutPatientInvoice` + `InPatientInvoice` + `EmergencyInvoice`

**Phụ thuộc:** MR-02 phải xong trước.

**Prompt riêng:**

```
Nhiệm vụ: Viết 3 cặp file kế thừa Invoice (đã có sẵn ở model/Invoice.h):
  model/OutPatientInvoice.h/.cpp
  model/InPatientInvoice.h/.cpp
  model/EmergencyInvoice.h/.cpp

Cả 3 class kế thừa public từ Invoice, constructor forward tham số lên lớp cha
(patientId, consultationFee, medicationFee, issuedDate) — giống cách
OutPatient.h kế thừa Patient.h trong dự án.

OutPatientInvoice::calculate() const override:
  return m_consultationFee + m_medicationFee; // không phụ phí

OutPatientInvoice::toSummaryString() const override:
  return QString("Hóa đơn ngoại trú #%1 — %2 VNĐ")
           .arg(m_invoiceCode).arg(calculate(), 0, 'f', 0);

InPatientInvoice::calculate() const override:
  return m_consultationFee + m_medicationFee; // (chưa tính phụ phí giường bệnh
  // theo ngày — nếu sau này cần, đây là chỗ mở rộng, để nguyên logic đơn giản
  // cho phạm vi hiện tại)

InPatientInvoice::toSummaryString() const override:
  return QString("Hóa đơn nội trú #%1 — %2 VNĐ")
           .arg(m_invoiceCode).arg(calculate(), 0, 'f', 0);

EmergencyInvoice::calculate() const override:
  return m_consultationFee + m_medicationFee; // Tính cơ bản, sẽ mở rộng nếu có thêm phụ phí cấp cứu sau này

EmergencyInvoice::toSummaryString() const override:
  return QString("Hóa đơn cấp cứu #%1 — %2 VNĐ")
           .arg(m_invoiceCode).arg(calculate(), 0, 'f', 0);
```

**Tiêu chí hoàn thành:** cả 3 class compile, `calculate()` trả đúng tổng đã cộng.

---

### MR-04 — DTO MedicalRecord

**Phụ thuộc:** MR-01 (dùng lại `VitalSigns`, `Diagnosis`).

**Prompt riêng:**

```
Nhiệm vụ: Viết dto/MedicalRecordDTOs.h.
#include "model/MedicalRecord.h" để dùng lại VitalSigns, Diagnosis.

struct MedicalRecordInsertDTO {
  int patientId;
  int doctorId;
  std::optional<int> appointmentId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QString treatment;
  std::optional<QDate> nextVisitDate;
  QVector<Diagnosis> diagnoses;
};

struct MedicalRecordResultDTO {
  int recordId;
  int patientId;
  int doctorId;
  std::optional<int> appointmentId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QString treatment;
  std::optional<QDate> nextVisitDate;
  QVector<Diagnosis> diagnoses;
};

Chỉ là struct dữ liệu thuần, không thêm hàm/logic gì khác.
```

**Tiêu chí hoàn thành:** compile độc lập, field đặt tên khớp 100% với 2 struct trên (đồng đội và các bước sau sẽ dùng đúng tên này).

---

### MR-05 — DTO Billing

**Prompt riêng:**

```
Nhiệm vụ: Viết dto/BillingDTOs.h.
#include "model/CommonEnums.h" (dùng PatientType).

struct InvoiceItemDTO {
  QString itemType;    // "CONSULTATION" / "MEDICATION" / "SERVICE"
  QString description;
  int quantity;
  double unitPrice;
  double subtotal;
};

struct InvoiceInsertDTO {
  int patientId;
  std::optional<int> recordId;
  PatientType patientType;
  double consultationFee;
  double medicationFee;
  double totalAmount;
  QDate issuedDate;
  QVector<InvoiceItemDTO> items;
};

struct InvoiceResultDTO {
  int invoiceId;
  QString invoiceCode;
  int patientId;
  std::optional<int> recordId;
  PatientType patientType;
  double consultationFee;
  double medicationFee;
  double totalAmount;
  QString status;
  QDate issuedDate;
  std::optional<QDate> paidDate;
  QVector<InvoiceItemDTO> items;
};

Chỉ là struct dữ liệu thuần.
```

**Tiêu chí hoàn thành:** field đặt tên khớp 100% với schema `invoices`/`invoice_items` đã tạo ở MR-00.

---

### MR-05B — (Tuỳ chọn) `InvoiceStatus` enum

**Prompt riêng:**

```
Nhiệm vụ: Mở file model/CommonEnums.h HIỆN CÓ (không tạo file mới), thêm vào
CUỐI file (không sửa enum có sẵn nào khác):

enum class InvoiceStatus { UNPAID, PAID, CANCELLED };

inline QString InvoiceStatusToString(InvoiceStatus status) {
  switch (status) {
  case InvoiceStatus::UNPAID:    return QString("UNPAID");
  case InvoiceStatus::PAID:      return QString("PAID");
  case InvoiceStatus::CANCELLED: return QString("CANCELLED");
  }
}

inline InvoiceStatus stringToInvoiceStatus(const QString &str) {
  if (str == "UNPAID")    return InvoiceStatus::UNPAID;
  if (str == "PAID")      return InvoiceStatus::PAID;
  if (str == "CANCELLED") return InvoiceStatus::CANCELLED;
  return InvoiceStatus::UNPAID;
}

CHỈ thêm đoạn trên, không đổi bất kỳ dòng nào khác đang có trong file.
```

**Tiêu chí hoàn thành:** file cũ không mất nội dung, chỉ có thêm đoạn mới ở cuối.

---

### MR-06 — Validation bổ sung

**Phụ thuộc:** MR-01.

**Prompt riêng:**

```
Nhiệm vụ: Mở Validation.h và Validation.cpp HIỆN CÓ, thêm 3 hàm mới (không
sửa hàm cũ). Thêm khai báo vào Validation.h, cài đặt vào Validation.cpp.
#include "model/MedicalRecord.h" ở Validation.h để dùng VitalSigns, Diagnosis.

QString validateVitalSigns(const VitalSigns &vitals);
  - temperature phải trong khoảng 30.0–45.0 (°C), ngoài khoảng → lỗi.
  - heartRate phải trong khoảng 30–250 (bpm), ngoài khoảng → lỗi.
  - weight phải > 0 và <= 500 (kg), ngoài khoảng → lỗi.
  - height phải > 0 và <= 250 (cm), ngoài khoảng → lỗi.
  - Hợp lệ → trả "".

QString validateChiefComplaint(const QString &complaint);
  - Rỗng hoặc chỉ toàn khoảng trắng (dùng .trimmed().isEmpty()) → lỗi
    "Lý do khám không được để trống.".
  - Hợp lệ → trả "".

QString validateDiagnosisList(const QVector<Diagnosis> &diagnoses);
  - diagnoses rỗng → lỗi "Phải có ít nhất một chẩn đoán.".
  - Với mỗi phần tử: description rỗng → lỗi.
  - Với mỗi phần tử: severity sau khi .toUpper() không thuộc
    {"MILD","MODERATE","SEVERE"} → lỗi "Mức độ chẩn đoán không hợp lệ.".
    (Đây là do cột diagnoses.severity trong DB có CHECK constraint bắt buộc
    chữ HOA đúng 3 giá trị này.)
  - Hợp lệ → trả "".

Theo đúng convention: hàm tự do (không phải method của class nào), trả "" nếu
hợp lệ, chuỗi lỗi tiếng Việt nếu không hợp lệ — giống validatePhoneNumber() đã
có sẵn trong file.
```

**Tiêu chí hoàn thành:** test nhanh 2-3 case hợp lệ/không hợp lệ mỗi hàm bằng `qDebug()`, không làm hỏng các hàm `validate...` đã có sẵn trong file.

---

### MR-07 — Repository `MedicalRecordRepository` (phần của bạn)

**Phụ thuộc:** MR-04, MR-06.

**Prompt riêng:**

```
Nhiệm vụ: Tạo repository/MedicalRecordRepository.h và .cpp — LƯU Ý đây là file
CHUNG với đồng đội (họ sẽ thêm các hàm liên quan Prescription vào cùng file
này sau). Header cần khai báo ĐỦ chữ ký của cả 2 phần (kể cả phần đồng đội,
dù bạn không cài đặt), .cpp CHỈ cài đặt phần của bạn.

repository/MedicalRecordRepository.h:

#pragma once
#include "dto/MedicalRecordDTOs.h"
#include <QString>
#include <QVector>
#include <optional>

// Forward declare — đồng đội sẽ #include header thật của họ khi PrescriptionDTOs.h
// tồn tại; để file này compile ngay bây giờ, tạm forward declare.
struct PrescriptionItemDTO;

class MedicalRecordRepository {
private:
  // Helper riêng của bạn — insert từng dòng chẩn đoán sau khi đã có recordId.
  bool insertDiagnoses(int recordId, const QVector<Diagnosis> &diagnoses);

public:
  // ── Phần của bạn ──
  int insertMedicalRecord(const MedicalRecordInsertDTO &dto); // trả -1 nếu lỗi
  std::optional<MedicalRecordResultDTO> findById(int recordId);
  QVector<MedicalRecordResultDTO> getHistoryByPatientId(int patientId);

  // ── Phần đồng đội — CHỈ khai báo, KHÔNG cài đặt, để trống cho họ ──
  bool insertPrescription(int recordId, const QVector<PrescriptionItemDTO> &items);
  std::optional<QVector<PrescriptionItemDTO>> getPrescriptionByRecordId(int recordId);
  bool confirmPrescription(int prescriptionId);
};

repository/MedicalRecordRepository.cpp — CHỈ cài đặt 4 hàm sau (không viết thân
hàm cho 3 hàm Prescription, để đồng đội tự thêm sau):

1. insertMedicalRecord(dto):
   - Dùng DatabaseManager::getInstance().beginTransaction().
   - INSERT INTO medical_records (patient_id, doctor_id, appointment_id,
     visit_datetime, temperature, blood_pressure, heart_rate, weight, height,
     chief_complaint, clinical_notes, treatment, next_visit_date)
     VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)
     — theo đúng pattern QVariant nullable đã mô tả ở Ngữ cảnh dùng chung cho
     appointmentId và nextVisitDate.
   - Lấy recordId bằng "SELECT last_insert_rowid()" giống insertBasePatient()
     trong PatientRepository.cpp.
   - Gọi insertDiagnoses(recordId, dto.diagnoses).
   - commitTransaction() nếu mọi bước OK, rollbackTransaction() + trả -1 nếu
     bất kỳ bước nào lỗi.

2. insertDiagnoses(recordId, diagnoses) [private]:
   - Với mỗi Diagnosis trong danh sách, INSERT INTO diagnoses
     (record_id, icd_code, description, severity) VALUES (?,?,?,?).
   - severity insert dưới dạng .toUpper() để khớp CHECK constraint.
   - Trả false ngay nếu 1 insert thất bại (để insertMedicalRecord() rollback).

3. findById(recordId):
   - SELECT * FROM medical_records WHERE record_id = ? AND is_deleted = 0.
   - Nếu không có dòng nào → trả std::nullopt.
   - Nếu có → map sang MedicalRecordResultDTO, sau đó chạy thêm
     SELECT * FROM diagnoses WHERE record_id = ? để đổ vào field diagnoses.

4. getHistoryByPatientId(patientId):
   - SELECT * FROM medical_records WHERE patient_id = ? AND is_deleted = 0
     ORDER BY visit_datetime DESC.
   - Với mỗi record, query thêm diagnoses tương ứng (giống findById) rồi gộp
     vào QVector<MedicalRecordResultDTO> trả về.

Include DatabaseManager.h, QSqlQuery, QVariant ở đầu .cpp.
```

**Tiêu chí hoàn thành:**

- Insert 1 medical record test kèm 2 diagnoses trong `runIntegrationTests()` (main.cpp), `recordId` trả về hợp lệ (> 0).
- `findById()` đọc lại đúng dữ liệu vừa insert, kể cả danh sách diagnoses.
- **Push code lên Git ngay sau khi test pass** — đồng đội đang chờ `recordId` thật.

---

### MR-08 — Repository `BillingRepository`

**Phụ thuộc:** MR-05.

**Prompt riêng:**

```
Nhiệm vụ: Tạo repository/BillingRepository.h và .cpp (file RIÊNG, toàn quyền).

repository/BillingRepository.h:

#pragma once
#include "dto/BillingDTOs.h"
#include <QVector>
#include <optional>

class BillingRepository {
private:
  QString generateInvoiceCode(); // helper sinh mã "INV-yyyyMMdd-NNNN"

public:
  bool insertInvoice(const InvoiceInsertDTO &dto);
  std::optional<InvoiceResultDTO> getInvoiceByRecordId(int recordId);
  QVector<InvoiceResultDTO> getInvoicesByPatientId(int patientId);
};

repository/BillingRepository.cpp:

1. generateInvoiceCode() [private]:
   - Format: "INV-" + ngày hiện tại (yyyyMMdd) + "-" + số thứ tự 4 chữ số.
   - Đếm số invoice đã tạo trong ngày (SELECT COUNT(*) FROM invoices WHERE
     invoice_code LIKE 'INV-yyyyMMdd-%') rồi +1, pad 4 chữ số bằng '0'
     — theo đúng cách Patient::generatePatientCode() đã làm.

2. insertInvoice(dto):
   - beginTransaction().
   - INSERT INTO invoices (invoice_code, patient_id, record_id, patient_type,
     consultation_fee, medication_fee, total_amount, status, issued_date)
     VALUES (?,?,?,?,?,?,?,'UNPAID',?)
     — patient_type insert bằng PatientTypeToString(dto.patientType).
   - Lấy invoiceId qua last_insert_rowid().
   - Với mỗi InvoiceItemDTO trong dto.items: INSERT INTO invoice_items
     (invoice_id, item_type, description, quantity, unit_price, subtotal)
     VALUES (?,?,?,?,?,?).
   - commitTransaction() nếu OK, rollbackTransaction() + trả false nếu lỗi.

3. getInvoiceByRecordId(recordId):
   - SELECT * FROM invoices WHERE record_id = ?.
   - Nếu có → map InvoiceResultDTO, query thêm invoice_items theo invoice_id
     để đổ vào field items. Nếu không có → std::nullopt.

4. getInvoicesByPatientId(patientId):
   - SELECT * FROM invoices WHERE patient_id = ? ORDER BY issued_date DESC.
   - Tương tự trên, đổ kèm items cho từng invoice.
```

**Tiêu chí hoàn thành:** insert 1 invoice test kèm 2-3 invoice_items, `getInvoiceByRecordId()` đọc lại đúng `total_amount` và danh sách items.

---

### MR-12 — Factory `IInvoiceFactory`

**Phụ thuộc:** MR-02, MR-03.

**Prompt riêng:**

```
Nhiệm vụ: Tạo factory/IInvoiceFactory.h, factory/OutPatientInvoiceFactory.h/.cpp,
factory/InPatientInvoiceFactory.h/.cpp, factory/EmergencyInvoiceFactory.h/.cpp.

factory/IInvoiceFactory.h:

#pragma once
#include "model/Invoice.h"
#include <memory>

class IInvoiceFactory {
public:
  virtual ~IInvoiceFactory() = default;
  virtual std::unique_ptr<Invoice> createInvoice(
      int patientId, double consultationFee, double medicationFee,
      const QDate &issuedDate) = 0;
};

OutPatientInvoiceFactory / InPatientInvoiceFactory / EmergencyInvoiceFactory kế thừa IInvoiceFactory,
override createInvoice() để new đúng loại Invoice tương ứng (OutPatientInvoice
/ InPatientInvoice / EmergencyInvoice từ MR-03), trả về qua std::make_unique.
```

**Tiêu chí hoàn thành:** `createInvoice()` của cả 3 factory trả đúng kiểu con tương ứng, `calculate()` gọi được ngay sau khi tạo.

---

### MR-09 — Service `MedicalRecordService` (phần của bạn)

**Phụ thuộc:** MR-07, MR-06, MR-04.

**Prompt riêng:**

```
Nhiệm vụ: Tạo service/MedicalRecordService.h và .cpp — file CHUNG với đồng đội.

service/MedicalRecordService.h:

#pragma once
#include "dto/MedicalRecordDTOs.h"
#include <memory>
#include <optional>

class MedicalRecordRepository;
class PatientService;
class PharmacyService;   // của đồng đội — forward declare, chưa tồn tại file thật
struct PrescriptionItemDTO; // của đồng đội — forward declare

class MedicalRecordService {
private:
  std::shared_ptr<MedicalRecordRepository> m_recordRepository;
  std::shared_ptr<PatientService> m_patientService;
  std::shared_ptr<PharmacyService> m_pharmacyService;

public:
  explicit MedicalRecordService(
      std::shared_ptr<MedicalRecordRepository> recordRepo,
      std::shared_ptr<PatientService> patientService,
      std::shared_ptr<PharmacyService> pharmacyService);

  // ── Phần của bạn ──
  int createMedicalRecord(const MedicalRecordInsertDTO &dto);
  QVector<MedicalRecordResultDTO> getMedicalHistory(int patientId);

  // ── Phần đồng đội — CHỈ khai báo, không cài đặt ──
  bool addPrescription(int recordId, const QVector<PrescriptionItemDTO> &items);
  std::optional<QVector<PrescriptionItemDTO>> getPrescriptionByRecordId(int recordId);
};

service/MedicalRecordService.cpp — CHỈ cài đặt constructor + 2 hàm của bạn
(KHÔNG viết thân hàm cho addPrescription()/getPrescriptionByRecordId(), để
trống cho đồng đội thêm sau, tránh trùng lặp symbol):

1. Constructor: gán 3 tham số vào 3 member tương ứng (giống PatientService).

2. createMedicalRecord(dto):
   - Validate tuần tự, dừng ngay và trả -1 khi gặp lỗi đầu tiên:
     validateVitalSigns(dto.vitals), validateChiefComplaint(dto.chiefComplaint),
     validateDiagnosisList(dto.diagnoses) — cả 3 hàm từ Validation.h (MR-06).
   - Nếu qua hết → gọi m_recordRepository->insertMedicalRecord(dto), trả
     thẳng kết quả (đã là int recordId hoặc -1).

3. getMedicalHistory(patientId):
   - Gọi thẳng m_recordRepository->getHistoryByPatientId(patientId), trả
     nguyên kết quả (Service không cần xử lý gì thêm ở đây).

LƯU Ý: vì PharmacyService và PrescriptionItemDTO thật chưa tồn tại, header
này chỉ compile được khi các hàm dùng chúng (constructor, 2 hàm đồng đội)
KHÔNG bị gọi ở nơi khác chưa có định nghĩa đầy đủ. Nếu compiler báo lỗi do
forward declaration chưa đủ khi liên kết (link), tạm thời không cần lo — đây
là trạng thái mong đợi cho tới khi đồng đội bổ sung file PharmacyService.h và
dto/PrescriptionDTOs.h thật.
```

**Tiêu chí hoàn thành:** `createMedicalRecord()` test với input hợp lệ trả `recordId > 0`, với input thiếu `chiefComplaint` trả `-1`.

---

### MR-10 — `PatientService::checkDrugAllergyConflict()`

**Ưu tiên cao nhất trong Đợt 1** — đồng đội đang chờ hàm này.

**Prompt riêng:**

```
Nhiệm vụ: Mở PatientRepository.h/.cpp và PatientService.h/.cpp HIỆN CÓ, thêm
hàm mới vào mỗi file (không sửa hàm cũ).

BƯỚC 1 — PatientRepository.h, thêm vào phần public:
  QString getAllergiesByPatientId(int patientId);
    // Trả về nội dung cột allergies, hoặc "" nếu không tìm thấy patientId.

BƯỚC 2 — PatientRepository.cpp, cài đặt:
  SELECT allergies FROM patients WHERE patient_id = ? AND is_deleted = 0.
  Nếu có dòng kết quả → trả query.value(0).toString().
  Nếu không có → trả "".

BƯỚC 3 — PatientService.h, thêm vào phần public:
  /**
   * @brief Kiểm tra bệnh nhân có dị ứng với loại thuốc chỉ định không.
   * @param patientId ID bệnh nhân cần kiểm tra.
   * @param drugName  Tên thuốc cần kiểm tra (so khớp với danh sách dị ứng).
   * @return true nếu có xung đột dị ứng, false nếu an toàn hoặc không có dữ liệu.
   */
  bool checkDrugAllergyConflict(int patientId, const QString &drugName) const;

BƯỚC 4 — PatientService.cpp, cài đặt:
  QString allergies = m_patientRepository->getAllergiesByPatientId(patientId);
  if (allergies.trimmed().isEmpty()) return false;
  const QStringList allergyList = allergies.split(',', Qt::SkipEmptyParts);
  const QString target = drugName.trimmed().toLower();
  for (const QString &entry : allergyList) {
    if (entry.trimmed().toLower() == target) return true;
  }
  return false;
  // Logic giống hệt Patient::hasAllergy() đã có trong Patient.cpp, chỉ khác
  // là lấy dữ liệu qua Repository thay vì từ object Patient có sẵn trong bộ nhớ.
```

**Tiêu chí hoàn thành:** test với 1 bệnh nhân có `allergies = "Penicillin, Aspirin"`, gọi `checkDrugAllergyConflict(id, "penicillin")` (chữ thường, có khoảng trắng thừa) phải trả `true`.

---

### MR-11 — Service `BillingService`

**Phụ thuộc:** MR-08, MR-12, và **`PrescriptionItemDTO` thật từ đồng đội** (xem rủi ro ở Phần 4).

**Prompt riêng:**

```
Nhiệm vụ: Tạo service/BillingService.h và .cpp (file RIÊNG, toàn quyền).

Nếu dto/PrescriptionDTOs.h (của đồng đội) CHƯA tồn tại khi bạn làm nhiệm vụ
này, tạm định nghĩa struct mock CÙNG TÊN CÙNG FIELD ở đầu BillingService.cpp
(không tạo file dto/PrescriptionDTOs.h thật — đó là việc của đồng đội) để có
thể code và test độc lập trước:

  struct PrescriptionItemDTO {
    int medicationId; QString medicationName; int quantity;
    double unitPrice; QString dosage; QString frequency;
    int durationDays; QString note;
  };

service/BillingService.h:

#pragma once
#include "dto/BillingDTOs.h"
#include "model/CommonEnums.h"
#include <memory>
#include <optional>

class BillingRepository;
class IInvoiceFactory;
struct PrescriptionItemDTO; // của đồng đội

class BillingService {
private:
  std::shared_ptr<BillingRepository> m_billingRepository;
  std::unique_ptr<IInvoiceFactory> selectFactory(PatientType type);

public:
  explicit BillingService(std::shared_ptr<BillingRepository> repo);

  bool generateInvoice(int patientId, int recordId, PatientType type,
                       double consultationFee,
                       const QVector<PrescriptionItemDTO> &prescriptionItems);

  double calculateMedicationTotal(const QVector<PrescriptionItemDTO> &items) const;
  std::optional<InvoiceResultDTO> getInvoiceByRecordId(int recordId);
};

service/BillingService.cpp:

1. selectFactory(type) [private]:
   - if (type == PatientType::INPATIENT) return std::make_unique<InPatientInvoiceFactory>();
   - else if (type == PatientType::EMERGENCY) return std::make_unique<EmergencyInvoiceFactory>();
   - else return std::make_unique<OutPatientInvoiceFactory>();

2. calculateMedicationTotal(items) const:
   - Cộng dồn (item.quantity * item.unitPrice) cho mọi phần tử, trả tổng.

3. generateInvoice(patientId, recordId, type, consultationFee, prescriptionItems):
   - double medicationFee = calculateMedicationTotal(prescriptionItems).
   - Build InvoiceInsertDTO:
     patientId, recordId (bọc std::optional), patientType = type,
     consultationFee, medicationFee,
     totalAmount = consultationFee + medicationFee,
     issuedDate = QDate::currentDate().
   - Build items: 1 dòng "CONSULTATION" (quantity=1, unitPrice=consultationFee,
     subtotal=consultationFee) + N dòng "MEDICATION" (1 dòng mỗi
     PrescriptionItemDTO, description=item.medicationName, quantity=item.quantity,
     unitPrice=item.unitPrice, subtotal=item.quantity*item.unitPrice).
   - Gọi m_billingRepository->insertInvoice(dto), trả thẳng kết quả.

4. getInvoiceByRecordId(recordId):
   - Gọi thẳng m_billingRepository->getInvoiceByRecordId(recordId).
```

**Tiêu chí hoàn thành:** test `generateInvoice()` với 2-3 `PrescriptionItemDTO` giả, kiểm tra `total_amount` lưu trong DB = `consultationFee + sum(quantity*unitPrice)` đúng tuyệt đối (so sánh double nên dùng sai số nhỏ, không so bằng tuyệt đối).

---

### MR-13 — UI `MedicalRecordDialog`

**Phụ thuộc:** MR-09.

**Prompt riêng:**

```
Nhiệm vụ: Tạo ui/dialogs/MedicalRecordDialog.h và .cpp — QDialog nhập thông
tin 1 lần khám.

Yêu cầu chức năng:
- Form nhập: chiefComplaint (QLineEdit/QTextEdit), clinicalNotes (QTextEdit),
  treatment (QTextEdit), vitals (4 QDoubleSpinBox/QSpinBox cho temperature,
  heartRate, weight, height + 1 QLineEdit cho bloodPressure),
  nextVisitDate (QDateEdit, có checkbox bật/tắt vì optional).
- Danh sách chẩn đoán: QTableWidget hoặc QListWidget cho phép thêm nhiều dòng
  Diagnosis (icdCode, description, severity — severity dùng QComboBox với 3
  lựa chọn Mild/Moderate/Severe, khi build DTO thì .toUpper() trước khi gửi
  xuống Service).
- Nút "Lưu": build MedicalRecordInsertDTO từ form, gọi
  m_medicalRecordService->createMedicalRecord(dto).
  - Nếu trả về recordId > 0: QMessageBox thông báo thành công, đóng dialog
    (accept()), lưu lại recordId để nơi gọi dialog dùng tiếp (vd mở tiếp màn
    kê đơn cho đồng đội, hoặc mở InvoiceDialog).
  - Nếu trả về -1: hiển thị lỗi validate (QMessageBox::warning) — vì Service
    hiện chỉ trả -1 khi lỗi mà không kèm message chi tiết, có thể tạm hiển thị
    thông báo lỗi chung "Dữ liệu không hợp lệ, vui lòng kiểm tra lại." hoặc đề
    xuất đồng đội bàn thêm về việc đổi createMedicalRecord() trả QString lỗi
    kèm theo nếu cần UX tốt hơn.

Constructor nhận std::shared_ptr<MedicalRecordService>, int patientId (bệnh
nhân đang được khám), int doctorId (bác sĩ đang đăng nhập).
```

**Tiêu chí hoàn thành:** bấm "Lưu" với dữ liệu hợp lệ tạo được record mới trong DB, kiểm tra lại bằng `sqlite3`/DB Browser.

---

### MR-14 — UI `InvoiceDialog`

**Phụ thuộc:** MR-11.

**Prompt riêng:**

```
Nhiệm vụ: Tạo ui/dialogs/InvoiceDialog.h và .cpp — xem trước hóa đơn.

Yêu cầu chức năng CƠ BẢN (bắt buộc):
- Nhận sẵn InvoiceResultDTO (đã tạo xong từ BillingService::generateInvoice()
  + getInvoiceByRecordId()) qua constructor, không tự gọi Service bên trong
  dialog này (giữ dialog thuần hiển thị).
- QTableWidget hiển thị từng InvoiceItemDTO (description, quantity, unitPrice,
  subtotal).
- Label tổng cộng hiển thị totalAmount, format có dấu phân cách hàng nghìn.
- Nút "Đóng".

Yêu cầu NÂNG CAO (tuỳ chọn, không bắt buộc cho phạm vi hiện tại):
- Nút "In hóa đơn" dùng QPrinter + QPrintDialog — chỉ làm nếu còn thời gian,
  không phải yêu cầu lõi.
```

**Tiêu chí hoàn thành:** dialog hiển thị đúng danh sách item và tổng tiền khớp dữ liệu DB.

---

### MR-15 — (Tuỳ chọn) `MedicalTimelineWidget`

**Phụ thuộc:** MR-07. **Ưu tiên thấp — chỉ làm nếu còn thời gian.**

**Prompt riêng:**

```
Nhiệm vụ: Tạo ui/widgets/MedicalTimelineWidget.h/.cpp — custom QWidget vẽ
bằng QPainter, hiển thị lịch sử khám của 1 bệnh nhân dạng dòng thời gian dọc.

Yêu cầu:
- Constructor nhận QVector<MedicalRecordResultDTO> (dữ liệu lấy sẵn từ
  MedicalRecordService::getMedicalHistory(), widget không tự gọi Service).
- override paintEvent(QPaintEvent*): vẽ 1 đường dọc, mỗi MedicalRecordResultDTO
  là 1 điểm mốc trên đường đó kèm ngày khám + chiefComplaint rút gọn.
- override sizeHint() trả kích thước hợp lý theo số lượng record.

Đây là tính năng bonus theo tài liệu kiến trúc, không phải yêu cầu lõi — có
thể làm sau cùng hoặc bỏ qua nếu thời gian hạn chế.
```

**Tiêu chí hoàn thành:** widget render không crash với danh sách rỗng và danh sách có nhiều record.

---

## 4. Rủi ro & phụ thuộc bên ngoài cần theo dõi

| Rủi ro | Ảnh hưởng | Cách xử lý |
|--------|-----------|------------|
| Đồng đội chưa xong `PrescriptionItemDTO` thật khi bạn đến MR-11 | `BillingService` không compile được với type thật | Dùng struct mock tạm thời (đã hướng dẫn trong prompt MR-11), thay bằng `#include` thật khi đồng đội xong |
| Đồng đội chưa xong `PharmacyService` khi bạn đến MR-09 | `MedicalRecordService.h` cần forward declare | Đã xử lý bằng forward declaration trong prompt MR-09, không block |
| 2 người cùng sửa `MedicalRecordRepository.h` hoặc `MedicalRecordService.h` cùng lúc | Git conflict | Áp dụng quy tắc ở Ngữ cảnh dùng chung: chỉ thêm phần mình, không sửa chữ ký người khác đã khai báo |
| Tên bảng/cột `staff`, `appointments` chưa được xác nhận bởi module Nhân sự/Lịch hẹn | FK trong `medical_records` trỏ sai bảng | Đã tạo bảng, nhưng cần confirm lại với người phụ trách 2 module đó trước khi module của họ hoàn thiện |

---

## 5. Checklist bàn giao cuối (trước khi ghép code với đồng đội)

- [ ] MR-07 đã push Git, đồng đội đã pull được `recordId` thật để test `addPrescription()`.
- [ ] MR-10 đã push Git, đồng đội đã gọi thử `checkDrugAllergyConflict()` thành công.
- [ ] Đã nhận `dto/PrescriptionDTOs.h` thật từ đồng đội, thay thế struct mock trong `BillingService.cpp` (MR-11).
- [ ] Đã xác nhận với đồng đội: nút "Xác nhận đơn thuốc" (trừ kho) và nút "Xuất hóa đơn" là 2 hành động UI tách biệt.
- [ ] Build toàn bộ project (cả 2 phần gộp lại) không lỗi, chạy `runIntegrationTests()` pass hết.
