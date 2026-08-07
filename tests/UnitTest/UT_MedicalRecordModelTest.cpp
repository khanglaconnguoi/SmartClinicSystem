#include <iostream>
#include <cstdio>
#include "../TestHelper.h"
#include <cmath>
#include "model/MedicalRecord.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_MedicalRecord_CalculateBMI_ValidVitals_ReturnsCorrectValue() {
    // Arrange
    MedicalRecord record(1 /*patientId*/, 2 /*doctorId*/);
    VitalSigns vitals;
    vitals.weight = 70.0;
    vitals.height = 175.0; // 1.75m
    vitals.temperature = 36.6;
    vitals.heartRate = 75;
    record.setVitals(vitals);
    
    // Act
    double bmi = record.calculateBMI();
    
    // Assert
    TEST_ASSERT_TRUE(bmi > 0.0);
    TEST_ASSERT_TRUE(std::abs(bmi - 22.86) < 0.1);
}

void UT_MedicalRecord_IsComplete_AllRequiredFieldsPresent_ReturnsTrue() {
    // Arrange
    MedicalRecord record(1, 2);
    record.setChiefComplaint("Sốt cao, ho kéo dài");
    
    VitalSigns vitals;
    vitals.temperature = 38.5;
    vitals.heartRate = 85;
    vitals.weight = 65.0;
    vitals.height = 170.0;
    record.setVitals(vitals);

    QList<Diagnosis> diagnoses;
    Diagnosis diag;
    diag.description = "Viêm phế quản cấp";
    diag.severity = "MODERATE";
    diagnoses.append(diag);
    record.setDiagnoses(diagnoses);

    // Act
    bool complete = record.isComplete();

    // Assert
    TEST_ASSERT_TRUE(complete == true);
}

// ---------------------------------------------------------
// EDGE CASE TESTS
// ---------------------------------------------------------
void UT_MedicalRecord_CalculateBMI_MissingVitalSigns_ReturnsZero() {
    // Arrange
    MedicalRecord record(1, 2);
    
    // Act
    double bmi = record.calculateBMI();
    
    // Assert
    TEST_ASSERT_TRUE(bmi == 0.0);
}

void UT_MedicalRecord_IsComplete_MissingChiefComplaint_ReturnsFalse() {
    // Arrange
    MedicalRecord record(1, 2);
    VitalSigns vitals;
    vitals.weight = 65.0;
    vitals.height = 170.0;
    record.setVitals(vitals);
    
    QList<Diagnosis> diagnoses;
    Diagnosis diag;
    diag.description = "Viêm phế quản cấp";
    diagnoses.append(diag);
    record.setDiagnoses(diagnoses);
    
    // Note: chiefComplaint is intentionally empty
    
    // Act
    bool complete = record.isComplete();

    // Assert
    TEST_ASSERT_TRUE(complete == false);
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_MedicalRecord_CalculateBMI_NegativeHeight_ReturnsZeroToPreventCrash() {
    // Arrange
    MedicalRecord record(1, 2);
    VitalSigns vitals;
    vitals.weight = 70.0;
    vitals.height = -10.0; // Negative height
    record.setVitals(vitals);
    
    // Act
    double bmi = record.calculateBMI();
    
    // Assert
    TEST_ASSERT_TRUE(bmi == 0.0);
}

void UT_MedicalRecord_IsComplete_NegativeWeight_ReturnsFalse() {
    // Arrange
    MedicalRecord record(1, 2);
    record.setChiefComplaint("Sốt cao");
    
    VitalSigns vitals;
    vitals.weight = -5.0; // Negative weight invalidates completeness
    vitals.height = 170.0;
    record.setVitals(vitals);
    
    QList<Diagnosis> diagnoses;
    Diagnosis diag;
    diag.description = "Sốt";
    diagnoses.append(diag);
    record.setDiagnoses(diagnoses);
    
    // Act
    bool complete = record.isComplete();

    // Assert
    TEST_ASSERT_TRUE(complete == false);
}


void runMedicalRecordModelTestSuite() {
    std::cout << "  [Suite 5/7] Running Medical Record & BMI Unit Tests..." << std::endl;
    RUN_TEST_CASE(UT_MedicalRecord_CalculateBMI_ValidVitals_ReturnsCorrectValue);
    RUN_TEST_CASE(UT_MedicalRecord_IsComplete_AllRequiredFieldsPresent_ReturnsTrue);
    RUN_TEST_CASE(UT_MedicalRecord_CalculateBMI_MissingVitalSigns_ReturnsZero);
    RUN_TEST_CASE(UT_MedicalRecord_IsComplete_MissingChiefComplaint_ReturnsFalse);
    RUN_TEST_CASE(UT_MedicalRecord_CalculateBMI_NegativeHeight_ReturnsZeroToPreventCrash);
    RUN_TEST_CASE(UT_MedicalRecord_IsComplete_NegativeWeight_ReturnsFalse);
    std::cout << "    ==> [PASSED] All Medical Record Test Cases (6/6)" << std::endl;
    std::fflush(stdout);
}
