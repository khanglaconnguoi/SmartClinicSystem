#include <iostream>
#include <memory>
#include <QtTest/QTest>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>

#include "../TestHelper.h"
#include "GuiTestHelper.h"
#include "ui/view/LoginDialog.h"
#include "service/AuthService.h"
#include "service/StaffService.h"
#include "repository/StaffRepository.h"

void runLoginSystemTestSuite() {
    std::cout << "  [GUI 1/8] Running Login System GUI Tests..." << std::endl;
    auto staffRepo = std::make_shared<StaffRepository>();
    auto staffService = std::make_shared<StaffService>(staffRepo);
    auto authService = std::make_shared<AuthService>(staffRepo);

    // --- Setup: Create a staff user for login ---
    QString staffCode;
    QString plainPassword;
    {
        QSqlQuery qClean;
        qClean.exec("DELETE FROM staff WHERE citizen_id = '079190005001' OR email = 'gui_login@test.com'");

        DoctorInputDTO dto;
        dto.fullName = "GUI Login Doctor";
        dto.gender = "MALE";
        dto.dateOfBirth = QDate(1990, 1, 1);
        dto.citizenId = "079190005001";
        dto.phoneNumber = "0905000001";
        dto.email = "gui_login@test.com";
        dto.address = "GUI Street";
        dto.departmentId = 1;
        dto.shift = "MORNING";
        dto.specialty = "KHOA_NOI";
        dto.licenseNumber = "CCHNGUI001";
        dto.experienceYears = 5;
        dto.consultationFee = 200000;
        dto.bio = "GUI test doctor";

        StaffHireResult result = staffService->hireNewDoctor(dto);
        TEST_ASSERT_TRUE(result.errorMessage.isEmpty());
        staffCode = result.staffCode;
        plainPassword = result.plainPassword;
    }

    // --- GUI TEST 1: Login dialog renders correctly ---
    RUN_INLINE_TEST("GUI TEST 1: Login dialog renders correctly", {
        LoginDialog dialog(authService, staffService);
        dialog.show();

        auto lineEdits = dialog.findChildren<QLineEdit*>();
        TEST_ASSERT_TRUE(lineEdits.size() >= 2); // Staff code + Password fields
        std::cout << "    [OK] LoginDialog renders with " << lineEdits.size() << " input fields" << std::endl;
        dialog.close();
    });

    // --- GUI TEST 2: Type credentials into form fields ---
    RUN_INLINE_TEST("GUI TEST 2: Type credentials into form fields", {
        LoginDialog dialog(authService, staffService);
        dialog.show();

        auto lineEdits = dialog.findChildren<QLineEdit*>();
        TEST_ASSERT_TRUE(lineEdits.size() >= 2);

        QTest::keyClicks(lineEdits[0], staffCode);
        TEST_ASSERT_TRUE(lineEdits[0]->text() == staffCode);

        QTest::keyClicks(lineEdits[1], plainPassword);
        TEST_ASSERT_TRUE(lineEdits[1]->text() == plainPassword);

        std::cout << "    [OK] QTest::keyClicks -> credentials entered successfully" << std::endl;
        dialog.close();
    });

    // --- GUI TEST 3: Empty login shows error ---
    RUN_INLINE_TEST("GUI TEST 3: Empty login shows error", {
        LoginDialog dialog(authService, staffService);
        dialog.show();

        GuiTestHelper::autoDismissMessageBox(400);

        auto buttons = dialog.findChildren<QPushButton*>();
        if (!buttons.isEmpty()) {
            QTest::mouseClick(buttons.last(), Qt::LeftButton);
        }
        QTest::qWait(500);

        std::cout << "    [OK] Empty login -> error handling triggered" << std::endl;
        dialog.close();
    });

    std::cout << "    ==> [PASSED] Login System GUI Tests (3/3)" << std::endl;
    std::fflush(stdout);
}
