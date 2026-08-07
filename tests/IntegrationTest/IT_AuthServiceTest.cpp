#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "service/AuthService.h"
#include "service/StaffService.h"
#include "service/UserSession.h"
#include "repository/StaffRepository.h"

void runAuthServiceTestSuite() {
    std::cout << "  [IT 2/9] Running Auth Service Integration Tests..." << std::endl;
    auto staffRepo = std::make_shared<StaffRepository>();
    AuthService authService(staffRepo);
    StaffService staffService(staffRepo);

    // --- Setup: Create a staff member for login tests ---
    QString staffCode;
    QString plainPassword;
    {
        DoctorInputDTO dto;
        dto.fullName = "Auth Test Doctor";
        dto.gender = "MALE";
        dto.dateOfBirth = QDate(1990, 1, 1);
        dto.citizenId = "079190001100";
        dto.phoneNumber = "0902100001";
        dto.email = "auth_test@clinic.com";
        dto.address = "Auth Street";
        dto.departmentId = 1;
        dto.shift = "MORNING";
        dto.specialty = "KHOA_NOI";
        dto.licenseNumber = "CCHNAUTH001";
        dto.experienceYears = 3;
        dto.consultationFee = 150000;
        dto.bio = "Auth integration test";

        StaffHireResult hireResult = staffService.hireNewDoctor(dto);
        TEST_ASSERT_TRUE(hireResult.errorMessage.isEmpty());
        staffCode = hireResult.staffCode;
        plainPassword = hireResult.plainPassword;
        std::cout << "    [OK] Created staff for auth tests: " << staffCode.toStdString() << std::endl;
    }

    // --- HAPPY: Login with correct credentials ---
    RUN_INLINE_TEST("HAPPY: Login with correct credentials", {
        LoginResult result = authService.login(staffCode, plainPassword);
        TEST_ASSERT_TRUE(result.isLoginSuccess == true);
        TEST_ASSERT_TRUE(UserSession::getInstance().isLoggedIn() == true);
        TEST_ASSERT_TRUE(UserSession::getInstance().getCurrentRole() == UserRole::Doctor);
        std::cout << "    [OK] login (correct credentials) -> success" << std::endl;
        authService.logout();
        TEST_ASSERT_TRUE(UserSession::getInstance().isLoggedIn() == false);
    });

    // --- NEGATIVE: Login with wrong password ---
    RUN_INLINE_TEST("NEGATIVE: Login with wrong password", {
        LoginResult result = authService.login(staffCode, "WrongPassword123!");
        TEST_ASSERT_TRUE(result.isLoginSuccess == false);
        TEST_ASSERT_TRUE(UserSession::getInstance().isLoggedIn() == false);
        std::cout << "    [OK] login (wrong password) -> rejected" << std::endl;
    });

    // --- NEGATIVE: Login with non-existent staff code ---
    RUN_INLINE_TEST("NEGATIVE: Login with non-existent staff code", {
        LoginResult result = authService.login("NONEXISTENT-999", "SomePassword");
        TEST_ASSERT_TRUE(result.isLoginSuccess == false);
        TEST_ASSERT_TRUE(UserSession::getInstance().isLoggedIn() == false);
        std::cout << "    [OK] login (non-existent code) -> rejected" << std::endl;
    });

    std::cout << "    ==> [PASSED] Auth Service Integration Tests (3/3)" << std::endl;
    std::fflush(stdout);
}
