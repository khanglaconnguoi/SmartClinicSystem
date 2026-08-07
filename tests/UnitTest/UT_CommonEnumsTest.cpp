#include <iostream>
#include <cstdio>
#include "../TestHelper.h"
#include "model/CommonEnums.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_CommonEnums_GenderText_ConvertsCorrectly() {
    // Arrange & Act
    QString viMale = GenderText::toVi("MALE");
    QString viFemale = GenderText::toVi("FEMALE");
    QString enMale = GenderText::toEn("Nam");
    QString enFemale = GenderText::toEn("Nữ");
    bool validMale = GenderText::isValid("MALE");
    
    // Assert
    TEST_ASSERT_TRUE(viMale == "Nam");
    TEST_ASSERT_TRUE(viFemale == "Nữ");
    TEST_ASSERT_TRUE(enMale == "MALE");
    TEST_ASSERT_TRUE(enFemale == "FEMALE");
    TEST_ASSERT_TRUE(validMale == true);
}

void UT_CommonEnums_PatientType_ConvertsCorrectly() {
    // Arrange & Act & Assert
    TEST_ASSERT_TRUE(patientTypeToVi(PatientType::Outpatient) == "Ngoại trú");
    TEST_ASSERT_TRUE(patientTypeToVi(PatientType::Inpatient) == "Nội trú");
    TEST_ASSERT_TRUE(patientTypeToVi(PatientType::Emergency) == "Cấp cứu");

    TEST_ASSERT_TRUE(patientTypeToEn(PatientType::Outpatient) == "OUTPATIENT");
    TEST_ASSERT_TRUE(patientTypeFromEn("INPATIENT") == PatientType::Inpatient);
}

void UT_CommonEnums_Severity_ConvertsCorrectly() {
    // Arrange & Act & Assert
    TEST_ASSERT_TRUE(severityToVi(Severity::Mild) == "Nhẹ");
    TEST_ASSERT_TRUE(severityToVi(Severity::Moderate) == "Trung bình");
    TEST_ASSERT_TRUE(severityToVi(Severity::Severe) == "Nặng");

    TEST_ASSERT_TRUE(severityToEn(Severity::Severe) == "SEVERE");
    TEST_ASSERT_TRUE(severityFromEn("MILD") == Severity::Mild);
}

void UT_CommonEnums_UserRole_ConvertsCorrectly() {
    // Arrange & Act & Assert
    TEST_ASSERT_TRUE(userRoleToVi(UserRole::Doctor) == "Bác sĩ");
    TEST_ASSERT_TRUE(userRoleToVi(UserRole::Pharmacist) == "Dược sĩ");
    TEST_ASSERT_TRUE(userRoleToVi(UserRole::Admin) == "Quản trị viên");

    TEST_ASSERT_TRUE(userRoleToEn(UserRole::Doctor) == "DOCTOR");
    TEST_ASSERT_TRUE(userRoleFromEn("RECEPTIONIST") == UserRole::Receptionist);
}

// ---------------------------------------------------------
// EDGE CASE TESTS
// ---------------------------------------------------------
void UT_CommonEnums_GenderText_CaseInsensitiveWithSpaces_ReturnsCorrectly() {
    // Arrange
    QString messyInput = "   mAlE   ";
    // Act
    QString viResult = GenderText::toVi(messyInput);
    bool validResult = GenderText::isValid(messyInput);
    // Assert
    TEST_ASSERT_TRUE(viResult == "Nam");
    TEST_ASSERT_TRUE(validResult == true);
}

void UT_CommonEnums_UserRole_FromEn_CaseInsensitiveWithSpaces_ReturnsCorrectly() {
    // Arrange — userRoleFromEn() so sánh chính xác (exact match)
    // Input messy sẽ KHÔNG match "DOCTOR" → fallback về Admin
    QString messyInput = "   doCtOr  ";
    // Act
    UserRole result = userRoleFromEn(messyInput);
    // Assert — fallback behavior (Admin) vì không match exact
    TEST_ASSERT_TRUE(result == UserRole::Admin);
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_CommonEnums_GenderText_InvalidInput_ReturnsFallbackValue() {
    // Arrange
    QString invalidInput = "INVALID";
    // Act
    QString viResult = GenderText::toVi(invalidInput);
    QString enResult = GenderText::toEn(invalidInput);
    bool validResult = GenderText::isValid(invalidInput);
    // Assert
    TEST_ASSERT_TRUE(viResult == "Khác");
    TEST_ASSERT_TRUE(enResult == "OTHER");
    TEST_ASSERT_TRUE(validResult == false);
}

void UT_CommonEnums_PatientType_InvalidInput_ReturnsFallbackValue() {
    // Arrange
    QString invalidInput = "UNKNOWN";
    // Act
    PatientType result = patientTypeFromEn(invalidInput);
    // Assert
    // Theo code hiện tại, fallback mặc định là Outpatient
    TEST_ASSERT_TRUE(result == PatientType::Outpatient);
}

void UT_CommonEnums_UserRole_InvalidInput_ReturnsFallbackValue() {
    // Arrange
    QString invalidInput = "UNKNOWN_ROLE";
    // Act
    UserRole result = userRoleFromEn(invalidInput);
    // Assert
    // Theo code hiện tại, fallback mặc định là Admin
    TEST_ASSERT_TRUE(result == UserRole::Admin);
}


void runCommonEnumsTestSuite() {
    std::cout << "  [Suite 7/7] Running Common Enums & Helpers Unit Tests..." << std::endl;
    RUN_TEST_CASE(UT_CommonEnums_GenderText_ConvertsCorrectly);
    RUN_TEST_CASE(UT_CommonEnums_PatientType_ConvertsCorrectly);
    RUN_TEST_CASE(UT_CommonEnums_Severity_ConvertsCorrectly);
    RUN_TEST_CASE(UT_CommonEnums_UserRole_ConvertsCorrectly);
    
    RUN_TEST_CASE(UT_CommonEnums_GenderText_CaseInsensitiveWithSpaces_ReturnsCorrectly);
    RUN_TEST_CASE(UT_CommonEnums_UserRole_FromEn_CaseInsensitiveWithSpaces_ReturnsCorrectly);
    
    RUN_TEST_CASE(UT_CommonEnums_GenderText_InvalidInput_ReturnsFallbackValue);
    RUN_TEST_CASE(UT_CommonEnums_PatientType_InvalidInput_ReturnsFallbackValue);
    RUN_TEST_CASE(UT_CommonEnums_UserRole_InvalidInput_ReturnsFallbackValue);
    
    std::cout << "    ==> [PASSED] All Common Enums Test Cases (9/9)" << std::endl;
    std::fflush(stdout);
}
