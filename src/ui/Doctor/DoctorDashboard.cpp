#include "DoctorDashboard.h"
#include "../../model/IAuthenticatable.h"
#include "../../model/SystemUser.h"
#include "../../service/PatientService.h"
#include "../../service/StaffService.h"
#include "../../service/AppointmentService.h"
#include "ClinicalExamWidget.h"
#include "../PatientWidget.h"
#include <QCalendarWidget>
#include <QDate>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QStackedWidget>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

DoctorDashboardWidget::DoctorDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<AppointmentService> appointmentService,
    QWidget *parent)
    : BaseDashboardWidget(user, staffService, patientService, appointmentService, parent),
      m_overviewPage(nullptr), m_patientsPage(nullptr),
      m_appointmentsPage(nullptr), m_settingsPage(nullptr),
      m_clinicalExamPage(nullptr), m_currentExaminingRow(-1) {
  initializeDashboard();
}

void DoctorDashboardWidget::fillDashboardData() {
  buildSidebar();

  if (m_currentUser && m_docNameLabel) {
    m_docNameLabel->setText(m_currentUser->getFullName());
  }

  if (m_docAvatarBtn && m_currentUser) {
    QPixmap rawPixmap = m_currentUser->getAvatar();
    if (rawPixmap.isNull()) {
      rawPixmap = QPixmap(36, 36);
      rawPixmap.fill(QColor("#4B94F2"));
    }

    int size = 36;
    m_docAvatarBtn->setFixedSize(size, size);
    QPixmap scaledPixmap = rawPixmap.scaled(
        size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    QPixmap targetPixmap(size, size);
    targetPixmap.fill(Qt::transparent);

    QPainter painter(&targetPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, scaledPixmap);
    painter.end();

    m_docAvatarBtn->setIconSize(QSize(size, size));
    m_docAvatarBtn->setIcon(QIcon(targetPixmap));
    m_docAvatarBtn->setStyleSheet("QPushButton { "
                                  "   background-color: transparent; "
                                  "   border: none; "
                                  "   width: 36px; "
                                  "   height: 36px; "
                                  "   padding: 0px; "
                                  "   margin: 0px; "
                                  "}");
    m_docAvatarBtn->update();

    disconnect(m_docAvatarBtn, &QPushButton::clicked, nullptr, nullptr);
    connect(m_docAvatarBtn, &QPushButton::clicked, this,
            []() { qDebug() << "Đang mở trang Profile của bác sĩ..."; });
  }

  m_stackedWidget = new QStackedWidget(m_mainContentWidget);
  m_stackedWidget->setObjectName("StackedWidget");
  m_stackedWidget->setStyleSheet(
      "QStackedWidget#StackedWidget > QWidget { background-color: #EEF2F6; }");
  m_mainContentLayout->addWidget(m_stackedWidget, 1);

  buildOverviewPage();
  buildPatientsPage();
  buildAppointmentsPage();
  buildSettingsPage();
  buildClinicalExamPage();

  refreshAppointmentsTables();

  switchPage(0, m_btnDash);
}

void DoctorDashboardWidget::buildSidebar() {
  if (!m_sidebarLayout)
    return;

  m_btnDash = new QPushButton("Tổng Quan", m_sidebarFrame);
  m_btnPatients = new QPushButton("Bệnh Nhân", m_sidebarFrame);
  m_btnAppoint = new QPushButton("Lịch Hẹn", m_sidebarFrame);
  m_btnSetting = new QPushButton("Cài Đặt", m_sidebarFrame);

  m_sidebarLayout->addWidget(m_btnDash);
  m_sidebarLayout->addWidget(m_btnPatients);
  m_sidebarLayout->addWidget(m_btnAppoint);
  m_sidebarLayout->addWidget(m_btnSetting);
  m_sidebarLayout->addStretch();

  m_btnLogout = new QPushButton("Đăng Xuất", m_sidebarFrame);
  m_btnLogout->setStyleSheet(
      "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; "
      "color: #D32F2F; border: none; border-radius: 0px; background-color: "
      "transparent; font-weight: bold; }"
      "QPushButton:hover { background-color: #FFEBEE; }");
  m_btnLogout->setCursor(Qt::PointingHandCursor);
  m_sidebarLayout->addWidget(m_btnLogout);
  m_sidebarLayout->addSpacing(30);
  connect(m_btnLogout, &QPushButton::clicked, this,
          &BaseDashboardWidget::logoutRequested);

  connect(m_btnDash, &QPushButton::clicked, this,
          [this]() { switchPage(0, m_btnDash); });
  connect(m_btnPatients, &QPushButton::clicked, this,
          [this]() { switchPage(1, m_btnPatients); });
  connect(m_btnAppoint, &QPushButton::clicked, this,
          [this]() { switchPage(2, m_btnAppoint); });
  connect(m_btnSetting, &QPushButton::clicked, this,
          [this]() { switchPage(3, m_btnSetting); });
}

void DoctorDashboardWidget::buildOverviewPage() {
  m_overviewPage = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(m_overviewPage);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  QScrollArea *scrollArea = new QScrollArea(m_overviewPage);
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet(
      "QScrollArea { background: transparent; border: none; }"
      "QScrollBar:vertical {"
      "   background: #F1F3F4; width: 6px; border-radius: 3px;"
      "}"
      "QScrollBar::handle:vertical {"
      "   background: #C5CAD4; border-radius: 3px; min-height: 30px;"
      "}"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: "
      "0px; }");

  QWidget *scrollContent = new QWidget();
  scrollContent->setStyleSheet("background: transparent;");
  QVBoxLayout *pageLayout = new QVBoxLayout(scrollContent);
  pageLayout->setContentsMargins(0, 0, 15, 20);
  pageLayout->setSpacing(20);

  createDoctorCards(scrollContent, pageLayout);
  createDoctorCharts(scrollContent, pageLayout);
  createDoctorTable(scrollContent, pageLayout);

  pageLayout->addStretch();
  scrollArea->setWidget(scrollContent);
  mainLayout->addWidget(scrollArea);

  m_stackedWidget->addWidget(m_overviewPage);
}

void DoctorDashboardWidget::buildPatientsPage() {
  m_patientsPage = new PatientWidget(this);
  m_stackedWidget->addWidget(m_patientsPage);
}

void DoctorDashboardWidget::buildAppointmentsPage() {
  m_appointmentsPage = new QWidget(this);
  QVBoxLayout *pageLayout = new QVBoxLayout(m_appointmentsPage);
  pageLayout->setContentsMargins(0, 0, 0, 0);
  pageLayout->setSpacing(20);

  QLabel *title = new QLabel("Lịch Hẹn Khám & Điều Trị", m_appointmentsPage);
  title->setStyleSheet("font-size: 18px; font-weight: bold; color: #111827;");
  pageLayout->addWidget(title);

  m_appointmentsTable = new QTableWidget(m_appointmentsPage);
  m_appointmentsTable->setColumnCount(6);
  m_appointmentsTable->setHorizontalHeaderLabels(
      {"Thời Gian", "Mã BN", "Tên Bệnh Nhân", "Dịch Vụ Khám", "Phòng Khám",
       "Trạng Thái"});
  m_appointmentsTable->setStyleSheet(
      "QTableWidget { background-color: #FFFFFF; border: 1px solid #EAEAEA; "
      "border-radius: 8px; gridline-color: #F1F3F4; }"
      "QHeaderView::section { background-color: #F8F9FA; padding: 10px; "
      "font-weight: bold; border: none; border-bottom: 2px solid #EAEAEA; "
      "color: #5F6368; }"
      "QTableWidget::item { padding: 12px; color: #3C4043; }");
  m_appointmentsTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_appointmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_appointmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_appointmentsTable->verticalHeader()->setVisible(false);

  connect(m_appointmentsTable, &QTableWidget::cellClicked, this,
          [this](int row, int /*col*/) {
            if (row >= 0 && row < m_apptPageMeta.size()) {
              const auto &meta = m_apptPageMeta[row];
              openClinicalExam(meta.name, meta.code, meta.time, meta.reason,
                               row, false);
            }
          });

  pageLayout->addWidget(m_appointmentsTable, 1);
  m_stackedWidget->addWidget(m_appointmentsPage);
}

void DoctorDashboardWidget::buildSettingsPage() {
  m_settingsPage = new QWidget(this);
  QVBoxLayout *pageLayout = new QVBoxLayout(m_settingsPage);
  pageLayout->setContentsMargins(0, 0, 0, 0);

  QLabel *title = new QLabel("Cài Đặt", m_settingsPage);
  title->setStyleSheet("font-size: 18px; font-weight: bold; color: #111827;");
  pageLayout->addWidget(title);

  QLabel *desc =
      new QLabel("Trang cấu hình và cài đặt thông tin cá nhân dành cho bác sĩ.",
                 m_settingsPage);
  desc->setStyleSheet("color: #5F6368; font-size: 14px;");
  pageLayout->addWidget(desc);
  pageLayout->addStretch();

  m_stackedWidget->addWidget(m_settingsPage);
}

void DoctorDashboardWidget::createDoctorCards(QWidget *parentPage,
                                              QVBoxLayout *pageLayout) {
  QHBoxLayout *cardsLayout = new QHBoxLayout();
  cardsLayout->setSpacing(20);

  QStringList titles = {"Tổng số ca khám hôm nay", "Số ca phẫu thuật",
                        "Doanh thu phòng khám"};
  QStringList values = {"18", "3", "3,500,000đ"};
  QStringList rates = {"▲ 12%", "▲ 8%", "▼ 1%"};

  for (int i = 0; i < 3; ++i) {
    QFrame *card = new QFrame(parentPage);
    QString accentColor = "#4B94F2";

    card->setStyleSheet(
        QString("QFrame { background-color: #FFFFFF; border: 1px solid "
                "#E5E7EB; border-left: 5px solid %1; border-radius: 12px; }")
            .arg(accentColor));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 20, 24, 20);
    cardLayout->setSpacing(8);

    QLabel *lblTitle = new QLabel(titles[i], card);
    lblTitle->setStyleSheet(
        "color: #6B7280; font-size: 14px; font-weight: 500; border: none;");

    QLabel *lblValue = new QLabel(values[i], card);
    lblValue->setStyleSheet("color: #111827; font-size: 32px; font-weight: "
                            "900; border: none; letter-spacing: -1px;");

    QLabel *lblRate = new QLabel(rates[i] + " so với tuần trước", card);

    if (i == 2) {
      // Màu đỏ cho giảm
      lblRate->setStyleSheet("background-color: #FEE2E2; color: #DC2626; "
                             "font-size: 12px; font-weight: bold; padding: 4px "
                             "8px; border-radius: 6px; border: none;");
    } else {
      // Màu xanh lục cho tăng
      lblRate->setStyleSheet("background-color: #DCFCE7; color: #16A34A; "
                             "font-size: 12px; font-weight: bold; padding: 4px "
                             "8px; border-radius: 6px; border: none;");
    }

    // Bọc rate label trong 1 layout phụ để nó tự thu nhỏ lại bằng nội dung chữ
    QHBoxLayout *rateLayout = new QHBoxLayout();
    rateLayout->addWidget(lblRate);
    rateLayout->addStretch();

    cardLayout->addWidget(lblTitle);
    cardLayout->addWidget(lblValue);
    cardLayout->addLayout(rateLayout);
    cardLayout->addStretch();

    cardsLayout->addWidget(card);
  }
  pageLayout->addLayout(cardsLayout);
}

