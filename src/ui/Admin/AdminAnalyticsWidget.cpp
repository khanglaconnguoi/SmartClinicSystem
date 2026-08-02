#include "ui/Admin/AdminAnalyticsWidget.h"

#include <QHeaderView>
#include <QLocale>
#include <QDate>
#include <QScrollBar>
#include <algorithm>

AdminAnalyticsWidget::AdminAnalyticsWidget(std::shared_ptr<AnalyticService> analyticService, QWidget *parent)
    : QWidget(parent), m_analyticService(std::move(analyticService)) {
    setupUI();
    // Default to last 30 days
    onPreset30DaysClicked();
}

void AdminAnalyticsWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // --- Header Title ---
    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *lblTitle = new QLabel("Thống Kê & Báo Cáo Quản Lý", this);
    lblTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #172B4D;");
    
    QLabel *lblSubtitle = new QLabel("Tổng quan hiệu suất phòng khám, doanh thu và chỉ số KPI", this);
    lblSubtitle->setStyleSheet("font-size: 13px; color: #5E6C84;");

    QVBoxLayout *headerTextLayout = new QVBoxLayout();
    headerTextLayout->setSpacing(2);
    headerTextLayout->addWidget(lblTitle);
    headerTextLayout->addWidget(lblSubtitle);
    titleLayout->addLayout(headerTextLayout);
    titleLayout->addStretch();

    mainLayout->addLayout(titleLayout);

    // --- Filter Bar Frame ---
    QFrame *filterFrame = new QFrame(this);
    filterFrame->setStyleSheet(
        "QFrame { background-color: #FFFFFF; border: 1px solid #DFE1E6; border-radius: 10px; }"
    );
    QHBoxLayout *filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(15, 12, 15, 12);
    filterLayout->setSpacing(10);

    QLabel *lblFrom = new QLabel("Từ ngày:", filterFrame);
    lblFrom->setStyleSheet("border: none; font-weight: 600; color: #172B4D; font-size: 13px;");
    m_startDateEdit = new QDateEdit(QDate::currentDate().addDays(-30), filterFrame);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_startDateEdit->setFixedWidth(120);

    QLabel *lblTo = new QLabel("Đến ngày:", filterFrame);
    lblTo->setStyleSheet("border: none; font-weight: 600; color: #172B4D; font-size: 13px;");
    m_endDateEdit = new QDateEdit(QDate::currentDate(), filterFrame);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDisplayFormat("dd/MM/yyyy");
    m_endDateEdit->setFixedWidth(120);

    m_btnFilter = new QPushButton(" Thống kê", filterFrame);
    m_btnFilter->setCursor(Qt::PointingHandCursor);
    m_btnFilter->setStyleSheet(
        "QPushButton { background-color: #0052CC; color: #FFFFFF; font-weight: bold; "
        "font-size: 13px; border-radius: 6px; padding: 6px 16px; border: none; }"
        "QPushButton:hover { background-color: #0065FF; }"
        "QPushButton:pressed { background-color: #0047B3; }"
    );

    // Preset Buttons
    auto createPresetBtn = [filterFrame](const QString &text) -> QPushButton* {
        QPushButton *btn = new QPushButton(text, filterFrame);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton { background-color: #F4F5F7; color: #42526E; font-size: 12px; "
            "font-weight: 600; border-radius: 6px; padding: 6px 12px; border: 1px solid #DFE1E6; }"
            "QPushButton:hover { background-color: #DEEBFF; color: #0052CC; border-color: #B3D4FF; }"
        );
        return btn;
    };

    m_btnToday = createPresetBtn("Hôm nay");
    m_btn7Days = createPresetBtn("7 ngày qua");
    m_btn30Days = createPresetBtn("30 ngày qua");
    m_btnThisMonth = createPresetBtn("Tháng này");

    filterLayout->addWidget(lblFrom);
    filterLayout->addWidget(m_startDateEdit);
    filterLayout->addWidget(lblTo);
    filterLayout->addWidget(m_endDateEdit);
    filterLayout->addWidget(m_btnFilter);
    filterLayout->addSpacing(15);
    filterLayout->addWidget(new QLabel("|", filterFrame));
    filterLayout->addWidget(m_btnToday);
    filterLayout->addWidget(m_btn7Days);
    filterLayout->addWidget(m_btn30Days);
    filterLayout->addWidget(m_btnThisMonth);
    filterLayout->addStretch();

    mainLayout->addWidget(filterFrame);

    // --- Summary Cards (Top Grid) ---
    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(14);

    cardsLayout->addWidget(createSummaryCard("Tổng Số Lượt Khám", m_lblTotalPatientsValue, m_lblTotalPatientsSub, "#0052CC"));
    cardsLayout->addWidget(createSummaryCard("Tổng Doanh Thu", m_lblTotalIncomeValue, m_lblTotalIncomeSub, "#00875A"));
    cardsLayout->addWidget(createSummaryCard("Thời Gian Chờ TB", m_lblAvgWaitTimeValue, m_lblAvgWaitTimeSub, "#FF9900"));
    cardsLayout->addWidget(createSummaryCard("KPI Bác Sĩ TB", m_lblAvgKpiValue, m_lblAvgKpiSub, "#6554C0"));

    mainLayout->addLayout(cardsLayout);

    // --- Detailed Data Tabs ---
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #DFE1E6; background-color: #FFFFFF; border-radius: 10px; }"
        "QTabBar::tab { background-color: #F4F5F7; color: #5E6C84; font-weight: bold; font-size: 13px; "
        "padding: 10px 20px; border-top-left-radius: 8px; border-top-right-radius: 8px; margin-right: 4px; }"
        "QTabBar::tab:selected { background-color: #FFFFFF; color: #0052CC; border-bottom: 3px solid #0052CC; }"
        "QTabBar::tab:hover:!selected { background-color: #EBECF0; color: #172B4D; }"
    );

    // Helper for table styling
    auto styleTable = [](QTableWidget *tbl) {
        tbl->setAlternatingRowColors(true);
        tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
        tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tbl->setStyleSheet(
            "QTableWidget { border: none; background-color: #FFFFFF; alternate-background-color: #FAFBFC; font-size: 13px; color: #172B4D; }"
            "QHeaderView::section { background-color: #F4F5F7; color: #5E6C84; font-weight: bold; font-size: 13px; padding: 10px; border: none; border-bottom: 2px solid #DFE1E6; }"
            "QTableWidget::item { padding: 8px; border-bottom: 1px solid #EBECF0; }"
        );
        tbl->verticalHeader()->setVisible(false);
    };

    // Tab 1: Doctor KPI Table
    m_tblDoctorKpi = new QTableWidget(this);
    m_tblDoctorKpi->setColumnCount(5);
    m_tblDoctorKpi->setHorizontalHeaderLabels({"Tên Bác Sĩ", "Lượt Khám", "Doanh Thu Khám (VNĐ)", "KPI %", "Đánh Giá Hiệu Suất"});
    styleTable(m_tblDoctorKpi);
    m_tblDoctorKpi->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tblDoctorKpi->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tblDoctorKpi->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tblDoctorKpi->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tblDoctorKpi->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_tabWidget->addTab(m_tblDoctorKpi, " Hiệu Suất & KPI Bác Sĩ");

    // Tab 2: Speciality Distribution Table
    m_tblSpeciality = new QTableWidget(this);
    m_tblSpeciality->setColumnCount(3);
    m_tblSpeciality->setHorizontalHeaderLabels({"Chuyên Khoa", "Số Lượt Khám", "Tỷ Lệ Phần Trăm"});
    styleTable(m_tblSpeciality);
    m_tblSpeciality->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tblSpeciality->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tblSpeciality->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tabWidget->addTab(m_tblSpeciality, " Phân Bố Theo Chuyên Khoa");

    // Tab 3: Daily Trend Breakdown Table
    m_tblDailyTrend = new QTableWidget(this);
    m_tblDailyTrend->setColumnCount(4);
    m_tblDailyTrend->setHorizontalHeaderLabels({"Ngày", "Số Lượt Khám", "Doanh Thu Hóa Đơn (VNĐ)", "Thời Gian Chờ TB (Phút)"});
    styleTable(m_tblDailyTrend);
    m_tblDailyTrend->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tblDailyTrend->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tblDailyTrend->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tblDailyTrend->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tabWidget->addTab(m_tblDailyTrend, " Diễn Biến Theo Ngày");

    mainLayout->addWidget(m_tabWidget, 1);

    // --- Signal Connections ---
    connect(m_btnFilter, &QPushButton::clicked, this, &AdminAnalyticsWidget::onFilterClicked);
    connect(m_btnToday, &QPushButton::clicked, this, &AdminAnalyticsWidget::onPresetTodayClicked);
    connect(m_btn7Days, &QPushButton::clicked, this, &AdminAnalyticsWidget::onPreset7DaysClicked);
    connect(m_btn30Days, &QPushButton::clicked, this, &AdminAnalyticsWidget::onPreset30DaysClicked);
    connect(m_btnThisMonth, &QPushButton::clicked, this, &AdminAnalyticsWidget::onPresetThisMonthClicked);
}

