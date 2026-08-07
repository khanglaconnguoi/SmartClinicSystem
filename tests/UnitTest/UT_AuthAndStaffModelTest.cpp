#include <iostream>
#include "../TestHelper.h"
#include "model/doctor.h"
#include "model/Pharmacist.h"
#include "ext/bcrypt.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_Auth_VerifyPassword_CorrectPassword_ReturnsTrue() {
    QString rawPassword = "DoctorPassword123!";
    std::string hash = bcrypt::generateHash(rawPassword.toStdString(), 10);
    QString passwordHash = QString::fromStdString(hash);

    QPixmap avatar; 
    Doctor doctor(1, "DOC-001", passwordHash, "Dr. Nguyen", avatar,
                  UserRole::Doctor, true, false, "KHOA_NOI", "CCHN-12345", 10, 150000, "Bio text");

    bool isMatch = doctor.verifyPassword(rawPassword);
    TEST_ASSERT_TRUE(isMatch == true);
}

void UT_Auth_RolePermissions_Doctor_GetsCorrectDisplayRoleAndMenu() {
    QPixmap avatar;
    Doctor doctor(1, "DOC-001", "hash", "Dr. Nguyen", avatar,
                  UserRole::Doctor, true, false, "KHOA_NOI", "CCHN-12345", 10, 150000, "Bio text");

    TEST_ASSERT_TRUE(doctor.getAccountType() == AccountType::Staff);
    TEST_ASSERT_TRUE(doctor.getRole() == UserRole::Doctor);
    TEST_ASSERT_TRUE(doctor.getDisplayRole() == "Doctor KHOA_NOI");

    QStringList menu = doctor.getMenuItems();
    TEST_ASSERT_TRUE(menu.contains("Dashboard"));
    TEST_ASSERT_TRUE(menu.contains("Appointments"));
}

void UT_Auth_RolePermissions_Pharmacist_GetsCorrectDisplayRoleAndMenu() {
    QPixmap avatar;
    Pharmacist pharmacist(2, "PHA-001", "hash", "Pharm. Tran", avatar,
                         UserRole::Pharmacist, true, false, "CCHN-67890", "DUOC_LAM_SANG", 5);

    TEST_ASSERT_TRUE(pharmacist.getRole() == UserRole::Pharmacist);
    TEST_ASSERT_TRUE(pharmacist.getDisplayRole() == "Dược sĩ — DUOC_LAM_SANG");

    QStringList menu = pharmacist.getMenuItems();
    TEST_ASSERT_TRUE(menu.contains("Dashboard"));
    TEST_ASSERT_TRUE(menu.contains("Pharmacy"));
}

// ---------------------------------------------------------
// BOUNDARY / EDGE CASE TESTS
// ---------------------------------------------------------
void UT_Auth_VerifyPassword_EmptyPasswordInput_ReturnsFalse() {
    QString rawPassword = "DoctorPassword123!";
    std::string hash = bcrypt::generateHash(rawPassword.toStdString(), 10);
    QString passwordHash = QString::fromStdString(hash);

    QPixmap avatar;
    Doctor doctor(1, "DOC-001", passwordHash, "Dr. Nguyen", avatar,
                  UserRole::Doctor, true, false, "KHOA_NOI", "CCHN-12345", 10, 150000, "Bio text");

    bool isMatch = doctor.verifyPassword("");
    TEST_ASSERT_TRUE(isMatch == false);
}

void UT_Auth_DoctorRole_PrescriptionEligibility_ChecksLicenseAndStatus() {
    QPixmap avatar;
    Doctor doctorActive(1, "DOC-001", "hash", "Dr. Active", avatar,
                        UserRole::Doctor, true /*active*/, false, "KHOA_NOI", "CCHN-12345", 10, 150000, "");

    TEST_ASSERT_TRUE(doctorActive.isEligibleToPrescribe() == true);

    Doctor doctorNoLicense(2, "DOC-002", "hash", "Dr. NoLicense", avatar,
                           UserRole::Doctor, true /*active*/, false, "KHOA_NOI", "" /*empty license*/, 0, 150000, "");

    TEST_ASSERT_TRUE(doctorNoLicense.isEligibleToPrescribe() == false);
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_Auth_VerifyPassword_WrongPassword_ReturnsFalse() {
    QString rawPassword = "DoctorPassword123!";
    std::string hash = bcrypt::generateHash(rawPassword.toStdString(), 10);
    QString passwordHash = QString::fromStdString(hash);

    QPixmap avatar;
    Doctor doctor(1, "DOC-001", passwordHash, "Dr. Nguyen", avatar,
                  UserRole::Doctor, true, false, "KHOA_NOI", "CCHN-12345", 10, 150000, "Bio text");

    bool isMatch = doctor.verifyPassword("WrongPassword");
    TEST_ASSERT_TRUE(isMatch == false);
}

void UT_Auth_VerifyPassword_EmptyHashInDatabase_ReturnsFalse() {
    QPixmap avatar;
    Doctor doctor(1, "DOC-001", "", "Dr. Nguyen", avatar,
                  UserRole::Doctor, true, false, "KHOA_NOI", "CCHN-12345", 10, 150000, "Bio text");

    bool isMatch = doctor.verifyPassword("AnyPassword");
    TEST_ASSERT_TRUE(isMatch == false);
}

void runAuthAndStaffModelTestSuite() {
    std::cout << "  [Suite 6/7] Running Auth, Staff & RBAC Unit Tests..." << std::endl;
    RUN_TEST_CASE(UT_Auth_VerifyPassword_CorrectPassword_ReturnsTrue);
    RUN_TEST_CASE(UT_Auth_RolePermissions_Doctor_GetsCorrectDisplayRoleAndMenu);
    RUN_TEST_CASE(UT_Auth_RolePermissions_Pharmacist_GetsCorrectDisplayRoleAndMenu);
    
    RUN_TEST_CASE(UT_Auth_VerifyPassword_EmptyPasswordInput_ReturnsFalse);
    RUN_TEST_CASE(UT_Auth_DoctorRole_PrescriptionEligibility_ChecksLicenseAndStatus);
    
    RUN_TEST_CASE(UT_Auth_VerifyPassword_WrongPassword_ReturnsFalse);
    RUN_TEST_CASE(UT_Auth_VerifyPassword_EmptyHashInDatabase_ReturnsFalse);
    
    std::cout << "    ==> [PASSED] All Auth & Staff Test Cases (7/7)" << std::endl;
    std::fflush(stdout);
}
