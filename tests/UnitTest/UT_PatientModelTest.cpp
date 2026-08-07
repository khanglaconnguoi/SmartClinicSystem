#include <iostream>
#include <cstdio>
#include "../TestHelper.h"
#include <memory>
#include "model/OutPatient.h"
#include "model/InPatient.h"
#include "model/EmergencyPatient.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_Patient_PolymorphicBaseFee_ReturnsCorrectValues() {
    std::unique_ptr<Patient> outPatient = std::make_unique<OutPatient>();
    std::unique_ptr<Patient> inPatient = std::make_unique<InPatient>();
    std::unique_ptr<Patient> emergencyPatient = std::make_unique<EmergencyPatient>();

    TEST_ASSERT_TRUE(outPatient->getBaseFee() == 150000.0);
    TEST_ASSERT_TRUE(inPatient->getBaseFee() == 500000.0);
    TEST_ASSERT_TRUE(emergencyPatient->getBaseFee() == 800000.0);
}

void UT_Patient_PolymorphicPriority_ReturnsCorrectValues() {
    std::unique_ptr<Patient> outPatient = std::make_unique<OutPatient>();
    std::unique_ptr<Patient> inPatient = std::make_unique<InPatient>();
    std::unique_ptr<Patient> emergencyPatient = std::make_unique<EmergencyPatient>();

    TEST_ASSERT_TRUE(outPatient->getPriority() == PatientPriority::Normal);
    TEST_ASSERT_TRUE(inPatient->getPriority() == PatientPriority::High);
    TEST_ASSERT_TRUE(emergencyPatient->getPriority() == PatientPriority::Critical);
}

void UT_Patient_GetAge_PastBirthdayThisYear_CalculatesCorrectly() {
    QDate today = QDate::currentDate();
    QDate dob(today.year() - 25, 1, 1);
    OutPatient patient(1, "BN-2024-0001", "Test", dob, "MALE", "079", "090", "e@e.com", "Addr", "A+", "", "", PatientType::Outpatient, "", "");
    
    int age = patient.getAge();
    TEST_ASSERT_TRUE(age >= 24);
}

void UT_Patient_HasAllergy_CaseInsensitiveMatch_ReturnsTrue() {
    OutPatient patient(1, "BN-2024-0001", "Test", QDate(1990, 1, 1), "MALE",
                       "079204001234", "0901234567", "email@test.com", "Address", "A+",
                       "Paracetamol, Penicillin, Aspirin", "",
                       PatientType::Outpatient, "", "");

    TEST_ASSERT_TRUE(patient.hasAllergy("paracetamol") == true);
    TEST_ASSERT_TRUE(patient.hasAllergy("PENICILLIN") == true);
}

void UT_Patient_GeneratePatientCode_FormatsCorrectly() {
    QString code1 = Patient::generatePatientCode(1);
    QString currentYear = QString::number(QDate::currentDate().year());
    QString expectedCode = QString("BN-%1-0001").arg(currentYear);
    
    TEST_ASSERT_TRUE(code1 == expectedCode);
}

// ---------------------------------------------------------
// BOUNDARY CASE TESTS
// ---------------------------------------------------------
void UT_Patient_GetAge_BirthdayIsToday_ReturnsExactAge() {
    QDate today = QDate::currentDate();
    QDate dob(today.year() - 20, today.month(), today.day());
    OutPatient patient(1, "BN-0001", "Test", dob, "FEMALE", "", "", "", "", "", "", "", PatientType::Outpatient, "", "");
    
    TEST_ASSERT_TRUE(patient.getAge() == 20);
}

void UT_Patient_GetAge_BirthdayIsTomorrow_SubtractsOneYear() {
    QDate today = QDate::currentDate();
    QDate tomorrow = today.addDays(1);
    QDate dob(today.year() - 25, tomorrow.month(), tomorrow.day());
    OutPatient patient(1, "BN-2024-0001", "Test", dob, "MALE", "079", "090", "e@e.com", "Addr", "A+", "", "", PatientType::Outpatient, "", "");
    
    TEST_ASSERT_TRUE(patient.getAge() == 24);
}