QFrame* AdminAnalyticsWidget::createSummaryCard(const QString &title, QLabel* &valueLabel, QLabel* &subLabel, const QString &accentColor) {
    QFrame *card = new QFrame(this);
    card->setStyleSheet(QString(
        "QFrame { background-color: #FFFFFF; border: 1px solid #DFE1E6; border-radius: 10px; border-left: 5px solid %1; }"
    ).arg(accentColor));

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(4);

    QLabel *lblTitle = new QLabel(title, card);
    lblTitle->setStyleSheet("border: none; font-size: 13px; font-weight: 600; color: #5E6C84;");

    valueLabel = new QLabel("0", card);
    valueLabel->setStyleSheet("border: none; font-size: 24px; font-weight: bold; color: #172B4D;");

    subLabel = new QLabel("--", card);
    subLabel->setStyleSheet("border: none; font-size: 12px; color: #7A869A;");

    layout->addWidget(lblTitle);
    layout->addWidget(valueLabel);
    layout->addWidget(subLabel);

    return card;
}

void AdminAnalyticsWidget::setPresetRange(const QDate &start, const QDate &end) {
    m_startDateEdit->setDate(start);
    m_endDateEdit->setDate(end);
    loadAnalyticsData();
}

void AdminAnalyticsWidget::onPresetTodayClicked() {
    QDate today = QDate::currentDate();
    setPresetRange(today, today);
}

