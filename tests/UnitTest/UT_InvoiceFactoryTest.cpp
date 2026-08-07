#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "factory/OutPatientInvoiceFactory.h"
#include "factory/InPatientInvoiceFactory.h"
#include "factory/EmergencyInvoiceFactory.h"
#include "model/OutPatientInvoice.h"
#include "model/InPatientInvoice.h"
#include "model/EmergencyInvoice.h"

// ---------------------------------------------------------
// HAPPY PATH TESTS
// ---------------------------------------------------------
void UT_InvoiceFactory_OutPatientFactory_CreatesOutPatientInvoice() {
    // Arrange
    OutPatientInvoiceFactory factory;
    
    // Act
    auto invoice = factory.createInvoice(100, 150000.0, 250000.0, QDate::currentDate());
    
    // Assert
    TEST_ASSERT_TRUE(invoice != nullptr);
    double total = invoice->calculate();
    TEST_ASSERT_TRUE(total == 400000.0);
}

void UT_InvoiceFactory_InPatientFactory_CreatesInPatientInvoice() {
    // Arrange
    InPatientInvoiceFactory factory;
    
    // Act
    auto invoice = factory.createInvoice(101, 500000.0, 300000.0, QDate::currentDate());
    
    // Assert
    TEST_ASSERT_TRUE(invoice != nullptr);
    double total = invoice->calculate();
    TEST_ASSERT_TRUE(total == 800000.0);
}

void UT_InvoiceFactory_EmergencyFactory_CreatesEmergencyInvoice() {
    // Arrange
    EmergencyInvoiceFactory factory;
    
    // Act
    auto invoice = factory.createInvoice(102, 700000.0, 400000.0, QDate::currentDate());
    
    // Assert
    TEST_ASSERT_TRUE(invoice != nullptr);
    double total = invoice->calculate();
    TEST_ASSERT_TRUE(total == 1100000.0);
}

// ---------------------------------------------------------
// EDGE CASE TESTS
// ---------------------------------------------------------
void UT_InvoiceFactory_Polymorphism_AllFactoriesReturnNonNull() {
    // Arrange
    std::unique_ptr<IInvoiceFactory> factories[] = {
        std::make_unique<OutPatientInvoiceFactory>(),
        std::make_unique<InPatientInvoiceFactory>(),
        std::make_unique<EmergencyInvoiceFactory>()
    };
    
    // Act & Assert
    for (auto& factory : factories) {
        auto invoice = factory->createInvoice(1, 100.0, 200.0, QDate::currentDate());
        TEST_ASSERT_TRUE(invoice != nullptr);
        TEST_ASSERT_TRUE(invoice->calculate() > 0);
    }
}

void UT_InvoiceFactory_ZeroFees_ReturnsZeroTotal() {
    // Arrange
    OutPatientInvoiceFactory factory;
    
    // Act
    auto invoice = factory.createInvoice(100, 0.0, 0.0, QDate::currentDate());
    
    // Assert
    TEST_ASSERT_TRUE(invoice != nullptr);
    TEST_ASSERT_TRUE(invoice->calculate() == 0.0);
}

// ---------------------------------------------------------
// NEGATIVE CASE TESTS
// ---------------------------------------------------------
void UT_InvoiceFactory_NegativeFees_ReturnsNegativeTotal() {
    // Arrange — hệ thống hiện tại không validate phí âm
    OutPatientInvoiceFactory factory;
    
    // Act
    auto invoice = factory.createInvoice(100, -50000.0, 100000.0, QDate::currentDate());
    
    // Assert — hành vi hiện tại: tổng = phí khám + phí thuốc
    TEST_ASSERT_TRUE(invoice != nullptr);
    double total = invoice->calculate();
    TEST_ASSERT_TRUE(total == 50000.0);
}

void runInvoiceFactoryTestSuite() {
    std::cout << "  [Suite 9/11] Running Invoice Factory Pattern Tests..." << std::endl;
    RUN_TEST_CASE(UT_InvoiceFactory_OutPatientFactory_CreatesOutPatientInvoice);
    RUN_TEST_CASE(UT_InvoiceFactory_InPatientFactory_CreatesInPatientInvoice);
    RUN_TEST_CASE(UT_InvoiceFactory_EmergencyFactory_CreatesEmergencyInvoice);
    RUN_TEST_CASE(UT_InvoiceFactory_Polymorphism_AllFactoriesReturnNonNull);
    RUN_TEST_CASE(UT_InvoiceFactory_ZeroFees_ReturnsZeroTotal);
    RUN_TEST_CASE(UT_InvoiceFactory_NegativeFees_ReturnsNegativeTotal);
    std::cout << "    ==> [PASSED] All Invoice Factory Test Cases (6/6)" << std::endl;
    std::fflush(stdout);
}
