---
title: "\U0001F3E5 TÀI LIỆU KIẾN TRÚC HỆ THỐNG"

---

# 🏥 TÀI LIỆU KIẾN TRÚC HỆ THỐNG
# Smart Hospital / Clinic Management System
### Dự án Lập trình Hướng Đối Tượng — Phân tích & Thiết kế Kiến trúc
> **Soạn thảo bởi:** Senior Solution Architect — C++ / Qt / OOP / Design Patterns  
> **Ngôn ngữ triển khai:** C++17 | Qt 6.x | SQLite (via Qt SQL Module)  
> **Phương pháp:** Kiến trúc phân tầng (Layered Architecture) + MVC-inspired  

---

# SECTION 1: PHÂN TÍCH & BRAINSTORM TÍNH NĂNG HỆ THỐNG

## 1.1 Tổng quan kiến trúc module

Hệ thống được chia thành **5 module lõi** hoạt động độc lập nhưng liên thông qua tầng dữ liệu (Data Layer) và tầng dịch vụ (Service Layer).

```
┌─────────────────────────────────────────────────────────┐
│              SMART HOSPITAL MANAGEMENT SYSTEM           │
├──────────┬───────────┬──────────┬────────────┬──────────┤
│  MODULE  │  MODULE   │  MODULE  │   MODULE   │  MODULE  │
│  NHÂN    │  BỆNH     │  XẾP     │   DƯỢC     │  BÁO     │
│  SỰ &    │  NHÂN &   │  HÀNG &  │   PHẨM &   │  CÁO &   │
│  PHÂN    │  HỒ SƠ    │  LỊCH    │   HÓA ĐƠN  │  PHÂN    │
│  QUYỀN   │  Y TẾ     │  HẸN     │            │  TÍCH    │
└──────────┴───────────┴──────────┴────────────┴──────────┘
           ↕            ↕           ↕              ↕
┌─────────────────────────────────────────────────────────┐
│         SERVICE LAYER — Business Logic (Pure C++)       │
├─────────────────────────────────────────────────────────┤
│              DATA LAYER — SQLite / Repository           │
└─────────────────────────────────────────────────────────┘
```

---

## 1.2 Module 1: Quản lý Nhân sự & Kiểm soát Truy cập theo Vai trò (RBAC)

### 🔹 Tính năng Cơ bản (CRUD — Điểm nền)

| Tính năng | Mô tả chi tiết |
|-----------|----------------|
| Thêm/Sửa/Xóa nhân viên | Thông tin đầy đủ: họ tên, ngày sinh, CCCD, chuyên khoa, số điện thoại |
| Quản lý vai trò | 4 vai trò: `ADMIN`, `DOCTOR`, `NURSE`, `RECEPTIONIST` |
| Tìm kiếm & Lọc | Theo tên, chuyên khoa, ca làm việc, trạng thái |
| Đăng nhập bảo mật | Xác thực username/password với mã hóa BCrypt-style |
| Xem danh sách | `QTableView` với phân trang, hiển thị đầy đủ thông tin |

### 🔸 Tính năng Nâng cao (Điểm Cộng — Vượt trội)

| Tính năng | Kỹ thuật OOP/Pattern | Điểm đặc biệt |
|-----------|----------------------|---------------|
| **Phân quyền động (Dynamic RBAC)** | Strategy Pattern — mỗi Role là một `PermissionStrategy` | Thêm vai trò mới không cần sửa code cũ |
| **Bảng chấm công thông minh** | Observer Pattern — khi bác sĩ thay đổi ca, lịch hẹn tự cập nhật | Đồng bộ toàn hệ thống |
| **Lịch sử đăng nhập** | Audit Log với thời gian, IP (giả lập), module truy cập | Bảo mật, kiểm toán |
| **Cây phân cấp bác sĩ** | `QTreeWidget` — Trưởng khoa → Bác sĩ → Thực tập sinh | Hiển thị trực quan hierarchy |
| **Dashboard bác sĩ cá nhân** | Hiển thị lịch hẹn hôm nay, số bệnh nhân, doanh thu cá nhân | Cá nhân hóa UX |

---

## 1.3 Module 2: Quản lý Bệnh nhân & Hồ sơ Y tế Điện tử (EMR)

### 🔹 Tính năng Cơ bản (CRUD)

| Tính năng | Mô tả chi tiết |
|-----------|----------------|
| Đăng ký bệnh nhân mới | Mã bệnh nhân tự sinh (UUID-style), thông tin dân số học |
| Hồ sơ bệnh án | Chẩn đoán, triệu chứng, kết quả xét nghiệm theo từng lần khám |
| Tìm kiếm bệnh nhân | Theo mã, tên, số điện thoại, CCCD |
| Xem lịch sử khám | Danh sách tất cả lần khám theo thứ tự thời gian |
| Cập nhật thông tin | Dị ứng thuốc, nhóm máu, tiền sử bệnh lý |

### 🔸 Tính năng Nâng cao

| Tính năng | Kỹ thuật OOP/Pattern | Điểm đặc biệt |
|-----------|----------------------|---------------|
| **Timeline Y tế Trực quan** | Custom `QWidget` vẽ bằng `QPainter` — mốc thời gian khám, nhập viện | Ấn tượng về UI/UX |
| **Cảnh báo Tương tác Thuốc** | Rule Engine đơn giản trong Business Logic — kiểm tra danh sách dị ứng với đơn thuốc | Logic nghiệp vụ phức tạp |
| **Phân loại bệnh nhân** | Inheritance: `OutPatient`, `InPatient`, `EmergencyPatient` kế thừa `Patient` | Minh họa Đa hình rõ ràng |
| **Tìm kiếm Full-text** | SQLite FTS5 hoặc LIKE query nâng cao | Hiệu năng truy vấn |
| **Xuất hồ sơ PDF** | Qt Print Support / QPdfWriter | Tính năng thực tế, gây ấn tượng |

---

## 1.4 Module 3: Xếp Hàng Thông Minh & Đặt Lịch Hẹn

### 🔹 Tính năng Cơ bản

| Tính năng | Mô tả chi tiết |
|-----------|----------------|
| Đặt lịch hẹn | Chọn bác sĩ, ngày, giờ; hệ thống kiểm tra xung đột |
| Xem lịch hẹn | `QCalendarWidget` + danh sách lịch hẹn theo ngày |
| Hủy / Dời lịch | Với lý do, thông báo tự động |
| Danh sách chờ hôm nay | Hiển thị số thứ tự, trạng thái (`WAITING`, `IN_ROOM`, `DONE`) |

### 🔸 Tính năng Nâng cao — **Đây là điểm nhấn kiến trúc**

| Tính năng | Kỹ thuật OOP/Pattern | Giải thích |
|-----------|----------------------|------------|
| **State Machine cho Trạng thái Hàng Đợi** | **State Pattern** — `QueueTicket` chuyển trạng thái: `REGISTERED → WAITING → CALLED → IN_PROGRESS → COMPLETED / CANCELLED` | Minh họa State Pattern hoàn chỉnh |
| **Ưu tiên hóa Thông Minh** | Thuật toán Priority Queue: Bệnh nhân cấp cứu → Cao tuổi (>70) → Đặt trước → Walk-in | Logic nghiệp vụ vượt CRUD |
| **Bảng Số Thứ Tự Thời gian thực** | Observer Pattern — khi số được gọi, màn hình chờ tự cập nhật | Đa hình qua Observer |
| **Phòng Khám Ảo (Room State)** | State Pattern cho `ExamRoom`: `AVAILABLE → OCCUPIED → CLEANING` | Mở rộng State Pattern |
| **Thống kê Thời gian Chờ** | Tính toán trung bình, phân phối theo giờ cao điểm | Phân tích dữ liệu |

---

## 1.5 Module 4: Kê Đơn, Quản lý Dược Phẩm & Hóa Đơn

### 🔹 Tính năng Cơ bản

| Tính năng | Mô tả chi tiết |
|-----------|----------------|
| Kê đơn thuốc | Bác sĩ tạo đơn liên kết với lần khám; chọn thuốc từ kho |
| Quản lý kho thuốc | Thêm/sửa/xóa thuốc, số lượng, giá, hạn sử dụng |
| Tạo hóa đơn | Tổng hợp phí khám + thuốc + dịch vụ |
| Lịch sử giao dịch | Xem hóa đơn theo bệnh nhân, theo ngày |

### 🔸 Tính năng Nâng cao

| Tính năng | Kỹ thuật OOP/Pattern | Giải thích |
|-----------|----------------------|------------|
| **Factory Pattern cho Hóa Đơn** | **Abstract Factory** — `InvoiceFactory::create()` trả về `OutPatientInvoice` hoặc `InPatientInvoice` | Minh họa Factory rõ ràng |
| **Cảnh báo Hết Hàng / Hết Hạn** | Observer — `InventoryService` thông báo khi tồn kho < ngưỡng | Kết hợp hai pattern |
| **Chiết khấu & Bảo hiểm** | Strategy Pattern — `PricingStrategy`: `StandardPricing`, `InsurancePricing`, `SeniorDiscount` | Strategy cho nghiệp vụ giá |
| **In / Xuất Hóa Đơn** | `QPrinter` + `QPrintDialog` | Tính thực tế cao |
| **Báo cáo Dược Phẩm** | Top thuốc bán chạy, doanh thu theo loại thuốc | Analytics Module |

---

## 1.6 Module 5: Dashboard Phân Tích & Báo Cáo

### 🔹 Tính năng Cơ bản

| Tính năng | Mô tả chi tiết |
|-----------|----------------|
| Tổng quan hệ thống | Số bệnh nhân hôm nay, doanh thu, số lịch hẹn |
| Báo cáo doanh thu | Theo ngày, tuần, tháng |
| Danh sách bác sĩ & hiệu suất | Số bệnh nhân đã khám |

