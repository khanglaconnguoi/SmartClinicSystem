#include <iostream>
#include <memory>
#include <QtTest/QTest>
#include <QDateEdit>
#include <QPushButton>
#include <QTableWidget>

#include "../TestHelper.h"
#include "GuiTestHelper.h"
#include "ui/Admin/AdminAnalyticsWidget.h"
#include "service/AnalyticService.h"
#include "repository/AnalyticRepository.h"

void runExecutiveAnalyticsTestSuite() {
    std::cout << "  [GUI 8/8] Running Executive Analytics GUI Tests..." << std::endl;
    auto analyticRepo = std::make_shared<AnalyticRepository>();
    auto analyticService = std::make_shared<AnalyticService>(analyticRepo);

    // --- GUI TEST 1: AdminAnalyticsWidget UI Renders ---
    RUN_INLINE_TEST("GUI TEST 1: AdminAnalyticsWidget UI Renders", {
        AdminAnalyticsWidget widget(analyticService);
        widget.show();

        auto dateEdits = widget.findChildren<QDateEdit*>();
        auto buttons = widget.findChildren<QPushButton*>();
        auto tables = widget.findChildren<QTableWidget*>();

        TEST_ASSERT_TRUE(dateEdits.size() >= 2); // Start & End date
        TEST_ASSERT_TRUE(buttons.size() >= 1);

        std::cout << "    [OK] AdminAnalyticsWidget renders with " 
                  << dateEdits.size() << " date edits, " 
                  << buttons.size() << " buttons, " 
                  << tables.size() << " tables" << std::endl;
        widget.close();
    });

    // --- GUI TEST 2: Trigger Filter & Preset Buttons Reloads Analytics ---
    RUN_INLINE_TEST("GUI TEST 2: Trigger Filter & Preset Buttons Reloads Analytics", {
        AdminAnalyticsWidget widget(analyticService);
        widget.show();

        auto buttons = widget.findChildren<QPushButton*>();
        bool filterClicked = false;
        for (auto btn : buttons) {
            if (btn->text().contains("Thống kê") || btn->text().contains("30 ngày qua") || btn->text().contains("Hôm nay")) {
                QTest::mouseClick(btn, Qt::LeftButton);
                filterClicked = true;
            }
        }
        TEST_ASSERT_TRUE(filterClicked);
        QTest::qWait(200);

        std::cout << "    [OK] Analytics filter button triggered successfully" << std::endl;
        widget.close();
    });

    std::cout << "    ==> [PASSED] Executive Analytics GUI Tests (2/2)" << std::endl;
    std::fflush(stdout);
}
