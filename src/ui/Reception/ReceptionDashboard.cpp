#include "ReceptionDashboard.h"
#include "../../model/Doctor.h"
#include "../../model/IAuthenticatable.h"

#include "../../service/AnalyticService.h"
#include "../../service/AppointmentService.h"
#include "../../service/PatientService.h"
#include "../../service/StaffService.h"
#include "../Doctor/PatientRegistrationDialog.h"
#include "../view/Profile.h"
#include "RoomQueueWidget.h"
#include "RoomQueueDialog.h"
#include "PatientEditDialog.h"
#include <QComboBox>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStringList>
#include <QTableWidget>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QTextCharFormat>
#include "../../model/CommonEnums.h"

ReceptionDashboardWidget::ReceptionDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<AppointmentService> appointmentService,
    std::shared_ptr<AnalyticService> analyticService, QWidget *parent)
    : BaseDashboardWidget(user, staffService, parent),
      m_patientService(patientService),
      m_appointmentService(appointmentService),
      m_analyticService(analyticService) {

  initializeDashboard();

  m_stackedWidget = new QStackedWidget(m_mainContentWidget);
  m_mainContentLayout->addWidget(m_stackedWidget, 1);

  buildOverviewPage();
  buildRegisterPage();
  buildPatientsPage();
  buildAppointmentsPage();
  buildRoomQueuePage();
  m_stackedWidget->addWidget(m_overviewPage);
  m_stackedWidget->addWidget(m_registerPage);
  m_stackedWidget->addWidget(m_patientsPage);
  m_stackedWidget->addWidget(m_appointmentsPage);
  m_stackedWidget->addWidget(m_roomQueuePage);

  // Default to Overview
  switchPage(0, m_btnOverview);

  fillDashboardData();
}

void ReceptionDashboardWidget::fillDashboardData() {
  buildSidebar();

  if (m_analyticService) {
    QDate today = QDate::currentDate();
    QDate startOfMonth = QDate(today.year(), today.month(), 1);

    IncomeStatsDTO incomeStats = m_analyticService->getIncomeStats(startOfMonth, today);
    if (m_lblRevenue) {
      m_lblRevenue->setText(
          QLocale(QLocale::Vietnamese).toCurrencyString(incomeStats.total, "₫"));
    }

    PatientStatsDTO patientStats = m_analyticService->getPatientStats(startOfMonth, today);
    if (m_lblPatientNum) {
      m_lblPatientNum->setText(QString("%1 lượt khám").arg(patientStats.total));
    }
  }

  refreshRecentActivity();
}

QFrame *ReceptionDashboardWidget::makeCard(QWidget *parent) {
  QFrame *card = new QFrame(parent);
  card->setStyleSheet(
      "QFrame { background-color: #FFFFFF; border-radius: 12px; }");

  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
  shadow->setBlurRadius(15);
  shadow->setColor(QColor(0, 0, 0, 15));
  shadow->setOffset(0, 5);
  card->setGraphicsEffect(shadow);

  return card;
}

void ReceptionDashboardWidget::buildSidebar() {
  if (!m_sidebarLayout)
    return;

  QLabel *lblTitle = new QLabel("LỄ TÂN", m_sidebarFrame);
  lblTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #2563EB; "
                          "letter-spacing: 1px; padding-left: 20px; margin-top: 5px; margin-bottom: 15px;");
  m_sidebarLayout->addWidget(lblTitle);

  m_btnOverview = new QPushButton("Tổng Quan", m_sidebarFrame);
  m_btnRegister = new QPushButton("Đăng Ký Khám", m_sidebarFrame);
  m_btnPatients = new QPushButton("Bệnh Nhân", m_sidebarFrame);
  m_btnManageAppts = new QPushButton("Lịch Hẹn", m_sidebarFrame);
  m_btnRoomQueue = new QPushButton("Hàng Đợi", m_sidebarFrame);

  m_btnOverview->setCursor(Qt::PointingHandCursor);
  m_btnRegister->setCursor(Qt::PointingHandCursor);
  m_btnPatients->setCursor(Qt::PointingHandCursor);
  m_btnManageAppts->setCursor(Qt::PointingHandCursor);
  m_btnRoomQueue->setCursor(Qt::PointingHandCursor);

  m_sidebarLayout->addWidget(m_btnOverview);
  m_sidebarLayout->addWidget(m_btnRegister);
  m_sidebarLayout->addWidget(m_btnPatients);
  m_sidebarLayout->addWidget(m_btnManageAppts);
  m_sidebarLayout->addWidget(m_btnRoomQueue);
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

  connect(m_btnOverview, &QPushButton::clicked, this,
          [this]() { switchPage(0, m_btnOverview); });
  connect(m_btnRegister, &QPushButton::clicked, this,
          [this]() { switchPage(1, m_btnRegister); });
  connect(m_btnPatients, &QPushButton::clicked, this,
          [this]() { 
              handlePatientFilterChanged();
              switchPage(2, m_btnPatients); 
          });
  connect(m_btnManageAppts, &QPushButton::clicked, this,
          [this]() { 
              updateAppointmentsTable();
              switchPage(3, m_btnManageAppts); 
          });
  connect(m_btnRoomQueue, &QPushButton::clicked, this,
          [this]() { 
              onRefreshRoomQueue();
              switchPage(4, m_btnRoomQueue); 
          });

  connect(m_btnLogout, &QPushButton::clicked, this,
          &BaseDashboardWidget::logoutRequested);
}

void ReceptionDashboardWidget::switchPage(int index, QPushButton *activeBtn) {
  if (!m_stackedWidget) return;
  m_stackedWidget->setCurrentIndex(index);

  QPushButton* buttons[] = { m_btnOverview, m_btnRegister, m_btnPatients, m_btnManageAppts, m_btnRoomQueue };
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

  if (index == 0) {
    refreshRecentActivity();
  }
}

void ReceptionDashboardWidget::buildOverviewPage() {
  m_overviewPage = new QWidget();
  m_overviewPage->setStyleSheet("background-color: #F8F9FA;");

  QVBoxLayout *layout = new QVBoxLayout(m_overviewPage);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(20);

  QLabel *lblTitle = new QLabel("Tổng quan tháng này", m_overviewPage);
  lblTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #202124;");
  layout->addWidget(lblTitle);

  // Cards layout
  QHBoxLayout *cardsLayout = new QHBoxLayout();
  cardsLayout->setSpacing(20);

  // Card 1: Revenue
  QFrame *card1 = makeCard(m_overviewPage);
  QVBoxLayout *c1Layout = new QVBoxLayout(card1);
  QLabel *c1Title = new QLabel("Doanh thu", card1);
  c1Title->setStyleSheet("color: #5F6368; font-size: 14px;");
  m_lblRevenue = new QLabel("0 VND", card1);
  m_lblRevenue->setStyleSheet(
      "color: #4B94F2; font-size: 28px; font-weight: bold;");
  c1Layout->addWidget(c1Title);
  c1Layout->addWidget(m_lblRevenue);
  cardsLayout->addWidget(card1);

  // Card 2: Patients
  QFrame *card2 = makeCard(m_overviewPage);
  QVBoxLayout *c2Layout = new QVBoxLayout(card2);
  QLabel *c2Title = new QLabel("Lượt khám", card2);
  c2Title->setStyleSheet("color: #5F6368; font-size: 14px;");
  m_lblPatientNum = new QLabel("0", card2);
  m_lblPatientNum->setStyleSheet(
      "color: #34A853; font-size: 28px; font-weight: bold;");
  c2Layout->addWidget(c2Title);
  c2Layout->addWidget(m_lblPatientNum);
  cardsLayout->addWidget(card2);

  cardsLayout->addStretch();
  layout->addLayout(cardsLayout);

  QLabel *lblSubTitle = new QLabel("Hoạt động gần đây", m_overviewPage);
  lblSubTitle->setStyleSheet(
      "font-size: 18px; font-weight: bold; color: #202124; margin-top: 20px;");
  layout->addWidget(lblSubTitle);

  m_recentActivityTable = new QTableWidget(0, 4, m_overviewPage);
  m_recentActivityTable->setHorizontalHeaderLabels(
      {"Thời gian", "Bệnh nhân", "Bác sĩ", "Trạng thái"});
  m_recentActivityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_recentActivityTable->setStyleSheet("QTableWidget { background-color: white; "
                           "border-radius: 8px; border: 1px solid #EAEAEA; color: #333333; outline: none; }"
                           "QTableWidget::item { outline: none; border: none; }"
                           "QTableWidget::item:focus { outline: none; border: none; }"
                           "QHeaderView::section { background-color: #F1F3F4; "
                           "font-weight: bold; border: none; padding: 8px; color: #5F6368; }");
  m_recentActivityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_recentActivityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_recentActivityTable->setFocusPolicy(Qt::NoFocus);
  layout->addWidget(m_recentActivityTable);
  layout->addStretch();
}

