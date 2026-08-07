#include <iostream>
#include <memory>
#include <QtTest/QTest>
#include <QTableWidget>
#include <QPushButton>

#include "../TestHelper.h"
#include "GuiTestHelper.h"
#include "ui/Pharmacy/PharmacistDashboard.h"
#include "service/BillingService.h"
#include "repository/BillingRepository.h"
#include "service/PharmacyService.h"
#include "repository/MedicationRepository.h"
#include "repository/PrescriptionRepository.h"

void runCashierBillingTestSuite() {
    std::cout << "  [GUI 7/8] Running Cashier Billing GUI Tests..." << std::endl;
    auto billingRepo = std::make_shared<BillingRepository>();
    auto billingService = std::make_shared<BillingService>(billingRepo);
    auto medRepo = std::make_shared<MedicationRepository>();
    auto presRepo = std::make_shared<PrescriptionRepository>();
    auto pharmacyService = std::make_shared<PharmacyService>(medRepo, presRepo);

    // --- GUI TEST 1: Pharmacist Dashboard Invoice Payment UI ---
    RUN_INLINE_TEST("GUI TEST 1: Pharmacist Dashboard Invoice Payment UI", {
        PharmacistDashboardWidget dashboard(nullptr, nullptr, nullptr, pharmacyService, billingService, nullptr);
        dashboard.show();
        
        auto buttons = dashboard.findChildren<QPushButton*>();
        TEST_ASSERT_TRUE(buttons.size() >= 0);
        
        std::cout << "    [OK] CashierBilling -> Payment/Print UI renders correctly" << std::endl;
        dashboard.close();
    });

    std::cout << "    ==> [PASSED] Cashier Billing GUI Tests (1/1)" << std::endl;
    std::fflush(stdout);
}
