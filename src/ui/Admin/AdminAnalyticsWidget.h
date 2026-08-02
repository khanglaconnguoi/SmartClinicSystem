#pragma once

#include <QWidget>
#include <QDate>
#include <QDateEdit>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <memory>

#include "service/AnalyticService.h"

class AdminAnalyticsWidget : public QWidget {
    Q_OBJECT

public:
    explicit AdminAnalyticsWidget(std::shared_ptr<AnalyticService> analyticService, QWidget *parent = nullptr);
    ~AdminAnalyticsWidget() override = default;

    void loadAnalyticsData();

private slots:
    void onFilterClicked();
    void onPresetTodayClicked();
    void onPreset7DaysClicked();
    void onPreset30DaysClicked();
    void onPresetThisMonthClicked();

private:
    void setupUI();
    QFrame* createSummaryCard(const QString &title, QLabel* &valueLabel, QLabel* &subLabel, const QString &accentColor);
    void setPresetRange(const QDate &start, const QDate &end);

    std::shared_ptr<AnalyticService> m_analyticService;

    // Filter controls
    QDateEdit *m_startDateEdit = nullptr;
    QDateEdit *m_endDateEdit = nullptr;
    QPushButton *m_btnFilter = nullptr;
    QPushButton *m_btnToday = nullptr;
    QPushButton *m_btn7Days = nullptr;
    QPushButton *m_btn30Days = nullptr;
    QPushButton *m_btnThisMonth = nullptr;

    // Summary Card Value Labels
    QLabel *m_lblTotalPatientsValue = nullptr;
    QLabel *m_lblTotalPatientsSub = nullptr;
    QLabel *m_lblTotalIncomeValue = nullptr;
    QLabel *m_lblTotalIncomeSub = nullptr;
    QLabel *m_lblAvgWaitTimeValue = nullptr;
    QLabel *m_lblAvgWaitTimeSub = nullptr;
    QLabel *m_lblAvgKpiValue = nullptr;
    QLabel *m_lblAvgKpiSub = nullptr;

    // Detailed Views
    QTabWidget *m_tabWidget = nullptr;
    QTableWidget *m_tblDoctorKpi = nullptr;
    QTableWidget *m_tblSpeciality = nullptr;
    QTableWidget *m_tblDailyTrend = nullptr;
};