void ReceptionDashboardWidget::refreshRecentActivity() {
  if (!m_recentActivityTable || !m_appointmentService) return;

  auto appts = m_appointmentService->getAppointmentsByDate(QDate::currentDate());
  int showCount = qMin<int>(appts.size(), 10);

  m_recentActivityTable->setRowCount(showCount);
  for (int i = 0; i < showCount; ++i) {
    const auto &a = appts[i];
    QString timeStr = a.startTime;
    if (!a.endTime.isEmpty()) timeStr += " - " + a.endTime;

    m_recentActivityTable->setItem(i, 0, new QTableWidgetItem(timeStr));
    m_recentActivityTable->setItem(i, 1, new QTableWidgetItem(a.patientName));
    m_recentActivityTable->setItem(i, 2, new QTableWidgetItem(a.doctorName));
    m_recentActivityTable->setItem(i, 3,
        new QTableWidgetItem(AppointmentStatusText::toVi(a.status)));
  }
}

void ReceptionDashboardWidget::onContinueClicked() {
  QString phone = m_txtPatientPhone->text().trimmed();
  QString citizenId = m_txtPatientCitizenId->text().trimmed();
  if (phone.isEmpty() && citizenId.isEmpty()) {
    QMessageBox::warning(
        this, "Lỗi", "Vui lòng nhập Số điện thoại hoặc CMND/CCCD bệnh nhân.");
    return;
  }

  auto patientOpt =
      m_patientService->getPatientByPhoneOrCitizenId(phone, citizenId);
  if (patientOpt) {
    m_currentPatientId = patientOpt->patientId;
    m_txtPatientPhone->setText(patientOpt->phone);
    m_txtPatientPhone->setReadOnly(true);
    m_txtPatientCitizenId->setReadOnly(true);

    m_apptCard->setVisible(true);
    m_btnContinue->setText("Đã xác nhận");
    m_btnContinue->setEnabled(false);
    m_btnContinue->setStyleSheet(
        "background-color: #EAEAEA; color: #999; padding: 10px 25px; "
        "border-radius: 6px; font-size: 15px; font-weight: bold;");
  } else {
    QMessageBox::warning(
        this, "Lỗi",
        "Không tìm thấy bệnh nhân. Vui lòng tạo bệnh nhân mới trước khi đăng ký khám.");
  }
}

void ReceptionDashboardWidget::onConfirmClicked() {
  if (m_currentPatientId == -1) {
    QMessageBox::warning(this, "Lỗi",
                         "Vui lòng xác nhận thông tin bệnh nhân trước.");
    return;
  }

  int doctorId = m_selectedDoctorId;
  if (doctorId <= 0) {
    QMessageBox::warning(this, "Lỗi", "Vui lòng chọn bác sĩ.");
    return;
  }

  if (m_selectedTimeSlot.isEmpty()) {
    QMessageBox::warning(this, "Lỗi", "Vui lòng chọn một khung giờ trống.");
    return;
  }

  QDate date = m_dateEdit->date();
  QString timeSlot = m_selectedTimeSlot;
  QStringList parts = timeSlot.split("-");
  QTime startTime = QTime::fromString(parts.first().trimmed(), "HH:mm");
  QTime endTime = (parts.size() > 1)
                      ? QTime::fromString(parts.last().trimmed(), "HH:mm")
                      : QTime();
  int createdBy = m_currentUser->getAccountId();

  AppointmentInputDTO input;
  input.patientId = m_currentPatientId;
  input.doctorId = doctorId;
  input.createdBy = createdBy;
  input.ticketNumber = 1;
  input.date = date;
  input.startTime = startTime;
  input.endTime = endTime;
  input.reason = "Khám bệnh";

  QString errorMsg = m_appointmentService->createAppointment(input);
  if (errorMsg.isEmpty()) {
    QMessageBox::information(this, "Thành công",
                             "Đăng ký lịch khám thành công!");

    // Reset form
    m_currentPatientId = -1;
    m_txtPatientPhone->clear();
    m_txtPatientPhone->setReadOnly(false);
    m_txtPatientCitizenId->clear();
    m_txtPatientCitizenId->setReadOnly(false);

    m_apptCard->setVisible(false);
    m_btnContinue->setText("Xác nhận & Tiếp tục");
    m_btnContinue->setEnabled(true);
    m_btnContinue->setStyleSheet(
        "background-color: #4B94F2; color: white; padding: 10px 25px; "
        "border-radius: 6px; font-size: 15px; font-weight: bold;");
        
    m_dateEdit->setDate(QDate::currentDate());
    m_comboSpecialty->setCurrentIndex(0);

    updateDoctorList();
  } else {
    QMessageBox::warning(this, "Lỗi", errorMsg);
  }
}

