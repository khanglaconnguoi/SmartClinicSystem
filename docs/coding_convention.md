# Coding Convention — Hospital Manager

## 1. Quy tắc đặt tên (Naming Convention)

### 1.1 Files

| Loại file | Quy tắc | Ví dụ |
| --- | --- | --- |
| Header C++ | `PascalCase.h` | `PatientModel.h` |
| Source C++ | `PascalCase.cpp` | `PatientModel.cpp` |
| Qt UI file | `lowercase_snake.ui` | `patient_form.ui` |
| Resource file | `lowercase_snake.qrc` | `icons.qrc` |

### 1.2 Classes

Dùng **PascalCase**. Tên class phải là danh từ mô tả rõ ý nghĩa.

```cpp
// ✅ Đúng
class PatientModel { };
class AppointmentController { };
class DatabaseManager { };

// ❌ Sai
class patient { };
class appt_ctrl { };
```

### 1.3 Hàm và phương thức

Dùng **camelCase**. Động từ đứng đầu, mô tả hành động rõ ràng.

```cpp
// ✅ Đúng
void addPatient(const Patient &patient);
QList<Doctor> getAllDoctors();
bool deleteAppointment(int appointmentId);

// ❌ Sai
void Add_Patient();
QList<Doctor> doctors();
bool del(int id);
```

### 1.4 Biến

Dùng **camelCase** cho biến thông thường.

```cpp
// ✅ Đúng
int patientAge;
QString doctorName;
bool isAvailable;

// ❌ Sai
int PatientAge;
QString doctor_name;
bool IsAvailable;
```

### 1.5 Biến thành viên của class (Member variables)

Thêm tiền tố `m_` để phân biệt với biến cục bộ.

```cpp
class Patient {
private:
    int     m_id;
    QString m_fullName;
    QString m_phoneNumber;
    bool    m_isActive;
};
```

### 1.6 Hằng số và Enum

Dùng **UPPER_SNAKE_CASE** cho hằng số.  
Dùng **PascalCase** cho tên enum và **PascalCase** cho giá trị enum.

```cpp
// Hằng số
const int MAX_PATIENTS_PER_DOCTOR = 50;
const QString DB_FILE_NAME = "hospital.db";

// Enum
enum class AppointmentStatus {
    Pending,
    Confirmed,
    Cancelled,
    Completed
};
```

### 1.7 Slots và Signals (Qt)

Slots dùng tiền tố `on_` kèm tên widget (nếu auto-connect) hoặc mô tả hành động.

```cpp
// Qt auto-connect slots
private slots:
    void on_btnAddPatient_clicked();
    void on_tablePatients_doubleClicked(const QModelIndex &index);

// Slots thông thường
private slots:
    void handlePatientSaved(int patientId);
    void refreshPatientTable();

// Signals
signals:
    void patientAdded(const Patient &patient);
    void appointmentStatusChanged(int appointmentId, AppointmentStatus status);
```

---

## 2. Quy tắc viết code

### 2.1 Indentation & Spacing

- Dùng **4 spaces** (không dùng tab).
- Giới hạn **100 ký tự** mỗi dòng.
- Dấu `{` mở block đặt **cùng dòng** với lệnh (K&R style).

```cpp
// ✅ Đúng
void PatientController::addPatient(const Patient &patient) {
    if (!patient.isValid()) {
        qWarning() << "Invalid patient data";
        return;
    }
    m_db->insertPatient(patient);
}

// ❌ Sai
void PatientController::addPatient(const Patient &patient)
{
    if(!patient.isValid()){
        qWarning() <<"Invalid patient data";
        return;
    }
}
```

### 2.2 Khoảng cách trong biểu thức

```cpp
// ✅ Đúng
int total = a + b * c;
if (age >= 18 && isActive) { }
for (int i = 0; i < count; ++i) { }

// ❌ Sai
int total=a+b*c;
if(age>=18&&isActive){}
for(int i=0;i<count;i++){}
```

### 2.3 Con trỏ và tham chiếu

Đặt `*` và `&` sát tên kiểu dữ liệu (không sát tên biến).

```cpp
// ✅ Đúng
Patient* patient;
const Doctor& doctor;
void setModel(PatientModel* model);

// ❌ Sai
Patient *patient;
const Doctor &doctor;
```

### 2.4 Ưu tiên `nullptr` thay vì `NULL` hay `0`

```cpp
// ✅ Đúng
Patient* p = nullptr;
if (m_currentPatient == nullptr) { }

// ❌ Sai
Patient* p = NULL;
if (m_currentPatient == 0) { }
```

### 2.5 Ưu tiên `auto` khi kiểu dữ liệu đã rõ ràng

