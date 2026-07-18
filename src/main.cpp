#include "dto/BillingDTOs.h"
#include "dto/MedicalRecordDTOs.h"
#include "dto/PatientDTOs.h"
#include "dto/PrescriptionDTOs.h"
#include "model/CommonEnums.h"
#include "repository/BillingRepository.h"
#include "repository/DatabaseManager.h"
#include "repository/MedicalRecordRepository.h"
#include "repository/PatientRepository.h"
#include "service/BillingService.h"
#include "service/MedicalRecordService.h"
#include "repository/StaffRepository.h"
#include "service/AuthService.h"
#include "service/PatientService.h"
#include "service/StaffService.h"
#include "repository/AppointmentRepository.h"
#include "service/AppointmentService.h"
#include "service/Validation.h"
#include "ui/MainWindow.h"
#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTextStream>
#include <memory>

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
  QSqlQuery q = DatabaseManager::getInstance().selectQuery(
      "SELECT MAX(patient_id) FROM patients");
  if (q.next())
    return q.value(0).toInt();
  return -1;
}

static void seedDatabase() {
  DatabaseManager &db = DatabaseManager::getInstance();
  db.executeQuery(
      "INSERT OR IGNORE INTO departments (department_id, department_code, "
      "department_name) VALUES (1, 'D01', 'Test Dept')");
  db.executeQuery("INSERT OR IGNORE INTO rooms (room_id, room_number, "
                  "room_type, status) VALUES (1, '101', 'WARD', 'AVAILABLE')");
  db.executeQuery(
      "INSERT OR IGNORE INTO staff (staff_id, staff_code, password_hash, "
      "full_name, role, gender, date_of_birth, citizen_id, phone_number, "
      "email, address, department_id) VALUES (1, 'S01', 'hash', 'Dr. Test', "
      "'DOCTOR', 'MALE', '1980-01-01', '001234567890', '0901234567', "
      "'test@test.com', 'Address', 1)");
}

// static int getLastInsertedPatientId() {
//   QSqlQuery q = DatabaseManager::getInstance().selectQuery(
//       "SELECT MAX(patient_id) FROM patients");
//   if (q.next())
//     return q.value(0).toInt();
//   return -1;
// }

static void testValidations() {
  qDebug() << "\n==================================================";
  qDebug() << "                TESTING VALIDATIONS               ";
  qDebug() << "==================================================";

  PatientInputDTO dto;

  // 1. Test Phone Normalization
  dto.phone = "+84 988.123-456";
  dto.emergencyContactPhone = " 090 123 4567 ";
  PatientService::normalizePatientInput(dto);
  qDebug() << "[1] Phone Normalization:";
  qDebug() << "    Expected: 0988123456 | Got:" << dto.phone;
  qDebug() << "    Expected: 0901234567 | Got:" << dto.emergencyContactPhone;

  // 2. Test Name Validation
  QString badName1 = "Nguyễn Văn A 123";
  QString badName2 = "Trần @ Anh";
  QString goodName = "Nguyễn Văn A";
  qDebug() << "\n[2] Name Validation:";
  qDebug() << "    Input:" << badName1
           << "-> Error:" << Validation::validateFullName(badName1);
  qDebug() << "    Input:" << badName2
           << "-> Error:" << Validation::validateFullName(badName2);
  qDebug() << "    Input:" << goodName
           << "-> Error:" << Validation::validateFullName(goodName);

  // 3. Test Age Validation
  QDate futureDate = QDate::currentDate().addDays(1);
  QDate veryOldDate = QDate::currentDate().addYears(-151);
  QDate normalDate = QDate(1990, 1, 1);
  qDebug() << "\n[3] Age Validation:";
  qDebug() << "    Future Date (" << futureDate.toString()
           << ") -> Error:" << Validation::validateDateOfBirth(futureDate);
  qDebug() << "    Age > 150 (" << veryOldDate.toString()
           << ") -> Error:" << Validation::validateDateOfBirth(veryOldDate);
  qDebug() << "    Normal Date (" << normalDate.toString()
           << ") -> Error:" << Validation::validateDateOfBirth(normalDate);

  // 4. Test Address and Reason length
  PatientInputDTO lengthDto;
  lengthDto.address = QString(300, 'A'); // > 255 chars
  qDebug() << "\n[4] Address Length Validation (> 255):";
  qDebug() << "    Error:"
           << PatientService::validateBaseInput(lengthDto, "PT001");

  InPatientInputDTO inDto;
  inDto.roomId = 1;
  inDto.doctorId = 1;
  inDto.reason = QString(1500, 'B'); // > 1000 chars
  qDebug() << "\n[5] InPatient Reason Length (> 1000):";
  // chua viet ham
  // qDebug() << "    Error:"
  //          << PatientService::validateInPatientReason(inDto.reason);

  // 6. Test Billing/Invoice Validation
  QList<PrescriptionItemDTO> badItems;
  PrescriptionItemDTO pBad;
  pBad.brandName = "Thuốc âm tiền";
  pBad.quantity = 0; // Invalid
  pBad.unitPrice = -5000; // Invalid
  badItems.append(pBad);
  
  BillingService billingService(nullptr); // Repo can be null for validation
  qDebug() << "\n[6] Invoice Validation (Bad Patient/Record ID & Bad Items):";
  qDebug() << "    Error:" << billingService.validateInvoiceInput(-1, -1, -50000, badItems);

  qDebug() << "==================================================\n";
}