### 🔸 Tính năng Nâng cao — **Điểm cộng lớn với QtCharts**

| Tính năng | Qt Widget | Giải thích |
|-----------|-----------|------------|
| **Biểu đồ Doanh thu** | `QBarSeries` + `QChart` | Doanh thu 7 ngày / 30 ngày |
| **Biểu đồ Tròn Chuyên Khoa** | `QPieSeries` | % bệnh nhân mỗi chuyên khoa |
| **Đường Thống kê Xu hướng** | `QLineSeries` | Xu hướng bệnh nhân theo tháng |
| **Bảng Hiệu suất Bác sĩ** | `QTableView` + custom delegate | Xếp hạng bác sĩ theo KPI |
| **Heatmap Giờ Cao Điểm** | Custom `QWidget` + `QPainter` | Giờ nào đông bệnh nhân nhất |

---

# SECTION 2: KIẾN TRÚC OOP & DESIGN PATTERNS

## 2.1 Kiến trúc Phân Tầng Tổng quan

```
┌──────────────────────────────────────────────────────────┐
│  PRESENTATION LAYER (View) — Qt Widgets, Qt GUI          │
│  LoginWindow, MainWindow, PatientView, DashboardView...  │
│  ⚠️  TUYỆT ĐỐI KHÔNG chứa Business Logic               │
│  ⚠️  Chỉ gọi Services; hiển thị kết quả                 │
├──────────────────────────────────────────────────────────┤
│  SERVICE LAYER (Controller/Application Logic)            │
│  PatientService, AppointmentService, BillingService...   │
│  ✅ Pure C++ — KHÔNG #include bất kỳ Qt GUI header       │
│  ✅ Chứa toàn bộ Business Rules, Validation              │
├──────────────────────────────────────────────────────────┤
│  REPOSITORY LAYER (Data Access)                          │
│  PatientRepository, AppointmentRepository...             │
│  ✅ Tách biệt hoàn toàn SQL queries khỏi Business Logic  │
├──────────────────────────────────────────────────────────┤
│  DATA LAYER — SQLite Database (via Qt SQL)               │
│  hospital.db — Toàn bộ dữ liệu persistent               │
└──────────────────────────────────────────────────────────┘
```

> **Quy tắc vàng về tầng View vs Business Logic:**  
> - File `*.h / *.cpp` trong thư mục `model/` và `service/` **KHÔNG ĐƯỢC** `#include <QWidget>`, `#include <QDialog>`, hay bất kỳ Qt GUI header.  
> - Chỉ được phép dùng `#include <QString>`, `#include <QList>`, `#include <QDateTime>` (Qt Core — không phải Qt Widgets).

---

## 2.1.1 🔮 Thiết kế Hướng Mở Rộng — Patient Portal trong Tương lai

> **Bối cảnh:** Phiên bản hiện tại (MVP) là **Staff-Only**: chỉ `ADMIN`, `DOCTOR`, `NURSE`, `RECEPTIONIST` đăng nhập và sử dụng app. Bệnh nhân chỉ là *dữ liệu* (model) được nhân viên quản lý.
>
> Tuy nhiên, kiến trúc được thiết kế theo **Open/Closed Principle** ngay từ đầu, để nếu nhóm còn thời gian, có thể bổ sung **"Patient Self-Service Portal"** (bệnh nhân tự đăng nhập, xem hồ sơ, đặt lịch hẹn) mà **KHÔNG cần refactor lại Service/Repository/Pattern đã viết** — chỉ cần *thêm* code mới.

Ba nguyên tắc thiết kế bắt buộc tuân thủ ngay từ Sprint 1 để đảm bảo điều này:

| # | Nguyên tắc | Áp dụng tại |
|---|-----------|-------------|
| 1 | Tách `IAuthenticatable` interface — không gắn cứng logic login vào `SystemUser` | `AuthService`, `model/i_authenticatable.h` |
| 2 | `MainWindow` xây sidebar **động** dựa trên `getMenuItems()` (đa hình) — không if/else theo `UserRole` | `ui/main_window.cpp` |
| 3 | `QStackedWidget` đăng ký page theo **module code (string)**, không hard-code 7 view cố định | `ui/main_window.cpp`, `ViewRegistry` |

> Chi phí áp dụng 3 nguyên tắc này ở Sprint 1 ≈ **0** (chỉ là cách viết interface/sidebar sạch hơn). Nhưng nếu bỏ qua, chi phí refactor sau này sẽ rất lớn (sửa lại `AuthService`, `MainWindow`, toàn bộ luồng điều hướng).

---

### 🔌 Interface `IAuthenticatable` — Điểm neo cho mọi loại tài khoản

```cpp
// File: model/i_authenticatable.h
// ⚠️ Interface thuần — Pure C++, không Qt GUI
#pragma once
#include <QString>
#include <vector>

enum class AccountType { STAFF, PATIENT }; // Mở rộng sau: PATIENT

class IAuthenticatable {
public:
    virtual ~IAuthenticatable() = default;

    // --- Định danh & xác thực ---
    virtual int      getAccountId()      const = 0;
    virtual QString  getUsername()       const = 0;
    virtual QString  getPasswordHash()   const = 0;
    virtual AccountType getAccountType() const = 0;

    // --- Đa hình điều khiển UI (dùng cho MainWindow) ---
    virtual std::vector<QString> getMenuItems()       const = 0;
    virtual bool canAccess(const QString& moduleCode) const = 0;
    virtual QString getDisplayRole()                  const = 0; // "Bác sĩ", "Bệnh nhân"...
};
```

> **Hiện tại (Sprint 1):** chỉ `SystemUser` implement `IAuthenticatable` (đã có sẵn `getMenuItems()`, `canAccess()` — chỉ cần thêm `getAccountId()`, `getAccountType()` trả về `STAFF`).
>
> **Tương lai (nếu mở rộng):** tạo thêm class `PatientAccount : public IAuthenticatable` (xem mục 2.1.2), `AuthService::login()` chỉ cần thêm 1 nhánh kiểm tra bảng `patients` — **không sửa logic login hiện có**.

### 🔌 `AuthService` — Thiết kế chấp nhận đa loại account

```cpp
// File: service/auth_service.h
class AuthService {
private:
    std::shared_ptr<StaffRepository>   m_staffRepo;
    std::shared_ptr<PatientRepository> m_patientRepo; // dùng ngay cả khi chưa có Patient login

public:
    // Trả về interface chung — KHÔNG trả về SystemUser* trực tiếp
    std::optional<std::shared_ptr<IAuthenticatable>> login(
        const QString& username, const QString& plainPassword);

    void logout();
    std::shared_ptr<IAuthenticatable> getCurrentAccount() const;
};
```

> Sprint 1: `login()` chỉ tìm trong `m_staffRepo`. Khi mở rộng, thêm `else if` tìm trong `m_patientRepo` — **chữ ký hàm, return type, MainWindow không đổi**.

---

## 2.2 Cây Kế thừa (Inheritance Hierarchy) — Minh họa Đa hình Động

### Phân cấp Nhân sự (Staff Hierarchy) — implement `IAuthenticatable`

```
                    ┌──────────────────────┐
                    │  <<interface>>       │
                    │  IAuthenticatable    │ ◄── Điểm neo mở rộng
                    │──────────────────────│      Patient Portal
                    │ +getUsername()  = 0  │      (xem mục 2.1.2)
                    │ +getMenuItems() = 0  │
                    │ +canAccess()    = 0  │
                    └──────────┬───────────┘
                                │ implements
                    ┌─────────────────┐
                    │  <<abstract>>   │
                    │   SystemUser    │
                    │─────────────────│
                    │ # id: int       │
                    │ # username: str │
                    │ # passwordHash  │
                    │ # role: Role    │
                    │─────────────────│
                    │ +login(): bool  │
                    │ +getRole()      │
                    │ +getMenuItems() │◄── virtual thuần (= 0)
                    │ +canAccess()    │◄── virtual thuần (= 0)
                    └────────┬────────┘
                             │ Kế thừa
              ┌──────────────┼──────────────────┐
              │              │                  │
    ┌──────────────┐  ┌──────────────┐  ┌───────────────────┐
    │    Doctor    │  │    Nurse     │  │   Receptionist    │
    │──────────────│  │──────────────│  │───────────────────│
    │ -specialty   │  │ -department  │  │ -shiftTime        │
    │ -licenseNo   │  │ -nurseLevel  │  │───────────────────│
    │──────────────│  │──────────────│  │+getMenuItems()✓   │
    │+prescribe()  │  │+takeVitals() │  │+canAccess()✓      │
    │+getMenuItems │  │+getMenuItems │  │+checkInPatient()  │
    │+canAccess()  │  │+canAccess()  │  └───────────────────┘
    └──────────────┘  └──────────────┘
```

> **Đa hình động được thể hiện tại đây:**  
> `std::vector<std::shared_ptr<SystemUser>> staff = ...;`  
> `for (auto& user : staff) { user->getMenuItems(); }` — Mỗi đối tượng gọi đúng phiên bản của mình tại runtime.

---

## 2.1.2 🔮 (Tương lai — KHÔNG code ở MVP) `PatientAccount` mở rộng `IAuthenticatable`

> Phần này **chỉ để minh họa điểm mở rộng**, không thuộc phạm vi Sprint 1–4 trừ khi nhóm còn dư thời gian.