void DoctorDashboardWidget::createDoctorCharts(QWidget *parentPage,
                                               QVBoxLayout *pageLayout) {
  QHBoxLayout *row2Layout = new QHBoxLayout();
  row2Layout->setSpacing(20);

  QFrame *chartCard = makeCard(parentPage);
  QVBoxLayout *cardLayout = new QVBoxLayout(chartCard);
  cardLayout->setContentsMargins(15, 15, 15, 15);

  QBarSet *setMedical = new QBarSet("Bệnh nhân nội trú");
  QBarSet *setAppointed = new QBarSet("Bệnh nhân vãng lai");
  setMedical->setColor(QColor("#4B94F2"));
  setAppointed->setColor(QColor("#398CBF"));

  *setMedical << 75 << 40 << 55 << 90 << 45 << 80 << 50;
  *setAppointed << 50 << 25 << 35 << 40 << 20 << 60 << 65;

  QBarSeries *series = new QBarSeries();
  series->append(setMedical);
  series->append(setAppointed);

  QChart *chart = new QChart();
  chart->addSeries(series);
  chart->setTitle("Thống kê bệnh nhân hàng tuần");
  chart->setAnimationOptions(QChart::SeriesAnimations);
  chart->setBackgroundVisible(false);

  QStringList categories = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  QBarCategoryAxis *axisX = new QBarCategoryAxis();
  axisX->append(categories);
  axisX->setGridLineVisible(false);
  chart->addAxis(axisX, Qt::AlignBottom);
  series->attachAxis(axisX);

  QValueAxis *axisY = new QValueAxis();
  axisY->setRange(0, 100);
  chart->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisY);

  QChartView *chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setMinimumHeight(230);

  cardLayout->addWidget(chartView);
  row2Layout->addWidget(chartCard, 2);

  QFrame *scheduleCard = makeCard(parentPage);
  QVBoxLayout *schedLayout = new QVBoxLayout(scheduleCard);
  schedLayout->setContentsMargins(15, 15, 15, 15);
  schedLayout->setSpacing(8);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *schedTitle = new QLabel("Lịch trực hôm nay", scheduleCard);
  schedTitle->setStyleSheet(
      "font-size: 16px; font-weight: bold; color: #111827;");

  QPushButton *btnLink = new QPushButton("Xem lịch hẹn →", scheduleCard);
  btnLink->setCursor(Qt::PointingHandCursor);
  btnLink->setStyleSheet(
      "QPushButton { background: transparent; color: #4B94F2; font-size: 11px; "
      "font-weight: bold; border: none; padding: 0; }"
      "QPushButton:hover { color: #398CBF; text-decoration: underline; }");
  headerLayout->addWidget(schedTitle);
  headerLayout->addStretch();
  headerLayout->addWidget(btnLink);
  schedLayout->addLayout(headerLayout);

  QCalendarWidget *calendar = new QCalendarWidget(scheduleCard);
  calendar->setGridVisible(false);
  calendar->setNavigationBarVisible(true);
  calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
  calendar->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
  calendar->setStyleSheet("QCalendarWidget {"
                          "   background-color: #FFFFFF;"
                          "   border: none;"
                          "}"
                          "QCalendarWidget QWidget#qt_calendar_navigationbar {"
                          "   background-color: #FFFFFF;"
                          "   border-bottom: 1px solid #F1F3F4;"
                          "}"
                          "QCalendarWidget QToolButton {"
                          "   color: #3C4043;"
                          "   font-family: 'Segoe UI';"
                          "   font-size: 11px;"
                          "   font-weight: bold;"
                          "   background-color: transparent;"
                          "   border: none;"
                          "   border-radius: 4px;"
                          "   padding: 3px;"
                          "}"
                          "QCalendarWidget QToolButton:hover {"
                          "   background-color: #F1F3F4;"
                          "}"
                          "QCalendarWidget QAbstractItemView:enabled {"
                          "   color: #3C4043;"
                          "   background-color: #FFFFFF;"
                          "   selection-background-color: #4B94F2;"
                          "   selection-color: #FFFFFF;"
                          "   font-family: 'Segoe UI';"
                          "   font-size: 11px;"
                          "   border: none;"
                          "}"
                          "QCalendarWidget QAbstractItemView:disabled {"
                          "   color: #C5CAD4;"
                          "}");
  schedLayout->addWidget(calendar, 1);

  auto handleNavigation = [this]() { switchPage(2, m_btnAppoint); };
  connect(btnLink, &QPushButton::clicked, this, handleNavigation);
  connect(calendar, &QCalendarWidget::clicked, this, handleNavigation);

  row2Layout->addWidget(scheduleCard, 1);
  pageLayout->addLayout(row2Layout);
}

