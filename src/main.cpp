#include "dto/PatientDTOs.h"
#include "model/CommonEnums.h"
#include "repository/DatabaseManager.h"
#include "repository/PatientRepository.h"
#include "service/PatientService.h"
#include "ui/MainWindow.h"
#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTextStream>

// Log handler ghi ra file để debug (vì WIN32 app không có console)
static QFile logFile;

void messageHandler(QtMsgType type, const QMessageLogContext &ctx,
                    const QString &msg) {
  Q_UNUSED(ctx);
  if (!logFile.isOpen())
    return;

  QString prefix;
  switch (type) {
  case QtDebugMsg:
    prefix = "DEBUG";
    break;
  case QtWarningMsg:
    prefix = "WARN ";
    break;
  case QtCriticalMsg:
    prefix = "ERROR";
    break;
  case QtFatalMsg:
    prefix = "FATAL";
    break;
  default:
    prefix = "INFO ";
    break;
  }

  QTextStream out(&logFile);
  out << prefix << ": " << msg << "\n";
  out.flush();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tích hợp Code Test trực tiếp vào main
// ─────────────────────────────────────────────────────────────────────────────

// Helper: lấy patient_id của bản ghi vừa insert (MAX = mới nhất)
static int getLastInsertedPatientId() {
  QSqlQuery q = DatabaseManager::getInstance().selectQuery(
      "SELECT MAX(patient_id) FROM patients");
  if (q.next())
    return q.value(0).toInt();
  return -1;
}

static void runIntegrationTests() {
  DatabaseManager::getInstance().executeQuery(
      "INSERT OR IGNORE INTO departments (department_id, department_code, "
      "department_name) VALUES (1, 'D01', 'Test Dept')");
  DatabaseManager::getInstance().executeQuery(
      "INSERT OR IGNORE INTO rooms (room_id, room_number, room_type, status) "
      "VALUES (1, '101', 'WARD', 'AVAILABLE')");
  DatabaseManager::getInstance().executeQuery(
      "INSERT OR IGNORE INTO staff (staff_id, staff_code, password_hash, "
      "full_name, role, gender, date_of_birth, citizen_id, phone_number, "
      "email, address, department_id) VALUES (1, 'S01', 'hash', 'Dr. Test', "
      "'DOCTOR', 'MALE', '1980-01-01', '001234567890', '0901234567', "
      "'test@test.com', 'Address', 1)");

  qDebug() << "========================================";
  qDebug() << "  Running Patient Insert Tests in main()";
  qDebug() << "========================================";

  auto sharedRepo = std::make_shared<PatientRepository>();
  PatientService service(sharedRepo);

  // patient_id lưu lại để dùng cho Update / Delete test
  int idOut = -1, idIn = -1, idEmer = -1;

  // ── Test 1: AddOutPatient qua Service ──────────────────────────────────────
  {
    if (service.AddOutPatient(
            0, // patientId = 0 (DB tự tăng)
            0, // doctorId (0 = chưa gán)
            "Nguyen Van A", QDate(1990, 5, 15), "Male",
            "001012345678", // CCCD 12 số, mã tỉnh 001 (HN) hợp lệ
            "0987654321",   // SĐT di động hợp lệ
            "nguyenvana@test.com", "123 Le Loi, Ha Noi", "A+",
            "None",    // allergies
            "BaoViet", // insurance
            PatientType::OUTPATIENT,
            "Contact A",     // emergencyContactName
            "0912345678")) { // emergencyContactPhone
      idOut = getLastInsertedPatientId();
      qDebug() << "[PASS] AddOutPatient via Service (patient_id:" << idOut
               << ")";
    } else {
      qDebug() << "[FAIL] AddOutPatient via Service";
    }
  }

  // ── Test 2: AddInPatient qua Service ───────────────────────────────────────
  {
    if (service.AddInPatient(
            0, "Tran Thi B", QDate(1985, 11, 20), "Female",
            "079085012345", // CCCD mã tỉnh 079 (TP.HCM) hợp lệ
            "0901234567", "tranthib@test.com", "456 Nguyen Hue, HCM", "B+",
            "Penicillin", "Bao Minh", PatientType::INPATIENT, "Contact B",
            "0923456789",
            "1", // roomId = "1" (phòng đã tạo sẵn)
            "1", // doctorId = "1" (bác sĩ đã tạo sẵn)
            QDate::currentDate(), QDate::currentDate().addDays(5),
            "Abdominal pain")) {
      idIn = getLastInsertedPatientId();
      qDebug() << "[PASS] AddInPatient via Service (patient_id:" << idIn << ")";
    } else {
      qDebug() << "[FAIL] AddInPatient via Service";
    }
  }

  // ── Test 3: AddEmergencyPatient qua Service ────────────────────────────────
  {
    if (service.AddEmergencyPatient(
            0, "Le Van C", QDate(2000, 3, 8), "Male",
            "048000012345", // CCCD mã tỉnh 048 (Da Nang) hợp lệ
            "0978901234", "levanc@test.com", "789 Tran Phu, Da Nang", "O+",
            "None", "BHYT", PatientType::EMERGENCY, "Contact C", "0934567890",
            "1", // roomId
            "1", // doctorId
            "Traffic accident", "Multiple lacerations", QDate::currentDate(),
            QDate::currentDate().addDays(3))) {
      idEmer = getLastInsertedPatientId();
      qDebug() << "[PASS] AddEmergencyPatient via Service (patient_id:"
               << idEmer << ")";
    } else {
      qDebug() << "[FAIL] AddEmergencyPatient via Service";
    }
  }

  qDebug() << "========================================";
  qDebug() << "  Running Patient Update Tests in main()";
  qDebug() << "========================================";

  // ── Test Update OutPatient qua Service ─────────────────────────────────────
  if (idOut > 0) {
    if (service.UpdateOutPatient(idOut, 1, "Nguyen Van A Updated",
                                 QDate(1990, 5, 15), "Male", "001012345678",
                                 "0987654321", "nguyenvana@test.com",
                                 "123 Le Loi, Ha Noi", "A+", "None", "BaoViet",
                                 "Contact A", "0912345678", "TREATMENT")) {
      qDebug() << "[PASS] UpdateOutPatient via Service (patient_id:" << idOut
               << ")";
    } else {
      qDebug() << "[FAIL] UpdateOutPatient via Service";
    }
  } else {
    qDebug() << "[SKIP] UpdateOutPatient — Insert failed";
  }

  // ── Test Update InPatient qua Service ──────────────────────────────────────
  if (idIn > 0) {
    if (service.UpdateInPatient(
            idIn, "Tran Thi B Updated", QDate(1985, 11, 20), "Female",
            "079085012345", "0901234567", "tranthib@test.com",
            "456 Nguyen Hue, HCM", "B+", "Penicillin", "Bao Minh", "Contact B",
            "0923456789",
            "1", // roomId
            "1", // doctorId
            QDate::currentDate(), QDate::currentDate().addDays(7), "Cured",
            "DISCHARGED")) {
      qDebug() << "[PASS] UpdateInPatient via Service (patient_id:" << idIn
               << ")";
    } else {
      qDebug() << "[FAIL] UpdateInPatient via Service";
    }
  } else {
    qDebug() << "[SKIP] UpdateInPatient — Insert failed";
  }

  // ── Test Update EmergencyPatient qua Service ───────────────────────────────
  if (idEmer > 0) {
    if (service.UpdateEmergencyPatient(
            idEmer, "Le Van C Updated", QDate(2000, 3, 8), "Male",
            "048000012345", "0978901234", "levanc@test.com",
            "789 Tran Phu, Da Nang", "O+", "None", "BHYT", "Contact C",
            "0934567890", "1", "1", "Recovered", "Treated successfully",
            QDate::currentDate(), QDate::currentDate().addDays(3),
            "DISCHARGED")) {
      qDebug() << "[PASS] UpdateEmergencyPatient via Service (patient_id:"
               << idEmer << ")";
    } else {
      qDebug() << "[FAIL] UpdateEmergencyPatient via Service";
    }
  } else {
    qDebug() << "[SKIP] UpdateEmergencyPatient — Insert failed";
  }

  qDebug() << "========================================";
  qDebug() << "  Running Patient Search Tests in main()";
  qDebug() << "========================================";

  // ── Test Search 1: Tìm theo tên ────────────────────────────────────────────
  {
    PatientSearchCriteria criteria;
    criteria.searchKey = "Nguyen Van A";
    auto results = service.searchPatients(criteria);
    int count = service.countSearchResults(criteria);
    if (results.size() > 0 && count > 0) {
      qDebug() << "[PASS] Search by Name (found:" << results.size()
               << ", total:" << count << ")";
    } else {
      qDebug() << "[FAIL] Search by Name (expected > 0, got:" << results.size()
               << ")";
    }
  }

  // ── Test Search 2: Invalid Date Range ──────────────────────────────────────
  {
    PatientSearchCriteria criteria;
    criteria.fromDate = QDate::currentDate();
    criteria.toDate = QDate::currentDate().addDays(-1); // toDate < fromDate
    auto results = service.searchPatients(criteria);
    int count = service.countSearchResults(criteria);
    if (results.isEmpty() && count == 0) {
      qDebug()
          << "[PASS] Search with Invalid Date Range (caught validation error)";
    } else {
      qDebug() << "[FAIL] Search with Invalid Date Range (expected empty "
                  "result due to validation)";
    }
  }

  qDebug() << "========================================";
  qDebug() << "  Running Patient Delete/Restore Tests in main()";
  qDebug() << "========================================";

  // ── Test Soft Delete / Restore ─────────────────────────────────────────────
  if (idOut > 0) {
    if (service.softDeletePatient(idOut)) {
      qDebug() << "[PASS] Soft Delete Patient (patient_id:" << idOut << ")";
      if (service.restorePatient(idOut)) {
        qDebug() << "[PASS] Restore Patient (patient_id:" << idOut << ")";
      } else {
        qDebug() << "[FAIL] Restore Patient";
      }
    } else {
      qDebug() << "[FAIL] Soft Delete Patient";
    }
  } else {
    qDebug() << "[SKIP] Delete/Restore — OutPatient Insert failed";
  }

  qDebug() << "========================================";
  qDebug() << "  Tests finished";
  qDebug() << "========================================";
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Ghi log ra file cùng thư mục exe
  logFile.setFileName(app.applicationDirPath() + "/debug.log");
  if (!logFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "Cannot open log file:" << logFile.fileName();
  }
  qInstallMessageHandler(messageHandler);

  qDebug() << "=== SmartClinicSystem started ===";
  qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

  // Khởi tạo CSDL — tạo file hospital.db và tất cả bảng nếu chưa có
  DatabaseManager &db = DatabaseManager::getInstance();

  if (!db.isOpen()) {
    qCritical() << "FATAL: Cannot open hospital.db — aborting startup.";
    QMessageBox::critical(
        nullptr, "Database Error",
        "Không thể mở hoặc tạo file hospital.db.\n\n"
        "Vui lòng kiểm tra:\n"
        "  • Thư mục 'database/' có tồn tại và có quyền ghi không?\n"
        "  • Driver QSQLITE có được cài đặt không?\n\n"
        "Xem chi tiết trong debug.log.");
    logFile.close();
    return 1;
  }

  qDebug() << "Database connection OK — hospital.db ready";

  // Chạy integration tests cho database
  runIntegrationTests();

  MainWindow window;
  window.show();

  int result = app.exec();

  qDebug() << "=== SmartClinicSystem exited ===";
  logFile.close();
  return result;
}