```
                    ┌──────────────────────┐
                    │  <<interface>>       │
                    │  IAuthenticatable    │
                    └──────────┬───────────┘
                ┌───────────────┴────────────────┐
                │ (đã có)                         │ (tương lai)
      ┌─────────────────┐               ┌──────────────────────┐
      │   SystemUser     │               │   PatientAccount     │
      │  (Staff)         │               │  (Patient Portal)    │
      │──────────────────│               │───────────────────────│
      │ ...              │               │ -patientId: int       │
      │                  │               │ -patient: shared_ptr  │
      │                  │               │         <Patient>     │
      │                  │               │───────────────────────│
      │ +getMenuItems()  │               │ +getMenuItems()        │
      │  → 7 modules     │               │  → ["Hồ sơ của tôi",   │
      │  (theo Role)     │               │     "Lịch hẹn của tôi",│
      │                  │               │     "Đặt lịch mới",    │
      │                  │               │     "Hóa đơn của tôi"] │
      │ +canAccess()     │               │ +canAccess()           │
      │  → theo RBAC     │               │  → chỉ module của      │
      │    Strategy      │               │    chính mình          │
      └──────────────────┘               └──────────────────────┘
```

**Khi triển khai (nếu có thời gian), chỉ cần:**
1. Thêm cột `username`, `password_hash` vào bảng `patients` (migration nhỏ).
2. Tạo file mới `model/patient_account.h/.cpp` implement `IAuthenticatable` — **không sửa `Patient`, `PatientService` hiện có**, chỉ wrap thêm.
3. Thêm 1 nhánh trong `AuthService::login()` kiểm tra `PatientRepository::findByUsername()`.
4. Tạo `ui/views/patient_portal/` với 3-4 view mới (`MyProfileView`, `MyAppointmentsView`, `BookAppointmentView`) — **tái sử dụng** `AppointmentService`, `PatientService`, `BillingService` đã có sẵn.
5. `MainWindow` **không cần sửa gì** — vì sidebar đã build động từ `getMenuItems()` (xem mục 4.3).

---

### Phân cấp Bệnh nhân (Patient Hierarchy)

```
                    ┌─────────────────┐
                    │  <<abstract>>   │
                    │    Patient      │
                    │─────────────────│
                    │ # patientId     │
                    │ # fullName      │
                    │ # dateOfBirth   │
                    │ # bloodType     │
                    │─────────────────│
                    │+getPriority()   │◄── virtual thuần
                    │+getBillingType()│◄── virtual thuần
                    │+getStatusLabel()│◄── virtual thuần
                    └────────┬────────┘
              ┌──────────────┼──────────────┐
              │              │              │
    ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐
    │  OutPatient  │  │  InPatient   │  │EmergencyPatient │
    │──────────────│  │──────────────│  │─────────────────│
    │ (Ngoại trú)  │  │ (Nội trú)    │  │ (Cấp cứu)       │
    │──────────────│  │ -roomNo      │  │ -severity: int  │
    │+getPriority()│  │ -admitDate   │  │-────────────────│
    │  → NORMAL    │  │+getPriority()│  │+getPriority()   │
    │+getBilling() │  │  → INPATIENT │  │  → CRITICAL     │
    └──────────────┘  └──────────────┘  └─────────────────┘
```

### Phân cấp Hóa Đơn (Invoice Hierarchy) — Kết hợp Factory Pattern

```
                    ┌─────────────────┐
                    │  <<abstract>>   │
                    │    Invoice      │
                    │─────────────────│
                    │ # invoiceId     │
                    │ # patientId     │
                    │ # items: vector │
                    │─────────────────│
                    │+calculate()=0   │◄── virtual thuần
                    │+print()=0       │◄── virtual thuần
                    │+toJson()=0      │◄── virtual thuần
                    └────────┬────────┘
              ┌──────────────┴──────────────┐
              │                             │
    ┌──────────────────┐          ┌──────────────────────┐
    │OutPatientInvoice │          │ InPatientInvoice     │
    │──────────────────│          │──────────────────────│
    │ (Phí khám + thuốc│          │ (Phí giường + điều   │
    │  ngoại trú)      │          │  trị + phẫu thuật)   │
    │+calculate() ✓    │          │+calculate() ✓        │
    │+print() ✓        │          │+print() ✓            │
    └──────────────────┘          └──────────────────────┘
```

---

## 2.3 Design Patterns — Phân tích & Triển khai Chi tiết

### 🎯 PATTERN 1: State Pattern — Máy Trạng thái Hàng Đợi

**Bài toán:** Vé hàng đợi (`QueueTicket`) cần chuyển trạng thái theo quy trình, và mỗi trạng thái có hành vi khác nhau (có thể hủy, có thể gọi, có thể hoàn thành...).

**Sơ đồ trạng thái:**
```
REGISTERED → WAITING → CALLED → IN_PROGRESS → COMPLETED
                   ↘                        ↗
                    ──────→ CANCELLED ←──────
```

**Triển khai C++:**
```cpp
// --- Lớp giao diện trạng thái (Interface) ---
class IQueueState {
public:
    virtual ~IQueueState() = default;
    virtual void callNext(QueueTicket* ctx)    = 0;
    virtual void startExam(QueueTicket* ctx)   = 0;
    virtual void complete(QueueTicket* ctx)    = 0;
    virtual void cancel(QueueTicket* ctx)      = 0;
    virtual QString getStatusLabel()     const = 0;
    virtual bool canBeCancelled()        const = 0;
};

// --- Một trạng thái cụ thể ---
class WaitingState : public IQueueState {
public:
    void callNext(QueueTicket* ctx) override;    // Chuyển sang CalledState
    void cancel(QueueTicket* ctx) override;      // Chuyển sang CancelledState
    void startExam(QueueTicket*) override { /* Không hợp lệ */ }
    void complete(QueueTicket*) override  { /* Không hợp lệ */ }
    QString getStatusLabel() const override { return "Đang chờ"; }
    bool canBeCancelled() const override { return true; }
};

// --- Context: Vé Hàng Đợi ---
class QueueTicket {
private:
    std::unique_ptr<IQueueState> m_state;
    int m_ticketNumber;
    int m_patientId;
    QDateTime m_registeredAt;
public:
    void setState(std::unique_ptr<IQueueState> newState) {
        m_state = std::move(newState);
    }
    void callNext()   { m_state->callNext(this); }
    void startExam()  { m_state->startExam(this); }
    void complete()   { m_state->complete(this); }
    QString getStatusLabel() const { return m_state->getStatusLabel(); }
};
```

**Tại sao phù hợp?** Thêm trạng thái mới (ví dụ `RESCHEDULED`) chỉ cần tạo class mới, không đụng vào code cũ — tuân thủ **Open/Closed Principle**.

---

### 🎯 PATTERN 2: Observer Pattern — Đồng bộ Giao diện Thời gian thực

**Bài toán:** Khi bác sĩ gọi số tiếp theo từ màn hình của mình, **bảng số thứ tự ở phòng chờ** phải tự động cập nhật mà không cần refresh thủ công.

**Triển khai C++:**
```cpp
// --- Subject (Nguồn sự kiện) ---
class IQueueObserver {
public:
    virtual ~IQueueObserver() = default;
    virtual void onQueueUpdated(const std::vector<QueueTicket>& queue) = 0;
    virtual void onTicketCalled(int ticketNumber, const QString& patientName) = 0;
};

class QueueService {
private:
    std::vector<std::weak_ptr<IQueueObserver>> m_observers;
    // weak_ptr để tránh circular reference / memory leak
public:
    void subscribe(std::shared_ptr<IQueueObserver> obs) {
        m_observers.push_back(obs);
    }
    void callNextPatient() {
        // ... Business Logic: cập nhật state vé ...
        notifyObservers();
    }
private:
    void notifyObservers() {
        for (auto it = m_observers.begin(); it != m_observers.end(); ) {
            if (auto obs = it->lock()) {
                obs->onTicketCalled(m_currentTicket, m_currentPatientName);
                ++it;
            } else {
                it = m_observers.erase(it); // Dọn dẹp expired observers
            }
        }
    }
};

// --- Concrete Observer: Màn hình chờ Qt ---
// ⚠️ Class này nằm trong tầng View — được phép dùng Qt
class WaitingRoomDisplayWidget : public QWidget,
                                  public IQueueObserver {
    Q_OBJECT
public:
    void onTicketCalled(int num, const QString& name) override {
        m_numberLabel->setText(QString::number(num));
        m_nameLabel->setText(name);
        // Phát âm thanh thông báo (optional)
    }
};
```

---

### 🎯 PATTERN 3: Abstract Factory Pattern — Tạo Hóa Đơn Đa Loại

**Bài toán:** Tùy theo loại bệnh nhân (`OutPatient`, `InPatient`), hệ thống phải tạo đúng loại hóa đơn với cách tính phí khác nhau.

**Triển khai C++:**
```cpp
// --- Abstract Factory ---
class IInvoiceFactory {
public:
    virtual ~IInvoiceFactory() = default;
    virtual std::unique_ptr<Invoice> createInvoice(int patientId) = 0;
    virtual std::unique_ptr<Invoice> createInvoiceWithItems(
        int patientId,
        const std::vector<InvoiceItem>& items) = 0;
};

// --- Concrete Factory cho Ngoại trú ---
class OutPatientInvoiceFactory : public IInvoiceFactory {
public:
    std::unique_ptr<Invoice> createInvoice(int patientId) override {
        return std::make_unique<OutPatientInvoice>(patientId);
    }
    // ...
};

// --- Cách sử dụng trong BillingService (Pure C++) ---
class BillingService {
private:
    std::unique_ptr<IInvoiceFactory> selectFactory(PatientType type) {
        if (type == PatientType::INPATIENT)
            return std::make_unique<InPatientInvoiceFactory>();
        return std::make_unique<OutPatientInvoiceFactory>();
    }
public:
    std::unique_ptr<Invoice> generateInvoice(int patientId, PatientType type) {
        auto factory = selectFactory(type);
        return factory->createInvoice(patientId);
    }
};
```

### 🎯 PATTERN 4 (Bonus): Singleton Pattern — Database Connection Manager