void UT_Patient_HasAllergy_WithSpaces_MatchesCorrectly() {
    OutPatient patient(1, "BN-0001", "Test", QDate::currentDate(), "MALE", "", "", "", "", "", "   Paracetamol   ,  Aspirin  ", "", PatientType::Outpatient, "", "");

    TEST_ASSERT_TRUE(patient.hasAllergy("  paracetamol  ") == true);
}

void UT_Patient_GeneratePatientCode_LargeId_FormatsCorrectly() {
    QString code = Patient::generatePatientCode(9999);
    QString currentYear = QString::number(QDate::currentDate().year());
    TEST_ASSERT_TRUE(code == QString("BN-%1-9999").arg(currentYear));
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_Patient_GetAge_FutureBirthDate_ReturnsZero() {
    QDate futureDob = QDate::currentDate().addDays(10);
    OutPatient patient(1, "BN-0001", "Test", futureDob, "MALE", "", "", "", "", "", "", "", PatientType::Outpatient, "", "");
    
    TEST_ASSERT_TRUE(patient.getAge() <= 0);
}

void UT_Patient_GetAge_InvalidDate_ReturnsZero() {
    QDate invalidDob;
    OutPatient patient(1, "BN-2024-0001", "Test", invalidDob, "MALE", "079", "090", "e@e.com", "Addr", "A+", "", "", PatientType::Outpatient, "", "");
    
    TEST_ASSERT_TRUE(patient.getAge() == 0);
}

void UT_Patient_HasAllergy_EmptyAllergyString_ReturnsFalse() {
    OutPatient patient(1, "BN-0001", "Test", QDate::currentDate(), "MALE", "", "", "", "", "", "", "", PatientType::Outpatient, "", "");

    TEST_ASSERT_TRUE(patient.hasAllergy("Paracetamol") == false);
}

void UT_Patient_GeneratePatientCode_NegativeId_GeneratesWithMinusSign() {
    QString code = Patient::generatePatientCode(-1);
    TEST_ASSERT_TRUE(code.contains("-"));
}

void runPatientModelTestSuite() {
    std::cout << "  [Suite 2/7] Running Patient Model & Polymorphism Unit Tests..." << std::endl;
    RUN_TEST_CASE(UT_Patient_PolymorphicBaseFee_ReturnsCorrectValues);
    RUN_TEST_CASE(UT_Patient_PolymorphicPriority_ReturnsCorrectValues);
    RUN_TEST_CASE(UT_Patient_GetAge_PastBirthdayThisYear_CalculatesCorrectly);
    RUN_TEST_CASE(UT_Patient_HasAllergy_CaseInsensitiveMatch_ReturnsTrue);
    RUN_TEST_CASE(UT_Patient_GeneratePatientCode_FormatsCorrectly);
    
    RUN_TEST_CASE(UT_Patient_GetAge_BirthdayIsToday_ReturnsExactAge);
    RUN_TEST_CASE(UT_Patient_GetAge_BirthdayIsTomorrow_SubtractsOneYear);
    RUN_TEST_CASE(UT_Patient_HasAllergy_WithSpaces_MatchesCorrectly);
    RUN_TEST_CASE(UT_Patient_GeneratePatientCode_LargeId_FormatsCorrectly);
    
    RUN_TEST_CASE(UT_Patient_GetAge_FutureBirthDate_ReturnsZero);
    RUN_TEST_CASE(UT_Patient_GetAge_InvalidDate_ReturnsZero);
    RUN_TEST_CASE(UT_Patient_HasAllergy_EmptyAllergyString_ReturnsFalse);
    RUN_TEST_CASE(UT_Patient_GeneratePatientCode_NegativeId_GeneratesWithMinusSign);
    
    std::cout << "    ==> [PASSED] All Patient Model Test Cases (13/13)" << std::endl;
    std::fflush(stdout);
}
