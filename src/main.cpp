#include "repository/DatabaseManager.h"
#include "repository/PatientRepository.h"
#include "dto/PatientDTOs.h"
#include "model/CommonEnums.h"
#include "ui/MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTextStream>
#include <QDate>

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
      qDebug() << "[PASS] Insert EmergencyPatient (patient_code: TEST-MAIN-EMER)";
    } else {
      qDebug() << "[FAIL] Insert EmergencyPatient";
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