```cpp
class DatabaseManager {
private:
    DatabaseManager() { /* Khởi tạo kết nối SQLite */ }
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    QSqlDatabase m_db;
    static std::once_flag s_onceFlag;
    static std::unique_ptr<DatabaseManager> s_instance;

public:
    static DatabaseManager& getInstance() {
        std::call_once(s_onceFlag, []() {
            s_instance = std::unique_ptr<DatabaseManager>(new DatabaseManager());
        });
        return *s_instance;
    }
    
    bool executeQuery(const QString& sql, const QVariantList& params = {});
    QSqlQuery selectQuery(const QString& sql, const QVariantList& params = {});
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
};
```

> **Tại sao dùng Singleton cho DB?** Đảm bảo toàn hệ thống chỉ có **một** kết nối SQLite, tránh lỗi concurrent write và resource leak.

---

# SECTION 3: ĐẶC TẢ LỚP & PHƯƠNG THỨC

## 3.1 Quy ước Quản lý Bộ nhớ

> **Chính sách Memory Management của dự án:**
> - **`std::unique_ptr<T>`**: Sở hữu độc quyền — dùng khi một đối tượng chỉ có một chủ sở hữu.
> - **`std::shared_ptr<T>`**: Sở hữu chung — dùng khi nhiều component cùng trỏ đến một đối tượng (ví dụ Observer).
> - **`std::weak_ptr<T>`**: Tham chiếu yếu — dùng trong Observer để tránh circular reference.
> - **TUYỆT ĐỐI CẤM** dùng `new`/`delete` raw pointer trong code mới.
> - Dùng `std::make_unique<>()` và `std::make_shared<>()` thay vì `new`.

---

## 3.2 Lớp `SystemUser` (Abstract Base Class) — implement `IAuthenticatable`

```cpp
// File: model/system_user.h
#pragma once
#include "i_authenticatable.h"   // 🔌 Điểm neo mở rộng Patient Portal
#include <QString>
#include <QDateTime>
#include <vector>
#include <memory>

enum class UserRole { ADMIN, DOCTOR, NURSE, RECEPTIONIST };

class SystemUser : public IAuthenticatable {
protected:
    int         m_id;
    QString     m_username;
    QString     m_passwordHash;   // Lưu hash, không lưu plain text
    QString     m_fullName;
    UserRole    m_role;
    bool        m_isActive;
    QDateTime   m_createdAt;

public:
    explicit SystemUser(int id, const QString& username,
                        const QString& fullName, UserRole role);
    virtual ~SystemUser() = default;

    // --- Getters ---
    QString         getFullName()    const { return m_fullName; }
    UserRole        getRole()        const { return m_role; }
    bool            isActive()       const { return m_isActive; }

    // --- Setters ---
    void setActive(bool active) { m_isActive = active; }
    void setFullName(const QString& name) { m_fullName = name; }

    // --- Business Logic ---
    bool verifyPassword(const QString& plainPassword) const;
    void setPassword(const QString& plainPassword);

    // --- Implement IAuthenticatable (chung cho mọi loại account) ---
    int         getAccountId()    const override { return m_id; }
    QString     getUsername()     const override { return m_username; }
    QString     getPasswordHash() const override { return m_passwordHash; }
    AccountType getAccountType()  const override { return AccountType::STAFF; }

    // --- Pure Virtual (Buộc override ở subclass — Doctor/Nurse/Receptionist) ---
    std::vector<QString> getMenuItems()                const override = 0;
    bool canAccess(const QString& moduleCode)          const override = 0;
    QString getDisplayRole()                           const override = 0;
};
```

> **Lưu ý mở rộng:** `getAccountType()` trả về `STAFF` được implement ngay tại `SystemUser` (không phải subclass) vì *mọi* nhân viên đều là STAFF. Sau này `PatientAccount` chỉ cần trả về `PATIENT` — `AuthService` dùng `getAccountType()` để biết log vào bảng nào, route vào `MainWindow` mode nào.

---

## 3.3 Lớp `Doctor` (Concrete Class)

```cpp
// File: model/doctor.h
#pragma once
#include "system_user.h"

class Doctor : public SystemUser {
private:
    QString m_specialty;        // Chuyên khoa
    QString m_licenseNumber;    // Số chứng chỉ hành nghề
    int     m_experienceYears;
    double  m_consultationFee;  // Phí khám mặc định

public:
    Doctor(int id, const QString& username, const QString& fullName,
           const QString& specialty, const QString& licenseNo);

    // --- Getters ---
    QString getSpecialty()       const { return m_specialty; }
    QString getLicenseNumber()   const { return m_licenseNumber; }
    double  getConsultationFee() const { return m_consultationFee; }

    // --- Override từ SystemUser / IAuthenticatable (Đa hình) ---
    std::vector<QString> getMenuItems()           const override;
    bool canAccess(const QString& moduleCode)     const override;
    QString getDisplayRole()                      const override { return "Bác sĩ"; }

    // --- Hành vi đặc thù của Doctor ---
    bool prescribeMedication(int patientId, const std::vector<PrescriptionItem>& items);
    bool createMedicalRecord(int patientId, const MedicalRecord& record);
};
```

---

## 3.4 Lớp `Patient` (Abstract)

```cpp
// File: model/patient.h
#pragma once
#include <QString>
#include <QDate>
#include <vector>

enum class PatientType   { OUTPATIENT, INPATIENT, EMERGENCY };
enum class PatientPriority { LOW, NORMAL, HIGH, CRITICAL };

class Patient {
protected:
    int         m_patientId;
    QString     m_patientCode;     // Mã bệnh nhân (VD: BN-2024-0001)
    QString     m_fullName;
    QDate       m_dateOfBirth;
    QString     m_gender;
    QString     m_phone;
    QString     m_address;
    QString     m_bloodType;
    QString     m_allergies;       // Danh sách dị ứng, phân cách bởi ","
    PatientType m_type;

public:
    explicit Patient(const QString& fullName, const QDate& dob, PatientType type);
    virtual ~Patient() = default;

    // --- Getters phổ biến ---
    int         getPatientId()   const { return m_patientId; }
    QString     getPatientCode() const { return m_patientCode; }
    QString     getFullName()    const { return m_fullName; }
    QDate       getDateOfBirth() const { return m_dateOfBirth; }
    int         getAge()         const;
    QString     getAllergies()   const { return m_allergies; }
    PatientType getType()        const { return m_type; }

    // --- Pure Virtual (Đa hình) ---
    virtual PatientPriority getPriority()       const = 0;
    virtual QString         getBillingType()    const = 0;
    virtual QString         getStatusLabel()    const = 0;
    virtual double          getBaseFee()        const = 0;

    // --- Utility ---
    bool hasAllergy(const QString& medicationName) const;
    static QString generatePatientCode(int id);
};
```

---

## 3.5 Lớp `PatientService` (Service Layer — Pure C++)

```cpp
// File: service/patient_service.h
// ⚠️ KHÔNG được #include bất kỳ Qt Widget header
#pragma once
#include <QString>
#include <vector>
#include <memory>
#include <optional>

class Patient;          // Forward declaration
class PatientRepository; // Forward declaration

struct PatientSearchCriteria {
    QString nameKeyword;
    QString phoneNumber;
    QString patientCode;
    int     minAge = -1;
    int     maxAge = -1;
    QString bloodType;
};

class PatientService {
private:
    std::shared_ptr<PatientRepository> m_repository;

public:
    explicit PatientService(std::shared_ptr<PatientRepository> repo);

    // --- CRUD Operations ---
    std::shared_ptr<Patient>              registerNewPatient(
                                            const QString& fullName,
                                            const QDate& dob,
                                            const QString& phone,
                                            PatientType type);

    bool                                  updatePatientInfo(
                                            int patientId,
                                            const QString& phone,
                                            const QString& address,
                                            const QString& allergies);

    bool                                  deactivatePatient(int patientId);
    std::optional<std::shared_ptr<Patient>> findById(int patientId) const;

    // --- Search ---
    std::vector<std::shared_ptr<Patient>> searchPatients(
                                            const PatientSearchCriteria& criteria) const;

    // --- Business Logic (Vượt CRUD) ---
    bool checkDrugAllergyConflict(int patientId, const QString& drugName) const;
    std::vector<MedicalRecord> getMedicalHistory(int patientId) const;
    PatientStatistics generatePatientStats(const QDate& from, const QDate& to) const;

    // --- Validation ---
    bool isPhoneNumberValid(const QString& phone) const;
    bool isPatientEligibleForInsurance(int patientId) const;
};
```

---

## 3.6 Lớp `QueueService` với Observer Pattern

```cpp
// File: service/queue_service.h
#pragma once
#include "model/queue_ticket.h"
#include <vector>
#include <memory>
#include <functional>

class IQueueObserver {
public:
    virtual ~IQueueObserver() = default;
    virtual void onQueueUpdated(const std::vector<QueueTicket>& allTickets) = 0;
    virtual void onTicketCalled(int ticketNumber, const QString& patientName, int roomNumber) = 0;
};

class QueueService {
private:
    std::vector<QueueTicket>                        m_tickets;
    std::vector<std::weak_ptr<IQueueObserver>>      m_observers;  // weak_ptr!
    int                                              m_nextTicketNumber = 1;

    // Priority Calculation (Business Logic)
    int calculatePriority(const QueueTicket& ticket) const;
    void sortByPriority();
    void notifyAll();

public:
    // --- Observer Management ---
    void subscribe(std::shared_ptr<IQueueObserver> observer);
    void unsubscribe(const std::shared_ptr<IQueueObserver>& observer);

    // --- Queue Operations ---
    QueueTicket registerToQueue(int patientId, int doctorId, bool isEmergency = false);
    bool        callNextPatient(int roomNumber);
    bool        startExamination(int ticketId);
    bool        completeExamination(int ticketId);
    bool        cancelTicket(int ticketId, const QString& reason);

    // --- Queries ---
    std::vector<QueueTicket> getActiveQueue()                     const;
    std::vector<QueueTicket> getQueueForDoctor(int doctorId)     const;
    int                      getEstimatedWaitTime(int patientId) const; // phút
    QueueStatistics          getTodayStatistics()                const;
};
```

