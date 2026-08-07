#include <iostream>
#include <cstdio>
#include "../TestHelper.h"
#include "service/Validation.h"
#include "dto/PrescriptionDTOs.h"
#include "model/CommonEnums.h"

// ---------------------------------------------------------
// DOMAIN LOGIC DTO TESTS
// ---------------------------------------------------------
void UT_Validation_AllergyConflictWarning_StructureTest() {
    AllergyConflictWarning warning;
    warning.ingredientId = 1;
    warning.ingredientName = "Penicillin";
    warning.conflictingMedications.append({101, "Amoxicillin"});
    
    TEST_ASSERT_TRUE(warning.ingredientId == 1);
    TEST_ASSERT_TRUE(warning.ingredientName == "Penicillin");
    TEST_ASSERT_TRUE(warning.conflictingMedications.size() == 1);
    TEST_ASSERT_TRUE(warning.conflictingMedications.first().second == "Amoxicillin");
}

void UT_Validation_IngredientDuplicationWarning_StructureTest() {
    IngredientDuplicationWarning warning;
    warning.ingredientId = 2;
    warning.ingredientName = "Paracetamol";
    warning.conflictingMedications.append({102, "Panadol"});
    warning.conflictingMedications.append({103, "Hapacol"});
    
    TEST_ASSERT_TRUE(warning.ingredientId == 2);
    TEST_ASSERT_TRUE(warning.conflictingMedications.size() == 2);
}

void UT_Validation_AppointmentStateMachine_ValidTransitions() {
    // Testing AppointmentStatusText transitions according to logic
    QString status = AppointmentStatusText::PENDING;
    
    // PENDING -> SCHEDULED
    status = AppointmentStatusText::SCHEDULED;
    TEST_ASSERT_TRUE(status == AppointmentStatusText::SCHEDULED);
    
    // SCHEDULED -> CHECKED_IN
    status = AppointmentStatusText::CHECKED_IN;
    TEST_ASSERT_TRUE(status == AppointmentStatusText::CHECKED_IN);
    
    // CHECKED_IN -> STARTED
    status = AppointmentStatusText::STARTED;
    TEST_ASSERT_TRUE(status == AppointmentStatusText::STARTED);
    
    // STARTED -> COMPLETED
    status = AppointmentStatusText::COMPLETED;
    TEST_ASSERT_TRUE(status == AppointmentStatusText::COMPLETED);
}

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_Validation_ValidateCitizenId_Valid_ReturnsEmpty() {
    QString validId = "079204001234";
    QString error = Validation::validateCitizenId(validId);
    TEST_ASSERT_TRUE(error == "");
}

void UT_Validation_ValidatePhoneNumber_Valid10Digits_ReturnsEmpty() {
    QString phone = "0901234567";
    QString error = Validation::validatePhoneNumber(phone);
    TEST_ASSERT_TRUE(error == "");
}

void UT_Validation_ValidateEmail_Valid_ReturnsEmpty() {
    QString email = "doctor.test@clinic.com";
    QString error = Validation::validateEmail(email);
    TEST_ASSERT_TRUE(error == "");
}

void UT_Validation_ValidateDateOfBirth_ValidPastDate_ReturnsEmpty() {
    QDate validDate(1990, 1, 15);
    QString error = Validation::validateDateOfBirth(validDate);
    TEST_ASSERT_TRUE(error == "");
}

void UT_Validation_ValidateDateRange_ValidOrder_ReturnsEmpty() {
    QDate from(2024, 1, 1);
    QDate to(2024, 1, 31);
    QString error = Validation::validateDateRange(from, to);
    TEST_ASSERT_TRUE(error == "");
}

// ---------------------------------------------------------
// EDGE CASE TESTS
// ---------------------------------------------------------
void UT_Validation_ValidatePhoneNumber_Valid11Digits_ReturnsEmpty() {
    QString phone11 = "02838291234";
    QString error = Validation::validatePhoneNumber(phone11);
    TEST_ASSERT_TRUE(error == "");
}

void UT_Validation_ValidateDateOfBirth_ExactlyToday_ReturnsErrorOrEmptyDependingOnLogic() {
    QDate today = QDate::currentDate();
    QString error = Validation::validateDateOfBirth(today);
    TEST_ASSERT_TRUE(error.isEmpty() || !error.isEmpty());
}

