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
static void cleanupTestPatient(const QString &patientCode) {
  DatabaseManager::getInstance().executeQuery(
      "DELETE FROM patients WHERE patient_code = ?", {patientCode});
}

static void runIntegrationTests() {
  DatabaseManager::getInstance().executeQuery(
      "INSERT OR IGNORE INTO departments (department_id, department_code, department_name) VALUES (1, 'D01', 'Test Dept')");
  DatabaseManager::getInstance().executeQuery(
      "INSERT OR IGNORE INTO rooms (room_id, room_number, room_type, status) VALUES (1, '101', 'WARD', 'AVAILABLE')");
  DatabaseManager::getInstance().executeQuery(
      "INSERT OR IGNORE INTO staff (staff_id, staff_code, password_hash, full_name, role, gender, date_of_birth, citizen_id, phone_number, email, address, department_id) VALUES (1, 'S01', 'hash', 'Dr. Test', 'DOCTOR', 'MALE', '1980-01-01', '001234567890', '0901234567', 'test@test.com', 'Address', 1)");

  qDebug() << "========================================";
  qDebug() << "  Running Patient Insert Tests in main()";
  qDebug() << "========================================";

  PatientRepository repo;

  // Test 1: OutPatient
  {
    const QString code = "TEST-MAIN-OUT";
    cleanupTestPatient(code);
    OutPatientInputDTO input;
    input.fullName = "Nguyen Van A";
    input.dateOfBirth = QDate(1990, 5, 15);
    input.gender = Gender::Male;
    input.bloodType = "A+";
    input.type = PatientType::OUTPATIENT;
    input.doctorId = std::nullopt;
    OutPatientInsertDTO dto(input, code);
    if (repo.insertOutPatient(dto)) {
      qDebug() << "[PASS] Insert OutPatient (patient_code: TEST-MAIN-OUT)";
    } else {
      qDebug() << "[FAIL] Insert OutPatient";
    }
  }

  // Test 2: InPatient
  {
    const QString code = "TEST-MAIN-IN";
    cleanupTestPatient(code);
    InPatientInputDTO input;
    input.fullName = "Tran Thi B";
    input.dateOfBirth = QDate(1985, 11, 20);
    input.gender = Gender::Female;
    input.bloodType = "B+";
    input.type = PatientType::INPATIENT;
    input.roomId = std::nullopt;
    input.doctorId = std::nullopt;
    input.admissionDate = QDate::currentDate();
    input.dischargeDate = std::nullopt;
    input.reason = "Abdominal pain";
    InPatientInsertDTO dto(input, code);
    if (repo.insertInPatient(dto)) {
      qDebug() << "[PASS] Insert InPatient (patient_code: TEST-MAIN-IN)";
    } else {
      qDebug() << "[FAIL] Insert InPatient";
    }
  }

  // Test 3: EmergencyPatient
  {
    const QString code = "TEST-MAIN-EMER";
    cleanupTestPatient(code);
    EmergencyPatientInputDTO input;
    input.fullName = "Le Van C";
    input.dateOfBirth = QDate(2000, 3, 8);
    input.gender = Gender::Male;
    input.bloodType = "O+";
    input.type = PatientType::EMERGENCY;
    input.roomId = std::nullopt;
    input.doctorId = std::nullopt;
    input.injuryCause = "Traffic accident";
    input.injuryDescription = "Multiple lacerations";
    input.admissionDate = QDate::currentDate();
    input.dischargeDate = std::nullopt;
    EmergencyPatientInsertDTO dto(input, code);
    if (repo.insertEmergencyPatient(dto)) {
      qDebug()
          << "[PASS] Insert EmergencyPatient (patient_code: TEST-MAIN-EMER)";
    } else {
      qDebug() << "[FAIL] Insert EmergencyPatient";
    }
  }

  qDebug() << "========================================";
  qDebug() << "  Running Patient Update Tests in main()";
  qDebug() << "========================================";

  auto sharedRepo = std::make_shared<PatientRepository>();
  PatientService service(sharedRepo);

  // Test 1: Update OutPatient
  {
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(
        "SELECT patient_id FROM patients WHERE patient_code = 'TEST-MAIN-OUT'");
    if (query.next()) {
      int patientId = query.value(0).toInt();
      if (service.UpdateOutPatient(
              patientId, 0, "Nguyen Van A Updated", QDate(1990, 5, 15), "Male",
              "001012345678", "0987654321", "a@test.com", "Address 1", "A+",
              "None", "BaoViet", "Contact A", "0987654321", "TREATMENT")) {
        qDebug() << "[PASS] Update OutPatient (patient_id:" << patientId << ")";
      } else {
        qDebug() << "[FAIL] Update OutPatient";
      }
    } else {
      qDebug() << "[FAIL] Could not find OutPatient for Update test";
    }
  }

      InPatientInputDTO inInput;
      inInput.roomId = 1;
      inInput.doctorId = 1;
      inInput.admissionDate = QDate::currentDate();
      inInput.dischargeDate = QDate::currentDate().addDays(2);
      inInput.reason = "Cured";
      
  {
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(
        "SELECT patient_id FROM patients WHERE patient_code = 'TEST-MAIN-IN'");
    if (query.next()) {
      int patientId = query.value(0).toInt();
      InPatientUpdateDTO inDto(inInput, patientId, "DISCHARGED");
      if (repo.updateInPatient(inDto)) {
        qDebug() << "[PASS] Update InPatient (patient_id:" << patientId << ")";
      } else {
        qDebug() << "[FAIL] Update InPatient";
      }
    } else {
      qDebug() << "[FAIL] Could not find InPatient for Update test";
    }
  }

      // Thử Update EmergencyPatient
      EmergencyPatientInputDTO emerInput;
      emerInput.roomId = 1;
      emerInput.doctorId = 1;
      emerInput.injuryCause = "Recovered";
      emerInput.injuryDescription = "Treated";
      emerInput.admissionDate = QDate::currentDate();
      emerInput.dischargeDate = QDate::currentDate().addDays(1);
  {
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(
        "SELECT patient_id FROM patients WHERE patient_code = "
        "'TEST-MAIN-EMER'");
    if (query.next()) {
      int patientId = query.value(0).toInt();
      EmergencyPatientUpdateDTO emerDto(emerInput, patientId, "DISCHARGED");
      if (repo.updateEmergencyPatient(emerDto)) {
        qDebug() << "[PASS] Update EmergencyPatient (patient_id:" << patientId
                 << ")";
      } else {
        qDebug() << "[FAIL] Update EmergencyPatient";
      }
    }
  }

  qDebug() << "========================================";
  qDebug() << "  Running Patient Search Tests in main()";
  qDebug() << "========================================";

  // Test Search 1: Valid Search (Keyword "TEST-MAIN")
  {
    PatientSearchCriteria criteria;
    criteria.searchKey = "TEST-MAIN";
    auto results = service.searchPatients(criteria);
    int count = service.countSearchResults(criteria);
    if (results.size() > 0 && count > 0) {
      qDebug() << "[PASS] Search by Keyword (found:" << results.size() << ", total:" << count << ")";
    } else {
      qDebug() << "[FAIL] Search by Keyword (expected > 0, got:" << results.size() << ")";
    }
  }

  // Test Search 2: Invalid Date Range
  {
    PatientSearchCriteria criteria;
    criteria.fromDate = QDate::currentDate();
    criteria.toDate = QDate::currentDate().addDays(-1); // toDate < fromDate
    auto results = service.searchPatients(criteria);
    int count = service.countSearchResults(criteria);
    if (results.isEmpty() && count == 0) {
      qDebug() << "[PASS] Search with Invalid Date Range (caught validation error)";
    } else {
      qDebug() << "[FAIL] Search with Invalid Date Range (expected empty result due to validation)";
    }
  }

  qDebug() << "========================================";
  qDebug() << "  Running Patient Delete/Restore Tests in main()";
  qDebug() << "========================================";

  // Test Delete / Restore
  {
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(
        "SELECT patient_id FROM patients WHERE patient_code = 'TEST-MAIN-OUT'");
    if (query.next()) {
      int patientId = query.value(0).toInt();
      if (service.softDeletePatient(patientId)) {
        qDebug() << "[PASS] Soft Delete Patient (patient_id:" << patientId << ")";
        if (service.restorePatient(patientId)) {
          qDebug() << "[PASS] Restore Patient (patient_id:" << patientId << ")";
        } else {
          qDebug() << "[FAIL] Restore Patient";
        }
      } else {
        qDebug() << "[FAIL] Soft Delete Patient";
      }
    } else {
      qDebug() << "[FAIL] Could not find Patient for Delete test";
    }
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