---

## 3.7 Lớp `MedicalRecord`

```cpp
// File: model/medical_record.h
#pragma once
#include <QString>
#include <QDateTime>
#include <vector>

struct Diagnosis {
    QString icdCode;        // Mã ICD-10
    QString description;
    QString severity;       // mild / moderate / severe
};

struct VitalSigns {
    double  temperature;    // °C
    int     bloodPressureSystolic;
    int     bloodPressureDiastolic;
    int     heartRate;      // bpm
    double  weight;         // kg
    double  height;         // cm
};

class MedicalRecord {
private:
    int             m_recordId;
    int             m_patientId;
    int             m_doctorId;
    QDateTime       m_visitDateTime;
    VitalSigns      m_vitals;
    QString         m_chiefComplaint;    // Lý do đến khám
    QString         m_clinicalNotes;     // Ghi chú lâm sàng
    std::vector<Diagnosis> m_diagnoses;
    QString         m_treatment;
    QDateTime       m_nextVisitDate;

public:
    explicit MedicalRecord(int patientId, int doctorId);

    // --- Getters ---
    int           getRecordId()     const { return m_recordId; }
    int           getPatientId()    const { return m_patientId; }
    QDateTime     getVisitDateTime()const { return m_visitDateTime; }
    VitalSigns    getVitals()       const { return m_vitals; }
    QString       getClinicalNotes()const { return m_clinicalNotes; }

    // --- Setters ---
    void setVitals(const VitalSigns& vitals)    { m_vitals = vitals; }
    void setClinicalNotes(const QString& notes) { m_clinicalNotes = notes; }
    void addDiagnosis(const Diagnosis& diag)    { m_diagnoses.push_back(diag); }

    // --- Business ---
    bool isComplete() const; // Kiểm tra đủ thông tin bắt buộc
    double calculateBMI() const;
    QString generateSummary() const;
};
```

---

# SECTION 4: THIẾT KẾ GIAO DIỆN GUI & CÁC Qt WIDGET

## 4.1 Luồng Điều hướng Màn hình

```
          ┌───────────────────┐
          │    LoginWindow    │
          │ (QDialog modal)   │
          └────────┬──────────┘
                   │ Đăng nhập thành công
                   ▼
          ┌───────────────────┐
          │   MainWindow      │
          │ (QMainWindow)     │
          │ ┌───────────────┐ │
          │ │ QStackedWidget│ │ ◄── Chuyển trang không tải lại
          │ │               │ │
          │ │  ┌──────────┐ │ │
          │ │  │Dashboard │ │ │
          │ │  ├──────────┤ │ │
          │ │  │Patients  │ │ │
          │ │  ├──────────┤ │ │
          │ │  │Queue/Appt│ │ │
          │ │  ├──────────┤ │ │
          │ │  │Pharmacy  │ │ │
          │ │  ├──────────┤ │ │
          │ │  │Staff     │ │ │
          │ │  ├──────────┤ │ │
          │ │  │Analytics │ │ │
          │ │  └──────────┘ │ │
          │ └───────────────┘ │
          │  Sidebar: QListWidget hoặc custom nav  │
          └───────────────────┘
```

---

## 4.2 Màn hình Đăng Nhập (`LoginWindow`)

```
╔════════════════════════════════════════╗
║     🏥 SMART CLINIC MANAGEMENT        ║
║        Hệ thống Quản lý Phòng Khám    ║
╠════════════════════════════════════════╣
║                                        ║
║    [Logo / Icon]                       ║
║                                        ║
║    Tên đăng nhập:                      ║
║    ┌──────────────────────────────┐    ║
║    │ QLineEdit                    │    ║
║    └──────────────────────────────┘    ║
║                                        ║
║    Mật khẩu:                           ║
║    ┌──────────────────────────────┐    ║
║    │ QLineEdit (echoMode=Password)│    ║
║    └──────────────────────────────┘    ║
║                                        ║
║         [ Đăng Nhập ]                  ║
║         QPushButton (Primary)          ║
║                                        ║
║    ⚠️ QLabel (hiện lỗi nếu sai)        ║
╚════════════════════════════════════════╝
```

**Qt Components:**
- `QDialog` với `Qt::FramelessWindowHint` (hiện đại, không viền)
- `QLineEdit` với `setEchoMode(QLineEdit::Password)` cho mật khẩu
- Validation: `QMessageBox::warning()` khi sai tài khoản
- `QGraphicsDropShadowEffect` cho hiệu ứng bóng đổ

---

## 4.3 Màn hình Chính (`MainWindow`) — Sidebar Động theo `getMenuItems()`

```
╔═══════════════════════════════════════════════════════════════╗
║ 🏥 Smart Clinic │ Xin chào, BS. Nguyễn An │ [🔔] [⚙️] [Exit]║
╠═══════════════╦═══════════════════════════════════════════════╣
║               ║                                               ║
║  [🏠] Dashboard║         CONTENT AREA                         ║
║  [👥] Bệnh nhân║         QStackedWidget                       ║
║  [📅] Lịch hẹn ║         (Nội dung thay đổi theo menu)        ║
║  [💊] Dược phẩm║                                               ║
║  [🧾] Hóa đơn  ║                                               ║
║  [👨‍⚕️] Nhân sự ║                                               ║
║  [📊] Báo cáo  ║                                               ║
║               ║                                               ║
║  ─────────────║                                               ║
║  [🚪] Đăng xuất║                                               ║
╚═══════════════╩═══════════════════════════════════════════════╝

Sidebar: QListWidget hoặc custom QPushButton vertical layout
```

**Qt Components:**
- `QMainWindow` làm khung chính
- `QWidget` sidebar bên trái (width: 200px cố định)
- `QStackedWidget` bên phải làm vùng nội dung chính
- `QStatusBar` ở dưới hiển thị thông tin thời gian thực
- `QSystemTrayIcon` (optional — thông báo background)

---

### 🔌 Thiết kế Mở rộng: Sidebar Đa hình + `ViewRegistry`

> **Nguyên tắc cốt lõi:** `MainWindow` **KHÔNG BIẾT** và **KHÔNG QUAN TÂM** người đăng nhập là `Doctor`, `Nurse`, `Receptionist`, hay (tương lai) `PatientAccount`. Nó chỉ thao tác qua interface `IAuthenticatable`.

**Bước 1 — Sau khi login thành công, `LoginWindow` chuyển giao `IAuthenticatable`:**

```cpp
// File: ui/main_window.h
class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    std::shared_ptr<IAuthenticatable> m_currentAccount; // KHÔNG phải SystemUser*
    QStackedWidget* m_contentArea;
    QWidget*        m_sidebar;
    ViewRegistry*   m_viewRegistry; // xem Bước 3

public:
    explicit MainWindow(std::shared_ptr<IAuthenticatable> account, QWidget* parent = nullptr);

private:
    void buildSidebar();   // ⚠️ KHÔNG if/else theo UserRole
    void onMenuItemClicked(const QString& moduleCode);
};
```

**Bước 2 — `buildSidebar()` loop qua `getMenuItems()` (đa hình tại runtime):**

```cpp
// File: ui/main_window.cpp
void MainWindow::buildSidebar() {
    // Đa hình: Doctor/Nurse/Receptionist (và tương lai PatientAccount)
    // mỗi loại trả về danh sách module code khác nhau
    std::vector<QString> menuItems = m_currentAccount->getMenuItems();

    for (const QString& moduleCode : menuItems) {
        QPushButton* btn = new QPushButton(ViewRegistry::displayName(moduleCode));
        connect(btn, &QPushButton::clicked, this, [this, moduleCode]() {
            onMenuItemClicked(moduleCode);
        });
        m_sidebarLayout->addWidget(btn);
    }
}

void MainWindow::onMenuItemClicked(const QString& moduleCode) {
    if (!m_currentAccount->canAccess(moduleCode)) {
        QMessageBox::warning(this, "Từ chối truy cập", "Bạn không có quyền vào module này.");
        return;
    }
    m_contentArea->setCurrentWidget(m_viewRegistry->getOrCreateView(moduleCode));
}
```

> Ví dụ giá trị trả về của `getMenuItems()`:
> - `Doctor::getMenuItems()` → `{"dashboard", "patients", "queue", "appointments", "pharmacy", "billing", "analytics"}`
> - `Receptionist::getMenuItems()` → `{"dashboard", "patients", "queue", "appointments"}`
> - *(Tương lai)* `PatientAccount::getMenuItems()` → `{"my_profile", "my_appointments", "book_appointment", "my_invoices"}`

**Bước 3 — `ViewRegistry`: đăng ký view theo module code (string), không hard-code 7 view cố định:**

```cpp
// File: ui/view_registry.h
// Map module code → factory function tạo QWidget* tương ứng
class ViewRegistry {
private:
    std::unordered_map<QString, std::function<QWidget*()>> m_factories;
    std::unordered_map<QString, QWidget*> m_cache; // lazy-load, tránh tạo lại
    QStackedWidget* m_stack;

public:
    void registerView(const QString& moduleCode, std::function<QWidget*()> factory) {
        m_factories[moduleCode] = factory;
    }

    QWidget* getOrCreateView(const QString& moduleCode) {
        if (m_cache.contains(moduleCode)) return m_cache[moduleCode];
        QWidget* view = m_factories[moduleCode]();
        m_stack->addWidget(view);
        m_cache[moduleCode] = view;
        return view;
    }

    static QString displayName(const QString& moduleCode); // "patients" → "Bệnh nhân"
};

// Đăng ký lúc khởi tạo app (main.cpp hoặc MainWindow constructor):
registry->registerView("dashboard",   []{ return new DashboardView(); });
registry->registerView("patients",    []{ return new PatientView(patientService); });
registry->registerView("queue",       []{ return new QueueView(queueService); });
// ... Tương lai chỉ cần thêm dòng:
// registry->registerView("my_appointments", []{ return new MyAppointmentsView(appointmentService); });
```