void ReceptionDashboardWidget::buildRegisterPage() {
  QScrollArea *mainScroll = new QScrollArea();
  mainScroll->setWidgetResizable(true);
  mainScroll->setFrameShape(QFrame::NoFrame);
  mainScroll->setStyleSheet(
      "QScrollArea { background-color: #F8F9FA; border: none; }");

  QWidget *contentWidget = new QWidget();
  contentWidget->setStyleSheet("background-color: transparent;");
  
  QVBoxLayout *layout = new QVBoxLayout(contentWidget);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(20);

  QHBoxLayout *titleLayout = new QHBoxLayout();
  QLabel *lblTitle = new QLabel("Đăng ký khám bệnh", contentWidget);
  lblTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #202124;");

  QPushButton *btnAddPatient =
      new QPushButton("+ Thêm bệnh nhân mới", contentWidget);
  btnAddPatient->setStyleSheet(
      "background-color: #34A853; color: white; padding: 8px 15px; "
      "border-radius: 6px; font-weight: bold;");
  btnAddPatient->setCursor(Qt::PointingHandCursor);

  titleLayout->addWidget(lblTitle);
  titleLayout->addStretch();
  titleLayout->addWidget(btnAddPatient);

  layout->addLayout(titleLayout);

  connect(btnAddPatient, &QPushButton::clicked, this, [this]() {
    PatientRegistrationDialog dialog(m_patientService, this);
    connect(&dialog, &PatientRegistrationDialog::saved, this,
            [this](const QString &phone, const QString &citizenId, const QString &name) {
              Q_UNUSED(name);
              if (!m_patientService) return;
              auto patientOpt = m_patientService->getPatientByPhoneOrCitizenId(phone, citizenId);
              if (patientOpt.has_value()) {
                m_currentPatientId = patientOpt->patientId;
                if (m_txtPatientPhone) m_txtPatientPhone->setText(patientOpt->phone);
                if (m_txtPatientCitizenId) m_txtPatientCitizenId->setText(citizenId);

                QMessageBox::information(this, "Xác Nhận Thông Tin Bệnh Nhân",
                    QString("Đã xác nhận bệnh nhân thành công!\n\n"
                            "Họ tên: %1\nMã bệnh nhân: %2\nSố điện thoại: %3\nCCCD: %4\n\n"
                            "Vui lòng chọn Chuyên khoa và Bác sĩ bên dưới để hoàn tất Đăng ký khám.")
                        .arg(patientOpt->fullName, patientOpt->patientCode, patientOpt->phone, citizenId));
              }
              if (m_txtPatientPhone) m_txtPatientPhone->setReadOnly(true);
              if (m_txtPatientCitizenId) m_txtPatientCitizenId->setReadOnly(true);

              if (m_btnContinue) {
                m_btnContinue->setText("Đã xác nhận");
                m_btnContinue->setEnabled(false);
                m_btnContinue->setStyleSheet(
                    "background-color: #EAEAEA; color: #999; padding: 10px 25px; "
                    "border-radius: 6px; font-size: 15px; font-weight: bold;");
              }

              if (m_apptCard) m_apptCard->setVisible(true);
            });
    dialog.exec();
  });

  QString inputStyle =
      "QLineEdit, QComboBox, QDateEdit { "
      "padding: 8px 10px; border: 1px solid #BDBDBD; "
      "border-radius: 6px; background-color: #FFFFFF; "
      "color: #333333; font-size: 14px; } "
      "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { "
      "border: 1px solid #4B94F2; } "
      "QComboBox QAbstractItemView { "
      "background-color: white; color: #333333; "
      "selection-background-color: #4B94F2; selection-color: white; outline: "
      "none; border: 1px solid #EAEAEA; }";

  QFrame *patientCard = makeCard(contentWidget);
  QVBoxLayout *patientLayout = new QVBoxLayout(patientCard);
  patientLayout->setContentsMargins(25, 25, 25, 25);
  patientLayout->setSpacing(15);

  QLabel *lblStep1 = new QLabel("Bước 1: Thông tin bệnh nhân", patientCard);
  lblStep1->setStyleSheet("font-size: 18px; font-weight: bold; color: #4B94F2;");
  patientLayout->addWidget(lblStep1);

  QHBoxLayout *patientFieldsLayout = new QHBoxLayout();
  QVBoxLayout *col1 = new QVBoxLayout();
  QLabel *lblPhone = new QLabel("Số điện thoại (*):", patientCard);
  lblPhone->setStyleSheet("font-weight: bold; color: #555;");
  m_txtPatientPhone = new QLineEdit(patientCard);
  m_txtPatientPhone->setPlaceholderText("Nhập số điện thoại để tìm kiếm...");
  m_txtPatientPhone->setStyleSheet(inputStyle);
  col1->addWidget(lblPhone);
  col1->addWidget(m_txtPatientPhone);

  QVBoxLayout *col2 = new QVBoxLayout();
  QLabel *lblCitizen = new QLabel("Căn cước công dân (*):", patientCard);
  lblCitizen->setStyleSheet("font-weight: bold; color: #555;");
  m_txtPatientCitizenId = new QLineEdit(patientCard);
  m_txtPatientCitizenId->setPlaceholderText("Nhập CMND/CCCD để tìm kiếm...");
  m_txtPatientCitizenId->setStyleSheet(inputStyle);
  col2->addWidget(lblCitizen);
  col2->addWidget(m_txtPatientCitizenId);

  patientFieldsLayout->addLayout(col1);
  patientFieldsLayout->addLayout(col2);
  patientLayout->addLayout(patientFieldsLayout);

  QHBoxLayout *btnNextLayout = new QHBoxLayout();
  btnNextLayout->addStretch();
  m_btnContinue = new QPushButton("Xác nhận & Tiếp tục", patientCard);
  m_btnContinue->setStyleSheet(
      "background-color: #4B94F2; color: white; padding: 10px 25px; "
      "border-radius: 6px; font-size: 15px; font-weight: bold;");
  m_btnContinue->setCursor(Qt::PointingHandCursor);
  btnNextLayout->addWidget(m_btnContinue);

  connect(m_btnContinue, &QPushButton::clicked, this,
          &ReceptionDashboardWidget::onContinueClicked);

  patientLayout->addLayout(btnNextLayout);
  layout->addWidget(patientCard);

  m_apptCard = makeCard(contentWidget);
  QVBoxLayout *apptLayout = new QVBoxLayout(m_apptCard);
  apptLayout->setContentsMargins(25, 25, 25, 25);
  apptLayout->setSpacing(15);
  m_apptCard->setVisible(false);

  QLabel *lblStep2 = new QLabel("Bước 2: Chọn khung giờ khám", m_apptCard);
  lblStep2->setStyleSheet("font-size: 18px; font-weight: bold; color: #4B94F2;");
  apptLayout->addWidget(lblStep2);

  QHBoxLayout *apptFieldsLayout = new QHBoxLayout();
  QVBoxLayout *colSpecialty = new QVBoxLayout();
  QLabel *lblSpecialty = new QLabel("Chuyên khoa:", m_apptCard);
  lblSpecialty->setStyleSheet("font-weight: bold; color: #555;");
  m_comboSpecialty = new QComboBox(m_apptCard);
  m_comboSpecialty->addItem("Tất cả");
  m_comboSpecialty->setStyleSheet(inputStyle);
  colSpecialty->addWidget(lblSpecialty);
  colSpecialty->addWidget(m_comboSpecialty);

  QVBoxLayout *colDate = new QVBoxLayout();
  QLabel *lblDate = new QLabel("Ngày khám:", m_apptCard);
  lblDate->setStyleSheet("font-weight: bold; color: #555;");
  m_dateEdit = new QDateEdit(QDate::currentDate(), m_apptCard);
  m_dateEdit->setMinimumDate(QDate::currentDate());
  m_dateEdit->setDisplayFormat("dd/MM/yyyy");
  m_dateEdit->setMinimumWidth(150);
  QCalendarWidget* apptCalendar = new QCalendarWidget();
  apptCalendar->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
  apptCalendar->setMinimumSize(330, 250);
  m_dateEdit->setCalendarWidget(apptCalendar);
  m_dateEdit->setCalendarPopup(true);
  m_dateEdit->setStyleSheet(inputStyle);
  
  // Custom calendar theme
  m_dateEdit->calendarWidget()->setStyleSheet(
      "QCalendarWidget { background-color: #FFFFFF; color: #333333; }"
      "QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: #FFFFFF; border-bottom: 1px solid #EAEAEA; min-height: 36px; }"
      "QCalendarWidget QToolButton { color: #333333; font-weight: bold; background-color: transparent; border: none; padding: 4px 6px; margin: 1px; font-size: 13px; }"
      "QCalendarWidget QToolButton:hover { background-color: #E3F2FD; border-radius: 4px; }"
      "QCalendarWidget QMenu { background-color: #FFFFFF; color: #333333; }"
      "QCalendarWidget QSpinBox { background-color: #FFFFFF; color: #333333; selection-background-color: #4B94F2; selection-color: white; font-size: 13px; }"
      "QCalendarWidget QAbstractItemView:enabled { font-size: 13px; color: #333333; background-color: #FFFFFF; selection-background-color: #4B94F2; selection-color: #FFFFFF; }"
      "QCalendarWidget QAbstractItemView:disabled { color: #CCCCCC; }"
  );
  
  colDate->addWidget(lblDate);
  colDate->addWidget(m_dateEdit);

  apptFieldsLayout->addLayout(colSpecialty);
  apptFieldsLayout->addLayout(colDate);
  apptFieldsLayout->addStretch();
  apptLayout->addLayout(apptFieldsLayout);

  QWidget* listWidget = new QWidget(m_apptCard);
  listWidget->setStyleSheet("background-color: transparent;");
  m_doctorListLayout = new QVBoxLayout(listWidget);
  m_doctorListLayout->setContentsMargins(0, 10, 0, 10);
  m_doctorListLayout->setSpacing(20);
  apptLayout->addWidget(listWidget);

  QHBoxLayout *btnSubmitLayout = new QHBoxLayout();
  btnSubmitLayout->addStretch();

  m_btnCancel = new QPushButton("Hủy bỏ", m_apptCard);
  m_btnCancel->setStyleSheet(
      "background-color: #F44336; color: white; padding: 12px 30px; "
      "border-radius: 6px; font-size: 16px; font-weight: bold;");
  m_btnCancel->setCursor(Qt::PointingHandCursor);
  btnSubmitLayout->addWidget(m_btnCancel);

  m_btnConfirm = new QPushButton("Hoàn tất đăng ký", m_apptCard);
  m_btnConfirm->setStyleSheet(
      "background-color: #34A853; color: white; padding: 12px 30px; "
      "border-radius: 6px; font-size: 16px; font-weight: bold;");
  m_btnConfirm->setCursor(Qt::PointingHandCursor);
  btnSubmitLayout->addWidget(m_btnConfirm);

  apptLayout->addLayout(btnSubmitLayout);
  layout->addWidget(m_apptCard);
  layout->addStretch();

  connect(m_comboSpecialty, &QComboBox::currentTextChanged, this,
          &ReceptionDashboardWidget::updateDoctorList);
  connect(m_dateEdit, &QDateEdit::dateChanged, this,
          &ReceptionDashboardWidget::updateDoctorList);
  connect(m_btnConfirm, &QPushButton::clicked, this,
          &ReceptionDashboardWidget::onConfirmClicked);
          
  connect(m_btnCancel, &QPushButton::clicked, this, [this]() {
    m_currentPatientId = -1;
    m_txtPatientPhone->clear();
    m_txtPatientCitizenId->clear();
    m_txtPatientPhone->setReadOnly(false);
    m_txtPatientCitizenId->setReadOnly(false);
    m_btnContinue->setText("Xác nhận & Tiếp tục");
    m_btnContinue->setEnabled(true);
    m_btnContinue->setStyleSheet(
        "background-color: #4B94F2; color: white; padding: 10px 25px; "
        "border-radius: 6px; font-size: 15px; font-weight: bold;");
    m_apptCard->setVisible(false);
    m_dateEdit->setDate(QDate::currentDate());
    m_comboSpecialty->setCurrentIndex(0);
    updateDoctorList();
  });

  mainScroll->setWidget(contentWidget);
  m_registerPage = mainScroll;

  updateDoctorList();
}