```cpp
// ✅ Đúng
auto patients = m_db->getAllPatients();   // rõ ràng từ hàm
auto it = patientMap.find(id);

// ❌ Tránh dùng auto khi kiểu không rõ
auto x = getValue();  // không biết getValue() trả về gì
```

---

## 3. Classes — Quy tắc thiết kế

### 3.1 Thứ tự khai báo trong class

```cpp
class PatientModel : public QAbstractTableModel {
    Q_OBJECT  // Luôn đặt đầu tiên nếu dùng Qt

public:
    // 1. Constructors / Destructor
    explicit PatientModel(QObject* parent = nullptr);
    ~PatientModel() override;

    // 2. Public methods
    void loadFromDatabase();
    bool addPatient(const Patient &patient);

    // 3. Override methods (Qt)
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

signals:
    void dataChanged(int patientId);

public slots:
    void onPatientUpdated(const Patient &patient);

private slots:
    void onDatabaseError(const QString &error);

private:
    // 4. Private methods
    void refreshCache();

    // 5. Member variables
    DatabaseManager*  m_db;
    QList<Patient>    m_patients;
};
```

### 3.2 Tách biệt header (.h) và implementation (.cpp)

Header chỉ chứa khai báo. Không viết logic phức tạp trong header (trừ template hoặc inline nhỏ).

```cpp
// PatientModel.h — chỉ khai báo
class PatientModel {
public:
    bool addPatient(const Patient &patient);
};

// PatientModel.cpp — implementation
bool PatientModel::addPatient(const Patient &patient) {
    // logic ở đây
}
```

### 3.3 Sử dụng `explicit` cho constructor đơn tham số

```cpp
// ✅ Đúng
explicit PatientModel(QObject* parent = nullptr);

// ❌ Sai (có thể gây implicit conversion không mong muốn)
PatientModel(QObject* parent = nullptr);
```

---

## 4. Qt — Quy tắc riêng

### 4.1 Kết nối Signal-Slot

Ưu tiên dùng cú pháp con trỏ hàm (Qt 5+) thay vì chuỗi `SIGNAL`/`SLOT`.

```cpp
// ✅ Đúng — type-safe, phát hiện lỗi lúc compile
connect(btnSave, &QPushButton::clicked,
        this,    &PatientForm::onSaveClicked);

// ❌ Tránh — lỗi chỉ phát hiện lúc runtime
connect(btnSave, SIGNAL(clicked()),
        this,    SLOT(onSaveClicked()));
```

### 4.2 Quản lý bộ nhớ Qt (Object Tree)

Luôn truyền `parent` cho QObject để Qt tự quản lý vòng đời.

```cpp
// ✅ Đúng
auto* label = new QLabel("Tên bệnh nhân:", this);  // this là parent
auto* btn   = new QPushButton("Lưu", this);

// ❌ Sai — có thể bị leak nếu không delete thủ công
auto* label = new QLabel("Tên bệnh nhân:");
```

### 4.3 Hiển thị thông báo lỗi

Dùng `qDebug()`, `qWarning()`, `qCritical()` thay vì `cout`.

```cpp
qDebug()    << "Loaded" << patients.size() << "patients";
qWarning()  << "Patient ID not found:" << id;
qCritical() << "Database connection failed!";
```

---

## 5. SQLite / Database — Quy tắc

### 5.1 Luôn dùng Prepared Statement

Không bao giờ nối chuỗi trực tiếp vào câu SQL (nguy cơ SQL Injection).

```cpp
// ✅ Đúng
QSqlQuery query;
query.prepare("SELECT * FROM patients WHERE id = :id AND is_active = :active");
query.bindValue(":id", patientId);
query.bindValue(":active", true);
query.exec();

// ❌ TUYỆT ĐỐI KHÔNG làm thế này
QString sql = "SELECT * FROM patients WHERE id = " + QString::number(id);
query.exec(sql);
```

### 5.2 Đặt tên bảng và cột SQLite

Dùng **lower_snake_case** cho tên bảng và cột.

```sql
CREATE TABLE patients (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    full_name    TEXT    NOT NULL,
    birth_date   TEXT,
    phone_number TEXT,
    is_active    INTEGER DEFAULT 1
);

CREATE TABLE appointments (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id  INTEGER NOT NULL,
    doctor_id   INTEGER NOT NULL,
    scheduled_at TEXT   NOT NULL,
    status      TEXT    DEFAULT 'pending',
    FOREIGN KEY (patient_id) REFERENCES patients(id),
    FOREIGN KEY (doctor_id)  REFERENCES doctors(id)
);
```

