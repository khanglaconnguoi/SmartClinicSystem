#include <QApplication>
#include <memory>

#include "ui/MainWindow.h"
#include "repository/DatabaseManager.h"
#include "repository/StaffRepository.h"
#include "repository/PatientRepository.h"
#include "repository/AppointmentRepository.h"
#include "repository/MedicalRecordRepository.h"
#include "repository/MedicationRepository.h"
#include "repository/PrescriptionRepository.h"

#include "service/AuthService.h"
#include "service/StaffService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"
#include "service/MedicalRecordService.h"
#include "service/PharmacyService.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Get DatabaseManager instance to ensure DB is initialized
    DatabaseManager::getInstance();

    // Create Repositories
    auto staffRepo = std::make_shared<StaffRepository>();
    auto patientRepo = std::make_shared<PatientRepository>();
    auto appointmentRepo = std::make_shared<AppointmentRepository>();
    auto medicalRecordRepo = std::make_shared<MedicalRecordRepository>();
    auto medicationRepo = std::make_shared<MedicationRepository>();
    auto prescriptionRepo = std::make_shared<PrescriptionRepository>();

    // Create Services
    auto authService = std::make_shared<AuthService>(staffRepo);
    auto staffService = std::make_shared<StaffService>(staffRepo);
    auto patientService = std::make_shared<PatientService>(patientRepo);
    auto appointmentService = std::make_shared<AppointmentService>(appointmentRepo, patientRepo, staffRepo);
    auto medicalRecordService = std::make_shared<MedicalRecordService>(medicalRecordRepo, patientService);
    auto pharmacyService = std::make_shared<PharmacyService>(medicationRepo, prescriptionRepo);

    // Create MainWindow
    MainWindow mainWindow(authService, staffService, patientService, appointmentService, medicalRecordService, pharmacyService);
    mainWindow.show();

    return app.exec();
}
