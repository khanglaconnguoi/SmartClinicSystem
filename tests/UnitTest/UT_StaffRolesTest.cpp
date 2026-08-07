#include <iostream>
#include "../TestHelper.h"
#include "model/Admin.h"
#include "model/Nurse.h"
#include "model/Receptionist.h"
#include "model/doctor.h"
#include "model/Pharmacist.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_StaffRoles_Admin_HasFullAccess() {
    // Arrange
    QPixmap avatar;
    Admin admin(1, "ADM-001", "hash", "Admin User", avatar,
                UserRole::Admin, true, false);

    // Act & Assert
    TEST_ASSERT_TRUE(admin.getDisplayRole() == "Administrator");
    TEST_ASSERT_TRUE(admin.getRole() == UserRole::Admin);
    TEST_ASSERT_TRUE(admin.canAccess("Dashboard") == true);
    TEST_ASSERT_TRUE(admin.canAccess("Staff") == true);
    TEST_ASSERT_TRUE(admin.canAccess("AnyModule") == true); // Admin can access everything
    
    QStringList menu = admin.getMenuItems();
    TEST_ASSERT_TRUE(menu.contains("Dashboard"));
    TEST_ASSERT_TRUE(menu.contains("Staff"));
    TEST_ASSERT_TRUE(menu.contains("Reports"));
    TEST_ASSERT_TRUE(menu.contains("Settings"));
}

void UT_StaffRoles_Nurse_HasLimitedAccess() {
    // Arrange
    QPixmap avatar;
    Nurse nurse(2, "NUR-001", "hash", "Nurse Tran", avatar,
                UserRole::Nurse, true, false, "Senior", "CCHN-N001");

    // Act & Assert
    TEST_ASSERT_TRUE(nurse.getDisplayRole() == "Nurse");
    TEST_ASSERT_TRUE(nurse.getRole() == UserRole::Nurse);
    TEST_ASSERT_TRUE(nurse.getNurseLevel() == "Senior");
    TEST_ASSERT_TRUE(nurse.getCertification() == "CCHN-N001");
    
    QStringList menu = nurse.getMenuItems();
    TEST_ASSERT_TRUE(menu.contains("Dashboard"));
    TEST_ASSERT_TRUE(menu.contains("Patients"));
    TEST_ASSERT_TRUE(nurse.canAccess("Patients") == true);
    TEST_ASSERT_TRUE(nurse.canAccess("Staff") == false); // Nurse cannot manage staff
}

void UT_StaffRoles_Receptionist_HasAppointmentAccess() {
    // Arrange
    QPixmap avatar;
    Receptionist receptionist(3, "REC-001", "hash", "Reception Le", avatar,
                              UserRole::Receptionist, true, false);

    // Act & Assert
    TEST_ASSERT_TRUE(receptionist.getDisplayRole() == "Receptionist");
    TEST_ASSERT_TRUE(receptionist.getRole() == UserRole::Receptionist);
    
    QStringList menu = receptionist.getMenuItems();
    TEST_ASSERT_TRUE(menu.contains("Dashboard"));
    TEST_ASSERT_TRUE(menu.contains("Appointments"));
    TEST_ASSERT_TRUE(menu.contains("Patients"));
    TEST_ASSERT_TRUE(menu.contains("Billing"));
    TEST_ASSERT_TRUE(receptionist.canAccess("Appointments") == true);
    TEST_ASSERT_TRUE(receptionist.canAccess("Reports") == false);
}