void DoctorDashboardWidget::createDoctorTable(QWidget *parentPage,
                                              QVBoxLayout *pageLayout) {
  QFrame *tableCard = makeCard(parentPage);
  QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
  cardLayout->setContentsMargins(20, 15, 20, 15);
  cardLayout->setSpacing(10);

  QLabel *tblTitle =
      new QLabel("Danh sách bệnh nhân hẹn khám hôm nay", tableCard);
  tblTitle->setStyleSheet(
      "font-size: 16px; font-weight: bold; color: #111827;");
  cardLayout->addWidget(tblTitle);

  m_patientTable = new QTableWidget(tableCard);
  m_patientTable->setColumnCount(5);
  m_patientTable->setHorizontalHeaderLabels({"Tên Bệnh Nhân", "Mã Định Danh",
                                             "Giờ Hẹn", "Chuyên Khoa",
                                             "Trạng Thái"});

  m_patientTable->setStyleSheet(
      "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: "
      "#F3F4F6; }"
      "QHeaderView::section { background-color: #F9FAFB; padding: 12px; "
      "font-weight: bold; border: none; border-bottom: 2px solid #E5E7EB; "
      "color: #6B7280; }"
      "QTableWidget::item { padding: 12px; color: #111827; border-bottom: 1px "
      "solid #F3F4F6; }");

  cardLayout->addWidget(m_patientTable);
  pageLayout->addWidget(tableCard);

  m_patientTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_patientTable->verticalHeader()->setVisible(false);

  connect(m_patientTable, &QTableWidget::cellClicked, this,
          [this](int row, int /*col*/) {
            if (row >= 0 && row < m_rowApptMeta.size()) {
              const auto &meta = m_rowApptMeta[row];
              openClinicalExam(meta.name, meta.code, meta.time, meta.reason,
                               row, true);
            }
          });

  m_patientTable->setMinimumHeight(220);
  pageLayout->addWidget(m_patientTable);
}

