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
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <memory>

#include "repository/DatabaseManager.h"
#include "repository/StaffRepository.h"
#include "repository/PatientRepository.h"
#include "repository/AppointmentRepository.h"
#include "repository/MedicationRepository.h"
#include "repository/PrescriptionRepository.h"
#include "repository/MedicalRecordRepository.h"

#include "service/StaffService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"
#include "service/PharmacyService.h"
#include "service/MedicalRecordService.h"
#include "service/UserSession.h"

#include "dto/StaffDTOs.h"
#include "model/doctor.h"
#include "ui/Doctor/DoctorDashboard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "=== Starting Doctor Interface Test ===";

    // 1. Kết nối CSDL hospital.db
    DatabaseManager &dbManager = DatabaseManager::getInstance();
    if (!dbManager.isOpen())
    {
        QMessageBox::critical(nullptr, "Lỗi Cơ Sở Dữ Liệu", "Không thể kết nối đến CSDL hospital.db!");
        return -1;
    }

    // 2. Khởi tạo Repositories & Services
    auto staffRepo = std::make_shared<StaffRepository>();
    auto patientRepo = std::make_shared<PatientRepository>();
    auto appointmentRepo = std::make_shared<AppointmentRepository>();

    auto staffService = std::make_shared<StaffService>(staffRepo);
    auto patientService = std::make_shared<PatientService>(patientRepo);
    auto appointmentService = std::make_shared<AppointmentService>(appointmentRepo);

    auto medicationRepo = std::make_shared<MedicationRepository>();
    auto prescriptionRepo = std::make_shared<PrescriptionRepository>();
    auto pharmacyService = std::make_shared<PharmacyService>(medicationRepo, prescriptionRepo);

    auto recordRepo = std::make_shared<MedicalRecordRepository>(patientRepo);
    auto recordService = std::make_shared<MedicalRecordService>(recordRepo, patientService);

    // 3. Giả lập Bác sĩ đang đăng nhập (Dr. Nguyễn Văn A - Mã BS01) dựa trên DoctorProfileDTO (StaffDTOs.h)
    DoctorProfileDTO docProfile;
    docProfile.staffId = 1;
    docProfile.staffCode = "BS01";
    docProfile.fullName = "BS. Nguyễn Văn A";
    docProfile.role = UserRole::Doctor;
    docProfile.gender = "Nam";
    docProfile.specialty = "Khoa Nội Nhi";
    docProfile.isActive = true;

    auto currentDoctor = std::make_shared<Doctor>(
        docProfile.staffId,
        docProfile.staffCode,
        "hashed_password",
        docProfile.fullName,
        docProfile.avatar,
        docProfile.role,
        docProfile.isActive,
        false,
        docProfile.specialty,
        "CCHN-12345",
        10,
        150000,
        "Bác sĩ chuyên khoa Nội Nhi");
    UserSession::getInstance().setCurrentAccount(currentDoctor);

    // 4. Khởi tạo và hiển thị Giao diện DoctorDashboard
    DoctorDashboardWidget doctorDashboard(currentDoctor, staffService, patientService, appointmentService, recordService, pharmacyService);
    doctorDashboard.setWindowTitle("Hệ Thống Phòng Khám - Phân Hệ Bác Sĩ (Test Mode)");
    doctorDashboard.resize(1280, 800);
    doctorDashboard.show();

    return app.exec();
}
