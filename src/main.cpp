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
#include "repository/BillingRepository.h"
#include "repository/ServiceRequestRepository.h"
#include "repository/AnalyticRepository.h"

#include "service/AuthService.h"
#include "service/StaffService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"
#include "service/MedicalRecordService.h"
#include "service/PharmacyService.h"
#include "service/BillingService.h"
#include "service/ServiceRequestService.h"
#include "service/AnalyticService.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Ép kiểu giao diện toàn ứng dụng: chữ tối (#172B4D) trên nền sáng (#FFFFFF / #F4F5F7)
    // tránh lỗi chữ trắng trên nền trắng do kế thừa theme OS (Windows Dark Mode)
    app.setStyleSheet(R"(
        QWidget {
            font-family: 'Segoe UI', Arial, sans-serif;
            color: #172B4D;
        }
        QMainWindow, QDialog {
            background-color: #F4F5F7;
            color: #172B4D;
        }
        QLabel {
            color: #172B4D;
        }
        QLineEdit, QTextEdit, QPlainTextEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit {
            background-color: #FFFFFF;
            color: #172B4D;
            border: 1px solid #DFE1E6;
            border-radius: 6px;
            padding: 5px 10px;
            font-size: 13px;
            selection-background-color: #DEEBFF;
            selection-color: #0052CC;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus {
            border: 1px solid #0052CC;
            background-color: #FFFFFF;
        }
        QComboBox QAbstractItemView {
            background-color: #FFFFFF;
            color: #172B4D;
            selection-background-color: #DEEBFF;
            selection-color: #0052CC;
            border: 1px solid #DFE1E6;
        }
        QTableWidget, QTableView {
            background-color: #FFFFFF;
            color: #172B4D;
            gridline-color: #EBECF0;
            border: 1px solid #DFE1E6;
            selection-background-color: #DEEBFF;
            selection-color: #0052CC;
        }
        QTableWidget::item, QTableView::item {
            color: #172B4D;
            background-color: #FFFFFF;
            padding: 6px;
        }
        QTableWidget::item:selected, QTableView::item:selected {
            background-color: #DEEBFF;
            color: #0052CC;
        }
        QHeaderView::section {
            background-color: #F4F5F7;
            color: #5E6C84;
            font-weight: bold;
            padding: 8px;
            border: none;
            border-bottom: 2px solid #DFE1E6;
        }
        QMessageBox {
            background-color: #FFFFFF;
            color: #172B4D;
        }
        QMessageBox QLabel {
            color: #172B4D;
            font-size: 14px;
        }
        QToolTip {
            background-color: #172B4D;
            color: #FFFFFF;
            border: none;
            padding: 5px 8px;
            border-radius: 4px;
        }
        QMenu {
            background-color: #FFFFFF;
            color: #172B4D;
            border: 1px solid #DFE1E6;
        }
        QMenu::item:selected {
            background-color: #DEEBFF;
            color: #0052CC;
        }
    )");

    // Get DatabaseManager instance to ensure DB is initialized
    DatabaseManager::getInstance();

    // Create Repositories
    auto staffRepo = std::make_shared<StaffRepository>();
    auto patientRepo = std::make_shared<PatientRepository>();
    auto appointmentRepo = std::make_shared<AppointmentRepository>();
    auto medicalRecordRepo = std::make_shared<MedicalRecordRepository>();
    auto medicationRepo = std::make_shared<MedicationRepository>();
    auto prescriptionRepo = std::make_shared<PrescriptionRepository>();
    auto billingRepo = std::make_shared<BillingRepository>();
    auto serviceRequestRepo = std::make_shared<ServiceRequestRepository>();
    auto analyticRepo = std::make_shared<AnalyticRepository>();

    // Create Services
    auto authService = std::make_shared<AuthService>(staffRepo);
    auto staffService = std::make_shared<StaffService>(staffRepo);
    auto patientService = std::make_shared<PatientService>(patientRepo);
    auto appointmentService = std::make_shared<AppointmentService>(appointmentRepo, patientRepo, staffRepo);
    auto medicalRecordService = std::make_shared<MedicalRecordService>(medicalRecordRepo, patientService);
    auto pharmacyService = std::make_shared<PharmacyService>(medicationRepo, prescriptionRepo);
    auto billingService = std::make_shared<BillingService>(billingRepo);
    auto serviceRequestService = std::make_shared<ServiceRequestService>(serviceRequestRepo);
    auto analyticService = std::make_shared<AnalyticService>(analyticRepo);

    auto x = patientService->getInsurance(4);
    qDebug() << "Coverage percent of patient 4: " << x.value().coveragePercent;

    // Create MainWindow
    MainWindow mainWindow(authService, staffService, patientService, appointmentService, medicalRecordService, pharmacyService, billingService, serviceRequestService, analyticService);
    mainWindow.show();

    return app.exec();
}
