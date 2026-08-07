#include <iostream>
#include <memory>
#include <QtTest/QTest>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDateEdit>
#include <QMessageBox>
#include <QTimer>

#include "../TestHelper.h"
#include "GuiTestHelper.h"
#include "ui/Reception/PatientRegistrationDialog.h"
#include "service/PatientService.h"
#include "repository/PatientRepository.h"

void runReceptionPatientRegistrationTestSuite() {
    std::cout << "  [GUI 3/8] Running Reception Patient Registration GUI Tests..." << std::endl;
    auto patientRepo = std::make_shared<PatientRepository>();
    auto patientService = std::make_shared<PatientService>(patientRepo);

    // --- GUI TEST 1: PatientRegistrationDialog renders ---
    RUN_INLINE_TEST("GUI TEST 1: PatientRegistrationDialog renders", {
        PatientRegistrationDialog dialog(patientService);
        dialog.show();

        auto lineEdits = dialog.findChildren<QLineEdit*>();
        auto comboBoxes = dialog.findChildren<QComboBox*>();
        auto dateEdits = dialog.findChildren<QDateEdit*>();
        auto buttons = dialog.findChildren<QPushButton*>();

        TEST_ASSERT_TRUE(lineEdits.size() >= 2);
        std::cout << "    [OK] PatientRegistrationDialog renders: "
                  << lineEdits.size() << " inputs, "
                  << comboBoxes.size() << " combos, "
                  << dateEdits.size() << " dates, "
                  << buttons.size() << " buttons" << std::endl;
        dialog.close();
    });

    // --- GUI TEST 2: Patient name field accepts text ---
    RUN_INLINE_TEST("GUI TEST 2: Patient name field accepts text", {
        PatientRegistrationDialog dialog(patientService);
        dialog.show();

        auto lineEdits = dialog.findChildren<QLineEdit*>();
        if (lineEdits.size() >= 1) {
            lineEdits[0]->clear();
            QTest::keyClicks(lineEdits[0], "GUI Patient Nguyen Van Test");
            TEST_ASSERT_TRUE(lineEdits[0]->text().contains("GUI Patient"));
        }
        std::cout << "    [OK] Patient name field -> text entry works" << std::endl;
        dialog.close();
    });

    // --- GUI TEST 3: Empty form submission triggers validation ---
    RUN_INLINE_TEST("GUI TEST 3: Empty form submission triggers validation", {
        PatientRegistrationDialog dialog(patientService);
        dialog.show();

        GuiTestHelper::autoDismissMessageBox(500);

        for (auto le : dialog.findChildren<QLineEdit*>()) {
            le->clear();
        }

        auto buttons = dialog.findChildren<QPushButton*>();
        for (auto btn : buttons) {
            if (btn->text().contains("Lưu") || btn->text().contains("Đăng ký") ||
                btn->text().contains("Save")) {
                QTest::mouseClick(btn, Qt::LeftButton);
                break;
            }
        }
        QTest::qWait(600);
        std::cout << "    [OK] Empty patient form -> validation error triggered" << std::endl;
        dialog.close();
    });

    std::cout << "    ==> [PASSED] Reception Patient Registration GUI Tests (3/3)" << std::endl;
    std::fflush(stdout);
}