> **Tại sao thiết kế này quan trọng cho mở rộng?**
> Khi thêm Patient Portal, bạn **chỉ cần**:
> 1. Tạo `PatientAccount::getMenuItems()` trả về module code mới (`"my_appointments"`...)
> 2. Tạo các view mới (`MyAppointmentsView`...) và `registerView()` cho chúng
>
> → **Không sửa một dòng nào** trong `MainWindow::buildSidebar()`, `onMenuItemClicked()`, hay `ViewRegistry` — đúng tinh thần **Open/Closed Principle**, và là câu trả lời "ăn điểm" khi giảng viên hỏi về khả năng mở rộng hệ thống.

---

## 4.4 Module Bệnh nhân — Giao diện

```
╔═══════════════════════════════════════════════════════════════╗
║  QUẢN LÝ BỆNH NHÂN                        [+ Thêm Bệnh Nhân] ║
╠══════════════════════════════════════════════════════════════╣
║  Tìm kiếm: [_______________] Lọc: [Tất cả ▼] [Tìm kiếm]     ║
╠══════════════════════════════════════════════════════════════╣
║  Mã BN    │ Họ tên       │ Tuổi│ Điện thoại │ Loại │ Hành động║
║  ─────────┼──────────────┼─────┼────────────┼──────┼──────────║
║  BN-0001  │ Nguyễn Văn A │ 45  │ 090...     │ Ngoại│ [Xem][Sửa]║
║  BN-0002  │ Trần Thị B   │ 32  │ 091...     │ Nội  │ [Xem][Sửa]║
║  ...      │              │     │            │      │          ║
╠══════════════════════════════════════════════════════════════╣
║  Tổng: 1,234 bệnh nhân    ◄ [1][2][3]...[10] ►               ║
╚═══════════════════════════════════════════════════════════════╝
```

**Qt Components:**
- `QTableView` + `QSortFilterProxyModel` (sắp xếp, lọc client-side)
- `QStyledItemDelegate` — tô màu hàng theo loại bệnh nhân
- `QDialog` modal cho form Thêm/Sửa bệnh nhân
- `QDateEdit` cho ngày sinh, `QComboBox` cho nhóm máu
- Phân trang: `QPushButton` left/right + `QLabel` trang hiện tại

---

## 4.5 Module Hàng Đợi — Giao diện Bảng Số Thứ Tự

```
╔═════════════════════════════════════════════════════════════╗
║  HÀNG ĐỢI HÔM NAY  │  10/06/2025  │  [+ Đăng ký Xếp Hàng] ║
╠══════════════════════╦══════════════════════════════════════╣
║  Đang được Khám      ║  Danh sách Đang chờ                   ║
║  ┌────────────────┐  ║  STT │ Bệnh nhân    │ Bác sĩ │ Giờ HH ║
║  │    SỐ 007      │  ║  ────┼──────────────┼────────┼────────║
║  │  Lê Văn Cường  │  ║  008 │ Phạm Thị D   │BS. An  │ ~10p   ║
║  │  Phòng 2       │  ║  009 │ Hoàng Văn E  │BS. An  │ ~20p   ║
║  └────────────────┘  ║  010 │ Vũ Thị F     │BS. Bình│ ~5p    ║
║                       ║  🚨  │ Nguyễn Cấp G │CẤP CỨU │ NGAY  ║
║  [Gọi Tiếp Theo]     ║                                       ║
║  [Hoàn thành]        ║                                       ║
╚══════════════════════╩═══════════════════════════════════════╝
```

---

## 4.6 Module Analytics — Dashboard Biểu đồ

```
╔══════════════════════════════════════════════════════════════╗
║  📊 TỔNG QUAN THỐNG KÊ              [Tuần này ▼] [Xuất PDF] ║
╠══════════════════════════════════════════════════════════════╣
║  ┌────────────┐ ┌────────────┐ ┌────────────┐ ┌───────────┐ ║
║  │ 127        │ │ 45.2 tr    │ │ 89%        │ │ 14.5 phút │ ║
║  │ Bệnh nhân  │ │ Doanh thu  │ │ Hài lòng   │ │ T.gian chờ│ ║
║  └────────────┘ └────────────┘ └────────────┘ └───────────┘ ║
╠════════════════════════════╦═════════════════════════════════╣
║  Doanh thu 7 ngày          ║  Phân bố Chuyên khoa           ║
║  QBarSeries (QtCharts)     ║  QPieSeries (QtCharts)         ║
║  ┌──────────────────────┐  ║  ┌─────────────────────────┐   ║
║  │  ▓▓ ▓▓▓ ▓ ▓▓ ▓▓▓ ▓▓ │  ║  │       🥧 Pie Chart      │   ║
║  │  T2 T3  T4 T5 T6 T7  │  ║  │  Nội: 35% │ Ngoại: 28% │   ║
║  └──────────────────────┘  ║  └─────────────────────────┘   ║
╠════════════════════════════╩═════════════════════════════════╣
║  Hiệu suất Bác sĩ (QTableView + Custom Delegate)            ║
║  BS. Nguyễn An │ 45 BN │ 12.5 tr │ ████████░░ 85%          ║
║  BS. Trần Bình  │ 38 BN │ 10.2 tr │ ███████░░░ 72%          ║
╚══════════════════════════════════════════════════════════════╝
```

**Qt Components cho Analytics:**
```cpp
// Cách tích hợp QtCharts
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>

// Ví dụ tạo Biểu đồ Doanh thu:
QBarSet* revenueSet = new QBarSet("Doanh thu (triệu VNĐ)");
*revenueSet << 8.5 << 12.3 << 7.8 << 14.2 << 11.0 << 9.5 << 15.1;

QBarSeries* series = new QBarSeries();
series->append(revenueSet);

QChart* chart = new QChart();
chart->addSeries(series);
chart->setTitle("Doanh thu 7 ngày gần nhất");
chart->setAnimationOptions(QChart::SeriesAnimations);

QChartView* chartView = new QChartView(chart);
chartView->setRenderHint(QPainter::Antialiasing);
```

---

## 4.7 Cấu trúc Thư mục Dự án

```
SmartClinicSystem/
├── CMakeLists.txt                    # Build system
├── hospital.db                       # SQLite DB (runtime)
│
├── src/
│   ├── main.cpp
│   │
│   ├── model/                        # Domain Models (Pure C++)
│   │   ├── i_authenticatable.h       # 🔌 Interface mở rộng (Patient Portal)
│   │   ├── system_user.h / .cpp
│   │   ├── doctor.h / .cpp
│   │   ├── nurse.h / .cpp
│   │   ├── receptionist.h / .cpp
│   │   ├── patient.h / .cpp
│   │   ├── outpatient.h / .cpp
│   │   ├── inpatient.h / .cpp
│   │   ├── emergency_patient.h / .cpp
│   │   ├── queue_ticket.h / .cpp
│   │   ├── medical_record.h / .cpp
│   │   ├── prescription.h / .cpp
│   │   ├── invoice.h / .cpp
│   │   └── medication.h / .cpp
│   │   └── (🔮 tương lai: patient_account.h / .cpp)
│   │
│   ├── state/                        # State Pattern
│   │   ├── i_queue_state.h
│   │   ├── registered_state.h / .cpp
│   │   ├── waiting_state.h / .cpp
│   │   ├── called_state.h / .cpp
│   │   ├── in_progress_state.h / .cpp
│   │   └── completed_state.h / .cpp
│   │
│   ├── factory/                      # Factory Pattern
│   │   ├── i_invoice_factory.h
│   │   ├── outpatient_invoice_factory.h / .cpp
│   │   └── inpatient_invoice_factory.h / .cpp
│   │
│   ├── service/                      # Business Logic (Pure C++)
│   │   ├── auth_service.h / .cpp
│   │   ├── patient_service.h / .cpp
│   │   ├── queue_service.h / .cpp    # + IQueueObserver
│   │   ├── appointment_service.h / .cpp
│   │   ├── billing_service.h / .cpp
│   │   ├── pharmacy_service.h / .cpp
│   │   └── analytics_service.h / .cpp
│   │
│   ├── repository/                   # Data Access Layer
│   │   ├── database_manager.h / .cpp # Singleton
│   │   ├── patient_repository.h / .cpp
│   │   ├── staff_repository.h / .cpp
│   │   ├── appointment_repository.h / .cpp
│   │   ├── medical_record_repository.h / .cpp
│   │   └── billing_repository.h / .cpp
│   │
│   └── ui/                           # Qt GUI Layer
│       ├── login_window.h / .cpp
│       ├── main_window.h / .cpp
│       ├── view_registry.h / .cpp    # 🔌 Đăng ký view theo module code
│       ├── views/
│       │   ├── dashboard_view.h / .cpp
│       │   ├── patient_view.h / .cpp
│       │   ├── queue_view.h / .cpp
│       │   ├── appointment_view.h / .cpp
│       │   ├── pharmacy_view.h / .cpp
│       │   ├── billing_view.h / .cpp
│       │   ├── staff_view.h / .cpp
│       │   └── analytics_view.h / .cpp
│       │   └── patient_portal/       # 🔮 (tương lai) my_appointments_view, book_appointment_view...
│       ├── dialogs/
│       │   ├── add_patient_dialog.h / .cpp
│       │   ├── medical_record_dialog.h / .cpp
│       │   ├── prescription_dialog.h / .cpp
│       │   └── invoice_dialog.h / .cpp
│       └── widgets/
│           ├── medical_timeline_widget.h / .cpp  # Custom QWidget
│           ├── queue_board_widget.h / .cpp       # Bảng số thứ tự
│           └── stat_card_widget.h / .cpp         # KPI Card
│
└── tests/                            # Unit Tests
    ├── test_patient_service.cpp
    ├── test_queue_service.cpp
    ├── test_billing_service.cpp
    └── test_state_machine.cpp
```

