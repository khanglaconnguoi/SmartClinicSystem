#include <iostream>
#include <cstdio>
#include <QApplication>
#include <QDir>

// Forward declarations of GUI test suite runners
void runLoginSystemTestSuite();
void runAdminStaffRegistrationTestSuite();
void runReceptionPatientRegistrationTestSuite();
void runNurseVitalSignsTestSuite();
void runDoctorExamTestSuite();
void runPharmacyInventoryTestSuite();
void runCashierBillingTestSuite();
void runExecutiveAnalyticsTestSuite();
void runFullClinicWorkflowTestSuite();

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Ensure the test database directory exists & remove old test database
    QString testEnvPath = QString::fromUtf8(PROJECT_ROOT_DIR);
    QDir().mkpath(testEnvPath + "/database");
    QString dbFile = testEnvPath + "/database/SmartClinicSystem.db";
    QFile::remove(dbFile);

    std::cout << "=======================================================" << std::endl;
    std::cout << "    SMART CLINIC SYSTEM — SYSTEM / GUI TEST RUNNER     " << std::endl;
    std::cout << "    Total: 9 Test Suites (8 Individual + 1 Full E2E Workflow) " << std::endl;
    std::cout << "    DB Path: " << (testEnvPath + "/database").toStdString() << std::endl;
    std::cout << "=======================================================" << std::endl << std::endl;
    std::fflush(stdout);

    try {
        runLoginSystemTestSuite();
        runAdminStaffRegistrationTestSuite();
        runReceptionPatientRegistrationTestSuite();
        runNurseVitalSignsTestSuite();
        runDoctorExamTestSuite();
        runPharmacyInventoryTestSuite();
        runCashierBillingTestSuite();
        runExecutiveAnalyticsTestSuite();
        runFullClinicWorkflowTestSuite();

        std::cout << std::endl << "=======================================================" << std::endl;
        std::cout << " ALL 9 SYSTEM/GUI TEST SUITES PASSED SUCCESSFULLY!    " << std::endl;
        std::cout << "=======================================================" << std::endl;
        std::fflush(stdout);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "\n[GUI TEST FAILED] Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n[GUI TEST FAILED] Unknown exception!" << std::endl;
        return 1;
    }
}