void DoctorDashboardWidget::switchPage(int index, QPushButton *activeBtn) {
  if (!m_stackedWidget)
    return;

  // Sử dụng danh sách các nút bấm kế thừa trực tiếp từ lớp cha
  // BaseDashboardWidget
  QPushButton *btns[] = {m_btnDash, m_btnPatients, m_btnAppoint, m_btnSetting};
  for (auto *btn : btns) {
    if (btn)
      btn->setObjectName("");
  }
  if (activeBtn)
    activeBtn->setObjectName("activeBtn");

  if (m_sidebarFrame) {
    m_sidebarFrame->setStyleSheet(m_sidebarFrame->styleSheet());
  }

  m_stackedWidget->setCurrentIndex(index);
}

void DoctorDashboardWidget::buildClinicalExamPage() {
  m_clinicalExamPage = new ClinicalExamWidget(this);
  m_stackedWidget->addWidget(m_clinicalExamPage);

  connect(m_clinicalExamPage, &ClinicalExamWidget::backToDashboardRequested,
          this, [this]() { switchPage(0, m_btnDash); });

  connect(m_clinicalExamPage,
          &ClinicalExamWidget::viewAppointmentsListRequested, this,
          [this]() { switchPage(2, m_btnAppoint); });

  connect(m_clinicalExamPage, &ClinicalExamWidget::finishExamRequested, this,
          &DoctorDashboardWidget::handlePatientExamFinished);
}

