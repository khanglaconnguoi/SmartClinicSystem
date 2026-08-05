#include "DoctorDashboard.h"

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
#include <QTimer>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

#include "ClinicalExamWidget.h"
#include "PatientWidget.h"
#include "PatientRecordHistoryDialog.h"
#include "ScheduleFollowUpDialog.h"
#include "model/CommonEnums.h"
#include "model/IAuthenticatable.h"
#include "model/SystemUser.h"
#include "service/AppointmentService.h"
#include "service/MedicalRecordService.h"
#include "service/PatientService.h"
#include "service/PharmacyService.h"
#include "service/StaffService.h"
#include "ui/view/Profile.h"

DoctorDashboardWidget::DoctorDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<AppointmentService> appointmentService,
    std::shared_ptr<MedicalRecordService> medicalRecordService,
    std::shared_ptr<PharmacyService> pharmacyService,
    std::shared_ptr<ServiceRequestService> serviceRequestService,
    QWidget *parent)
    : BaseDashboardWidget(user, staffService, parent),
      m_patientService(patientService),
      m_appointmentService(appointmentService),
      m_medicalRecordService(medicalRecordService),
      m_currentExaminingRow(-1),
      m_pharmacyService(pharmacyService),
      m_serviceRequestService(serviceRequestService),
      m_overviewPage(nullptr), m_patientsPage(nullptr),
      m_appointmentsPage(nullptr), m_settingsPage(nullptr),
      m_clinicalExamPage(nullptr) {
  initializeDashboard();
}