---

# SECTION 5: KẾ HOẠCH TRIỂN KHAI & PHÂN CÔNG SPRINT

## 5.1 Phân chia Vai trò Nhóm (6 Thành viên)

| # | Vai trò | Trách nhiệm Chính |
|---|---------|-------------------|
| M1 | **Tech Lead & Core OOP Architect** | Thiết kế toàn bộ class hierarchy, abstract classes, thiết lập project CMake, code review |
| M2 | **Core Logic Developer — Staff & Auth** | Module Nhân sự, RBAC, Đăng nhập, AuthService, StaffRepository |
| M3 | **Core Logic Developer — Patient & Queue** | Module Bệnh nhân, EMR, QueueService + State Pattern, PatientService |
| M4 | **Qt GUI Developer — Patient & Queue UI** | UI cho Patient Module, Queue Board Widget, Medical Timeline Widget |
| M5 | **Qt GUI Developer & Database Specialist** | Analytics View (QtCharts), Billing UI, SQLite Schema, tất cả Repository |
| M6 | **QA, Tester & Documentation Lead** | Test cases, Bug tracking, Technical Report, Slide, AI Usage Log tổng hợp |

---

## 5.2 SPRINT 1: Nền tảng Kiến trúc & Database (Tuần 1–2)

| Task ID | Tên Task | Thành viên | Chi tiết Hành động | Deliverable & Git Commit |
|---------|----------|------------|---------------------|--------------------------|
| S1-T1 | Khởi tạo Project & CMakeLists | M1 | Tạo project Qt6, cấu hình `CMakeLists.txt` với Qt6::Widgets, Qt6::Sql, Qt6::Charts; thiết lập cấu trúc thư mục | `feat: initialize Qt6 project with CMake structure` |
| S1-T2 | Thiết kế Schema SQLite | M5 | Tạo file `schema.sql` với 8 bảng: `users`, `patients`, `medical_records`, `appointments`, `queue_tickets`, `medications`, `prescriptions`, `invoices` | `feat: design and create SQLite database schema` |
| S1-T3 | `DatabaseManager` Singleton | M5 | Implement Singleton thread-safe với `std::call_once`; các method `executeQuery()`, `selectQuery()`, `beginTransaction()` | `feat: implement DatabaseManager singleton with Qt SQL` |
| S1-T4 | Abstract Class `SystemUser` & Hierarchy + `IAuthenticatable` | M1 | Code `i_authenticatable.h` (interface mở rộng); `system_user.h/cpp` implement interface; `doctor.h/cpp`, `nurse.h/cpp`, `receptionist.h/cpp`; implement pure virtual functions | `feat: implement IAuthenticatable interface and SystemUser hierarchy` |
| S1-T5 | Abstract Class `Patient` & Hierarchy | M3 | Code `patient.h/cpp`, `outpatient.h`, `inpatient.h`, `emergency_patient.h`; implement `getPriority()`, `getBillingType()` | `feat: implement Patient class hierarchy with polymorphism` |
| S1-T6 | Abstract Class `Invoice` & Factory | M1+M5 | Code `invoice.h`, `outpatient_invoice.h`, `i_invoice_factory.h`, hai concrete factory; Factory Pattern | `feat: implement Invoice factory pattern with abstract base` |
| S1-T7 | State Machine `QueueTicket` | M3 | Code `IQueueState`, 5 concrete states (`RegisteredState`...), `QueueTicket` context class | `feat: implement State Pattern for queue ticket lifecycle` |
| S1-T8 | `LoginWindow` + `MainWindow` Khung Mở rộng | M4 | Tạo `login_window.h/cpp` với `QDialog`, validation, kết nối signal/slot với `AuthService`; tạo `main_window.h/cpp` với sidebar **động** từ `getMenuItems()` và `view_registry.h/cpp` | `feat: create login window and extensible MainWindow with dynamic sidebar via ViewRegistry` |

**AI Usage Log Sprint 1 (Ví dụ cho M1):**
| Task | Công cụ AI | Prompt đã dùng | Kiểm tra & Chỉnh sửa |
|------|-----------|---------------|----------------------|
| S1-T4 | ChatGPT | "Show me C++ abstract class design for hospital staff with pure virtual methods" | Điều chỉnh tên method cho phù hợp context VN, thêm `getMenuItems()` theo yêu cầu RBAC |
| S1-T6 | GitHub Copilot | Gợi ý tự động khi gõ class tên | Kiểm tra pattern đúng Abstract Factory, bổ sung error handling |

---

## 5.3 SPRINT 2: Business Logic & Service Layer (Tuần 2–3)

| Task ID | Tên Task | Thành viên | Chi tiết Hành động | Deliverable & Git Commit |
|---------|----------|------------|---------------------|--------------------------|
| S2-T1 | `AuthService` — Đăng nhập & RBAC | M2 | Implement `login()`, `logout()`, `hashPassword()`, `verifyPassword()`, `getCurrentUser()`, kiểm tra quyền theo module | `feat: implement AuthService with password hashing and RBAC` |
| S2-T2 | `StaffRepository` & CRUD | M2 | Toàn bộ SQL queries cho nhân sự: INSERT, UPDATE, SELECT, soft-delete; bind parameters để tránh SQL injection | `feat: implement StaffRepository with parameterized queries` |
| S2-T3 | `PatientService` — Logic Nghiệp vụ | M3 | `registerNewPatient()`, `checkDrugAllergyConflict()`, `getMedicalHistory()`, `generatePatientCode()` | `feat: implement PatientService with business validation logic` |
| S2-T4 | `PatientRepository` & Full-text Search | M5 | SQL queries cho patient CRUD; implement tìm kiếm theo `PatientSearchCriteria`; SQLite LIKE query | `feat: implement PatientRepository with multi-criteria search` |
| S2-T5 | `QueueService` + Observer Pattern | M3 | Implement `subscribe()`/`notifyAll()` với `weak_ptr`; `callNextPatient()` với priority sort; `getEstimatedWaitTime()` | `feat: implement QueueService with Observer pattern and priority algorithm` |
| S2-T6 | `AppointmentService` — Chống Xung Đột | M2 | Logic kiểm tra xung đột lịch hẹn theo doctor+time slot; `isTimeSlotAvailable()` | `feat: implement AppointmentService with conflict detection` |
| S2-T7 | `BillingService` — Factory Integration | M5 | Tích hợp `IInvoiceFactory`; `generateInvoice()`, `calculateTotal()` với tax/discount; `PricingStrategy` | `feat: implement BillingService using InvoiceFactory and Strategy pattern` |
| S2-T8 | `PharmacyService` — Kiểm soát Kho | M5 | `updateStock()`, `checkLowStock()`, Observer thông báo hết hàng, kiểm tra hạn dùng | `feat: implement PharmacyService with inventory control and alerts` |
| S2-T9 | `AnalyticsService` — Tổng hợp Dữ liệu | M5 | SQL aggregate queries: doanh thu theo ngày, top bác sĩ, phân bố chuyên khoa; trả `QVariantMap` cho View | `feat: implement AnalyticsService with aggregated reporting queries` |
| S2-T10 | Unit Tests Sprint 2 | M6 | Viết test cases cho `PatientService`, `QueueService` (state transitions), `BillingService` (tính toán); log bug tìm thấy | `test: add unit tests for service layer` |

---

## 5.4 SPRINT 3: Qt GUI Implementation (Tuần 3–4)

| Task ID | Tên Task | Thành viên | Chi tiết Hành động | Deliverable & Git Commit |
|---------|----------|------------|---------------------|--------------------------|
| S3-T1 | `MainWindow` + Navigation Sidebar | M4 | `QMainWindow` layout; `QStackedWidget` content; custom sidebar với `QPushButton`; phân quyền menu theo role | `feat: implement MainWindow with role-based sidebar navigation` |
| S3-T2 | `PatientView` — Danh sách & Tìm kiếm | M4 | `QTableView` + custom `QSortFilterProxyModel`; `QLineEdit` search; Toolbar buttons; kết nối `PatientService` | `feat: implement PatientView with sortable table and search` |
| S3-T3 | `AddPatientDialog` — Form Nhập liệu | M4 | `QDialog` modal; validation real-time trên mỗi `QLineEdit`; `QDateEdit`, `QComboBox`; error highlight bằng stylesheet | `feat: implement patient form dialog with real-time validation` |
| S3-T4 | `MedicalTimelineWidget` — Custom Widget | M4 | Kế thừa `QWidget`, override `paintEvent()`, vẽ timeline bằng `QPainter`; click vào mốc hiển thị chi tiết | `feat: implement custom medical history timeline widget` |
| S3-T5 | `QueueView` + `QueueBoardWidget` | M4 | Danh sách đang chờ dạng `QListWidget` style; implement `IQueueObserver` để tự động update khi `QueueService::callNextPatient()` | `feat: implement real-time queue view with Observer pattern` |
| S3-T6 | `StaffView` & `AppointmentView` | M2 | CRUD staff trong `QTableView`; `QCalendarWidget` cho lịch hẹn; dialog xem chi tiết lịch | `feat: implement staff management and appointment calendar views` |
| S3-T7 | `PharmacyView` & `BillingView` | M5 | Kho thuốc dạng table; form kê đơn; `InvoiceDialog` preview hóa đơn; `QPrinter` in hóa đơn | `feat: implement pharmacy inventory and billing invoice views` |
| S3-T8 | `AnalyticsView` — QtCharts Dashboard | M5 | `QBarSeries` doanh thu, `QPieSeries` chuyên khoa, `QLineSeries` xu hướng; `QChartView` responsive; `StatCardWidget` | `feat: implement analytics dashboard with QtCharts visualizations` |
| S3-T9 | Stylesheet & UX Polish | M4+M2 | Viết `styles.qss` thống nhất toàn app; màu sắc theme y tế (xanh dương + trắng); responsive sizing | `style: apply consistent QSS stylesheet across all views` |
| S3-T10 | Integration Testing | M6 | Test luồng từ đầu đến cuối: Đăng nhập → Đăng ký BN → Xếp hàng → Khám → Kê đơn → Thanh toán; log bug | `test: integration test complete patient visit workflow` |