void ReceptionDashboardWidget::updateDoctorList() {
  QLayoutItem* item;
  while ((item = m_doctorListLayout->takeAt(0)) != nullptr) {
    if (item->widget()) item->widget()->deleteLater();
    delete item;
  }
  
  m_selectedDoctorId = -1;
  m_selectedTimeSlot = "";
  m_selectedSlotButton = nullptr;

  DoctorSearchCriteria allCriteria;
  allCriteria.onlyActive = true;
  auto allDoctors = m_staffService ? m_staffService->searchDoctorsPaged(allCriteria).items : QList<std::shared_ptr<SystemUser>>();

  // Rebuild specialty combo dynamically from active doctor list
  QString savedSpecialty = m_comboSpecialty->currentText();
  m_comboSpecialty->blockSignals(true);
  m_comboSpecialty->clear();
  m_comboSpecialty->addItem("Tất cả");

  QStringList knownSpecialties;
  for (const auto &doc : allDoctors) {
      auto docModel = std::dynamic_pointer_cast<Doctor>(doc);
      if (!docModel) continue;
      QString sp = docModel->getSpecialty().trimmed();
      if (!sp.isEmpty() && !knownSpecialties.contains(sp)) {
          knownSpecialties.append(sp);
          m_comboSpecialty->addItem(sp);
      }
  }

  int idx = m_comboSpecialty->findText(savedSpecialty);
  m_comboSpecialty->setCurrentIndex(idx >= 0 ? idx : 0);
  m_comboSpecialty->blockSignals(false);

  QString specialty = m_comboSpecialty->currentText();
  if (specialty == "Tất cả") specialty = "";

  QDate date = m_dateEdit->date();

  QList<std::shared_ptr<SystemUser>> doctors;
  if (specialty.isEmpty()) {
      doctors = allDoctors;
  } else {
      for (const auto &doc : allDoctors) {
          auto docModel = std::dynamic_pointer_cast<Doctor>(doc);
          if (docModel && docModel->getSpecialty().trimmed() == specialty) {
              doctors.append(doc);
          }
      }
  }

  if (doctors.isEmpty()) {
      QLabel* lblEmpty = new QLabel("Không có bác sĩ nào cho chuyên khoa này.");
      lblEmpty->setAlignment(Qt::AlignCenter);
      lblEmpty->setStyleSheet("color: #777; font-size: 14px; margin-top: 20px;");
      m_doctorListLayout->addWidget(lblEmpty);
  }

  for (const auto &doc : doctors) {
    auto docModel = std::dynamic_pointer_cast<Doctor>(doc);
    if (!docModel) continue;

    int doctorId = docModel->getAccountId();
    QFrame* docCard = new QFrame(m_doctorListLayout->parentWidget());
    docCard->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px; }");
    QVBoxLayout* cardLayout = new QVBoxLayout(docCard);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    cardLayout->setSpacing(10);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* lblAvatar = new QLabel(docCard);
    lblAvatar->setFixedSize(60, 60);
    lblAvatar->setStyleSheet("background-color: #F1F3F4; border-radius: 30px;");
    
    QVBoxLayout* infoLayout = new QVBoxLayout();
    QLabel* lblName = new QLabel(docModel->getFullName(), docCard);
    lblName->setStyleSheet("font-size: 16px; font-weight: bold; color: #1976D2; border: none;");
    
    auto profile = m_staffService->getOwnProfile(doctorId);
    QString shiftInfo = profile ? profile->shift : "Cả ngày";
    QString shiftDisplay = shiftInfo.isEmpty() ? "Không có" : shiftInfo;
    QLabel* lblDesc = new QLabel("Phòng Khám - Ca: " + shiftDisplay, docCard);
    lblDesc->setStyleSheet("color: #555; font-size: 13px; border: none;");
    
    QPushButton* btnInfo = new QPushButton("Thông tin bác sĩ >", docCard);
    btnInfo->setCursor(Qt::PointingHandCursor);
    btnInfo->setStyleSheet("background-color: #E3F2FD; color: #1976D2; border-radius: 10px; padding: 4px 10px; font-size: 12px; border: none; text-align: left; max-width: 120px;");

    connect(btnInfo, &QPushButton::clicked, this, [this, doctorId]() {
        ProfileWidget dialog(m_staffService, this);
        dialog.loadProfile(doctorId);
        dialog.setReadOnlyMode();
        dialog.exec();
    });

    infoLayout->addWidget(lblName);
    infoLayout->addWidget(lblDesc);
    infoLayout->addWidget(btnInfo);
    
    headerLayout->addWidget(lblAvatar);
    headerLayout->addLayout(infoLayout);
    headerLayout->addStretch();
    cardLayout->addLayout(headerLayout);
    
    QFrame* hLine = new QFrame(docCard);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setStyleSheet("border: none; border-top: 1px dashed #DDD; background: transparent;");
    cardLayout->addWidget(hLine);

    QLabel* lblDateHeader = new QLabel(date.toString("dd/MM/yyyy") + " - Ca: " + shiftDisplay, docCard);
    lblDateHeader->setStyleSheet("color: #2E7D32; font-weight: bold; font-size: 14px; border: none; margin-top: 5px;");
    cardLayout->addWidget(lblDateHeader);

    QStringList timeSlots = m_appointmentService->getAvailableTimeSlots(doctorId, date);
    
    if (timeSlots.isEmpty()) {
        QString emptyMsg = "ĐÃ HẾT SỐ ĐẶT TRƯỚC";
        if (shiftInfo.isEmpty() || shiftInfo.trimmed().toUpper() == "NONE") {
            emptyMsg = "BÁC SĨ KHÔNG CÓ LỊCH KHÁM";
        }
        QLabel* lblFull = new QLabel(emptyMsg, docCard);
        lblFull->setAlignment(Qt::AlignCenter);
        lblFull->setStyleSheet("color: #999; font-size: 13px; font-style: italic; border: none; margin: 10px;");
        cardLayout->addWidget(lblFull);
    } else {
        QGridLayout* gridLayout = new QGridLayout();
        gridLayout->setSpacing(10);
        int row = 0, col = 0;
        for (const QString& slot : timeSlots) {
            QPushButton* btnSlot = new QPushButton(slot, docCard);
            btnSlot->setCursor(Qt::PointingHandCursor);
            btnSlot->setStyleSheet("QPushButton { background-color: #F5F5F5; color: #333; border: none; border-radius: 8px; padding: 10px; font-weight: bold; font-size: 14px; }");
            connect(btnSlot, &QPushButton::clicked, this, [this, doctorId, slot, btnSlot]() {
                this->onTimeSlotClicked(doctorId, slot, btnSlot);
            });
            gridLayout->addWidget(btnSlot, row, col);
            col++;
            if (col > 1) { col = 0; row++; }
        }
        cardLayout->addLayout(gridLayout);
    }
    m_doctorListLayout->addWidget(docCard);
  }
  m_doctorListLayout->addStretch();
}