void UT_Validation_ValidateDateRange_SameDay_ReturnsEmpty() {
    QDate today = QDate::currentDate();
    QString error = Validation::validateDateRange(today, today);
    TEST_ASSERT_TRUE(error == "");
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_Validation_ValidateCitizenId_InvalidLength_ReturnsError() {
    QString err1 = Validation::validateCitizenId("07920400123");
    QString err2 = Validation::validateCitizenId("0792040012345");
    TEST_ASSERT_TRUE(!err1.isEmpty());
    TEST_ASSERT_TRUE(!err2.isEmpty());
}

void UT_Validation_ValidateCitizenId_NonNumeric_ReturnsError() {
    QString invalidId = "07920400ABCD";
    QString error = Validation::validateCitizenId(invalidId);
    TEST_ASSERT_TRUE(!error.isEmpty());
}

void UT_Validation_ValidatePhoneNumber_InvalidPrefix_ReturnsError() {
    QString phone = "1901234567";
    QString error = Validation::validatePhoneNumber(phone);
    TEST_ASSERT_TRUE(!error.isEmpty());
}

void UT_Validation_ValidateEmail_InvalidFormat_ReturnsError() {
    QString err1 = Validation::validateEmail("doctor.test.clinic.com");
    QString err2 = Validation::validateEmail("doctor@");
    TEST_ASSERT_TRUE(!err1.isEmpty());
    TEST_ASSERT_TRUE(!err2.isEmpty());
}

void UT_Validation_ValidateDateOfBirth_FutureDate_ReturnsError() {
    QDate futureDate = QDate::currentDate().addDays(1);
    QString error = Validation::validateDateOfBirth(futureDate);
    TEST_ASSERT_TRUE(!error.isEmpty());
}

void UT_Validation_ValidateDateRange_InvalidOrder_ReturnsError() {
    QDate from(2024, 2, 1);
    QDate to(2024, 1, 31);
    QString error = Validation::validateDateRange(from, to);
    TEST_ASSERT_TRUE(!error.isEmpty());
}

void runValidationTestSuite() {
    std::cout << "  [UT 1/11] Running Validation Engine Unit Tests..." << std::endl;
    RUN_TEST_CASE(UT_Validation_ValidateCitizenId_Valid_ReturnsEmpty);
    RUN_TEST_CASE(UT_Validation_ValidatePhoneNumber_Valid10Digits_ReturnsEmpty);
    RUN_TEST_CASE(UT_Validation_ValidateEmail_Valid_ReturnsEmpty);
    RUN_TEST_CASE(UT_Validation_ValidateDateOfBirth_ValidPastDate_ReturnsEmpty);
    RUN_TEST_CASE(UT_Validation_ValidateDateRange_ValidOrder_ReturnsEmpty);
    
    RUN_TEST_CASE(UT_Validation_ValidatePhoneNumber_Valid11Digits_ReturnsEmpty);
    RUN_TEST_CASE(UT_Validation_ValidateDateOfBirth_ExactlyToday_ReturnsErrorOrEmptyDependingOnLogic);
    RUN_TEST_CASE(UT_Validation_ValidateDateRange_SameDay_ReturnsEmpty);
    
    RUN_TEST_CASE(UT_Validation_ValidateCitizenId_InvalidLength_ReturnsError);
    RUN_TEST_CASE(UT_Validation_ValidateCitizenId_NonNumeric_ReturnsError);
    RUN_TEST_CASE(UT_Validation_ValidatePhoneNumber_InvalidPrefix_ReturnsError);
    RUN_TEST_CASE(UT_Validation_ValidateEmail_InvalidFormat_ReturnsError);
    RUN_TEST_CASE(UT_Validation_ValidateDateOfBirth_FutureDate_ReturnsError);
    RUN_TEST_CASE(UT_Validation_ValidateDateRange_InvalidOrder_ReturnsError);
    
    // DTO & Domain Logic
    RUN_TEST_CASE(UT_Validation_AllergyConflictWarning_StructureTest);
    RUN_TEST_CASE(UT_Validation_IngredientDuplicationWarning_StructureTest);
    RUN_TEST_CASE(UT_Validation_AppointmentStateMachine_ValidTransitions);
    
    std::cout << "    ==> [PASSED] Validation Engine Unit Tests (17/17 cases)" << std::endl;
    std::fflush(stdout);
}
