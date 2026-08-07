#include <iostream>
#include <cstdio>
#include <QGuiApplication>

// Declarations of test suite runners
void runValidationTestSuite();
void runPatientModelTestSuite();
void runInvoiceModelTestSuite();
void runMedicationModelTestSuite();
void runMedicalRecordModelTestSuite();
void runAuthAndStaffModelTestSuite();
void runCommonEnumsTestSuite();
void runStaffRolesTestSuite();
void runInvoiceFactoryTestSuite();
void runPaginationTestSuite();
void runUserSessionTestSuite();

int main(int argc, char *argv[]) {
    // QGuiApplication nạp đủ Qt platform integration nếu cần
    QGuiApplication app(argc, argv);

    std::cout << "=======================================================" << std::endl;
    std::cout << "    SMART CLINIC SYSTEM — UNIT TEST SUITE RUNNER       " << std::endl;
    std::cout << "    Total: 11 Test Suites                              " << std::endl;
    std::cout << "=======================================================" << std::endl << std::endl;
    std::fflush(stdout);

    try {
        runValidationTestSuite();
        runPatientModelTestSuite();
        runInvoiceModelTestSuite();
        runMedicationModelTestSuite();
        runMedicalRecordModelTestSuite();
        runAuthAndStaffModelTestSuite();
        runCommonEnumsTestSuite();
        runStaffRolesTestSuite();
        runInvoiceFactoryTestSuite();
        runPaginationTestSuite();
        runUserSessionTestSuite();

        std::cout << std::endl << "=======================================================" << std::endl;
        std::cout << " ALL 11 UNIT TEST SUITES PASSED SUCCESSFULLY (100%)!  " << std::endl;
        std::cout << "=======================================================" << std::endl;
        std::fflush(stdout);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "\n[TEST FAILED] Exception caught: " << e.what() << std::endl;
        std::fflush(stderr);
        return 1;
    } catch (...) {
        std::cerr << "\n[TEST FAILED] Unknown exception caught!" << std::endl;
        std::fflush(stderr);
        return 1;
    }
}
