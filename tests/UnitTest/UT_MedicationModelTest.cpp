#include <iostream>
#include <cstdio>
#include "../TestHelper.h"
#include "model/Medication.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_Medication_IsAvailable_RequestedAmountLesserThanStock_ReturnsTrue() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"ANTIBIOTICS"};
    QDate futureExpiry = QDate::currentDate().addMonths(6);
    Medication med(1, "Amoxicillin 500mg", ingredients, categories, "TABLET",
                   5000.0, 100 /*stock*/, 20 /*minStock*/, 30 /*reorder*/,
                   futureExpiry, "Pharmaco", "Description", true /*isActive*/);

    bool available = med.isAvailable(50);
    TEST_ASSERT_TRUE(available == true);
}

void UT_Medication_StockStatus_SufficientStock_EvaluatesCorrectly() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"ANALGESICS"};
    QDate futureExpiry = QDate::currentDate().addMonths(12);
    Medication medNormal(1, "Paracetamol 500mg", ingredients, categories, "TABLET",
                      2000.0, 50 /*stock*/, 20 /*minStock*/, 30 /*reorder*/,
                      futureExpiry, "Pharmaco", "Description", true);

    TEST_ASSERT_TRUE(medNormal.isLowStock() == false);
    TEST_ASSERT_TRUE(medNormal.isCriticalStock() == false);
}

void UT_Medication_IsEligibleForPrescription_ActiveValidAndStocked_ReturnsTrue() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"ANTIBIOTICS"};
    QDate validExpiry = QDate::currentDate().addMonths(6);
    Medication medEligible(2, "Eligible Med", ingredients, categories, "TABLET",
                          5000.0, 100, 10, 20, validExpiry, "Pharmaco", "", true /*active*/);

    bool eligible = medEligible.isEligibleForPrescription(50);
    TEST_ASSERT_TRUE(eligible == true);
}

// ---------------------------------------------------------
// BOUNDARY CASE TESTS
// ---------------------------------------------------------
void UT_Medication_IsAvailable_RequestedAmountEqualsStock_ReturnsTrue() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"ANTIBIOTICS"};
    QDate futureExpiry = QDate::currentDate().addMonths(6);
    Medication med(1, "Amoxicillin 500mg", ingredients, categories, "TABLET",
                   5000.0, 100 /*stock*/, 20 /*minStock*/, 30 /*reorder*/,
                   futureExpiry, "Pharmaco", "Description", true /*isActive*/);

    bool available = med.isAvailable(100);
    TEST_ASSERT_TRUE(available == true);
}

void UT_Medication_StockStatus_AtThreshold_EvaluatesCorrectly() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"ANALGESICS"};
    QDate futureExpiry = QDate::currentDate().addMonths(12);
    Medication medLow(1, "Paracetamol 500mg", ingredients, categories, "TABLET",
                      2000.0, 30 /*stock matches reorder*/, 20 /*minStock*/, 30 /*reorder*/,
                      futureExpiry, "Pharmaco", "Description", true);

    TEST_ASSERT_TRUE(medLow.isLowStock() == true);
    TEST_ASSERT_TRUE(medLow.isCriticalStock() == false);

    Medication medCritical(2, "Paracetamol 500mg", ingredients, categories, "TABLET",
                          2000.0, 20 /*stock matches minStock*/, 20 /*minStock*/, 30 /*reorder*/,
                          futureExpiry, "Pharmaco", "Description", true);

    TEST_ASSERT_TRUE(medCritical.isCriticalStock() == true);
}

void UT_Medication_ExpiryStatus_PastAndSoon_EvaluatesCorrectly() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"VITAMINS"};

    QDate pastExpiry = QDate::currentDate().addDays(-5);
    Medication medExpired(1, "Vitamin C", ingredients, categories, "TABLET",
                         1000.0, 100, 10, 20, pastExpiry, "Pharmaco", "", true);

    QDate expiringSoonDate = QDate::currentDate().addDays(10);
    Medication medExpiring(2, "Vitamin D", ingredients, categories, "TABLET",
                          1000.0, 100, 10, 20, expiringSoonDate, "Pharmaco", "", true);

    TEST_ASSERT_TRUE(medExpired.isExpired() == true);
    TEST_ASSERT_TRUE(medExpiring.isExpired() == false);
    TEST_ASSERT_TRUE(medExpiring.isExpiringSoon(30) == true);
}