void ReceptionDashboardWidget::onTimeSlotClicked(int doctorId, const QString& timeSlot, QPushButton* btn) {
    if (m_selectedSlotButton) {
        m_selectedSlotButton->setStyleSheet("QPushButton { background-color: #F5F5F5; color: #333; border: none; border-radius: 8px; padding: 10px; font-weight: bold; font-size: 14px; }");
    }
    m_selectedDoctorId = doctorId;
    m_selectedTimeSlot = timeSlot;
    m_selectedSlotButton = btn;
    btn->setStyleSheet("QPushButton { background-color: #0B57D0; color: white; border: none; border-radius: 8px; padding: 10px; font-weight: bold; font-size: 14px; }");
}

void ReceptionDashboardWidget::buildPatientsPage() {
  m_patientsPage = new QWidget();
  m_patientsPage->setStyleSheet("background-color: #F8F9FA;");

  QVBoxLayout *pageLayout = new QVBoxLayout(m_patientsPage);
  pageLayout->setContentsMargins(20, 20, 20, 20);
  pageLayout->setSpacing(15);

  // Top header layout
  QHBoxLayout *topLayout = new QHBoxLayout();
  QLabel *lblTitle = new QLabel("Danh sách Bệnh nhân", m_patientsPage);
  lblTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1E293B;");
  topLayout->addWidget(lblTitle);
  topLayout->addStretch();

  pageLayout->addLayout(topLayout);

  // Card filter
  QFrame *filterCard = makeCard(m_patientsPage);
  QHBoxLayout *filterLayout = new QHBoxLayout(filterCard);
  filterLayout->setContentsMargins(15, 12, 15, 12);
  filterLayout->setSpacing(12);

  m_txtPatientSearchKey = new QLineEdit(filterCard);
  m_txtPatientSearchKey->setPlaceholderText("Tìm kiếm họ tên, mã BN, CCCD, SĐT...");
  m_txtPatientSearchKey->setStyleSheet(
      "QLineEdit { padding: 6px 12px; border: 1px solid #D1D5DB; border-radius: 6px; font-size: 13px; min-height: 32px; background: white; min-width: 250px; }"
      "QLineEdit:focus { border: 1px solid #2563EB; }");

  m_cbPatientTypeFilter = new QComboBox(filterCard);
  m_cbPatientTypeFilter->setStyleSheet(
      "QComboBox { padding: 6px 12px; border: 1px solid #D1D5DB; border-radius: 6px; font-size: 13px; min-height: 32px; background: white; }"
      "QComboBox:focus { border: 1px solid #2563EB; }");
  m_cbPatientTypeFilter->addItem("Tất cả loại BN", -1);
  m_cbPatientTypeFilter->addItem("Ngoại trú", static_cast<int>(PatientType::Outpatient));
  m_cbPatientTypeFilter->addItem("Nội trú", static_cast<int>(PatientType::Inpatient));
  m_cbPatientTypeFilter->addItem("Cấp cứu", static_cast<int>(PatientType::Emergency));

  m_btnResetPatientFilters = new QPushButton("Đặt lại", filterCard);
  m_btnResetPatientFilters->setCursor(Qt::PointingHandCursor);
  m_btnResetPatientFilters->setStyleSheet(
      "QPushButton { background-color: #EF4444; color: white; border-radius: 6px; padding: 6px 15px; font-weight: bold; border: none; min-height: 32px; }"
      "QPushButton:hover { background-color: #DC2626; }");

  filterLayout->addWidget(m_txtPatientSearchKey);
  filterLayout->addWidget(m_cbPatientTypeFilter);
  filterLayout->addWidget(m_btnResetPatientFilters);
  filterLayout->addStretch();
  pageLayout->addWidget(filterCard);

  // Table Card
  QFrame *tableCard = makeCard(m_patientsPage);
  QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
  cardLayout->setContentsMargins(0, 0, 0, 0);

  m_patientsTable = new QTableWidget(0, 4, tableCard);
  m_patientsTable->setHorizontalHeaderLabels({"Mã BN", "Họ Tên", "Giới Tính", "Thao tác"});
  m_patientsTable->horizontalHeader()->setStretchLastSection(false);
  m_patientsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  m_patientsTable->horizontalHeader()->resizeSection(0, 140);
  m_patientsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_patientsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
  m_patientsTable->horizontalHeader()->resizeSection(2, 120);
  m_patientsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
  m_patientsTable->horizontalHeader()->resizeSection(3, 240);

  m_patientsTable->verticalHeader()->setDefaultSectionSize(46);
  m_patientsTable->verticalHeader()->setVisible(false);
  m_patientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_patientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_patientsTable->setFocusPolicy(Qt::NoFocus);
  m_patientsTable->setStyleSheet(
      "QTableWidget { border: none; outline: none; gridline-color: #E2E8F0; font-size: 13px; background-color: white; color: #0F172A; }"
      "QTableWidget::item { outline: none; border: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }"
      "QHeaderView::section { background-color: #F8FAFC; padding: 10px; font-weight: bold; border: none; border-bottom: 1px solid #E2E8F0; color: #1E293B; }");

  // Pagination layout
  QHBoxLayout *paginationLayout = new QHBoxLayout();
  paginationLayout->setContentsMargins(15, 10, 15, 15);

  m_btnPatientPrevPage = new QPushButton("Trang trước", tableCard);
  m_btnPatientPrevPage->setCursor(Qt::PointingHandCursor);
  m_btnPatientPrevPage->setStyleSheet(
      "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: 6px; padding: 6px 12px; font-weight: bold; border: none; min-height: 32px; }"
      "QPushButton:hover { background-color: #CBD5E1; }"
      "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }");

  m_lblPatientPageInfo = new QLabel("Trang 1 / 1", tableCard);
  m_lblPatientPageInfo->setStyleSheet(
      "background: transparent; border: none; font-size: 13px; font-weight: bold; color: #475569;");

  m_btnPatientNextPage = new QPushButton("Trang sau", tableCard);
  m_btnPatientNextPage->setCursor(Qt::PointingHandCursor);
  m_btnPatientNextPage->setStyleSheet(
      "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: 6px; padding: 6px 12px; font-weight: bold; border: none; min-height: 32px; }"
      "QPushButton:hover { background-color: #CBD5E1; }"
      "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }");

  paginationLayout->addWidget(m_btnPatientPrevPage);
  paginationLayout->addStretch();
  paginationLayout->addWidget(m_lblPatientPageInfo);
  paginationLayout->addStretch();
  paginationLayout->addWidget(m_btnPatientNextPage);

  cardLayout->addWidget(m_patientsTable);
  cardLayout->addLayout(paginationLayout);
  pageLayout->addWidget(tableCard);

  connect(m_txtPatientSearchKey, &QLineEdit::textChanged, this, &ReceptionDashboardWidget::handlePatientFilterChanged);
  connect(m_cbPatientTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReceptionDashboardWidget::handlePatientFilterChanged);
  connect(m_btnResetPatientFilters, &QPushButton::clicked, this, &ReceptionDashboardWidget::handlePatientResetFilters);
  connect(m_btnPatientPrevPage, &QPushButton::clicked, this, &ReceptionDashboardWidget::handlePatientPrevPage);
  connect(m_btnPatientNextPage, &QPushButton::clicked, this, &ReceptionDashboardWidget::handlePatientNextPage);

  refreshPatientsTable();
}

void ReceptionDashboardWidget::handlePatientFilterChanged() {
  m_patientCurrentPage = 1;
  refreshPatientsTable();
}

void ReceptionDashboardWidget::handlePatientResetFilters() {
  if (m_txtPatientSearchKey) m_txtPatientSearchKey->clear();
  if (m_cbPatientTypeFilter) m_cbPatientTypeFilter->setCurrentIndex(0);
  m_patientCurrentPage = 1;
  refreshPatientsTable();
}

