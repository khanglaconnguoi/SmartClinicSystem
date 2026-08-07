#include <iostream>
#include <cstdio>
#include <QGuiApplication>
#include <QDir>

// Forward declarations of test suite runners
void runPatientServiceTestSuite();
void runMedicalRecordServiceTestSuite();
void runStaffServiceTestSuite();
void runAuthServiceTestSuite();
void runPharmacyServiceTestSuite();
void runBillingServiceTestSuite();
void runAppointmentServiceTestSuite();
void runServiceRequestServiceTestSuite();
void runAnalyticServiceTestSuite();
void runRepositoryCrudTestSuite();

#include <QFile>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Ensure the test database directory exists & remove old test database
    QString testEnvPath = QString::fromUtf8(PROJECT_ROOT_DIR);
    QDir().mkpath(testEnvPath + "/database");
    QString dbFile = testEnvPath + "/database/SmartClinicSystem.db";
    QFile::remove(dbFile);

    std::cout << "=======================================================" << std::endl;
    std::cout << "    SMART CLINIC SYSTEM — INTEGRATION TEST RUNNER      " << std::endl;
    std::cout << "    Total: 10 Test Suites (Service + Repository + DB)  " << std::endl;
    std::cout << "    DB Path: " << (testEnvPath + "/database").toStdString() << std::endl;
    std::cout << "=======================================================" << std::endl << std::endl;
    std::fflush(stdout);

    try {
        runStaffServiceTestSuite();
        runAuthServiceTestSuite();
        runPatientServiceTestSuite();
        runMedicalRecordServiceTestSuite();
        runPharmacyServiceTestSuite();
        runBillingServiceTestSuite();
        runAppointmentServiceTestSuite();
        runServiceRequestServiceTestSuite();
        runAnalyticServiceTestSuite();
        runRepositoryCrudTestSuite();

        std::cout << std::endl << "=======================================================" << std::endl;
        std::cout << " ALL 10 INTEGRATION TEST SUITES PASSED SUCCESSFULLY!  " << std::endl;
        std::cout << "=======================================================" << std::endl;
        std::fflush(stdout);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "\n[IT FAILED] Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n[IT FAILED] Unknown exception!" << std::endl;
        return 1;
    }
}