void DoctorDashboardWidget::openClinicalExam(
    const QString &name, const QString &id, const QString &time,
    const QString &specialty, int rowIndex, bool isFromTodayList) {
  if (!m_clinicalExamPage || !m_stackedWidget)
    return;

  m_currentExaminingRow = rowIndex;
  m_isExaminingFromTodayList = isFromTodayList;
  m_clinicalExamPage->loadPatientInfo(name, id, time, specialty);

  int idx = m_stackedWidget->indexOf(m_clinicalExamPage);
  if (idx != -1) {
    switchPage(idx, nullptr);
  }
}

void DoctorDashboardWidget::handlePatientExamFinished() {
  if (m_currentExaminingRow == -1)
    return;

  if (m_isExaminingFromTodayList) {
    if (!m_patientTable)
      return;

    if (m_currentExaminingRow >= 0 &&
        m_currentExaminingRow < m_rowApptMeta.size()) {
      int apptId = m_rowApptMeta[m_currentExaminingRow].appointmentId;
      m_baseAppointmentService->updateAppointmentStatus(apptId,
                                                             "COMPLETED");
    }

    refreshAppointmentsTables();

    int nextRow = m_currentExaminingRow + 1;
    if (m_patientTable && nextRow < m_patientTable->rowCount()) {
      m_currentExaminingRow = nextRow;

      QString name = m_patientTable->item(nextRow, 0)->text();
      QString id = m_patientTable->item(nextRow, 1)->text();
      QString time = m_patientTable->item(nextRow, 2)->text();
      QString dept = m_patientTable->item(nextRow, 3)->text();

      m_clinicalExamPage->loadPatientInfo(name, id, time, dept);

      QMessageBox::information(this, "Nova Care Clinic",
                               QString("Đã kết thúc ca khám hiện tại.\nChuyển "
                                       "sang bệnh nhân tiếp theo: %1")
                                   .arg(name));
    } else {
      QMessageBox::information(
          this, "Nova Care Clinic",
          "Đã hoàn thành khám cho toàn bộ bệnh nhân trong danh sách hôm nay!");
      m_currentExaminingRow = -1;
      switchPage(0, m_btnDash);
    }
  } else {
    if (m_currentExaminingRow >= 0 &&
        m_currentExaminingRow < m_apptPageMeta.size()) {
      int apptId = m_apptPageMeta[m_currentExaminingRow].appointmentId;
      m_baseAppointmentService->updateAppointmentStatus(apptId,
                                                             "COMPLETED");
    }

    refreshAppointmentsTables();

    QMessageBox::information(this, "Nova Care Clinic",
                             "Đã kết thúc ca khám thành công!");
    m_currentExaminingRow = -1;
    switchPage(2, m_btnAppoint);
  }
}

