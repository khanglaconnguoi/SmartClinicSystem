#include "DoctorDashboard.h"
#include "../../model/IAuthenticatable.h"
#include "../../model/SystemUser.h"
#include "../../service/AppointmentService.h"
#include "../../service/PatientService.h"
#include "../../service/StaffService.h"
#include "../../service/AppointmentService.h"
#include "../../service/PharmacyService.h"
#include "ClinicalExamWidget.h"

#include "../Patient/PatientWidget.h"
#include <QCalendarWidget>
#include <QDate>
#include <QDateEdit>
#include <QDebug>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTextEdit>
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
    std::shared_ptr<PharmacyService> pharmacyService, QWidget *parent)
    : BaseDashboardWidget(user, staffService, patientService,
                          appointmentService, parent),
      m_currentExaminingRow(-1), m_pharmacyService(pharmacyService), m_overviewPage(nullptr),
      m_patientsPage(nullptr), m_appointmentsPage(nullptr),
      m_settingsPage(nullptr), m_clinicalExamPage(nullptr) {
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
  buildLeaveManagePage();
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
  m_btnLeaveManage = new QPushButton("Nghỉ Phép", m_sidebarFrame);
  m_btnSetting = new QPushButton("Cài Đặt", m_sidebarFrame);

  m_sidebarLayout->addWidget(m_btnDash);
  m_sidebarLayout->addWidget(m_btnPatients);
  m_sidebarLayout->addWidget(m_btnAppoint);
  m_sidebarLayout->addWidget(m_btnLeaveManage);
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
  connect(m_btnLeaveManage, &QPushButton::clicked, this, [this]() {
    onLeaveTabSelected();
    switchPage(3, m_btnLeaveManage);
  });
  connect(m_btnSetting, &QPushButton::clicked, this,
          [this]() { switchPage(4, m_btnSetting); });
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

    card->setObjectName("CardFrame");
    card->setStyleSheet(
        QString("#CardFrame { background-color: #FFFFFF; border: 1px solid "
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
  calendar->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
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
  m_stackedWidget->setCurrentIndex(index);

  m_btnDash->setChecked(false);
  m_btnPatients->setChecked(false);
  m_btnAppoint->setChecked(false);
  if (m_btnLeaveManage)
    m_btnLeaveManage->setChecked(false);
  m_btnSetting->setChecked(false);

  if (activeBtn)
    activeBtn->setChecked(true);
}

void DoctorDashboardWidget::buildClinicalExamPage() {
  m_clinicalExamPage = new ClinicalExamWidget(this);
  m_clinicalExamPage->setServices(m_pharmacyService, m_basePatientService, m_baseAppointmentService);
  m_stackedWidget->addWidget(m_clinicalExamPage);


  connect(m_clinicalExamPage, &ClinicalExamWidget::backToDashboardRequested,
          this, [this]() { switchPage(0, m_btnDash); });

  connect(m_clinicalExamPage,
          &ClinicalExamWidget::viewAppointmentsListRequested, this,
          [this]() { switchPage(2, m_btnAppoint); });

  connect(m_clinicalExamPage, &ClinicalExamWidget::finishExamRequested, this,
          &DoctorDashboardWidget::handlePatientExamFinished);

  connect(m_clinicalExamPage, &ClinicalExamWidget::callPatientRequested, this,
          &DoctorDashboardWidget::handleCallPatientRequested);
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

void DoctorDashboardWidget::handleCallPatientRequested() {
  if (m_currentExaminingRow < 0) {
    QMessageBox::warning(
        this, "Thông báo",
        "Vui lòng chọn một bệnh nhân từ danh sách để gọi khám.");
    return;
  }

  int apptId = -1;
  if (m_isExaminingFromTodayList &&
      m_currentExaminingRow < m_rowApptMeta.size()) {
    apptId = m_rowApptMeta[m_currentExaminingRow].appointmentId;
  } else if (!m_isExaminingFromTodayList &&
             m_currentExaminingRow < m_apptPageMeta.size()) {
    apptId = m_apptPageMeta[m_currentExaminingRow].appointmentId;
  }

  if (apptId == -1) {
    QMessageBox::warning(this, "Lỗi", "Không tìm thấy thông tin bệnh nhân.");
    return;
  }

  QPair<int, QString> result =
      m_baseAppointmentService->callSpecificPatient(apptId);

  if (result.first > 0) {
    QMessageBox msgBox(this);
    msgBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                          Qt::WindowStaysOnTopHint);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("Gọi Khám Thành Công");
    msgBox.setText(QString("Đã gọi bệnh nhân:\n\nSố vé: %1\nTên: %2")
                       .arg(result.first)
                       .arg(result.second));
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #FFFFFF; border: 2px solid #4B94F2; "
        "border-radius: 8px; }"
        "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
        "QPushButton { background-color: #4B94F2; color: white; border-radius: "
        "4px; padding: 6px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #3b82f6; }");
    msgBox.exec();

    // Refresh the tables
    refreshAppointmentsTables();
  } else {
    QMessageBox msgBox(this);
    msgBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                          Qt::WindowStaysOnTopHint);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Thông báo");
    msgBox.setText("Không còn bệnh nhân nào đang chờ (CHECKED_IN).");
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #FFFFFF; border: 2px solid #F59E0B; "
        "border-radius: 8px; }"
        "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
        "QPushButton { background-color: #F59E0B; color: white; border-radius: "
        "4px; padding: 6px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #D97706; }");
    msgBox.exec();
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
      m_baseAppointmentService->updateAppointmentStatus(apptId, "COMPLETED");
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
      m_baseAppointmentService->updateAppointmentStatus(apptId, "COMPLETED");
    }

    refreshAppointmentsTables();

    QMessageBox::information(this, "Nova Care Clinic",
                             "Đã kết thúc ca khám thành công!");
    m_currentExaminingRow = -1;
    switchPage(2, m_btnAppoint);
  }
}

