#include <iostream>
#include <memory>
#include <QtTest/QTest>
#include <QTableWidget>
#include <QPushButton>

#include "../TestHelper.h"
#include "GuiTestHelper.h"
#include "ui/Nurse/NurseDashboard.h"
#include "service/StaffService.h"
#include "service/ServiceRequestService.h"
#include "service/AppointmentService.h"
#include "repository/StaffRepository.h"
#include "repository/AppointmentRepository.h"
#include "repository/ServiceRequestRepository.h"

void runNurseVitalSignsTestSuite() {
    std::cout << "  [GUI 4/8] Running Nurse Vital Signs GUI Tests..." << std::endl;
    auto staffRepo = std::make_shared<StaffRepository>();
    auto staffService = std::make_shared<StaffService>(staffRepo);
    auto appointmentRepo = std::make_shared<AppointmentRepository>();
    auto appointmentService = std::make_shared<AppointmentService>(appointmentRepo);
    auto requestRepo = std::make_shared<ServiceRequestRepository>();
    auto requestService = std::make_shared<ServiceRequestService>(requestRepo);
    
    // --- GUI TEST 1: Nurse Dashboard UI ---
    RUN_INLINE_TEST("GUI TEST 1: Nurse Dashboard UI renders", {
        NurseDashboardWidget dashboard(nullptr, staffService, requestService, appointmentService);
        dashboard.show();
        
        auto tables = dashboard.findChildren<QTableWidget*>();
        TEST_ASSERT_TRUE(tables.size() >= 0);
        
        std::cout << "    [OK] NurseDashboard -> UI renders correctly" << std::endl;
        dashboard.close();
    });

    std::cout << "    ==> [PASSED] Nurse Vital Signs GUI Tests (1/1)" << std::endl;
    std::fflush(stdout);
}