void ReceptionDashboardWidget::handlePatientPrevPage() {
  if (m_patientCurrentPage > 1) {
    m_patientCurrentPage--;
    refreshPatientsTable();
  }
}

void ReceptionDashboardWidget::handlePatientNextPage() {
  if (m_patientCurrentPage < m_patientTotalPages) {
    m_patientCurrentPage++;
    refreshPatientsTable();
  }
}

void ReceptionDashboardWidget::refreshPatientsTable() {
  if (!m_patientsTable) return;
  m_patientsTable->setRowCount(0);

  PatientSearchCriteria criteria;
  if (m_txtPatientSearchKey) {
    criteria.searchKey = m_txtPatientSearchKey->text().trimmed();
  }
  if (m_cbPatientTypeFilter && m_cbPatientTypeFilter->currentIndex() > 0) {
    int val = m_cbPatientTypeFilter->currentData().toInt();
    if (val >= 0) {
      criteria.type = static_cast<PatientType>(val);
    }
  }
  criteria.page = m_patientCurrentPage;
  criteria.pageSize = 10;

  auto pagedResult = m_patientService ? m_patientService->searchPatientsPaged(criteria) : PagedResult<PatientSearchResultDTO>();
  auto patients = pagedResult.items;

  m_patientTotalPages = qMax(1, pagedResult.totalPages());
  m_patientCurrentPage = qBound(1, pagedResult.page, m_patientTotalPages);

  if (m_lblPatientPageInfo) {
    m_lblPatientPageInfo->setText(QString("%1 / %2").arg(m_patientCurrentPage).arg(m_patientTotalPages));
  }
  if (m_btnPatientPrevPage) m_btnPatientPrevPage->setEnabled(m_patientCurrentPage > 1);
  if (m_btnPatientNextPage) m_btnPatientNextPage->setEnabled(m_patientCurrentPage < m_patientTotalPages);

  m_patientsTable->setRowCount(patients.size());
  for (int i = 0; i < patients.size(); ++i) {
      const auto& p = patients[i];
      m_patientsTable->setItem(i, 0, new QTableWidgetItem(p.patientCode));
      m_patientsTable->setItem(i, 1, new QTableWidgetItem(p.fullName));
      m_patientsTable->setItem(i, 2, new QTableWidgetItem(GenderText::toVi(p.gender)));

      QWidget* actionWidget = new QWidget();
      QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
      actionLayout->setContentsMargins(4, 4, 4, 4);
      actionLayout->setSpacing(8);
      actionWidget->setStyleSheet("background: transparent;");

      QPushButton* btnHistory = new QPushButton("Lịch sử khám");
      btnHistory->setCursor(Qt::PointingHandCursor);
      btnHistory->setStyleSheet("QPushButton { background-color: #1A73E8; color: white; border-radius: 4px; padding: 5px 12px; font-weight: bold; font-size: 12px; min-width: 95px; }"
                                "QPushButton:hover { background-color: #1557B0; }");

      QPushButton* btnEdit = new QPushButton("Sửa");
      btnEdit->setCursor(Qt::PointingHandCursor);
      btnEdit->setStyleSheet("QPushButton { background-color: #F59E0B; color: white; border-radius: 4px; padding: 5px 12px; font-weight: bold; font-size: 12px; min-width: 55px; }"
                                "QPushButton:hover { background-color: #D97706; }");

      int pId = p.patientId;
      QString pName = p.fullName;
      connect(btnHistory, &QPushButton::clicked, this, [this, pId, pName]() {
          showPatientHistoryDialog(pId, pName);
      });

      connect(btnEdit, &QPushButton::clicked, this, [this, pId]() {
          PatientEditDialog dialog(pId, m_patientService, this);
          connect(&dialog, &PatientEditDialog::patientUpdated, this, [this]() {
              refreshPatientsTable();
          });
          dialog.exec();
      });

      actionLayout->addWidget(btnHistory);
      actionLayout->addWidget(btnEdit);
      actionLayout->setAlignment(Qt::AlignCenter);
      m_patientsTable->setCellWidget(i, 3, actionWidget);
  }
}

void ReceptionDashboardWidget::buildAppointmentsPage() {
  m_appointmentsPage = new QWidget();
  m_appointmentsPage->setStyleSheet("background-color: #F8F9FA;");

  QVBoxLayout *layout = new QVBoxLayout(m_appointmentsPage);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(20);

  QHBoxLayout *topLayout = new QHBoxLayout();
  QLabel *lblTitle = new QLabel("Quản lý Lịch hẹn", m_appointmentsPage);
  lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #202124;");
  
  QLabel *lblFilter = new QLabel("Ngày:", m_appointmentsPage);
  lblFilter->setStyleSheet("font-size: 16px; font-weight: bold; color: #555;");
  
  m_apptDateEdit = new QDateEdit(QDate::currentDate(), m_appointmentsPage);
  m_apptDateEdit->setDisplayFormat("dd/MM/yyyy");
  m_apptDateEdit->setMinimumWidth(150);
  QCalendarWidget* manageCalendar = new QCalendarWidget();
  manageCalendar->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
  manageCalendar->setMinimumSize(330, 250);
  m_apptDateEdit->setCalendarWidget(manageCalendar);
  m_apptDateEdit->setCalendarPopup(true);
  m_apptDateEdit->setStyleSheet(
      "QDateEdit { padding: 8px 10px; border: 1px solid #BDBDBD; border-radius: 6px; background-color: #FFFFFF; color: #333333; font-size: 14px; } "
      "QDateEdit:focus { border: 1px solid #4B94F2; } "
  );
  
  m_apptDateEdit->calendarWidget()->setStyleSheet(
      "QCalendarWidget { background-color: #FFFFFF; color: #333333; }"
      "QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: #FFFFFF; border-bottom: 1px solid #EAEAEA; min-height: 36px; }"
      "QCalendarWidget QToolButton { color: #333333; font-weight: bold; background-color: transparent; border: none; padding: 4px 6px; margin: 1px; font-size: 13px; }"
      "QCalendarWidget QToolButton:hover { background-color: #E3F2FD; border-radius: 4px; }"
      "QCalendarWidget QMenu { background-color: #FFFFFF; color: #333333; }"
      "QCalendarWidget QSpinBox { background-color: #FFFFFF; color: #333333; selection-background-color: #4B94F2; selection-color: white; font-size: 13px; }"
      "QCalendarWidget QAbstractItemView:enabled { font-size: 13px; color: #333333; background-color: #FFFFFF; selection-background-color: #4B94F2; selection-color: #FFFFFF; }"
      "QCalendarWidget QAbstractItemView:disabled { color: #CCCCCC; }"
  );

  connect(m_apptDateEdit, &QDateEdit::dateChanged, this, &ReceptionDashboardWidget::updateAppointmentsTable);

  topLayout->addWidget(lblTitle);
  topLayout->addStretch();
  topLayout->addWidget(lblFilter);
  topLayout->addWidget(m_apptDateEdit);
  layout->addLayout(topLayout);

  m_appointmentsTable = new QTableWidget(0, 6, m_appointmentsPage);
  m_appointmentsTable->setHorizontalHeaderLabels({"Thời gian", "Bệnh nhân", "Bác sĩ", "Phòng", "Trạng thái", "Thao tác"});
  m_appointmentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_appointmentsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_appointmentsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
  m_appointmentsTable->setStyleSheet("QTableWidget { background-color: white; border-radius: 8px; border: 1px solid #EAEAEA; color: #333333; outline: none; }"
                       "QHeaderView::section { background-color: #F1F3F4; font-weight: bold; border: none; padding: 10px; color: #5F6368; }"
                       "QTableWidget::item { padding: 5px; border-bottom: 1px solid #EAEAEA; color: #333333; outline: none; }"
                       "QTableWidget::item:focus { outline: none; border: none; }");
  m_appointmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_appointmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_appointmentsTable->setFocusPolicy(Qt::NoFocus);
  layout->addWidget(m_appointmentsTable);

  updateAppointmentsTable();
}

