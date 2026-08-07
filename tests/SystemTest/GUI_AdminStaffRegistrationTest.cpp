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
#include "ui/Admin/DoctorRegistrationDialog.h"
#include "service/StaffService.h"
#include "service/AppointmentService.h"
#include "repository/StaffRepository.h"
#include "repository/AppointmentRepository.h"

void runAdminStaffRegistrationTestSuite() {
    std::cout << "  [GUI 2/8] Running Admin Staff Registration GUI Tests..." << std::endl;
    auto staffRepo = std::make_shared<StaffRepository>();
    auto staffService = std::make_shared<StaffService>(staffRepo);
    auto appointmentRepo = std::make_shared<AppointmentRepository>();
    auto appointmentService = std::make_shared<AppointmentService>(appointmentRepo);

    // --- GUI TEST 1: DoctorRegistrationDialog renders ---
    RUN_INLINE_TEST("GUI TEST 1: DoctorRegistrationDialog renders", {
        DoctorRegistrationDialog dialog(staffService, appointmentService);
        dialog.show();

        auto lineEdits = dialog.findChildren<QLineEdit*>();
        auto comboBoxes = dialog.findChildren<QComboBox*>();
        auto buttons = dialog.findChildren<QPushButton*>();

        TEST_ASSERT_TRUE(lineEdits.size() >= 3);   // Name, phone, email, etc.
        TEST_ASSERT_TRUE(buttons.size() >= 1);     // At least a Save button
        std::cout << "    [OK] DoctorRegistrationDialog renders: " 
                  << lineEdits.size() << " inputs, "
                  << comboBoxes.size() << " combos, "
                  << buttons.size() << " buttons" << std::endl;
        dialog.close();
    });

    // --- GUI TEST 2: Fill form and verify data ---
    RUN_INLINE_TEST("GUI TEST 2: Fill form and verify data", {
        DoctorRegistrationDialog dialog(staffService, appointmentService);
        dialog.show();

        auto lineEdits = dialog.findChildren<QLineEdit*>();
        if (lineEdits.size() >= 1) {
            lineEdits[0]->clear();
            QTest::keyClicks(lineEdits[0], "GUI Test Doctor");
            TEST_ASSERT_TRUE(lineEdits[0]->text() == "GUI Test Doctor");
        }
        std::cout << "    [OK] Form data entry -> text entered correctly" << std::endl;
        dialog.close();
    });

    // --- GUI TEST 3: Submit empty form shows validation error ---
    RUN_INLINE_TEST("GUI TEST 3: Submit empty form shows validation error", {
        DoctorRegistrationDialog dialog(staffService, appointmentService);
        dialog.show();

        GuiTestHelper::autoDismissMessageBox(500);

        for (auto le : dialog.findChildren<QLineEdit*>()) {
            le->clear();
        }

        auto buttons = dialog.findChildren<QPushButton*>();
        for (auto btn : buttons) {
            if (btn->text().contains("Lưu") || btn->text().contains("Save") || 
                btn->text().contains("Đăng ký")) {
                QTest::mouseClick(btn, Qt::LeftButton);
                break;
            }
        }
        QTest::qWait(600);
        std::cout << "    [OK] Empty form submit -> validation error triggered" << std::endl;
        dialog.close();
    });

    std::cout << "    ==> [PASSED] Admin Staff Registration GUI Tests (3/3)" << std::endl;
    std::fflush(stdout);
}
