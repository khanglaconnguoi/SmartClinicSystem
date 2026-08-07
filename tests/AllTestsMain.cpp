#include <iostream>
#include <cstdio>
#include <QApplication>
#include "TestHelper.h"

// Declarations for Unit Test Suites (11)
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
void runStringSanitizeTestSuite();

// Declarations for Integration Test Suites (9)
void runStaffServiceTestSuite();
void runAuthServiceTestSuite();
void runPatientServiceTestSuite();
void runMedicalRecordServiceTestSuite();
void runPharmacyServiceTestSuite();
void runBillingServiceTestSuite();
void runAppointmentServiceTestSuite();
void runServiceRequestServiceTestSuite();
void runAnalyticServiceTestSuite();

// Declarations for System / GUI Test Suites (7)
void runLoginSystemTestSuite();
void runAdminStaffRegistrationTestSuite();
void runReceptionPatientRegistrationTestSuite();
void runDoctorExamTestSuite();
void runPharmacyInventoryTestSuite();
void runCashierBillingTestSuite();
void runNurseVitalSignsTestSuite();

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDateTime>

int main(int argc, char *argv[]) {
    // QApplication hỗ trợ cả GUI, QWidget, QTest, QGuiApplication và QCoreApplication
    QApplication app(argc, argv);

    // Tự động xóa file DB test cũ để đảm bảo dữ liệu test luôn sạch (Fresh DB)
    QString testEnvPath = QString::fromUtf8(PROJECT_ROOT_DIR);
    QString dbFile = testEnvPath + "/database/SmartClinicSystem.db";
    QFile::remove(dbFile);

    std::cout << "=========================================================================" << std::endl;
    std::cout << "        SMART CLINIC SYSTEM — MASTER AUTOMATED TEST RUNNER              " << std::endl;
    std::cout << "        Executing All 27 Test Suites (Unit, Integration & System)        " << std::endl;
    std::cout << "=========================================================================" << std::endl << std::endl;
    std::fflush(stdout);

    try {
        std::cout << "-------------------------------------------------------------------------" << std::endl;
        std::cout << ">>> LEVEL 1: UNIT TESTS (11 Test Suites)" << std::endl;
        std::cout << "-------------------------------------------------------------------------" << std::endl;
        std::fflush(stdout);

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
        runStringSanitizeTestSuite();

        std::cout << std::endl << "-------------------------------------------------------------------------" << std::endl;
        std::cout << ">>> LEVEL 2: INTEGRATION TESTS (9 Test Suites)" << std::endl;
        std::cout << "-------------------------------------------------------------------------" << std::endl;
        std::fflush(stdout);

        runStaffServiceTestSuite();
        runAuthServiceTestSuite();
        runPatientServiceTestSuite();
        runMedicalRecordServiceTestSuite();
        runPharmacyServiceTestSuite();
        runBillingServiceTestSuite();
        runAppointmentServiceTestSuite();
        runServiceRequestServiceTestSuite();
        runAnalyticServiceTestSuite();

        std::cout << std::endl << "-------------------------------------------------------------------------" << std::endl;
        std::cout << ">>> LEVEL 3: SYSTEM / GUI TESTS (7 Test Suites)" << std::endl;
        std::cout << "-------------------------------------------------------------------------" << std::endl;
        std::fflush(stdout);

        runLoginSystemTestSuite();
        runAdminStaffRegistrationTestSuite();
        runReceptionPatientRegistrationTestSuite();
        runDoctorExamTestSuite();
        runPharmacyInventoryTestSuite();
        runCashierBillingTestSuite();
        runNurseVitalSignsTestSuite();

        std::cout << std::endl << "=========================================================================" << std::endl;
        std::cout << "                 SUMMARY OF TEST RESULTS EXECUTION                       " << std::endl;
        std::cout << "=========================================================================" << std::endl;
        std::cout << "  TOTAL TEST CASES ATTEMPTED : " << (g_totalPass + g_totalFail) << std::endl;
        std::cout << "  - PASSED CASES             : " << g_totalPass << " [PASS]" << std::endl;
        std::cout << "  - FAILED CASES             : " << g_totalFail << " [FAIL]" << std::endl;
        std::cout << "=========================================================================" << std::endl;
        if (g_totalFail == 0) {
            std::cout << "  ==> RESULT: OVERALL SUCCESS (100% PASSED)                            " << std::endl;
        } else {
            std::cout << "  ==> RESULT: OVERALL FAILURE (" << g_totalFail << " CASE(S) FAILED)     " << std::endl;
        }
        std::cout << "=========================================================================" << std::endl;
        std::fflush(stdout);

        // --- EXPORT RESULTS TO TXT FILE ---
        QString reportFile = testEnvPath + "/tests/TestResultsSummary.txt";
        QFile file(reportFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "=========================================================================\n";
            out << "        SMART CLINIC SYSTEM - TEST RUN REPORT\n";
            out << "        Date: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
            out << "=========================================================================\n";
            out << "  TOTAL TEST CASES ATTEMPTED : " << (g_totalPass + g_totalFail) << "\n";
            out << "  - PASSED CASES             : " << g_totalPass << " [PASS]\n";
            out << "  - FAILED CASES             : " << g_totalFail << " [FAIL]\n";
            out << "=========================================================================\n";
            if (g_totalFail == 0) {
                out << "  ==> RESULT: OVERALL SUCCESS (100% PASSED)\n";
            } else {
                out << "  ==> RESULT: OVERALL FAILURE (" << g_totalFail << " CASE(S) FAILED)\n";
            }
            out << "=========================================================================\n";
            file.close();
            std::cout << "\n[INFO] Test summary has been exported to: " << reportFile.toStdString() << std::endl;
        }

        return (g_totalFail == 0) ? 0 : 1;
    } catch (const std::exception &e) {
        std::cerr << "\n[ALL TESTS FAILED] Exception caught: " << e.what() << std::endl;
        std::fflush(stderr);
        return 1;
    } catch (...) {
        std::cerr << "\n[ALL TESTS FAILED] Unknown exception caught!" << std::endl;
        std::fflush(stderr);
        return 1;
    }
}