void ReceptionDashboardWidget::updateAppointmentsTable() {
    if (!m_appointmentsTable || !m_appointmentService) return;
    QDate date = m_apptDateEdit->date();
    auto appts = m_appointmentService->getAppointmentsByDate(date);
    m_appointmentsTable->setRowCount(appts.size());
    for (int i = 0; i < appts.size(); ++i) {
        const auto& a = appts[i];
        QString timeStr = a.startTime;
        if (!a.endTime.isEmpty()) timeStr += " - " + a.endTime;
        
        m_appointmentsTable->setItem(i, 0, new QTableWidgetItem(timeStr));
        m_appointmentsTable->setItem(i, 1, new QTableWidgetItem(a.patientName));
        m_appointmentsTable->setItem(i, 2, new QTableWidgetItem(a.doctorName));
        m_appointmentsTable->setItem(i, 3, new QTableWidgetItem(a.roomNumber));
        
        QString statusText = AppointmentStatusText::toVi(a.status);
        m_appointmentsTable->setItem(i, 4, new QTableWidgetItem(statusText));
        
        QDate apptDate = QDate::fromString(a.appointmentDate, "yyyy-MM-dd");
        QDate todayDate = QDate::currentDate();

        if (a.status == "SCHEDULED" && apptDate >= todayDate) {
            QWidget* widget = new QWidget();
            QHBoxLayout* l = new QHBoxLayout(widget);
            l->setContentsMargins(4, 4, 4, 4);
            l->setSpacing(8);

            QPushButton* btnCheckIn = nullptr;
            if (apptDate == todayDate) {
                btnCheckIn = new QPushButton("Check-in");
                btnCheckIn->setStyleSheet("background-color: #2563EB; color: white; border-radius: 4px; padding: 5px 10px; font-weight: bold;");
                btnCheckIn->setCursor(Qt::PointingHandCursor);
            }

            QPushButton* btnCancel = new QPushButton("Hủy hẹn");
            btnCancel->setStyleSheet("background-color: #EF4444; color: white; border-radius: 4px; padding: 5px 10px; font-weight: bold;");
            btnCancel->setCursor(Qt::PointingHandCursor);

            QString pName = a.patientName;
            QString dName = a.doctorName;
            QString rName = a.roomNumber;
            QString aDate = a.appointmentDate;
            QString tStr = timeStr;
            int apptId = a.appointmentId;

            if (btnCheckIn) {
                connect(btnCheckIn, &QPushButton::clicked, this, [this, apptId, pName, dName, rName, aDate, tStr]() {
                    QMessageBox confirmBox(this);
                    confirmBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                    confirmBox.setWindowTitle("Xác nhận");
                    confirmBox.setText("Tiến hành Check-in và phát số thứ tự cho bệnh nhân?");
                    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    confirmBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #4B94F2; border-radius: 8px; }"
                                             "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                             "QPushButton { background-color: #4B94F2; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                             "QPushButton:hover { background-color: #3b82f6; }");
                    if (confirmBox.exec() == QMessageBox::Yes) {
                        auto result = m_appointmentService->checkInPatient(apptId);
                        if (result.second > 0) {
                            QMessageBox msgBox(this);
                            msgBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                            msgBox.setIcon(QMessageBox::Information);
                            msgBox.setWindowTitle("Check-in Thành công");
                            
                            QString info = QString("Bệnh nhân: %1\nNgày: %2  -  Giờ: %3\nBác sĩ: %4\nPhòng: %5\nSỐ THỨ TỰ: %6")
                                           .arg(pName).arg(aDate).arg(tStr).arg(dName).arg(rName).arg(result.second);
                                           
                            msgBox.setText("ĐÃ PHÁT SỐ THỨ TỰ THÀNH CÔNG\n\n" + info);
                            msgBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #059669; border-radius: 8px; }"
                                                 "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                                 "QPushButton { background-color: #059669; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                                 "QPushButton:hover { background-color: #047857; }");
                            msgBox.exec();
                            updateAppointmentsTable();
                        } else {
                            QMessageBox errBox(this);
                            errBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                            errBox.setIcon(QMessageBox::Warning);
                            errBox.setWindowTitle("Lỗi");
                            errBox.setText(result.first.isEmpty() ? "Không thể Check-in bệnh nhân." : result.first);
                            errBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #F44336; border-radius: 8px; }"
                                                 "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                                 "QPushButton { background-color: #F44336; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                                 "QPushButton:hover { background-color: #D32F2F; }");
                            errBox.exec();
                        }
                    }
                });
            }

            connect(btnCancel, &QPushButton::clicked, this, [this, apptId]() {
                QMessageBox confirmBox(this);
                confirmBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                confirmBox.setWindowTitle("Xác nhận");
                confirmBox.setText("Bạn có chắc chắn muốn hủy lịch hẹn này?");
                confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                confirmBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #F59E0B; border-radius: 8px; }"
                                         "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                         "QPushButton { background-color: #F59E0B; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                         "QPushButton:hover { background-color: #D97706; }");
                if (confirmBox.exec() == QMessageBox::Yes) {
                    QString err = m_appointmentService->cancelAppointment(apptId);
                    if (err.isEmpty()) {
                        QMessageBox okBox(this);
                        okBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                        okBox.setIcon(QMessageBox::Information);
                        okBox.setWindowTitle("Thành công");
                        okBox.setText("Hủy lịch hẹn thành công.");
                        okBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #059669; border-radius: 8px; }"
                                            "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                            "QPushButton { background-color: #059669; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                            "QPushButton:hover { background-color: #047857; }");
                        okBox.exec();
                        updateAppointmentsTable();
                    } else {
                        QMessageBox errBox(this);
                        errBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                        errBox.setIcon(QMessageBox::Warning);
                        errBox.setWindowTitle("Lỗi");
                        errBox.setText(err);
                        errBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #F44336; border-radius: 8px; }"
                                             "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                             "QPushButton { background-color: #F44336; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                             "QPushButton:hover { background-color: #D32F2F; }");
                        errBox.exec();
                    }
                }
            });

            if (btnCheckIn) l->addWidget(btnCheckIn);
            l->addWidget(btnCancel);
            l->setAlignment(Qt::AlignCenter);
            m_appointmentsTable->setCellWidget(i, 5, widget);
        } else {
            m_appointmentsTable->removeCellWidget(i, 5);
            QTableWidgetItem* emptyItem = new QTableWidgetItem("-");
            emptyItem->setTextAlignment(Qt::AlignCenter);
            m_appointmentsTable->setItem(i, 5, emptyItem);
        }
    }
}

void ReceptionDashboardWidget::buildRoomQueuePage() {
    m_roomQueuePage = new QWidget();
    m_roomQueuePage->setStyleSheet("background-color: #F8F9FA;");

    QVBoxLayout *layout = new QVBoxLayout(m_roomQueuePage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel *lblTitle = new QLabel("Quản lý Hàng đợi Phòng khám", m_roomQueuePage);
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #202124;");
    layout->addWidget(lblTitle);

    QScrollArea *scrollArea = new QScrollArea(m_roomQueuePage);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("background-color: transparent;");

    m_roomQueueLayout = new QGridLayout(scrollContent);
    m_roomQueueLayout->setSpacing(20);
    m_roomQueueLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea);
}

void ReceptionDashboardWidget::onRefreshRoomQueue() {
    if (!m_roomQueueLayout || !m_appointmentService) return;

    // Clear existing items
    QLayoutItem *child;
    while ((child = m_roomQueueLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    QDate today = QDate::currentDate();
    auto statuses = m_appointmentService->getRoomQueueStatuses(today);

    int row = 0;
    int col = 0;
    int maxCols = 4; // 4 cards per row

    for (const auto& st : statuses) {
        auto *card = new RoomQueueWidget(st.roomId, st.roomNumber, st.doctorId, st.doctorName, st.currentTicketNumber, st.nextTicketNumber, this);
        connect(card, &RoomQueueWidget::clicked, this, [this](int rId, int dId) {
            auto items = m_appointmentService->getDoctorQueue(dId, QDate::currentDate());
            auto *dialog = new RoomQueueDialog(rId, "Phòng khám", items, this);
            dialog->exec();
            dialog->deleteLater();
            onRefreshRoomQueue(); // Refresh after closing
        });

        m_roomQueueLayout->addWidget(card, row, col);
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void ReceptionDashboardWidget::showPatientHistoryDialog(int patientId, const QString& patientName) {
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    dialog.setStyleSheet("QDialog { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px; }");
    dialog.setMinimumSize(900, 500);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* lblTitle = new QLabel(QString("Lịch sử khám - %1").arg(patientName));
    lblTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #202124;");
    
    QPushButton* btnClose = new QPushButton("✕");
    btnClose->setFixedSize(30, 30);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet("QPushButton { border: none; font-size: 18px; font-weight: bold; color: #5F6368; }"
                            "QPushButton:hover { color: #D32F2F; background-color: #FEE2E2; border-radius: 15px; }");
    connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    headerLayout->addWidget(lblTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(btnClose);
    layout->addLayout(headerLayout);

    QTableWidget* table = new QTableWidget(0, 6, &dialog);
    table->setHorizontalHeaderLabels({"Ngày khám", "Giờ khám", "Bác sĩ", "Phòng", "Trạng thái", "Thao tác"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    table->setStyleSheet("QTableWidget { background-color: white; border-radius: 8px; border: 1px solid #EAEAEA; color: #333333; outline: none; }"
                         "QHeaderView::section { background-color: #F1F3F4; font-weight: bold; border: none; padding: 10px; color: #5F6368; }"
                         "QTableWidget::item { padding: 5px; border-bottom: 1px solid #EAEAEA; color: #333333; outline: none; }"
                         "QTableWidget::item:focus { outline: none; border: none; }");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setFocusPolicy(Qt::NoFocus);
    
    auto appts = m_appointmentService->getPatientAppointments(patientId);
    table->setRowCount(appts.size());
    
    for (int i = 0; i < appts.size(); ++i) {
        const auto& a = appts[i];
        QString timeStr = a.startTime;
        if (!a.endTime.isEmpty()) timeStr += " - " + a.endTime;
        
        table->setItem(i, 0, new QTableWidgetItem(a.appointmentDate));
        table->setItem(i, 1, new QTableWidgetItem(timeStr));
        table->setItem(i, 2, new QTableWidgetItem(a.doctorName));
        table->setItem(i, 3, new QTableWidgetItem(a.roomNumber));
        
        QString statusText = AppointmentStatusText::toVi(a.status);
        table->setItem(i, 4, new QTableWidgetItem(statusText));
        
        QDate apptDate = QDate::fromString(a.appointmentDate, "yyyy-MM-dd");
        QDate todayDate = QDate::currentDate();

        if (a.status == "SCHEDULED" && apptDate >= todayDate) {
            QWidget* widget = new QWidget();
            QHBoxLayout* l = new QHBoxLayout(widget);
            l->setContentsMargins(4, 4, 4, 4);
            l->setSpacing(8);

            QPushButton* btnCheckIn = nullptr;
            if (apptDate == todayDate) {
                btnCheckIn = new QPushButton("Check-in");
                btnCheckIn->setStyleSheet("background-color: #34A853; color: white; border-radius: 4px; padding: 5px 10px; font-weight: bold;");
                btnCheckIn->setCursor(Qt::PointingHandCursor);
            }

            QPushButton* btnCancel = new QPushButton("Hủy");
            btnCancel->setStyleSheet("background-color: #F44336; color: white; border-radius: 4px; padding: 5px 10px; font-weight: bold;");
            btnCancel->setCursor(Qt::PointingHandCursor);

            QString pName = patientName;
            QString dName = a.doctorName;
            QString rName = a.roomNumber;
            QString aDate = a.appointmentDate;
            QString tStr = timeStr;
            int apptId = a.appointmentId;
            QDialog* dlgPtr = &dialog;

            if (btnCheckIn) {
                connect(btnCheckIn, &QPushButton::clicked, &dialog, [this, dlgPtr, apptId, pName, dName, rName, aDate, tStr]() {
                    QMessageBox confirmBox(dlgPtr);
                    confirmBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                    confirmBox.setWindowTitle("Xác nhận");
                    confirmBox.setText("Tiến hành Check-in và phát số thứ tự cho bệnh nhân?");
                    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    confirmBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #4B94F2; border-radius: 8px; }"
                                             "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                             "QPushButton { background-color: #4B94F2; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                             "QPushButton:hover { background-color: #3b82f6; }");
                    if (confirmBox.exec() == QMessageBox::Yes) {
                        auto result = m_appointmentService->checkInPatient(apptId);
                        if (result.second > 0) {
                            QMessageBox msgBox(dlgPtr);
                            msgBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                            msgBox.setIcon(QMessageBox::Information);
                            msgBox.setWindowTitle("Check-in Thành công");
                            
                            QString info = QString("Bệnh nhân: %1\nNgày: %2  -  Giờ: %3\nBác sĩ: %4\nPhòng: %5\nSỐ THỨ TỰ: %6")
                                           .arg(pName).arg(aDate).arg(tStr).arg(dName).arg(rName).arg(result.second);
                                           
                            msgBox.setText("ĐÃ PHÁT SỐ THỨ TỰ THÀNH CÔNG\n\n" + info);
                            msgBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #059669; border-radius: 8px; }"
                                                 "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                                 "QPushButton { background-color: #059669; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                                 "QPushButton:hover { background-color: #047857; }");
                            msgBox.exec();
                            dlgPtr->accept();
                            if (m_appointmentsTable) {
                                updateAppointmentsTable();
                            }
                        } else {
                            QMessageBox errBox(dlgPtr);
                            errBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                            errBox.setIcon(QMessageBox::Warning);
                            errBox.setWindowTitle("Lỗi");
                            errBox.setText(result.first.isEmpty() ? "Không thể Check-in bệnh nhân." : result.first);
                            errBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #F44336; border-radius: 8px; }"
                                                 "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                                 "QPushButton { background-color: #F44336; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                                 "QPushButton:hover { background-color: #D32F2F; }");
                            errBox.exec();
                        }
                    }
                });
            }

            connect(btnCancel, &QPushButton::clicked, &dialog, [this, dlgPtr, apptId]() {
                QMessageBox confirmBox(dlgPtr);
                confirmBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                confirmBox.setWindowTitle("Xác nhận");
                confirmBox.setText("Bạn có chắc chắn muốn hủy lịch hẹn này?");
                confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                confirmBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #F59E0B; border-radius: 8px; }"
                                         "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                         "QPushButton { background-color: #F59E0B; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                         "QPushButton:hover { background-color: #D97706; }");
                if (confirmBox.exec() == QMessageBox::Yes) {
                    QString err = m_appointmentService->cancelAppointment(apptId);
                    if (err.isEmpty()) {
                        QMessageBox okBox(dlgPtr);
                        okBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                        okBox.setIcon(QMessageBox::Information);
                        okBox.setWindowTitle("Thành công");
                        okBox.setText("Đã hủy lịch hẹn.");
                        okBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #059669; border-radius: 8px; }"
                                            "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                            "QPushButton { background-color: #059669; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                            "QPushButton:hover { background-color: #047857; }");
                        okBox.exec();
                        dlgPtr->accept();
                        if (m_appointmentsTable) {
                            updateAppointmentsTable();
                        }
                    } else {
                        QMessageBox errBox(dlgPtr);
                        errBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                        errBox.setIcon(QMessageBox::Warning);
                        errBox.setWindowTitle("Lỗi");
                        errBox.setText(err);
                        errBox.setStyleSheet("QMessageBox { background-color: #FFFFFF; border: 2px solid #F44336; border-radius: 8px; }"
                                             "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                                             "QPushButton { background-color: #F44336; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                                             "QPushButton:hover { background-color: #D32F2F; }");
                        errBox.exec();
                    }
                }
            });

            if (btnCheckIn) l->addWidget(btnCheckIn);
            l->addWidget(btnCancel);
            l->setAlignment(Qt::AlignCenter);
            table->setCellWidget(i, 5, widget);
        } else {
            table->removeCellWidget(i, 5);
            QTableWidgetItem* emptyItem = new QTableWidgetItem("-");
            emptyItem->setTextAlignment(Qt::AlignCenter);
            table->setItem(i, 5, emptyItem);
        }
    }
    
    layout->addWidget(table);
    dialog.exec();
}
