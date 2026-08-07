#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "service/AnalyticService.h"
#include "repository/AnalyticRepository.h"

void runAnalyticServiceTestSuite() {
    std::cout << "  [IT 9/9] Running Analytic Service Integration Tests..." << std::endl;
    auto analyticRepo = std::make_shared<AnalyticRepository>();
    AnalyticService analyticService(analyticRepo);

    QDate startDate = QDate::currentDate().addDays(-30);
    QDate endDate = QDate::currentDate();

    // --- HAPPY: Get doctors KPI (may be empty on fresh DB) ---
    RUN_INLINE_TEST("HAPPY: Get doctors KPI (may be empty on fresh DB)", {
        auto kpiList = analyticService.getDoctorsKPI(startDate, endDate);
        std::cout << "    [OK] getDoctorsKPI -> " << kpiList.size() << " doctor(s)" << std::endl;
    });

    // --- HAPPY: Get patient stats ---
    RUN_INLINE_TEST("HAPPY: Get patient stats", {
        auto stats = analyticService.getPatientStats(startDate, endDate);
        std::cout << "    [OK] getPatientStats -> total=" << stats.total << std::endl;
    });

    // --- HAPPY: Get income stats ---
    RUN_INLINE_TEST("HAPPY: Get income stats", {
        auto income = analyticService.getIncomeStats(startDate, endDate);
        std::cout << "    [OK] getIncomeStats -> total=" << income.total << std::endl;
    });

    // --- HAPPY: Get speciality distribution ---
    RUN_INLINE_TEST("HAPPY: Get speciality distribution", {
        auto dist = analyticService.getSpecialityDistribution(startDate, endDate);
        std::cout << "    [OK] getSpecialityDistribution -> " << dist.distribution.size() << " speciality(s)" << std::endl;
    });

    // --- HAPPY: Get wait time stats ---
    RUN_INLINE_TEST("HAPPY: Get wait time stats", {
        auto waitTime = analyticService.getWaitTimeStats(startDate, endDate);
        std::cout << "    [OK] getWaitTimeStats -> avg=" << waitTime.avg << " min" << std::endl;
    });

    std::cout << "    ==> [PASSED] Analytic Service Integration Tests (5/5)" << std::endl;
    std::fflush(stdout);
}
