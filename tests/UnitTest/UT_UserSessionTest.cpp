#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "service/UserSession.h"
#include "model/doctor.h"
#include "model/Admin.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_UserSession_Singleton_ReturnsSameInstance() {
    // Arrange & Act
    UserSession& session1 = UserSession::getInstance();
    UserSession& session2 = UserSession::getInstance();
    
    // Assert
    TEST_ASSERT_TRUE(&session1 == &session2);
}

void UT_UserSession_SetCurrentAccount_IsLoggedIn() {
    // Arrange
    UserSession& session = UserSession::getInstance();
    session.clear(); // Đảm bảo clean state
    QPixmap avatar;
    auto doctor = std::make_shared<Doctor>(
        1, "DOC-001", "hash", "Dr. Test", avatar,
        UserRole::Doctor, true, false, "KHOA_NOI", "CCHN-001", 5, 150000, "Bio");
    
    // Act
    session.setCurrentAccount(doctor);
    
    // Assert
    TEST_ASSERT_TRUE(session.isLoggedIn() == true);
    TEST_ASSERT_TRUE(session.getCurrentRole().has_value());
    TEST_ASSERT_TRUE(session.getCurrentRole().value() == UserRole::Doctor);
    TEST_ASSERT_TRUE(session.isDoctor() == true);
    TEST_ASSERT_TRUE(session.isAdmin() == false);
    
    // Cleanup
    session.clear();
}

void UT_UserSession_Clear_LogsOut() {
    // Arrange
    UserSession& session = UserSession::getInstance();
    QPixmap avatar;
    auto admin = std::make_shared<Admin>(
        1, "ADM-001", "hash", "Admin", avatar,
        UserRole::Admin, true, false);
    session.setCurrentAccount(admin);
    TEST_ASSERT_TRUE(session.isLoggedIn() == true);
    
    // Act
    session.clear();
    
    // Assert
    TEST_ASSERT_TRUE(session.isLoggedIn() == false);
    TEST_ASSERT_TRUE(session.getCurrentAccount() == nullptr);
    TEST_ASSERT_TRUE(session.getCurrentRole().has_value() == false);
}

// ---------------------------------------------------------
// EDGE CASE TESTS
// ---------------------------------------------------------
void UT_UserSession_RoleChecks_AllReturnCorrectly() {
    // Arrange
    UserSession& session = UserSession::getInstance();
    session.clear();
    QPixmap avatar;
    auto admin = std::make_shared<Admin>(
        1, "ADM-001", "hash", "Admin", avatar,
        UserRole::Admin, true, false);
    
    // Act
    session.setCurrentAccount(admin);
    
    // Assert — only isAdmin should return true
    TEST_ASSERT_TRUE(session.isAdmin() == true);
    TEST_ASSERT_TRUE(session.isDoctor() == false);
    TEST_ASSERT_TRUE(session.isNurse() == false);
    TEST_ASSERT_TRUE(session.isReceptionist() == false);
    TEST_ASSERT_TRUE(session.isPharmacist() == false);
    
    // Cleanup
    session.clear();
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_UserSession_NotLoggedIn_RoleIsNullopt() {
    // Arrange
    UserSession& session = UserSession::getInstance();
    session.clear();
    
    // Act & Assert
    TEST_ASSERT_TRUE(session.isLoggedIn() == false);
    TEST_ASSERT_TRUE(session.getCurrentRole().has_value() == false);
    TEST_ASSERT_TRUE(session.isAdmin() == false);
    TEST_ASSERT_TRUE(session.isDoctor() == false);
}

void UT_UserSession_ClearTwice_NoError() {
    // Arrange
    UserSession& session = UserSession::getInstance();
    session.clear();
    
    // Act — clearing again should not crash
    session.clear();
    
    // Assert
    TEST_ASSERT_TRUE(session.isLoggedIn() == false);
}

void runUserSessionTestSuite() {
    std::cout << "  [Suite 11/11] Running UserSession Singleton Tests..." << std::endl;
    RUN_TEST_CASE(UT_UserSession_Singleton_ReturnsSameInstance);
    RUN_TEST_CASE(UT_UserSession_SetCurrentAccount_IsLoggedIn);
    RUN_TEST_CASE(UT_UserSession_Clear_LogsOut);
    RUN_TEST_CASE(UT_UserSession_RoleChecks_AllReturnCorrectly);
    RUN_TEST_CASE(UT_UserSession_NotLoggedIn_RoleIsNullopt);
    RUN_TEST_CASE(UT_UserSession_ClearTwice_NoError);
    std::cout << "    ==> [PASSED] All UserSession Test Cases (6/6)" << std::endl;
    std::fflush(stdout);
}
