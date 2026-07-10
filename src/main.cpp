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
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static int getLastInsertedPatientId() {
  QSqlQuery q =
      DatabaseManager::getInstance().selectQuery("SELECT MAX(patient_id) FROM patients");
  if (q.next())
    return q.value(0).toInt();
  return -1;
}

static void seedDatabase() {
  DatabaseManager &db = DatabaseManager::getInstance();
  db.executeQuery("INSERT OR IGNORE INTO departments (department_id, "
                  "department_code, department_name) VALUES (1, 'D01', 'Test Dept')");
  db.executeQuery("INSERT OR IGNORE INTO rooms (room_id, room_number, "
                  "room_type, status) VALUES (1, '101', 'WARD', 'AVAILABLE')");
  db.executeQuery(
      "INSERT OR IGNORE INTO staff (staff_id, staff_code, password_hash, "
      "full_name, role, gender, date_of_birth, citizen_id, phone_number, "
      "email, address, department_id) VALUES (1, 'S01', 'hash', 'Dr. Test', "
      "'DOCTOR', 'MALE', '1980-01-01', '001234567890', '0901234567', "
      "'test@test.com', 'Address', 1)");
}

// ─────────────────────────────────────────────────────────────────────────────
// SECTION: Allergy & Insurance Tests
// ─────────────────────────────────────────────────────────────────────────────