void AdminAnalyticsWidget::onPreset7DaysClicked() {
    QDate today = QDate::currentDate();
    setPresetRange(today.addDays(-6), today);
}

void AdminAnalyticsWidget::onPreset30DaysClicked() {
    QDate today = QDate::currentDate();
    setPresetRange(today.addDays(-29), today);
}

void AdminAnalyticsWidget::onPresetThisMonthClicked() {
    QDate today = QDate::currentDate();
    QDate startOfMonth(today.year(), today.month(), 1);
    setPresetRange(startOfMonth, today);
}

void AdminAnalyticsWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    loadAnalyticsData();
}

void AdminAnalyticsWidget::onFilterClicked() {
    loadAnalyticsData();
}

void AdminAnalyticsWidget::loadAnalyticsData() {
    if (!m_analyticService) { return; }

    QDate start = m_startDateEdit->date();
    QDate end = m_endDateEdit->date();

    if (start > end) {
        m_lblTotalPatientsValue->setText("Lỗi ngày");
        m_lblTotalIncomeValue->setText("Lỗi ngày");
        return;
    }

    QLocale viLocale(QLocale::Vietnamese, QLocale::Vietnam);

    // 1. Patient Stats
    PatientStatsDTO patientStats = m_analyticService->getPatientStats(start, end);
    m_lblTotalPatientsValue->setText(QString::number(patientStats.total));
    m_lblTotalPatientsSub->setText(QString("Trong %1 ngày thống kê").arg(start.daysTo(end) + 1));

    // 2. Income Stats
    IncomeStatsDTO incomeStats = m_analyticService->getIncomeStats(start, end);
    m_lblTotalIncomeValue->setText(viLocale.toCurrencyString(incomeStats.total, "VNĐ"));
    m_lblTotalIncomeSub->setText("Tổng thanh toán hóa đơn");

    // 3. Wait Time Stats
    WaitTimeStatsDTO waitStats = m_analyticService->getWaitTimeStats(start, end);
    m_lblAvgWaitTimeValue->setText(QString::number(waitStats.avg, 'f', 1) + " phút");
    m_lblAvgWaitTimeSub->setText("Thời gian chờ trước khi khám");

    // 4. Doctor KPI Stats
    QList<DoctorKPI> kpiList = m_analyticService->getDoctorsKPI(start, end);
    double totalKpi = 0.0;
    for (const auto &doc : kpiList) {
        totalKpi += doc.kpi;
    }
    double avgKpiPercent = kpiList.isEmpty() ? 0.0 : (totalKpi / kpiList.size()) * 100.0;
    m_lblAvgKpiValue->setText(QString::number(avgKpiPercent, 'f', 1) + " %");
    m_lblAvgKpiSub->setText(QString("Dựa trên %1 bác sĩ").arg(kpiList.size()));

    // --- Fill Tab 1: Doctor KPI Table ---
    m_tblDoctorKpi->setRowCount(0);
    for (int i = 0; i < kpiList.size(); ++i) {
        const auto &doc = kpiList[i];
        m_tblDoctorKpi->insertRow(i);

        QTableWidgetItem *itemDoctor = new QTableWidgetItem(doc.name);
        itemDoctor->setFont(QFont("Segoe UI", 10, QFont::Bold));
        m_tblDoctorKpi->setItem(i, 0, itemDoctor);

        QTableWidgetItem *itemVisits = new QTableWidgetItem(QString::number(doc.patientCount));
        itemVisits->setTextAlignment(Qt::AlignCenter);
        m_tblDoctorKpi->setItem(i, 1, itemVisits);

        QTableWidgetItem *itemIncome = new QTableWidgetItem(viLocale.toCurrencyString(doc.income, "VNĐ"));
        itemIncome->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tblDoctorKpi->setItem(i, 2, itemIncome);

        // KPI Progress bar item
        double kpiPercent = doc.kpi * 100.0;
        QProgressBar *kpiBar = new QProgressBar(m_tblDoctorKpi);
        kpiBar->setRange(0, 100);
        kpiBar->setValue(static_cast<int>(std::round(kpiPercent)));
        kpiBar->setTextVisible(true);
        kpiBar->setFormat(QString::number(kpiPercent, 'f', 1) + "%");
        kpiBar->setAlignment(Qt::AlignCenter);

        QString barColor = "#00875A"; // Green
        if (kpiPercent < 50.0) {
            barColor = "#DE350B"; // Red
        } else if (kpiPercent < 80.0) {
            barColor = "#FFAB00"; // Yellow/Orange
        }

        kpiBar->setStyleSheet(QString(
            "QProgressBar { border: 1px solid #DFE1E6; border-radius: 4px; text-align: center; font-weight: bold; color: #172B4D; background-color: #EBECF0; }"
            "QProgressBar::chunk { background-color: %1; border-radius: 3px; }"
        ).arg(barColor));

        m_tblDoctorKpi->setCellWidget(i, 3, kpiBar);

        // Evaluation status label
        QString evalText = "Xuất sắc";
        QString evalColor = "#00875A";
        if (kpiPercent < 50.0) {
            evalText = "Cần cải thiện";
            evalColor = "#DE350B";
        } else if (kpiPercent < 80.0) {
            evalText = "Đạt yêu cầu";
            evalColor = "#FFAB00";
        }

        QTableWidgetItem *itemEval = new QTableWidgetItem(evalText);
        itemEval->setTextAlignment(Qt::AlignCenter);
        itemEval->setForeground(QColor(evalColor));
        itemEval->setFont(QFont("Segoe UI", 9, QFont::Bold));
        m_tblDoctorKpi->setItem(i, 4, itemEval);
    }

    // --- Fill Tab 2: Speciality Distribution ---
    SpecialityDistDTO specDto = m_analyticService->getSpecialityDistribution(start, end);
    m_tblSpeciality->setRowCount(0);
    int grandTotalVisits = 0;
    for (const auto &pair : specDto.distribution) {
        grandTotalVisits += pair.second;
    }

    for (int i = 0; i < specDto.distribution.size(); ++i) {
        const auto &pair = specDto.distribution[i];
        m_tblSpeciality->insertRow(i);

        QTableWidgetItem *itemSpec = new QTableWidgetItem(pair.first.isEmpty() ? "Đa khoa" : pair.first);
        itemSpec->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
        m_tblSpeciality->setItem(i, 0, itemSpec);

        QTableWidgetItem *itemCount = new QTableWidgetItem(QString::number(pair.second));
        itemCount->setTextAlignment(Qt::AlignCenter);
        m_tblSpeciality->setItem(i, 1, itemCount);

        double pct = grandTotalVisits > 0 ? (pair.second * 100.0 / grandTotalVisits) : 0.0;
        QProgressBar *pctBar = new QProgressBar(m_tblSpeciality);
        pctBar->setRange(0, 100);
        pctBar->setValue(static_cast<int>(std::round(pct)));
        pctBar->setFormat(QString::number(pct, 'f', 1) + "%");
        pctBar->setAlignment(Qt::AlignCenter);
        pctBar->setStyleSheet(
            "QProgressBar { border: 1px solid #DFE1E6; border-radius: 4px; text-align: center; font-weight: bold; color: #172B4D; background-color: #EBECF0; }"
            "QProgressBar::chunk { background-color: #4B94F2; border-radius: 3px; }"
        );

        m_tblSpeciality->setCellWidget(i, 2, pctBar);
    }

    // --- Fill Tab 3: Daily Trend ---
    m_tblDailyTrend->setRowCount(0);
    QMap<QDate, int> patientMap;
    for (const auto &p : patientStats.patientsPerDate) {
        patientMap[p.first] = p.second;
    }
    QMap<QDate, double> incomeMap;
    for (const auto &p : incomeStats.incomePerDate) {
        incomeMap[p.first] = p.second;
    }
    QMap<QDate, double> waitMap;
    for (const auto &p : waitStats.waitTimePerDate) {
        waitMap[p.first] = p.second;
    }

    int rowIdx = 0;
    for (QDate d = start; d <= end; d = d.addDays(1)) {
        m_tblDailyTrend->insertRow(rowIdx);

        QTableWidgetItem *itemDate = new QTableWidgetItem(d.toString("dd/MM/yyyy (ddd)"));
        itemDate->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        m_tblDailyTrend->setItem(rowIdx, 0, itemDate);

        int pCount = patientMap.value(d, 0);
        QTableWidgetItem *itemPCount = new QTableWidgetItem(QString::number(pCount));
        itemPCount->setTextAlignment(Qt::AlignCenter);
        m_tblDailyTrend->setItem(rowIdx, 1, itemPCount);

        double inc = incomeMap.value(d, 0.0);
        QTableWidgetItem *itemInc = new QTableWidgetItem(viLocale.toCurrencyString(inc, "VNĐ"));
        itemInc->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tblDailyTrend->setItem(rowIdx, 2, itemInc);

        double wTime = waitMap.value(d, 0.0);
        QTableWidgetItem *itemWTime = new QTableWidgetItem(QString::number(wTime, 'f', 1) + " phút");
        itemWTime->setTextAlignment(Qt::AlignCenter);
        m_tblDailyTrend->setItem(rowIdx, 3, itemWTime);

        rowIdx++;
    }
}
