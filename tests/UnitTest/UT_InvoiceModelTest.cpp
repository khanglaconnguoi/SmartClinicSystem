#include <iostream>
#include <cstdio>
#include "../TestHelper.h"
#include <memory>
#include "model/OutPatientInvoice.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_Invoice_OutPatientInvoice_Calculate_ValidFees_ReturnsCorrectTotal() {
    double consultationFee = 150000.0;
    double medicationFee = 250000.0;
    OutPatientInvoice invoice(100, consultationFee, medicationFee, QDate::currentDate());
    
    double total = invoice.calculate();
    TEST_ASSERT_TRUE(total == 400000.0);
}

void UT_Invoice_SummaryString_ContainsInvoiceDetails() {
    OutPatientInvoice invoice(100, 150000.0, 250000.0, QDate::currentDate());
    QString summary = invoice.toSummaryString();
    
    TEST_ASSERT_TRUE(summary.contains("Hóa đơn ngoại trú"));
    TEST_ASSERT_TRUE(summary.contains("400000"));
}

void UT_Invoice_PolymorphicTypeCheck_ReturnsCorrectType() {
    std::unique_ptr<Invoice> outInv = std::make_unique<OutPatientInvoice>(100, 100, 100, QDate::currentDate());

    TEST_ASSERT_TRUE(outInv->getInvoiceType() == "OUTPATIENT");
}

// ---------------------------------------------------------
// BOUNDARY CASE TESTS
// ---------------------------------------------------------
void UT_Invoice_Calculate_ZeroFees_ReturnsZero() {
    OutPatientInvoice invoice(103, 0.0, 0.0, QDate::currentDate());
    double total = invoice.calculate();
    TEST_ASSERT_TRUE(total == 0.0);
}

void UT_Invoice_Calculate_LargeAmounts_CalculatesWithoutOverflow() {
    OutPatientInvoice invoice(105, 50000000.0, 150000000.0, QDate::currentDate());
    double total = invoice.calculate();
    TEST_ASSERT_TRUE(total == 200000000.0);
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_Invoice_Calculate_NegativeFees_ReturnsNegativeTotal() {
    OutPatientInvoice invoice(104, -50000.0, -100000.0, QDate::currentDate());
    double total = invoice.calculate();
    TEST_ASSERT_TRUE(total == -150000.0);
}

void runInvoiceModelTestSuite() {
    std::cout << "  [Suite 3/7] Running Invoice Model & Polymorphism Unit Tests..." << std::endl;
    RUN_TEST_CASE(UT_Invoice_OutPatientInvoice_Calculate_ValidFees_ReturnsCorrectTotal);
    RUN_TEST_CASE(UT_Invoice_SummaryString_ContainsInvoiceDetails);
    RUN_TEST_CASE(UT_Invoice_PolymorphicTypeCheck_ReturnsCorrectType);
    
    RUN_TEST_CASE(UT_Invoice_Calculate_ZeroFees_ReturnsZero);
    RUN_TEST_CASE(UT_Invoice_Calculate_LargeAmounts_CalculatesWithoutOverflow);
    
    RUN_TEST_CASE(UT_Invoice_Calculate_NegativeFees_ReturnsNegativeTotal);
    
    std::cout << "    ==> [PASSED] All Invoice Model Test Cases (6/6)" << std::endl;
    std::fflush(stdout);
}