### 5.3 Xử lý lỗi database

Luôn kiểm tra kết quả truy vấn trước khi dùng dữ liệu.

```cpp
if (!query.exec()) {
    qCritical() << "Query failed:" << query.lastError().text();
    return false;
}

while (query.next()) {
    Patient p;
    p.setId(query.value("id").toInt());
    p.setFullName(query.value("full_name").toString());
    patients.append(p);
}
```

### 5.4 Đóng gói database trong một class riêng

Không viết SQL trực tiếp trong View hoặc Controller. Tất cả truy vấn phải thông qua lớp `DatabaseManager` hoặc Repository tương ứng.

```cpp
// ✅ Đúng
// PatientController.cpp
void PatientController::loadPatients() {
    m_patients = m_patientRepo->findAll();  // gọi qua repository
}

// ❌ Sai
// PatientForm.cpp (View không được viết SQL)
QSqlQuery q("SELECT * FROM patients");
```

---

## 6. Comments & Documentation

### 6.1 File header

Mỗi file `.h` và `.cpp` bắt đầu bằng comment mô tả.

```cpp
/**
 * @file    PatientModel.h
 * @brief   Model quản lý dữ liệu bệnh nhân (MVC pattern)
 * @author  [Tên thành viên]
 */
```

### 6.2 Comment cho hàm public

Dùng Doxygen-style cho tất cả hàm public trong header.

```cpp
/**
 * @brief Thêm một bệnh nhân mới vào cơ sở dữ liệu.
 * @param patient Đối tượng Patient cần lưu.
 * @return true nếu thêm thành công, false nếu có lỗi.
 */
bool addPatient(const Patient &patient);
```

### 6.3 Inline comment

Comment ngắn giải thích *tại sao*, không phải *cái gì*.

```cpp
// ✅ Tốt — giải thích lý do
// Qt yêu cầu model reset để view cập nhật đúng khi thay đổi cấu trúc
beginResetModel();
m_patients.clear();
endResetModel();

// ❌ Thừa — chỉ nhắc lại code
m_patients.clear();  // clear danh sách bệnh nhân
```

### 6.4 TODO / FIXME

Dùng tag chuẩn để dễ tìm kiếm trong project.

```cpp
// TODO: Thêm phân trang khi số bệnh nhân > 1000
// FIXME: Crash khi patient_id là null — cần xử lý trước khi release
// NOTE: Giữ nguyên logic này vì liên quan đến nghiệp vụ bảo hiểm
```

---

## 7. Git Workflow

### 7.1 Tên nhánh

```text
main              — nhánh chính, luôn ổn định
develop           — nhánh tích hợp chính
feature/<tên>     — tính năng mới
bugfix/<tên>      — sửa lỗi
hotfix/<tên>      — sửa lỗi khẩn cấp trên main
```

**Ví dụ:**

```text
feature/patient-registration
feature/appointment-scheduling
bugfix/doctor-list-crash
```

### 7.2 Commit message

Theo chuẩn **Conventional Commits**:

```text
<type>: <mô tả ngắn gọn bằng tiếng Anh hoặc tiếng Việt>
```

| Type | Khi nào dùng |
| --- | --- |
| `feat` | Thêm tính năng mới |
| `fix` | Sửa lỗi |
| `refactor` | Refactor code, không thay đổi logic |
| `style` | Chỉnh format, naming (không ảnh hưởng logic) |
| `docs` | Cập nhật tài liệu |
| `test` | Thêm/sửa test |
| `chore` | Cấu hình build, dependencies |

**Ví dụ commit message:**

```text
feat: thêm chức năng tìm kiếm bệnh nhân theo tên
fix: sửa lỗi không lưu được ngày hẹn khi chọn từ DatePicker
refactor: tách DatabaseManager thành PatientRepo và DoctorRepo
docs: cập nhật README hướng dẫn cài đặt SQLite
```

---

## 8. Checklist trước khi tạo Pull Request

Trước khi tạo PR, tự kiểm tra các mục sau:

- [ ] Code biên dịch không có warning
- [ ] Tên biến, hàm, class tuân theo convention
- [ ] Không có SQL string nối trực tiếp (dùng prepared statement)
- [ ] Không có memory leak (QObject có parent hoặc đã được delete đúng)
- [ ] Đã comment các hàm public bằng Doxygen
- [ ] Commit message đúng format
- [ ] Không commit file `.pro.user`, build artifacts, hoặc `hospital.db`
- [ ] Đã test chức năng trên máy local trước khi push

---

*Tài liệu được duy trì bởi nhóm. Góp ý hoặc đề xuất thay đổi qua Issues trên repository.*
