#include "dto/BillingDTOs.h"
#include "dto/MedicalRecordDTOs.h"
#include "dto/PatientDTOs.h"
#include "dto/PrescriptionDTOs.h"
#include "model/CommonEnums.h"
#include "repository/AppointmentRepository.h"
#include "repository/BillingRepository.h"
#include "repository/DatabaseManager.h"
#include "repository/MedicalRecordRepository.h"
#include "repository/PatientRepository.h"
#include "repository/StaffRepository.h"
#include "service/AppointmentService.h"
#include "service/AuthService.h"
#include "service/BillingService.h"
#include "service/MedicalRecordService.h"
#include "service/PatientService.h"
#include "service/StaffService.h"
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

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  qDebug() << "=== SmartClinicSystem started ===";

  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.isOpen()) {
    qCritical() << "FATAL: Cannot open hospital.db — aborting startup.";
    QMessageBox::critical(nullptr, "Database Error", "Không thể mở CSDL.");
    return 1;
  }

  auto staffRepo = std::make_shared<StaffRepository>();
  auto authService = std::make_shared<AuthService>(staffRepo);
  auto staffService = std::make_shared<StaffService>(staffRepo);

  auto patientRepo = std::make_shared<PatientRepository>();
  auto patientService = std::make_shared<PatientService>(patientRepo);

  auto appointmentRepo = std::make_shared<AppointmentRepository>();
  auto appointmentService = std::make_shared<AppointmentService>(
      appointmentRepo, patientRepo, staffRepo);

  MainWindow window(authService, staffService, patientService,
                    appointmentService);
  window.show();

  return app.exec();
}