void DoctorDashboardWidget::fillDashboardData() {
  buildSidebar();

  if (m_currentUser && m_nameLabel) {
    m_nameLabel->setText(m_currentUser->getFullName());
  }

  if (m_avatarBtn && m_currentUser) {
    QPixmap rawPixmap = m_currentUser->getAvatar();
    if (rawPixmap.isNull()) {
      rawPixmap = QPixmap(36, 36);
      rawPixmap.fill(QColor("#4B94F2"));
    }

    int size = 36;
    m_avatarBtn->setFixedSize(size, size);
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

    m_avatarBtn->setIconSize(QSize(size, size));
    m_avatarBtn->setIcon(QIcon(targetPixmap));
    m_avatarBtn->setStyleSheet("QPushButton { "
                                  "   background-color: transparent; "
                                  "   border: none; "
                                  "   width: 36px; "
                                  "   height: 36px; "
                                  "   padding: 0px; "
                                  "   margin: 0px; "
                                  "}");
    m_avatarBtn->update();

    disconnect(m_avatarBtn, &QPushButton::clicked, nullptr, nullptr);
    connect(m_avatarBtn, &QPushButton::clicked, this, [this]() {
      if (!m_currentUser || !m_staffService) return;
      ProfileWidget dialog(m_staffService, this);
      dialog.loadProfile(m_currentUser->getAccountId());
      dialog.exec();
      if (m_nameLabel) {
        m_nameLabel->setText(m_currentUser->getFullName());
      }
      if (m_avatarBtn && m_currentUser) {
        QPixmap raw = m_currentUser->getAvatar();
        if (!raw.isNull()) {
          int sz = 36;
          QPixmap scaled = raw.scaled(sz, sz, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
          QPixmap target(sz, sz);
          target.fill(Qt::transparent);
          QPainter p(&target);
          p.setRenderHint(QPainter::Antialiasing);
          QPainterPath clipPath;
          clipPath.addEllipse(0, 0, sz, sz);
          p.setClipPath(clipPath);
          p.drawPixmap(0, 0, scaled);
          p.end();
          m_avatarBtn->setIcon(QIcon(target));
        }
      }
    });
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

  refreshOverviewCards();

  pageLayout->addStretch();
  scrollArea->setWidget(scrollContent);
  mainLayout->addWidget(scrollArea);

  m_stackedWidget->addWidget(m_overviewPage);
}

void DoctorDashboardWidget::buildPatientsPage() {
  m_patientsPage = new PatientWidget(this);
  if (m_patientService) {
    m_patientsPage->setPatientService(m_patientService);
  }
  int docId = m_currentUser ? m_currentUser->getAccountId() : -1;
  m_patientsPage->setServices(m_pharmacyService, m_medicalRecordService, m_appointmentService, docId);
  m_stackedWidget->addWidget(m_patientsPage);
}

void DoctorDashboardWidget::buildAppointmentsPage() {
  m_appointmentsPage = new QWidget(this);
  QVBoxLayout *pageLayout = new QVBoxLayout(m_appointmentsPage);
  pageLayout->setContentsMargins(0, 0, 0, 0);
  pageLayout->setSpacing(20);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *title = new QLabel("Lịch Hẹn Khám", m_appointmentsPage);
  title->setStyleSheet("font-size: 18px; font-weight: bold; color: #111827;");
  
  QLabel *lblFilter = new QLabel("Ngày:", m_appointmentsPage);
  lblFilter->setStyleSheet("font-size: 14px; font-weight: bold; color: #374151;");

  m_apptDateFilter = new QDateEdit(QDate::currentDate(), m_appointmentsPage);
  m_apptDateFilter->setCalendarPopup(true);
  m_apptDateFilter->setDisplayFormat("dd/MM/yyyy");
  m_apptDateFilter->setCursor(Qt::PointingHandCursor);
  m_apptDateFilter->setStyleSheet(
      "QDateEdit { background-color: #FFFFFF; color: #1E293B; border: 1px solid #CBD5E1; "
      "border-radius: 6px; padding: 6px 12px; font-size: 13px; font-weight: 600; min-width: 130px; }"
      "QDateEdit:hover { border-color: #2563EB; }"
      "QDateEdit::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 26px; "
      "border-left-width: 1px; border-left-color: #CBD5E1; border-left-style: solid; border-top-right-radius: 6px; border-bottom-right-radius: 6px; background-color: #F8FAFC; }"
      "QDateEdit::drop-down:hover { background-color: #EFF6FF; }"
  );
  connect(m_apptDateFilter, &QDateEdit::dateChanged, this, &DoctorDashboardWidget::refreshAppointmentsTables);

  QPushButton *btnRefresh = new QPushButton("Tải lại", m_appointmentsPage);
  btnRefresh->setCursor(Qt::PointingHandCursor);
  btnRefresh->setStyleSheet("QPushButton { background-color: #EFF6FF; color: #2563EB; border: 1px solid #2563EB; font-weight: bold; padding: 6px 14px; border-radius: 6px; } QPushButton:hover { background-color: #DBEAFE; }");
  connect(btnRefresh, &QPushButton::clicked, this, &DoctorDashboardWidget::refreshAppointmentsTables);

  headerLayout->addWidget(title);
  headerLayout->addStretch();
  headerLayout->addWidget(lblFilter);
  headerLayout->addWidget(m_apptDateFilter);
  headerLayout->addSpacing(10);
  headerLayout->addWidget(btnRefresh);
  pageLayout->addLayout(headerLayout);

  m_appointmentsTable = new QTableWidget(m_appointmentsPage);
  m_appointmentsTable->setColumnCount(7);
  m_appointmentsTable->setHorizontalHeaderLabels(
      {"Thời Gian", "Mã BN", "Tên Bệnh Nhân", "Dịch Vụ Khám", "Phòng Khám",
       "Trạng Thái", "Thao Tác"});
  m_appointmentsTable->setStyleSheet(
      "QTableWidget { background-color: #FFFFFF; border: 1px solid #EAEAEA; "
      "border-radius: 8px; gridline-color: #F1F3F4; outline: none; }"
      "QHeaderView::section { background-color: #F8F9FA; padding: 10px; "
      "font-weight: bold; border: none; border-bottom: 2px solid #EAEAEA; "
      "color: #5F6368; }"
      "QTableWidget::item { padding: 12px; color: #3C4043; outline: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }");
  m_appointmentsTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_appointmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_appointmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_appointmentsTable->setFocusPolicy(Qt::NoFocus);
  m_appointmentsTable->verticalHeader()->setVisible(false);

  connect(m_appointmentsTable, &QTableWidget::cellClicked, this,
          [this](int row, int col) {
            if (col == 6) return;
            if (row >= 0 && row < m_apptPageMeta.size()) {
              ApptMeta meta = m_apptPageMeta[row];
              openClinicalExamWithIds(meta.patientId, meta.appointmentId, meta.name, meta.code, meta.time, meta.reason);
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

  QStringList titles = {"Tổng số ca khám hôm nay", "Doanh thu phòng khám"};

  for (int i = 0; i < 2; ++i) {
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

    QLabel *lblValue = new QLabel("—", card);
    lblValue->setStyleSheet("color: #111827; font-size: 32px; font-weight: "
                            "900; border: none; letter-spacing: -1px;");

    if (i == 0) {
      m_lblCardAppointments = lblValue;
    } else {
      m_lblCardRevenue = lblValue;
    }

    QLabel *lblSubtitle = new QLabel("Hôm nay", card);
    lblSubtitle->setStyleSheet(
        "color: #6B7280; font-size: 12px; font-weight: 500; border: none;");

    QHBoxLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(lblSubtitle);
    subLayout->addStretch();

    cardLayout->addWidget(lblTitle);
    cardLayout->addWidget(lblValue);
    cardLayout->addLayout(subLayout);
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

  QBarSet *setMedical = new QBarSet("Đã khám");
  QBarSet *setAppointed = new QBarSet("Chờ khám");
  setMedical->setColor(QColor("#4B94F2"));
  setAppointed->setColor(QColor("#398CBF"));

  if (m_currentUser && m_appointmentService) {
    int docId = m_currentUser->getAccountId();
    QDate today = QDate::currentDate();
    for (int d = 6; d >= 0; --d) {
      QDate day = today.addDays(-d);
      auto appts = m_appointmentService->getDoctorAppointments(docId, day);
      int completed = 0, pending = 0;
      for (const auto &a : appts) {
        if (a.status == AppointmentStatusText::COMPLETED) completed++;
        else pending++;
      }
      *setMedical   << completed;
      *setAppointed << pending;
    }
  } else {
    for (int d = 0; d < 7; ++d) {
      *setMedical   << 0;
      *setAppointed << 0;
    }
  }

  QBarSeries *series = new QBarSeries();
  series->append(setMedical);
  series->append(setAppointed);

  QChart *chart = new QChart();
  chart->addSeries(series);
  chart->setTitle("Thống kê bệnh nhân hàng tuần");
  chart->setAnimationOptions(QChart::SeriesAnimations);
  chart->setBackgroundVisible(false);

  QStringList categories;
  QDate todayDate = QDate::currentDate();
  for (int d = 6; d >= 0; --d) {
    categories << todayDate.addDays(-d).toString("dd/MM");
  }
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

  QHBoxLayout *tblHeaderLayout = new QHBoxLayout();
  QLabel *tblTitle = new QLabel("Danh sách bệnh nhân đang khám hôm nay", tableCard);
  tblTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #111827;");
  
  QPushButton *btnRefresh = new QPushButton("Tải lại", tableCard);
  btnRefresh->setCursor(Qt::PointingHandCursor);
  btnRefresh->setStyleSheet("QPushButton { background-color: #EFF6FF; color: #2563EB; border: 1px solid #2563EB; font-weight: bold; padding: 4px 12px; border-radius: 6px; } QPushButton:hover { background-color: #DBEAFE; }");
  connect(btnRefresh, &QPushButton::clicked, this, &DoctorDashboardWidget::refreshAppointmentsTables);

  tblHeaderLayout->addWidget(tblTitle);
  tblHeaderLayout->addStretch();
  tblHeaderLayout->addWidget(btnRefresh);
  cardLayout->addLayout(tblHeaderLayout);

  m_patientTable = new QTableWidget(tableCard);
  m_patientTable->setColumnCount(5);
  m_patientTable->setHorizontalHeaderLabels({"Tên Bệnh Nhân", "Mã Định Danh",
                                             "Giờ Hẹn", "Chuyên Khoa",
                                             "Trạng Thái"});

  m_patientTable->setStyleSheet(
      "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: "
      "#F3F4F6; outline: none; }"
      "QHeaderView::section { background-color: #F9FAFB; padding: 12px; "
      "font-weight: bold; border: none; border-bottom: 2px solid #E5E7EB; "
      "color: #6B7280; }"
      "QTableWidget::item { padding: 12px; color: #111827; border-bottom: 1px "
      "solid #F3F4F6; outline: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }");

  cardLayout->addWidget(m_patientTable);
  pageLayout->addWidget(tableCard);

  m_patientTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_patientTable->setFocusPolicy(Qt::NoFocus);
  m_patientTable->verticalHeader()->setVisible(false);

  connect(m_patientTable, &QTableWidget::cellClicked, this,
          [this](int row, int col) {
            if (col == 6) return;
            if (row >= 0 && row < m_rowApptMeta.size()) {
              ApptMeta meta = m_rowApptMeta[row];
              openClinicalExamWithIds(meta.patientId, meta.appointmentId, meta.name, meta.code, meta.time, meta.reason);
            }
          });

  m_patientTable->setMinimumHeight(220);
  pageLayout->addWidget(m_patientTable);
}

void DoctorDashboardWidget::switchPage(int index, QPushButton *activeBtn) {
  if (!m_stackedWidget)
    return;
  m_stackedWidget->setCurrentIndex(index);

  QPushButton* buttons[] = { m_btnDash, m_btnPatients, m_btnAppoint, m_btnLeaveManage, m_btnSetting };
  for (auto* btn : buttons) {
    if (btn) {
      btn->setObjectName("");
      btn->style()->unpolish(btn);
      btn->style()->polish(btn);
    }
  }

  if (activeBtn) {
    activeBtn->setObjectName("activeBtn");
    activeBtn->style()->unpolish(activeBtn);
    activeBtn->style()->polish(activeBtn);
  }

  if (index == 0 || index == 2) {
    refreshAppointmentsTables();
  } else if (index == 1 && m_patientsPage) {
    m_patientsPage->loadPatientsData();
  }
}

void DoctorDashboardWidget::buildClinicalExamPage() {
  m_clinicalExamPage = new ClinicalExamWidget(m_medicalRecordService, this);
  m_clinicalExamPage->setServices(m_pharmacyService, m_patientService, m_appointmentService, m_serviceRequestService);
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

  int patientId = 0;
  int appointmentId = 0;
  if (isFromTodayList) {
    if (rowIndex >= 0 && rowIndex < m_rowApptMeta.size()) {
      patientId = m_rowApptMeta[rowIndex].patientId;
      appointmentId = m_rowApptMeta[rowIndex].appointmentId;
    }
  } else {
    if (rowIndex >= 0 && rowIndex < m_apptPageMeta.size()) {
      patientId = m_apptPageMeta[rowIndex].patientId;
      appointmentId = m_apptPageMeta[rowIndex].appointmentId;
    }
  }

  m_clinicalExamPage->loadPatientInfo(patientId, appointmentId, name, id, time, specialty);

  int idx = m_stackedWidget->indexOf(m_clinicalExamPage);
  if (idx != -1) {
    switchPage(idx, nullptr);
  }
}

void DoctorDashboardWidget::openClinicalExamWithIds(
    int patientId, int appointmentId, const QString &name, const QString &code,
    const QString &time, const QString &reason) {
  if (!m_clinicalExamPage || !m_stackedWidget)
    return;

  if (m_appointmentService && appointmentId > 0) {
    m_appointmentService->updateAppointmentStatus(appointmentId, AppointmentStatusText::STARTED);
  }

  m_clinicalExamPage->loadPatientInfo(patientId, appointmentId, name, code, time, reason);

  int idx = m_stackedWidget->indexOf(m_clinicalExamPage);
  if (idx != -1) {
    switchPage(idx, nullptr);
  }
}

void DoctorDashboardWidget::handleCallPatientRequested(int targetApptId) {
  if (!m_appointmentService) {
    QMessageBox::warning(this, "Lỗi", "Chưa kết nối dịch vụ cuộc hẹn.");
    return;
  }

  int apptId = targetApptId;
  if (apptId <= 0 && m_clinicalExamPage) {
    apptId = m_clinicalExamPage->currentAppointmentId();
  }

  if (apptId <= 0) {
    QMessageBox::warning(this, "Thông báo", "Vui lòng chọn một bệnh nhân từ danh sách để gọi khám.");
    return;
  }

  QPair<int, QString> result =
      m_appointmentService->callSpecificPatient(apptId);

  if (result.first > 0) {
    QMessageBox::information(
        this, "Gọi Khám Thành Công",
        QString("Đã gọi bệnh nhân:\n\nSố vé: %1\nTên: %2")
            .arg(result.first)
            .arg(result.second));

    QTimer::singleShot(0, this, [this]() {
        refreshAppointmentsTables();
    });
  } else {
    QMessageBox::warning(this, "Thông báo", "Không thể gọi bệnh nhân được chọn hoặc không tìm thấy dữ liệu lượt khám.");
  }
}

void DoctorDashboardWidget::handlePatientExamFinished() {
  if (!m_appointmentService) {
    QMessageBox::warning(this, "Lỗi", "Chưa kết nối dịch vụ cuộc hẹn.");
    return;
  }

  int apptId = 0;
  if (m_clinicalExamPage) {
    apptId = m_clinicalExamPage->currentAppointmentId();
  }

  if (apptId > 0) {
    m_appointmentService->updateAppointmentStatus(apptId, AppointmentStatusText::COMPLETED);
  }

  QTimer::singleShot(0, this, [this]() {
      refreshAppointmentsTables();
      QMessageBox::information(this, "Nova Care Clinic", "Đã kết thúc ca khám thành công!");
      switchPage(0, m_btnDash);
  });
}

void DoctorDashboardWidget::refreshAppointmentsTables() {

  int docId = m_currentUser ? m_currentUser->getAccountId() : -1;
  QDate today = QDate::currentDate();

  if (m_patientTable) {
    auto records =
        m_appointmentService ? m_appointmentService->getDoctorAppointments(docId, today) : QList<AppointmentRecordDTO>();
    m_rowApptMeta.clear();
    m_patientTable->setRowCount(0);
    int rowIdx = 0;
    for (const auto &rec : records) {
      // Chỉ hiển thị bệnh nhân Đã check-in (chờ khám) và Đang khám
      if (rec.status != AppointmentStatusText::CHECKED_IN && 
          rec.status != AppointmentStatusText::STARTED) {
        continue;
      }

      QString statusText = AppointmentStatusText::toVi(rec.status);
      QString statusColor = "#2563EB";

      if (rec.status == AppointmentStatusText::STARTED) {
        statusColor = "#EAB308";
      }

      m_patientTable->insertRow(rowIdx);

      QTableWidgetItem *nameItem = new QTableWidgetItem(rec.patientName);
      QTableWidgetItem *codeItem = new QTableWidgetItem(rec.patientCode);
      QTableWidgetItem *timeItem = new QTableWidgetItem(rec.startTime);
      QTableWidgetItem *deptItem = new QTableWidgetItem(rec.reason);
      QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);

      statusItem->setForeground(QBrush(QColor(statusColor)));
      statusItem->setFont(QFont("Segoe UI", 10, QFont::Bold));

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
    QDate filterDate = m_apptDateFilter ? m_apptDateFilter->date() : today;
    auto records = m_appointmentService ? m_appointmentService->getDoctorAppointments(docId, filterDate) : QList<AppointmentRecordDTO>();
    m_apptPageMeta.clear();
    m_appointmentsTable->setRowCount(0);
    int rowIdx = 0;
    for (const auto &rec : records) {
      // Hiển thị tất cả lịch hẹn chưa hủy/chưa vắng mặt (bao gồm Đã hẹn, Đã check-in, Đang khám, Đã khám)
      if (rec.status == AppointmentStatusText::CANCELLED || 
          rec.status == AppointmentStatusText::NO_SHOW) {
        continue;
      }

      QString statusText = AppointmentStatusText::toVi(rec.status);
      QString statusColor = "#2563EB"; // Blue for SCHEDULED

      if (rec.status == AppointmentStatusText::CHECKED_IN) {
        statusColor = "#059669"; // Green for CHECKED_IN
      } else if (rec.status == AppointmentStatusText::STARTED) {
        statusColor = "#D97706"; // Amber for STARTED
      } else if (rec.status == AppointmentStatusText::COMPLETED) {
        statusColor = "#6B7280"; // Gray for COMPLETED
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

      // Action Buttons Widget (Single Call Patient Button)
      QWidget *actWidget = new QWidget(m_appointmentsTable);
      QHBoxLayout *actLayout = new QHBoxLayout(actWidget);
      actLayout->setContentsMargins(4, 3, 4, 3);
      int captureApptId = rec.appointmentId;
      int capturePatientId = rec.patientId;
      QString captureName = rec.patientName;
      QString captureCode = rec.patientCode;
      QString captureTime = rec.startTime;
      QString captureReason = rec.reason;

      if (rec.status != AppointmentStatusText::COMPLETED) {
          QPushButton *btnCall = new QPushButton("Gọi Khám", actWidget);
          btnCall->setCursor(Qt::PointingHandCursor);
          btnCall->setMinimumHeight(32);
          btnCall->setStyleSheet(
              "QPushButton { background-color: #2563EB; color: white; font-size: 12px; font-weight: bold; font-family: 'Segoe UI'; border-radius: 6px; padding: 6px 16px; border: none; }"
              "QPushButton:hover { background-color: #1D4ED8; }"
          );

          connect(btnCall, &QPushButton::clicked, this, [this, capturePatientId, captureApptId, captureName, captureCode, captureTime, captureReason]() {
              if (!m_appointmentService) return;
              auto res = m_appointmentService->callSpecificPatient(captureApptId);
              if (res.first > 0) {
                  QMessageBox::information(this, "Gọi Khám Thành Công", QString("Đã gọi bệnh nhân: %1 (Vé: %2)").arg(res.second).arg(res.first));
              }
              QTimer::singleShot(0, this, [this, capturePatientId, captureApptId, captureName, captureCode, captureTime, captureReason]() {
                  openClinicalExamWithIds(capturePatientId, captureApptId, captureName, captureCode, captureTime, captureReason);
                  refreshAppointmentsTables();
              });
          });
          actLayout->addWidget(btnCall);
      }
      actLayout->setAlignment(Qt::AlignCenter);

      m_appointmentsTable->setCellWidget(rowIdx, 6, actWidget);
      m_appointmentsTable->setRowHeight(rowIdx, 54);

      m_apptPageMeta.append(ApptMeta{rec.appointmentId, rec.patientId,
                                     rec.patientName, rec.patientCode,
                                     rec.startTime, rec.reason});
      rowIdx++;
    }
  }

  refreshOverviewCards();
}

void DoctorDashboardWidget::refreshOverviewCards() {
  if (!m_currentUser || !m_appointmentService) return;

  int docId = m_currentUser->getAccountId();
  QDate today = QDate::currentDate();

  auto appts = m_appointmentService->getDoctorAppointments(docId, today);
  if (m_lblCardAppointments) {
    m_lblCardAppointments->setText(QString::number(appts.size()));
  }

  if (m_lblCardRevenue) {
    m_lblCardRevenue->setText("N/A");
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
  header->setSectionResizeMode(2, QHeaderView::Stretch);

  m_tableLeaveHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableLeaveHistory->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tableLeaveHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableLeaveHistory->setFocusPolicy(Qt::NoFocus);
  m_tableLeaveHistory->setStyleSheet(
      "QTableWidget { border: 1px solid #DADCE0; border-radius: 4px; color: "
      "#202124; background-color: white; outline: none; }"
      "QTableWidget::item { outline: none; border: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }"
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
    LeaveBalanceDTO balance = m_staffService->getLeaveBalance(
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
  auto historyList = m_staffService->getOwnLeaveHistory(staffId);

  for (const auto &req : historyList) {
    int row = m_tableLeaveHistory->rowCount();
    m_tableLeaveHistory->insertRow(row);

    m_tableLeaveHistory->setItem(
        row, 0, new QTableWidgetItem(req.startDate.toString("dd/MM/yyyy")));
    m_tableLeaveHistory->setItem(
        row, 1, new QTableWidgetItem(req.endDate.toString("dd/MM/yyyy")));
    m_tableLeaveHistory->setItem(row, 2, new QTableWidgetItem(req.reason));

    QTableWidgetItem *statusItem = new QTableWidgetItem(LeaveStatusText::toVi(req.status));
    statusItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
    if (req.status == LeaveStatusText::PENDING) {
      statusItem->setForeground(QColor("#F29900"));
    } else if (req.status == LeaveStatusText::APPROVED) {
      statusItem->setForeground(QColor("#34A853"));
    } else if (req.status == LeaveStatusText::REJECTED) {
      statusItem->setForeground(QColor("#EA4335"));
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
        m_staffService->registerLeave(staffId, startDate, endDate, reason);
    if (errorMsg.isEmpty()) {
      int year = startDate.year();

      if (m_appointmentService) {
        int cancelledCount =
            m_appointmentService->cancelAppointmentsForDoctor(
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
          m_staffService->getLeaveBalance(staffId, year);
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