void DoctorDashboardWidget::refreshAppointmentsTables() {

  int docId = m_currentUser ? m_currentUser->getAccountId() : -1;
  QDate today = QDate::currentDate();

  if (m_patientTable) {
    auto records =
        m_baseAppointmentService->getDoctorAppointments(docId, today);
    m_rowApptMeta.clear();
    m_patientTable->setRowCount(0);
    int rowIdx = 0;
    for (const auto &rec : records) {
      // Bỏ qua lọc COMPLETED / CANCELLED để hiển thị tất cả
      // và cho phép sắp xếp theo queue.

      QString statusText = AppointmentStatusText::toVi(rec.status);
      QString statusColor = "#3C4043";

      if (rec.status == AppointmentStatusText::SCHEDULED ||
          rec.status == AppointmentStatusText::CHECKED_IN) {
        statusColor = "#1A73E8";
      } else if (rec.status == AppointmentStatusText::COMPLETED) {
        statusColor = "#059669";
      } else if (rec.status == AppointmentStatusText::CANCELLED) {
        statusColor = "#D93025";
      } else if (rec.status == AppointmentStatusText::STARTED) {
        statusColor = "#F29900";
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
      QString statusText = AppointmentStatusText::toVi(rec.status);
      QString statusColor = "#3C4043";

      if (rec.status == AppointmentStatusText::SCHEDULED ||
          rec.status == AppointmentStatusText::CHECKED_IN) {
        statusColor = "#1A73E8";
      } else if (rec.status == AppointmentStatusText::COMPLETED) {
        statusColor = "#059669";
      } else if (rec.status == AppointmentStatusText::CANCELLED) {
        statusColor = "#D93025";
      } else if (rec.status == AppointmentStatusText::STARTED) {
        statusColor = "#F29900";
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

QFrame *DoctorDashboardWidget::makeCard(QWidget *parent) {
  QFrame *card = new QFrame(parent);
  card->setObjectName("DashboardCard");
  card->setStyleSheet("QFrame#DashboardCard {"
                      "   background-color: #FFFFFF;"
                      "   border: 1px solid #E5E7EB;"
                      "   border-radius: 14px;"
                      "}");
  return card;
}

void DoctorDashboardWidget::buildLeaveManagePage() {
  m_leaveManagePage = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(m_leaveManagePage);
  layout->setContentsMargins(40, 40, 40, 40);
  layout->setSpacing(20);

  QLabel *lblTitle = new QLabel("Quản Lý Nghỉ Phép", m_leaveManagePage);
  lblTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #202124;");
  layout->addWidget(lblTitle);

  m_leaveTabWidget = new QTabWidget(m_leaveManagePage);
  m_leaveTabWidget->setStyleSheet(
      "QTabWidget::pane { border: 1px solid #DADCE0; border-radius: 8px; "
      "background: white; }"
      "QTabBar::tab { padding: 10px 20px; font-weight: bold; color: #5F6368; "
      "background: #F1F3F4; border: 1px solid #DADCE0; border-bottom: none; "
      "border-top-left-radius: 8px; border-top-right-radius: 8px; "
      "margin-right: 2px; }"
      "QTabBar::tab:selected { color: #1A73E8; background: white; }");

  // --- Tab 1: Đăng ký nghỉ phép ---
  QWidget *tabRegister = new QWidget();
  QVBoxLayout *layRegister = new QVBoxLayout(tabRegister);
  layRegister->setContentsMargins(20, 20, 20, 20);

  QFrame *formFrame = makeCard(tabRegister);
  formFrame->setStyleSheet(formFrame->styleSheet() +
                           " QLabel { color: #111827; }");
  QFormLayout *formLayout = new QFormLayout(formFrame);
  formLayout->setContentsMargins(20, 20, 20, 20);
  formLayout->setSpacing(15);

  m_lblLeaveBalance = new QLabel("Đang tải dữ liệu...", formFrame);
  m_lblLeaveBalance->setStyleSheet("color: #5F6368; font-style: italic;");

  auto createCustomCalendar = []() {
    QCalendarWidget *cal = new QCalendarWidget();
    cal->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
    cal->setMinimumSize(330, 250);
    cal->setStyleSheet(
        "QCalendarWidget { background-color: #FFFFFF; color: #333333; }"
        "QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: "
        "#FFFFFF; border-bottom: 1px solid #EAEAEA; min-height: 36px; }"
        "QCalendarWidget QToolButton { color: #333333; font-weight: bold; "
        "background-color: transparent; border: none; padding: 4px 6px; margin: "
        "1px; font-size: 13px; }"
        "QCalendarWidget QToolButton:hover { background-color: #E3F2FD; "
        "border-radius: 4px; }"
        "QCalendarWidget QMenu { background-color: #FFFFFF; color: #333333; }"
        "QCalendarWidget QSpinBox { background-color: #FFFFFF; color: #333333; "
        "selection-background-color: #4B94F2; selection-color: white; font-size: 13px; }"
        "QCalendarWidget QAbstractItemView:enabled { font-size: 13px; color: "
        "#333333; background-color: #FFFFFF; selection-background-color: "
        "#4B94F2; selection-color: #FFFFFF; }"
        "QCalendarWidget QAbstractItemView:disabled { color: #CCCCCC; }");
    return cal;
  };

  m_leaveStartDate = new QDateEdit(QDate::currentDate(), formFrame);
  m_leaveStartDate->setCalendarWidget(createCustomCalendar());
  m_leaveStartDate->setCalendarPopup(true);
  m_leaveStartDate->setMinimumDate(QDate::currentDate());
  m_leaveStartDate->setDisplayFormat("dd/MM/yyyy");
  m_leaveStartDate->setStyleSheet(
      "padding: 8px; border: 1px solid #DADCE0; border-radius: 4px; color: "
      "#111827; background-color: #FFFFFF;");

  m_leaveEndDate = new QDateEdit(QDate::currentDate(), formFrame);
  m_leaveEndDate->setCalendarWidget(createCustomCalendar());
  m_leaveEndDate->setCalendarPopup(true);
  m_leaveEndDate->setMinimumDate(QDate::currentDate());
  m_leaveEndDate->setDisplayFormat("dd/MM/yyyy");
  m_leaveEndDate->setStyleSheet(
      "padding: 8px; border: 1px solid #DADCE0; border-radius: 4px; color: "
      "#111827; background-color: #FFFFFF;");

  m_txtLeaveReason = new QTextEdit(formFrame);
  m_txtLeaveReason->setFixedHeight(80);
  m_txtLeaveReason->setStyleSheet(
      "padding: 8px; border: 1px solid #DADCE0; border-radius: 4px; color: "
      "#111827; background-color: #FFFFFF;");

  formLayout->addRow("Quỹ phép còn lại:", m_lblLeaveBalance);
  formLayout->addRow("Từ ngày:", m_leaveStartDate);
  formLayout->addRow("Đến ngày:", m_leaveEndDate);
  formLayout->addRow("Lý do:", m_txtLeaveReason);

  QPushButton *btnSubmit = new QPushButton("Gửi yêu cầu", formFrame);
  btnSubmit->setCursor(Qt::PointingHandCursor);
  btnSubmit->setStyleSheet(
      "background-color: #1A73E8; color: white; padding: 10px 20px; "
      "font-weight: bold; border-radius: 4px; border: none;");

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  btnLayout->addWidget(btnSubmit);

  layRegister->addWidget(formFrame);
  layRegister->addLayout(btnLayout);
  layRegister->addStretch();

  m_leaveTabWidget->addTab(tabRegister, "Đăng ký nghỉ phép");

  // --- Tab 2: Lịch sử nghỉ phép ---
  QWidget *tabHistory = new QWidget();
  QVBoxLayout *layHistory = new QVBoxLayout(tabHistory);
  layHistory->setContentsMargins(20, 20, 20, 20);

  m_tableLeaveHistory = new QTableWidget(tabHistory);
  m_tableLeaveHistory->setColumnCount(4);
  m_tableLeaveHistory->setHorizontalHeaderLabels(
      {"Từ ngày", "Đến ngày", "Lý do", "Trạng thái"});

  QHeaderView *header = m_tableLeaveHistory->horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Interactive);
  header->setSectionResizeMode(2, QHeaderView::Stretch); // Reason stretches

  m_tableLeaveHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableLeaveHistory->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tableLeaveHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableLeaveHistory->setStyleSheet(
      "QTableWidget { border: 1px solid #DADCE0; border-radius: 4px; color: "
      "#202124; background-color: white; }"
      "QHeaderView::section { background-color: #F1F3F4; color: #5F6368; "
      "font-weight: bold; padding: 4px; border: 1px solid #DADCE0; }");

  layHistory->addWidget(m_tableLeaveHistory);
  m_leaveTabWidget->addTab(tabHistory, "Lịch sử nghỉ phép");

  layout->addWidget(m_leaveTabWidget);

  connect(btnSubmit, &QPushButton::clicked, this,
          &DoctorDashboardWidget::onSubmitLeaveRequest);

  connect(m_leaveTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
    if (index == 1) {
      loadLeaveHistory();
    }
  });

  m_stackedWidget->addWidget(m_leaveManagePage);
}

void DoctorDashboardWidget::onLeaveTabSelected() {
  if (!m_currentUser)
    return;

  if (auto sysUser = std::dynamic_pointer_cast<SystemUser>(m_currentUser)) {
    int staffId = sysUser->getAccountId();
    LeaveBalanceDTO balance = m_baseStaffService->getLeaveBalance(
        staffId, QDate::currentDate().year());
    if (balance.totalDays > 0) {
      int remaining = balance.totalDays - balance.usedDays;
      m_lblLeaveBalance->setText(QString("%1 ngày").arg(remaining));
      m_lblLeaveBalance->setStyleSheet("color: #1A73E8; font-weight: bold;");
    } else {
      m_lblLeaveBalance->setText("Không thể lấy dữ liệu quỹ phép.");
      m_lblLeaveBalance->setStyleSheet("color: red;");
    }
  }
}

void DoctorDashboardWidget::loadLeaveHistory() {
  if (!m_currentUser || !m_tableLeaveHistory)
    return;

  auto sysUser = std::dynamic_pointer_cast<SystemUser>(m_currentUser);
  if (!sysUser)
    return;
  int staffId = sysUser->getAccountId();

  m_tableLeaveHistory->setRowCount(0);
  auto historyList = m_baseStaffService->getOwnLeaveHistory(staffId);

  for (const auto &req : historyList) {
    int row = m_tableLeaveHistory->rowCount();
    m_tableLeaveHistory->insertRow(row);

    m_tableLeaveHistory->setItem(
        row, 0, new QTableWidgetItem(req.startDate.toString("dd/MM/yyyy")));
    m_tableLeaveHistory->setItem(
        row, 1, new QTableWidgetItem(req.endDate.toString("dd/MM/yyyy")));
    m_tableLeaveHistory->setItem(row, 2, new QTableWidgetItem(req.reason));

    QTableWidgetItem *statusItem = new QTableWidgetItem(req.status);
    statusItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
    if (req.status == "PENDING") {
      statusItem->setForeground(QColor("#F29900")); // Yellow
    } else if (req.status == "APPROVED") {
      statusItem->setForeground(QColor("#34A853")); // Green
    } else if (req.status == "REJECTED") {
      statusItem->setForeground(QColor("#EA4335")); // Red
    }

    m_tableLeaveHistory->setItem(row, 3, statusItem);
  }
}

void DoctorDashboardWidget::onSubmitLeaveRequest() {
  if (!m_currentUser)
    return;

  auto sysUser = std::dynamic_pointer_cast<SystemUser>(m_currentUser);
  if (!sysUser)
    return;
  int staffId = sysUser->getAccountId();

  QDate startDate = m_leaveStartDate->date();
  QDate endDate = m_leaveEndDate->date();
  QString reason = m_txtLeaveReason->toPlainText().trimmed();

  if (startDate < QDate::currentDate()) {
    QMessageBox::warning(this, "Lỗi", "Ngày bắt đầu không được ở quá khứ.");
    return;
  }

  if (endDate < startDate) {
    QMessageBox::warning(this, "Lỗi",
                         "Ngày kết thúc phải lớn hơn hoặc bằng ngày bắt đầu.");
    return;
  }

  if (reason.isEmpty()) {
    QMessageBox::warning(this, "Lỗi", "Vui lòng nhập lý do nghỉ phép.");
    return;
  }

  LeaveRequestDTO req;
  req.staffId = staffId;
  req.startDate = startDate;
  req.endDate = endDate;
  req.reason = reason;

  try {
    QString errorMsg =
        m_baseStaffService->registerLeave(staffId, startDate, endDate, reason);
    if (errorMsg.isEmpty()) {
      int year = startDate.year();

      if (m_baseAppointmentService) {
        int cancelledCount =
            m_baseAppointmentService->cancelAppointmentsForDoctor(
                staffId, startDate, endDate);
        QString msg =
            QString("Đăng ký nghỉ phép thành công!\n\nHệ thống đã tự động HỦY "
                    "%1 lịch hẹn trong khoảng thời gian này.")
                .arg(cancelledCount);
        QMessageBox::information(this, "Thành công", msg);
      } else {
        QMessageBox::information(
            this, "Thành công",
            "Đăng ký nghỉ phép thành công! Đơn của bạn đang ở trạng thái CHỜ "
            "DUYỆT.\n\nLịch khám sẽ tự động hủy nếu Admin duyệt đơn.");
      }

      LeaveBalanceDTO balance =
          m_baseStaffService->getLeaveBalance(staffId, year);
      int remaining = balance.totalDays - balance.usedDays;
      m_lblLeaveBalance->setText(QString("%1 ngày").arg(remaining));

      m_txtLeaveReason->clear();

      refreshAppointmentsTables();
    } else {
      QMessageBox::warning(this, "Lỗi", errorMsg);
    }
  } catch (const std::exception &e) {
    QMessageBox::warning(this, "Lỗi", e.what());
  }
}