void DoctorDashboardWidget::refreshAppointmentsTables() {
  QString docId = "BS01";
  if (auto sysUser = std::dynamic_pointer_cast<SystemUser>(m_currentUser)) {
    docId = sysUser->getStaffCode();
  }
  QString todayStr = QDate::currentDate().toString("yyyy-MM-dd");

  if (m_patientTable) {
    auto records = m_baseAppointmentService->getDoctorAppointments(docId, todayStr);
    m_rowApptMeta.clear();
    m_patientTable->setRowCount(0);
    int rowIdx = 0;
    for (const auto &rec : records) {
      if (rec.status == "COMPLETED" || rec.status == "CANCELLED") {
        continue;
      }

      QString statusText = rec.status;
      QString statusColor = "#3C4043";
      if (rec.status == "SCHEDULED") {
        statusText = "Đang chờ";
        statusColor = "#1A73E8";
      } else if (rec.status == "COMPLETED") {
        statusText = "Đã khám xong";
        statusColor = "#059669";
      } else if (rec.status == "CANCELLED") {
        statusText = "Đã hủy ca";
        statusColor = "#D93025";
      }

      m_patientTable->insertRow(rowIdx);

      QTableWidgetItem *nameItem = new QTableWidgetItem(rec.patientName);
      QTableWidgetItem *codeItem = new QTableWidgetItem(rec.patientCode);
      QTableWidgetItem *timeItem = new QTableWidgetItem(rec.startTime);
      QTableWidgetItem *deptItem = new QTableWidgetItem(rec.reason);
      QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);

      statusItem->setForeground(QBrush(QColor(statusColor)));

      m_patientTable->setItem(rowIdx, 0, nameItem);
      m_patientTable->setItem(rowIdx, 1, codeItem);
      m_patientTable->setItem(rowIdx, 2, timeItem);
      m_patientTable->setItem(rowIdx, 3, deptItem);
      m_patientTable->setItem(rowIdx, 4, statusItem);

      m_rowApptMeta.append(ApptMeta{rec.appointmentId, rec.patientId,
                                    rec.patientName, rec.patientCode,
                                    rec.startTime, rec.reason});
      rowIdx++;
    }
  }

  if (m_appointmentsTable) {
    auto records = m_baseAppointmentService->getDoctorAppointments(docId);
    m_apptPageMeta.clear();
    m_appointmentsTable->setRowCount(0);
    int rowIdx = 0;
    for (const auto &rec : records) {
      QString statusText = rec.status;
      QString statusColor = "#3C4043";
      if (rec.status == "SCHEDULED") {
        statusText = "Đang chờ";
        statusColor = "#1A73E8";
      } else if (rec.status == "COMPLETED") {
        statusText = "Đã khám";
        statusColor = "#059669";
      } else if (rec.status == "CANCELLED") {
        statusText = "Đã hủy";
        statusColor = "#D93025";
      }

      m_appointmentsTable->insertRow(rowIdx);

      QString timeStr =
          QString("%1 %2").arg(rec.appointmentDate, rec.startTime);

      QTableWidgetItem *timeItem = new QTableWidgetItem(timeStr);
      QTableWidgetItem *codeItem = new QTableWidgetItem(rec.patientCode);
      QTableWidgetItem *nameItem = new QTableWidgetItem(rec.patientName);
      QTableWidgetItem *reasonItem = new QTableWidgetItem(rec.reason);
      QTableWidgetItem *roomItem = new QTableWidgetItem(rec.roomNumber);
      QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);

      statusItem->setForeground(QBrush(QColor(statusColor)));

      m_appointmentsTable->setItem(rowIdx, 0, timeItem);
      m_appointmentsTable->setItem(rowIdx, 1, codeItem);
      m_appointmentsTable->setItem(rowIdx, 2, nameItem);
      m_appointmentsTable->setItem(rowIdx, 3, reasonItem);
      m_appointmentsTable->setItem(rowIdx, 4, roomItem);
      m_appointmentsTable->setItem(rowIdx, 5, statusItem);

      m_apptPageMeta.append(ApptMeta{rec.appointmentId, rec.patientId,
                                     rec.patientName, rec.patientCode,
                                     rec.startTime, rec.reason});
      rowIdx++;
    }
  }
}

QFrame* DoctorDashboardWidget::makeCard(QWidget* parent) {
  QFrame* card = new QFrame(parent);
  card->setObjectName("DashboardCard");
  card->setStyleSheet("QFrame#DashboardCard {"
                      "   background-color: #FFFFFF;"
                      "   border: 1px solid #E5E7EB;"
                      "   border-radius: 14px;"
                      "}");
  return card;
}