static void runAllergyInsuranceTests() {
  qDebug() << "\n========================================";
  qDebug() << "  Allergy & Insurance Integration Tests";
  qDebug() << "========================================";

  auto repo = std::make_shared<PatientRepository>();
  PatientService service(repo);

  // ── Test 1: Insert OutPatient với nhiều dị ứng và bảo hiểm ─────────────────
  qDebug() << "\n--- Test 1: AddOutPatient với allergies & insurance ---";
  bool t1ok = service.AddOutPatient(
      0,   // patientId = 0 (auto)
      0,   // doctorId  = 0 (chưa gán)
      "Ho Thi Mai",
      QDate(1992, 7, 20),
      "Female",
      "079092012345",           // CCCD hợp lệ
      "0912000111",
      "hothi.mai@test.com",
      "12 Pham Ngu Lao, HCM",
      "AB+",
      "Penicillin, Aspirin, Pollen",  // 3 dị ứng cách nhau bởi dấu phẩy
      "BHYT-123456789",               // số thẻ bảo hiểm
      PatientType::OUTPATIENT,
      "Nguyen Van Hai",
      "0900111222"
  );

  int pid1 = -1;
  if (t1ok) {
    pid1 = getLastInsertedPatientId();
    qDebug() << "[PASS] AddOutPatient (patient_id:" << pid1 << ")";
  } else {
    qDebug() << "[FAIL] AddOutPatient";
  }

  // ── Test 2: Đọc lại allergies từ DB ─────────────────────────────────────────
  qDebug() << "\n--- Test 2: Đọc allergies của bệnh nhân vừa thêm ---";
  if (pid1 > 0) {
    QList<AllergyResultDTO> allergies = service.getAllergies(pid1);
    if (allergies.size() == 3) {
      qDebug() << "[PASS] getAllergies trả về đúng 3 mục:";
      for (const auto &a : allergies) {
        qDebug() << "       -" << a.allergenName << "| severity:" << a.severity
                 << "| isActive:" << a.isActive;
      }
    } else {
      qDebug() << "[FAIL] getAllergies – expected 3, got:" << allergies.size();
      for (const auto &a : allergies) {
        qDebug() << "       -" << a.allergenName;
      }
    }
  } else {
    qDebug() << "[SKIP] – Insert bước 1 thất bại";
  }

  // ── Test 3: Đọc lại insurance từ DB ─────────────────────────────────────────
  qDebug() << "\n--- Test 3: Đọc insurance của bệnh nhân vừa thêm ---";
  if (pid1 > 0) {
    auto ins = service.getInsurance(pid1);
    if (ins.has_value()) {
      qDebug() << "[PASS] getInsurance OK:";
      qDebug() << "       Provider    :" << ins->providerName;
      qDebug() << "       PolicyNumber:" << ins->policyNumber;
      qDebug() << "       Type        :" << ins->insuranceType;
      qDebug() << "       Coverage    :" << ins->coveragePercent << "%";
      qDebug() << "       isActive    :" << ins->isActive;
    } else {
      qDebug() << "[FAIL] getInsurance – trả về nullopt";
    }
  } else {
    qDebug() << "[SKIP] – Insert bước 1 thất bại";
  }

  // ── Test 4: getPatientById – allergies + insurance trong DTO ─────────────────
  qDebug() << "\n--- Test 4: getPatientById trả về đúng allergies & insurance ---";
  if (pid1 > 0) {
    auto detail = service.getPatientById(pid1);
    if (detail.has_value()) {
      qDebug() << "[PASS] getPatientById OK – fullName:" << detail->fullName;

      // Kiểm tra allergies
      if (detail->allergies.size() == 3) {
        qDebug() << "[PASS] PatientDetailDTO.allergies có 3 mục:";
        for (const auto &a : detail->allergies) {
          qDebug() << "       -" << a.allergenName;
        }
      } else {
        qDebug() << "[FAIL] PatientDetailDTO.allergies – expected 3, got:"
                 << detail->allergies.size();
      }

      // Kiểm tra insurance
      if (detail->insurance.has_value()) {
        qDebug() << "[PASS] PatientDetailDTO.insurance có giá trị:"
                 << detail->insurance->policyNumber;
      } else {
        qDebug() << "[FAIL] PatientDetailDTO.insurance – nullopt";
      }
    } else {
      qDebug() << "[FAIL] getPatientById trả về nullopt";
    }
  } else {
    qDebug() << "[SKIP] – Insert bước 1 thất bại";
  }

  // ── Test 5: Bệnh nhân không có dị ứng, không có bảo hiểm ───────────────────
  qDebug() << "\n--- Test 5: AddOutPatient KHÔNG có allergies & insurance ---";
  bool t5ok = service.AddOutPatient(
      0, 0,
      "Phan Van Binh",
      QDate(1975, 3, 10),
      "Male",
      "001075056789",
      "0908777888",
      "phanvanbinh@test.com",
      "45 Hang Bai, HN",
      "O-",
      "",   // không có dị ứng
      "",   // không có bảo hiểm
      PatientType::OUTPATIENT,
      "Phan Thi Lan",
      "0901234000"
  );
  int pid5 = -1;
  if (t5ok) {
    pid5 = getLastInsertedPatientId();
    qDebug() << "[PASS] AddOutPatient no-allergy (patient_id:" << pid5 << ")";

    QList<AllergyResultDTO> allergies5 = service.getAllergies(pid5);
    if (allergies5.isEmpty()) {
      qDebug() << "[PASS] getAllergies trả về rỗng (đúng)";
    } else {
      qDebug() << "[FAIL] getAllergies – expected 0, got:" << allergies5.size();
    }

    auto ins5 = service.getInsurance(pid5);
    if (!ins5.has_value()) {
      qDebug() << "[PASS] getInsurance trả về nullopt (đúng – chưa có BH)";
    } else {
      qDebug() << "[FAIL] getInsurance – expected nullopt";
    }
  } else {
    qDebug() << "[FAIL] AddOutPatient no-allergy";
  }

  // ── Test 6: InPatient với allergies ─────────────────────────────────────────
  qDebug() << "\n--- Test 6: AddInPatient với allergies ---";
  bool t6ok = service.AddInPatient(
      0,
      "Le Thi Cam",
      QDate(1988, 9, 5),
      "Female",
      "048088099001",
      "0977555666",
      "lethicam@test.com",
      "22 Tran Hung Dao, Da Nang",
      "B-",
      "Shellfish, Latex",  // 2 dị ứng
      "PRIVATE-987654",    // bảo hiểm tư nhân
      PatientType::INPATIENT,
      "Le Van Dung",
      "0901222333",
      "1",                 // roomId
      "1",                 // doctorId
      QDate::currentDate(),
      QDate::currentDate().addDays(7),
      "Appendicitis"
  );
  int pid6 = -1;
  if (t6ok) {
    pid6 = getLastInsertedPatientId();
    qDebug() << "[PASS] AddInPatient (patient_id:" << pid6 << ")";

    QList<AllergyResultDTO> allergies6 = service.getAllergies(pid6);
    if (allergies6.size() == 2) {
      qDebug() << "[PASS] Allergies InPatient – 2 mục:";
      for (const auto &a : allergies6)
        qDebug() << "       -" << a.allergenName;
    } else {
      qDebug() << "[FAIL] Allergies InPatient – expected 2, got:" << allergies6.size();
    }
  } else {
    qDebug() << "[FAIL] AddInPatient";
  }

  // ── Test 7: UNIQUE constraint – thêm lại cùng allergen_name ─────────────────
  qDebug() << "\n--- Test 7: Kiểm tra UNIQUE allergen_name (INSERT OR IGNORE) ---";
  if (pid1 > 0) {
    // Thêm trực tiếp qua repository để test constraint
    QList<AllergyInsertDTO> dupes;
    AllergyInsertDTO dup;
    dup.patientId    = pid1;
    dup.allergenName = "PENICILLIN"; // cùng tên nhưng chữ hoa → COLLATE NOCASE → bị IGNORE
    dup.severity     = "SEVERE";
    dupes.append(dup);

    bool insertDupe = repo->insertAllergies(dupes);
    // INSERT OR IGNORE → vẫn trả true nhưng không tạo bản ghi mới
    QList<AllergyResultDTO> afterDupe = service.getAllergies(pid1);
    if (afterDupe.size() == 3) {
      qDebug() << "[PASS] UNIQUE COLLATE NOCASE hoạt động – vẫn còn 3 mục (không bị duplicate)";
    } else {
      qDebug() << "[FAIL] Duplicate allergen – expected 3, got:" << afterDupe.size();
    }
    Q_UNUSED(insertDupe);
  } else {
    qDebug() << "[SKIP]";
  }

  // ── Test 8: upsertInsurance – cập nhật bảo hiểm ─────────────────────────────
  qDebug() << "\n--- Test 8: upsertInsurance cập nhật bảo hiểm hiện có ---";
  if (pid1 > 0) {
    InsuranceInsertDTO newIns;
    newIns.patientId      = pid1;
    newIns.providerName   = "Bao Viet";
    newIns.policyNumber   = "BHYT-NEW-99999";
    newIns.insuranceType  = "PRIVATE";
    newIns.coveragePercent = 90.0;
    newIns.validFrom      = "2024-01-01";
    newIns.validTo        = "2026-12-31";

    bool upserted = repo->upsertInsurance(newIns);
    if (upserted) {
      auto updated = service.getInsurance(pid1);
      if (updated.has_value() && updated->policyNumber == "BHYT-NEW-99999") {
        qDebug() << "[PASS] upsertInsurance cập nhật thành công:";
        qDebug() << "       PolicyNumber:" << updated->policyNumber;
        qDebug() << "       Provider    :" << updated->providerName;
        qDebug() << "       Type        :" << updated->insuranceType;
        qDebug() << "       Coverage    :" << updated->coveragePercent << "%";
      } else {
        qDebug() << "[FAIL] upsertInsurance – giá trị sau update không đúng";
      }
    } else {
      qDebug() << "[FAIL] upsertInsurance – exec thất bại";
    }
  } else {
    qDebug() << "[SKIP]";
  }

  qDebug() << "\n========================================";
  qDebug() << "  Allergy & Insurance Tests DONE";
  qDebug() << "========================================\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  logFile.setFileName(app.applicationDirPath() + "/debug.log");
  if (!logFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "Cannot open log file:" << logFile.fileName();
  }
  qInstallMessageHandler(messageHandler);

  qDebug() << "=== SmartClinicSystem started ===";
  qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

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

  seedDatabase();
  runAllergyInsuranceTests();

  MainWindow window;
  window.show();

  int result = app.exec();

  qDebug() << "=== SmartClinicSystem exited ===";
  logFile.close();
  return result;
}