// ---------------------------------------------------------
// EDGE CASE TESTS
// ---------------------------------------------------------
void UT_StaffRoles_Polymorphism_AllRolesReturnUniqueMenus() {
    // Arrange
    QPixmap avatar;
    Admin admin(1, "ADM-001", "h", "A", avatar, UserRole::Admin, true, false);
    Doctor doctor(2, "DOC-001", "h", "D", avatar, UserRole::Doctor, true, false,
                  "KHOA_NOI", "CCHN-D001", 5, 200000, "Bio");
    Nurse nurse(3, "NUR-001", "h", "N", avatar, UserRole::Nurse, true, false,
                "Junior", "CCHN-N001");
    Receptionist rec(4, "REC-001", "h", "R", avatar, UserRole::Receptionist, true, false);
    Pharmacist pharm(5, "PHA-001", "h", "P", avatar, UserRole::Pharmacist, true, false,
                     "CCHN-P001", "DUOC_LAM_SANG", 3);

    // Act
    QStringList adminMenu = admin.getMenuItems();
    QStringList doctorMenu = doctor.getMenuItems();
    QStringList nurseMenu = nurse.getMenuItems();
    QStringList recMenu = rec.getMenuItems();
    QStringList pharmMenu = pharm.getMenuItems();

    // Assert - each role should have different menus
    TEST_ASSERT_TRUE(adminMenu != doctorMenu);
    TEST_ASSERT_TRUE(doctorMenu != nurseMenu);
    TEST_ASSERT_TRUE(nurseMenu != recMenu);
    TEST_ASSERT_TRUE(recMenu != pharmMenu);
}

void UT_StaffRoles_ToPublicProfile_ReturnsCorrectData() {
    // Arrange
    QPixmap avatar;
    Admin admin(10, "ADM-010", "hash", "Nguyen Van A", avatar,
                UserRole::Admin, true, false);

    // Act
    auto profile = admin.toPublicProfile();

    // Assert
    TEST_ASSERT_TRUE(profile != nullptr);
    TEST_ASSERT_TRUE(profile->staffId == 10);
    TEST_ASSERT_TRUE(profile->fullName == "Nguyen Van A");
    TEST_ASSERT_TRUE(profile->displayRole == "Administrator");
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_StaffRoles_Nurse_CannotAccessStaffModule() {
    // Arrange
    QPixmap avatar;
    Nurse nurse(1, "NUR-001", "hash", "Nurse", avatar,
                UserRole::Nurse, true, false, "Junior", "CCHN");

    // Act
    bool canAccessStaff = nurse.canAccess("Staff");
    bool canAccessSettings = nurse.canAccess("Settings");
    bool canAccessReports = nurse.canAccess("Reports");

    // Assert
    TEST_ASSERT_TRUE(canAccessStaff == false);
    TEST_ASSERT_TRUE(canAccessSettings == false);
    TEST_ASSERT_TRUE(canAccessReports == false);
}

void UT_StaffRoles_Receptionist_CannotAccessPharmacy() {
    // Arrange
    QPixmap avatar;
    Receptionist rec(1, "REC-001", "hash", "Rec", avatar,
                     UserRole::Receptionist, true, false);

    // Act & Assert
    TEST_ASSERT_TRUE(rec.canAccess("Pharmacy") == false);
    TEST_ASSERT_TRUE(rec.canAccess("Staff") == false);
}

void runStaffRolesTestSuite() {
    std::cout << "  [Suite 8/11] Running Staff Roles & Polymorphism Tests..." << std::endl;
    RUN_TEST_CASE(UT_StaffRoles_Admin_HasFullAccess);
    RUN_TEST_CASE(UT_StaffRoles_Nurse_HasLimitedAccess);
    RUN_TEST_CASE(UT_StaffRoles_Receptionist_HasAppointmentAccess);
    RUN_TEST_CASE(UT_StaffRoles_Polymorphism_AllRolesReturnUniqueMenus);
    RUN_TEST_CASE(UT_StaffRoles_ToPublicProfile_ReturnsCorrectData);
    RUN_TEST_CASE(UT_StaffRoles_Nurse_CannotAccessStaffModule);
    RUN_TEST_CASE(UT_StaffRoles_Receptionist_CannotAccessPharmacy);
    std::cout << "    ==> [PASSED] All Staff Roles Test Cases (7/7)" << std::endl;
    std::fflush(stdout);
}
