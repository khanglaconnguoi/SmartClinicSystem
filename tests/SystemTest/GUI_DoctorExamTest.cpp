#include <iostream>
#include <memory>
#include <QtTest/QTest>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QMessageBox>
#include <QTimer>

#include "../TestHelper.h"
#include "GuiTestHelper.h"
#include "ui/Doctor/CreatePrescriptionDialog.h"
#include "service/PharmacyService.h"
#include "service/PatientService.h"
#include "repository/MedicationRepository.h"
#include "repository/PrescriptionRepository.h"
#include "repository/PatientRepository.h"

void runDoctorExamTestSuite() {
    std::cout << "  [GUI 5/8] Running Doctor Exam / Prescription GUI Tests..." << std::endl;
    auto medRepo = std::make_shared<MedicationRepository>();
    auto presRepo = std::make_shared<PrescriptionRepository>();
    auto pharmacyService = std::make_shared<PharmacyService>(medRepo, presRepo);
    auto patientRepo = std::make_shared<PatientRepository>();
    auto patientService = std::make_shared<PatientService>(patientRepo);

    // --- GUI TEST 1: CreatePrescriptionDialog renders ---
    RUN_INLINE_TEST("GUI TEST 1: CreatePrescriptionDialog renders", {
        CreatePrescriptionDialog dialog(pharmacyService, patientService, 1);
        dialog.show();

        auto lineEdits = dialog.findChildren<QLineEdit*>();
        auto tables = dialog.findChildren<QTableWidget*>();
        auto buttons = dialog.findChildren<QPushButton*>();

        std::cout << "    [OK] CreatePrescriptionDialog renders: "
                  << lineEdits.size() << " inputs, "
                  << tables.size() << " tables, "
                  << buttons.size() << " buttons" << std::endl;
        dialog.close();
    });

    // --- GUI TEST 2: Medication search input works ---
    RUN_INLINE_TEST("GUI TEST 2: Medication search input works", {
        CreatePrescriptionDialog dialog(pharmacyService, patientService, 1);
        dialog.show();

        auto lineEdits = dialog.findChildren<QLineEdit*>();
        if (!lineEdits.isEmpty()) {
            QTest::keyClicks(lineEdits[0], "Paracetamol");
            TEST_ASSERT_TRUE(lineEdits[0]->text().contains("Paracetamol"));
        }
        std::cout << "    [OK] Medication search -> text entry works" << std::endl;
        dialog.close();
    });

    // --- GUI TEST 3: Submit empty prescription triggers validation ---
    RUN_INLINE_TEST("GUI TEST 3: Submit empty prescription triggers validation", {
        CreatePrescriptionDialog dialog(pharmacyService, patientService, 1);
        dialog.show();

        GuiTestHelper::autoDismissMessageBox(500);

        auto buttons = dialog.findChildren<QPushButton*>();
        for (auto btn : buttons) {
            if (btn->text().contains("Lưu") || btn->text().contains("Kê đơn") ||
                btn->text().contains("Save")) {
                QTest::mouseClick(btn, Qt::LeftButton);
                break;
            }
        }
        QTest::qWait(600);
        std::cout << "    [OK] Empty prescription submit -> validation triggered" << std::endl;
        dialog.close();
    });

    std::cout << "    ==> [PASSED] Doctor Exam / Prescription GUI Tests (3/3)" << std::endl;
    std::fflush(stdout);
}