void UT_Medication_ExpiryStatus_ExpiredToday_IsExpiredTrue() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"VITAMINS"};

    QDate todayExpiry = QDate::currentDate();
    Medication medToday(1, "Vitamin B", ingredients, categories, "TABLET",
                        1000.0, 100, 10, 20, todayExpiry, "Pharmaco", "", true);

    TEST_ASSERT_TRUE(medToday.isExpired() == true);
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_Medication_IsAvailable_RequestedAmountGreaterThanStock_ReturnsFalse() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"ANTIBIOTICS"};
    QDate futureExpiry = QDate::currentDate().addMonths(6);
    Medication med(1, "Amoxicillin 500mg", ingredients, categories, "TABLET",
                   5000.0, 100 /*stock*/, 20 /*minStock*/, 30 /*reorder*/,
                   futureExpiry, "Pharmaco", "Description", true /*isActive*/);

    bool available = med.isAvailable(101);
    TEST_ASSERT_TRUE(available == false);
}

void UT_Medication_IsAvailable_ZeroOrNegativeQuantity_ReturnsFalse() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"ANTIBIOTICS"};
    QDate futureExpiry = QDate::currentDate().addMonths(6);
    Medication med(1, "Amoxicillin 500mg", ingredients, categories, "TABLET",
                   5000.0, 100, 20, 30, futureExpiry, "Pharmaco", "Description", true);

    TEST_ASSERT_TRUE(med.isAvailable(0) == false);
    TEST_ASSERT_TRUE(med.isAvailable(-5) == false);
}

void UT_Medication_Constructor_NegativePriceAndStock_AllowsCreationButMayBeFlawed() {
    QList<MedicationIngredientDTO> ingredients;
    QList<QString> categories = {"ANTIBIOTICS"};
    QDate futureExpiry = QDate::currentDate().addMonths(6);
    Medication med(1, "Amoxicillin", ingredients, categories, "TABLET",
                   -5000.0 /* negative price */, -10 /* negative stock */, 
                   20, 30, futureExpiry, "Pharmaco", "Desc", true);

    TEST_ASSERT_TRUE(med.getUnitPrice() == -5000.0);
    TEST_ASSERT_TRUE(med.getStockQuantity() == -10);
    TEST_ASSERT_TRUE(med.isAvailable(5) == false);
}

void runMedicationModelTestSuite() {
    std::cout << "  [Suite 4/7] Running Medication & Inventory Unit Tests..." << std::endl;
    RUN_TEST_CASE(UT_Medication_IsAvailable_RequestedAmountLesserThanStock_ReturnsTrue);
    RUN_TEST_CASE(UT_Medication_StockStatus_SufficientStock_EvaluatesCorrectly);
    RUN_TEST_CASE(UT_Medication_IsEligibleForPrescription_ActiveValidAndStocked_ReturnsTrue);
    
    RUN_TEST_CASE(UT_Medication_IsAvailable_RequestedAmountEqualsStock_ReturnsTrue);
    RUN_TEST_CASE(UT_Medication_StockStatus_AtThreshold_EvaluatesCorrectly);
    RUN_TEST_CASE(UT_Medication_ExpiryStatus_PastAndSoon_EvaluatesCorrectly);
    RUN_TEST_CASE(UT_Medication_ExpiryStatus_ExpiredToday_IsExpiredTrue);
    
    RUN_TEST_CASE(UT_Medication_IsAvailable_RequestedAmountGreaterThanStock_ReturnsFalse);
    RUN_TEST_CASE(UT_Medication_IsAvailable_ZeroOrNegativeQuantity_ReturnsFalse);
    RUN_TEST_CASE(UT_Medication_Constructor_NegativePriceAndStock_AllowsCreationButMayBeFlawed);
    
    std::cout << "    ==> [PASSED] All Medication Model Test Cases (10/10)" << std::endl;
    std::fflush(stdout);
}