static void runComprehensiveTests() {
  qDebug() << ">>> TESTING 1 2 3: runComprehensiveTests IS CALLED";
  qDebug() << "\n==================================================";
  qDebug() << "          COMPREHENSIVE INTEGRATION TESTS         ";
  qDebug() << "==================================================";

  auto patientRepo = std::make_shared<PatientRepository>();
  auto patientService = std::make_shared<PatientService>(patientRepo);

  auto mrRepo = std::make_shared<MedicalRecordRepository>();
  MedicalRecordService mrService(mrRepo, patientService);

  auto billingRepo = std::make_shared<BillingRepository>();
  BillingService billingService(billingRepo);

  // ---------------------------------------------------------
  // 1. PATIENT SERVICE TESTS
  // ---------------------------------------------------------
  qDebug() << "\n[1] TEST: PATIENT SERVICE";

  // 1.1 Add OutPatient
  OutPatientInputDTO pOut;
  pOut.fullName = "  Le Thi Xuan  ";
  pOut.dateOfBirth = QDate(1995, 2, 14);
  pOut.gender = "Nữ";
  pOut.citizenId = "048195000123";
  pOut.phone = "0909111222";
  pOut.address = "Hai Chau, Da Nang";
  pOut.bloodType = " AB- ";
  pOut.type = PatientType::Outpatient;
  pOut.email = "lexuan@test.com";
  pOut.emergencyContactName = "Le Van A";
  pOut.emergencyContactPhone = "0900111222";
  pOut.insurance.policyNumber = "BHYT-123456";
  bool okOut = patientService->addOutPatient(pOut);
  int pidOut = getLastInsertedPatientId();
  qDebug() << "  -> Add OutPatient:" << (okOut ? "PASS" : "FAIL")
           << "ID:" << pidOut;

  // 1.2 Add InPatient
  InPatientInputDTO pIn;
  pIn.fullName = "Tran Van Y";
  pIn.dateOfBirth = QDate(1980, 8, 8);
  pIn.gender = "Nam";
  pIn.citizenId = "048080000321";
  pIn.phone = "0988777666";
  pIn.address = "Thanh Khe, Da Nang";
  pIn.bloodType = "O+";
  pIn.type = PatientType::Inpatient;
  pIn.email = "tranvany@test.com";
  pIn.emergencyContactName = "Tran Thi Z";
  pIn.emergencyContactPhone = "0988000111";
  pIn.roomId = 1;
  pIn.doctorId = 1;
  pIn.admissionDate = QDate::currentDate();
  pIn.reason = "Dengue fever";
  bool okIn = patientService->addInPatient(pIn);
  int pidIn = getLastInsertedPatientId();
  qDebug() << "  -> Add InPatient:" << (okIn ? "PASS" : "FAIL")
           << "ID:" << pidIn;

  // 1.3 Add Emergency
  EmergencyPatientInputDTO pEm;
  pEm.fullName = "Vo Z";
  pEm.dateOfBirth = QDate(2000, 1, 1);
  pEm.gender = "Nam";
  pEm.citizenId = "048200000999";
  pEm.phone = "0911222333";
  pEm.address = "Son Tra, Da Nang";
  pEm.bloodType = "UNKNOWN";
  pEm.type = PatientType::Emergency;
  pEm.email = "voz@test.com";
  pEm.emergencyContactName = "Vo Van W";
  pEm.emergencyContactPhone = "0911000222";
  pEm.roomId = 1;
  pEm.doctorId = 1;
  pEm.admissionDate = QDate::currentDate();
  pEm.injuryCause = "Traffic accident";
  pEm.injuryDescription = "Head trauma";
  bool okEm = patientService->addEmergencyPatient(pEm);
  int pidEm = getLastInsertedPatientId();
  qDebug() << "  -> Add EmergencyPatient:" << (okEm ? "PASS" : "FAIL")
           << "ID:" << pidEm;

  // 1.4 Search Patient
  PatientSearchCriteria pSearch;
  pSearch.searchKey = "XUAN";
  auto searchRes = patientService->searchPatients(pSearch);
  qDebug() << "  -> Search Patient 'XUAN':"
           << (searchRes.size() > 0 ? "PASS" : "FAIL")
           << "Found:" << searchRes.size();

  // 1.5 Update Patient
  if (pidOut > 0) {
    OutPatientInputDTO uDto;
    uDto.fullName = "Le Thi Xuan Updated";
    uDto.dateOfBirth = QDate(1995, 2, 14);
    uDto.gender = "Nữ";
    uDto.citizenId = "048195000123";
    uDto.phone = "0909999888";
    uDto.address = "Lien Chieu, Da Nang";
    uDto.bloodType = "AB-";
    uDto.email = "lexuan@test.com";
    uDto.emergencyContactName = "Le Van A";
    uDto.emergencyContactPhone = "0900111222";
    uDto.type = PatientType::Outpatient;
    uDto.doctorId = 1;
    bool okUpdate = patientService->updateOutPatient(pidOut, uDto);
    qDebug() << "  -> Update Patient:" << (okUpdate ? "PASS" : "FAIL");
  }

  // 1.6 Soft Delete Patient
  if (pidEm > 0) {
    bool okDel = patientService->softDeletePatient(pidEm);
    qDebug() << "  -> Soft Delete Patient:" << (okDel ? "PASS" : "FAIL");
  }

  // ---------------------------------------------------------
  // 2. MEDICAL RECORD SERVICE TESTS
  // ---------------------------------------------------------
  qDebug() << "\n[2] TEST: MEDICAL RECORD SERVICE";
  int recordId = -1;

  if (pidOut > 0) {
    // 2.1 Create Medical Record
    MedicalRecordInsertDTO mrDto;
    mrDto.patientId = pidOut;
    mrDto.doctorId = 1;
    mrDto.visitDateTime = QDateTime::currentDateTime();
    mrDto.vitals.bloodPressure = "110/70";
    mrDto.vitals.heartRate = 80;
    mrDto.vitals.temperature = 37.5;
    mrDto.vitals.weight = 50.0;
    mrDto.vitals.height = 160.0;
    mrDto.chiefComplaint = "  Sốt, ho khan  ";
    mrDto.clinicalNotes = "Họng đỏ";
    mrDto.treatment = "Paracetamol, uống nhiều nước";
    mrDto.nextVisitDate = QDate::currentDate().addDays(3);

    Diagnosis diag1;
    diag1.icdCode = "j02.9"; // -> J02.9
    diag1.description = "Viêm họng cấp";
    diag1.severity = "MODERATE";
    mrDto.diagnoses.append(diag1);

    // Dị ứng phát hiện trong lần khám này
    AllergyInputDTO allergy1;
    allergy1.allergenName = "Seafood";
    allergy1.severity = "MILD";
    allergy1.notes = "Phát hiện trong lần khám đầu tiên";
    mrDto.newAllergies.append(allergy1);

    recordId = mrService.createMedicalRecord(mrDto);
    qDebug() << "  -> Create MedicalRecord:" << (recordId > 0 ? "PASS" : "FAIL")
             << "RecordID:" << recordId;

    // 2.2 Update Medical Record
    if (recordId > 0) {
      MedicalRecordUpdateDTO uMrDto;
      uMrDto.recordId = recordId;
      uMrDto.doctorId = 1;
      uMrDto.visitDateTime = QDateTime::currentDateTime();
      uMrDto.vitals.bloodPressure = "110/70";
      uMrDto.vitals.heartRate = 85;
      uMrDto.vitals.temperature = 38.5;
      uMrDto.vitals.weight = 50.0;
      uMrDto.vitals.height = 160.0;
      uMrDto.chiefComplaint = "Sốt, ho nhiều";
      uMrDto.clinicalNotes = "Viêm họng nặng hơn";
      uMrDto.treatment = "Kháng sinh + Paracetamol";

      Diagnosis diag2;
      diag2.icdCode = "J02.9";
      diag2.description = "Viêm họng cấp tính";
      diag2.severity = "SEVERE";
      uMrDto.diagnoses.append(diag2);

      bool okMrUpdate = mrService.updateMedicalRecord(uMrDto);
      qDebug() << "  -> Update MedicalRecord:"
               << (okMrUpdate ? "PASS" : "FAIL");
    }

    // 2.3 Search Medical Record
    MedicalRecordSearchCriteria mrSearch;
    mrSearch.searchKey = "Sốt";
    mrSearch.patientId = pidOut;
    auto mrSearchRes = mrService.searchMedicalRecords(mrSearch);
    qDebug() << "  -> Search MedicalRecord 'Sốt':"
             << (mrSearchRes.size() > 0 ? "PASS" : "FAIL")
             << "Found:" << mrSearchRes.size();
  }

  // ---------------------------------------------------------
  // 3. BILLING SERVICE TESTS
  // ---------------------------------------------------------
  qDebug() << "\n[3] TEST: BILLING SERVICE";
  int invoiceId = -1;

  if (pidOut > 0 && recordId > 0) {
    // 3.1 Generate Invoice
    QList<PrescriptionItemDTO> pItems;
    PrescriptionItemDTO p1, p2;
    p1.brandName = "Paracetamol 500mg";
    p1.quantity = 15;
    p1.unitPrice = 2000.0;
    p2.brandName = "Amoxicillin 250mg";
    p2.quantity = 20;
    p2.unitPrice = 3000.0;
    pItems.append(p1);
    pItems.append(p2);

    bool billOk = billingService.createInvoice(
        pidOut, recordId, PatientType::Outpatient, 200000.0, pItems);
    qDebug() << "  -> Generate Invoice:" << (billOk ? "PASS" : "FAIL");

    // 3.2 Get Invoice
    if (billOk) {
      auto invoices = billingService.getInvoiceByRecordId(recordId);
      if (invoices.has_value()) {
        invoiceId = invoices->invoiceId;
        qDebug() << "  -> Invoice Code:" << invoices->invoiceCode
                 << "| Total:" << invoices->totalAmount;
      }
    }

    // 3.3 Search Invoice
    InvoiceSearchCriteria invSearch;
    invSearch.patientId = pidOut;
    auto invSearchRes = billingService.searchInvoices(invSearch);
    qDebug() << "  -> Search Invoices by Patient:"
             << (invSearchRes.size() > 0 ? "PASS" : "FAIL")
             << "Found:" << invSearchRes.size();

    // 3.4 Cancel Invoice (Soft delete/status update)
    if (invoiceId > 0) {
      bool cancelOk = billingService.cancelInvoice(invoiceId);
      qDebug() << "  -> Cancel Invoice:" << (cancelOk ? "PASS" : "FAIL");
    }
  }

  qDebug() << "\n==================================================";
  qDebug() << "             ALL TESTS COMPLETED                  ";
  qDebug() << "==================================================\n";
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  qDebug() << "=== SmartClinicSystem started ===";

  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.isOpen()) {
    qCritical() << "FATAL: Cannot open hospital.db — aborting startup.";
    QMessageBox::critical(nullptr, "Database Error", "Không thể mở CSDL.");
    return 1;
  }

  // Tự động seed dữ liệu mẫu
  seedDatabase();

  testValidations();
  
  // Chạy thêm luồng tạo dữ liệu mẫu (bệnh nhân, hồ sơ khám, hóa đơn)
  runComprehensiveTests();

  auto staffRepo = std::make_shared<StaffRepository>();
  auto authService = std::make_shared<AuthService>(staffRepo);
  auto staffService = std::make_shared<StaffService>(staffRepo);

  auto patientRepo = std::make_shared<PatientRepository>();
  auto patientService = std::make_shared<PatientService>(patientRepo);

  auto appointmentRepo = std::make_shared<AppointmentRepository>();
  auto appointmentService = std::make_shared<AppointmentService>(appointmentRepo);

  MainWindow window(authService, staffService, patientService, appointmentService);
  window.show();

  return app.exec();
}