---

## 5.5 SPRINT 4: Kiểm thử, Báo cáo & Demo (Tuần 4–5)

| Task ID | Tên Task | Thành viên | Chi tiết Hành động | Deliverable & Git Commit |
|---------|----------|------------|---------------------|--------------------------|
| S4-T1 | Bug Fixing Toàn Diện | Tất cả | Dựa trên bug log từ M6; fix priority: crash → data corruption → UI glitch | `fix: resolve critical bugs from integration testing` |
| S4-T2 | Seeding Dữ liệu Demo | M5 | Tạo script SQL seed 50+ bệnh nhân, 5 bác sĩ, lịch sử khám 3 tháng; đảm bảo analytics có dữ liệu đẹp | `data: add demo seed data for presentation` |
| S4-T3 | Tối ưu Hiệu năng Query | M5 | Thêm INDEX cho các cột hay filter trong SQLite; đo query time; tối ưu N+1 query problem | `perf: add database indexes and optimize slow queries` |
| S4-T4 | UML Class Diagram | M1 | Vẽ UML đầy đủ bằng draw.io hoặc PlantUML; thể hiện rõ inheritance, associations, dependencies | Xuất file PNG/PDF vào `docs/` |
| S4-T5 | Báo cáo Kỹ thuật | M6 | Viết đầy đủ 7 section theo yêu cầu đề; mỗi thành viên tự viết phần của mình; M6 tổng hợp | File `[GroupID-Report].pdf` |
| S4-T6 | AI Usage Log Cá nhân | Tất cả | Mỗi người tổng hợp log AI sử dụng: tool, prompt, chỉnh sửa; format theo bảng đề yêu cầu | File `[GroupID-AIUsageLog].pdf` |
| S4-T7 | Slide Thuyết trình | M6+M1 | 15–20 slides: Intro → Problem → Architecture → OOP Demo → Patterns → Demo → Conclusion | File `[GroupID-Slide].pptx` |
| S4-T8 | Quay Video Demo | Tất cả | Mỗi người thuyết trình phần của mình; demo live app; giải thích UML và Pattern | Link video upload YouTube (unlisted) |
| S4-T9 | Code Review Cuối | M1 | Rà soát toàn bộ code: không còn raw pointer, không leak Qt header vào service layer, đặt tên nhất quán | `refactor: final code review and cleanup` |
| S4-T10 | Đóng gói & Nộp | M6 | Đóng ZIP theo format: SourceCode link, Report, Slide, Video, AIUsageLog, Contribution Evidence | Nộp đúng hạn |

---

## 5.5.1 🔮 SPRINT 5 (TÙY CHỌN — Nếu còn thời gian): Patient Self-Service Portal

> **Điều kiện thực hiện:** Chỉ bắt đầu khi Sprint 1–4 đã **hoàn thành và ổn định** (đặc biệt là Sprint 1-T4, T8 đã áp dụng đúng `IAuthenticatable` + `ViewRegistry`). Đây là phần "điểm cộng vượt yêu cầu", không bắt buộc.

| Task ID | Tên Task | Thành viên gợi ý | Chi tiết Hành động | Deliverable & Git Commit |
|---------|----------|------------------|---------------------|--------------------------|
| S5-T1 | Migration DB cho Patient Login | M5 | Thêm cột `username`, `password_hash` vào bảng `patients`; script migration không phá dữ liệu cũ | `feat: add patient login columns via migration script` |
| S5-T2 | `PatientAccount` Model | M1 hoặc M3 | Tạo `model/patient_account.h/.cpp` implement `IAuthenticatable`; wrap `shared_ptr<Patient>`; `getMenuItems()` trả về `{"my_profile","my_appointments","book_appointment","my_invoices"}` | `feat: implement PatientAccount as IAuthenticatable` |
| S5-T3 | Mở rộng `AuthService::login()` | M2 | Thêm nhánh: nếu không tìm thấy trong `StaffRepository`, tìm trong `PatientRepository::findByUsername()`; trả về `shared_ptr<IAuthenticatable>` tương ứng | `feat: extend AuthService to support patient accounts` |
| S5-T4 | `MyProfileView` & `MyAppointmentsView` | M4 | Tạo view mới trong `ui/views/patient_portal/`; `registerView()` vào `ViewRegistry`; tái sử dụng `PatientService::findById()`, `AppointmentService::getAppointmentsForPatient()` | `feat: implement patient profile and appointments view` |
| S5-T5 | `BookAppointmentView` | M3+M4 | UI chọn bác sĩ/ngày/giờ; gọi lại `AppointmentService::isTimeSlotAvailable()` và `registerToQueue()` đã có sẵn — **không sửa Service** | `feat: implement self-service appointment booking` |
| S5-T6 | Đăng ký Tài khoản Bệnh nhân (Receptionist hỗ trợ) | M2 | Receptionist tạo username/password cho bệnh nhân khi đăng ký lần đầu (vì bệnh nhân VN thường không tự đăng ký online) | `feat: allow receptionist to provision patient portal accounts` |
| S5-T7 | Test & Demo bổ sung | M6 | Test luồng: Bệnh nhân login → xem hồ sơ → đặt lịch → Receptionist/Doctor thấy lịch hẹn mới xuất hiện trong Queue | `test: add patient portal end-to-end test` |

> **Lưu ý khi trình bày Oral Defense:** Nếu hoàn thành Sprint 5, đây là **bằng chứng mạnh nhất** cho Open/Closed Principle — hãy chuẩn bị sẵn câu trả lời: *"Chúng em đã thêm tính năng X mà không sửa một dòng nào trong `MainWindow`/`AuthService` cũ, nhờ thiết kế interface `IAuthenticatable` và `ViewRegistry` từ Sprint 1."*

---

## 5.6 Phân bổ Tổng quan Theo Thành viên

| Thành viên | Sprint 1 | Sprint 2 | Sprint 3 | Sprint 4 | Git Commits ước tính |
|-----------|----------|----------|----------|----------|----------------------|
| **M1** — Tech Lead | T1, T4, T6 | Review | Review | T4, T9 | 30–40 commits |
| **M2** — Staff/Auth | T8 | T1, T2, T6 | T6, T9 | T1, T6 | 25–35 commits |
| **M3** — Patient/Queue | T5, T7 | T3, T5 | T5 | T1, T6 | 25–35 commits |
| **M4** — GUI Patient/Queue | T8 | Support | T1–T5, T9 | T1, T6 | 30–40 commits |
| **M5** — DB/Analytics/Billing | T2, T3, T6 | T4, T7–T9 | T7, T8 | T2, T3, T6 | 35–45 commits |
| **M6** — QA/Docs | Review | T10 | T10 | T5–T8, T10 | 15–20 commits + docs |

---

## 5.7 Checklist Điểm Tối đa — Oral Defense

> Mỗi thành viên phải sẵn sàng trả lời các câu hỏi sau về phần mình phụ trách:

**Về OOP (M1, M3):**
- [ ] Giải thích tại sao `SystemUser` phải là abstract class (không thể instantiate trực tiếp)
- [ ] Demo runtime polymorphism: gọi `getMenuItems()` trên `std::vector<std::shared_ptr<SystemUser>>`
- [ ] Giải thích sự khác biệt giữa `override` và `virtual` trong C++
- [ ] Tại sao destructor của `SystemUser` phải là `virtual`?

**Về Design Patterns (M1, M3, M5):**
- [ ] Vẽ sơ đồ State Machine của `QueueTicket` và giải thích từng chuyển đổi
- [ ] Giải thích tại sao dùng `weak_ptr` trong Observer thay vì `shared_ptr`
- [ ] Tại sao Factory Pattern giúp hệ thống mở rộng dễ hơn (Open/Closed Principle)?
- [ ] Singleton của `DatabaseManager` — tại sao thread-safe với `std::call_once`?

**Về Qt & GUI (M4, M2):**
- [ ] Giải thích cơ chế Signal/Slot của Qt khác gì với function pointer thuần?
- [ ] `QSortFilterProxyModel` hoạt động như thế nào?
- [ ] Tại sao Business Logic KHÔNG được nằm trong lớp Qt Widget?

**Về Database & Repository (M5):**
- [ ] Giải thích cách Repository Pattern tách biệt SQL khỏi Business Logic
- [ ] Tại sao dùng parameterized query (`bindValue`) thay vì nối chuỗi SQL?
- [ ] SQLite transaction hoạt động như thế nào trong trường hợp tạo hóa đơn?

**Về Khả năng Mở rộng (Tất cả — câu hỏi nâng cao):**
- [ ] Nếu muốn cho bệnh nhân tự đăng nhập đặt lịch (Patient Portal), cần sửa những gì trong code hiện tại?
- [ ] `IAuthenticatable` giải quyết vấn đề gì? Tại sao `MainWindow` không cần biết người dùng là `Doctor` hay `PatientAccount`?
- [ ] `ViewRegistry` giúp thêm module mới mà không sửa `MainWindow` như thế nào? Đây là ví dụ của nguyên tắc OOP nào (Open/Closed Principle)?

---

*Tài liệu này được soạn thảo để phục vụ mục đích học tập và phát triển dự án sinh viên.  
Tất cả AI-generated content phải được sinh viên xem xét, hiểu rõ và có khả năng giải thích trước giảng viên.*