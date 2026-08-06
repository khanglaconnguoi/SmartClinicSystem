#include "ReceptionDashboard.h"
#include "../../model/Doctor.h"
#include "../../model/IAuthenticatable.h"

#include "../../model/CommonEnums.h"
#include "../../service/AnalyticService.h"
#include "../../service/AppointmentService.h"
#include "../../service/PatientService.h"
#include "../../service/StaffService.h"
#include "../view/Profile.h"
#include "PatientEditDialog.h"
#include "PatientRegistrationDialog.h"
#include "RoomQueueDialog.h"
#include "RoomQueueWidget.h"
#include <QCalendarWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QDebug>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStringList>
#include <QTableWidget>
#include <QTextCharFormat>

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
}

void ReceptionDashboardWidget::fillDashboardData() {
  buildSidebar();

  if (m_currentUser && m_nameLabel) {
    m_nameLabel->setText((m_currentUser->getFullName() + " (LỄ TÂN)").toUpper());
  }

  m_stackedWidget = new QStackedWidget(m_mainContentWidget);
  m_mainContentLayout->addWidget(m_stackedWidget, 1);

  buildOverviewPage();
  buildRegisterPage();
  buildPatientsPage();
  buildRoomQueuePage();
  buildLeaveManagePage();

  m_stackedWidget->addWidget(m_overviewPage);
  m_stackedWidget->addWidget(m_registerPage);
  m_stackedWidget->addWidget(m_patientsPage);
  m_stackedWidget->addWidget(m_roomQueuePage);
  m_stackedWidget->addWidget(m_leaveManagePage);

  switchPage(0, m_btnOverview);

  refreshOverviewStats();
}

void ReceptionDashboardWidget::refreshOverviewStats() {
  QDate today = QDate::currentDate();

  if (m_appointmentService) {
    auto appts = m_appointmentService->getAppointmentsByDate(today);

    int completedCount = 0;
    int startedCount = 0;
    int checkedInCount = 0;
    int scheduledCount = 0;
    int noShowCancelledCount = 0;

    for (const auto &a : appts) {
      if (a.status == AppointmentStatusText::COMPLETED) {
        completedCount++;
      } else if (a.status == AppointmentStatusText::STARTED) {
        startedCount++;
      } else if (a.status == AppointmentStatusText::CHECKED_IN) {
        checkedInCount++;
      } else if (a.status == AppointmentStatusText::SCHEDULED) {
        scheduledCount++;
      } else if (a.status == AppointmentStatusText::CANCELLED ||
                 a.status == AppointmentStatusText::NO_SHOW) {
        noShowCancelledCount++;
      }
    }

    if (m_lblCompletedToday)
      m_lblCompletedToday->setText(QString("%1 ca").arg(completedCount));
    if (m_lblStartedToday)
      m_lblStartedToday->setText(QString("Đang khám: %1 ca").arg(startedCount));
    if (m_lblCheckedInToday)
      m_lblCheckedInToday->setText(QString("%1 bệnh nhân").arg(checkedInCount));
    if (m_lblScheduledToday)
      m_lblScheduledToday->setText(QString("%1 ca").arg(scheduledCount));
    if (m_lblNoShowCancelled)
      m_lblNoShowCancelled->setText(
          QString("Hủy / Vắng mặt: %1 ca").arg(noShowCancelledCount));
    if (m_lblTotalApptsToday)
      m_lblTotalApptsToday->setText(QString("%1 lượt").arg(appts.size()));

    auto roomQueues = m_appointmentService->getRoomQueueStatuses(today);
    int activeRooms = 0;
    for (const auto &rq : roomQueues) {
      if (rq.currentTicketNumber > 0 || rq.nextTicketNumber > 0 ||
          rq.doctorId > 0) {
        activeRooms++;
      }
    }
    if (m_lblActiveRooms) {
      m_lblActiveRooms->setText(
          QString("Phòng đang hoạt động: %1 phòng")
              .arg(activeRooms > 0 ? activeRooms : roomQueues.size()));
    }
  }

  if (m_analyticService) {
    WaitTimeStatsDTO waitStats =
        m_analyticService->getWaitTimeStats(today, today);
    if (m_lblAvgWaitTime) {
      m_lblAvgWaitTime->setText(
          QString("Thời gian chờ TB: %1 phút")
              .arg(QString::number(waitStats.avg, 'f', 1)));
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

  // Prevent duplicate sidebar creation
  if (m_btnOverview)
    return;

  QLabel *roleBadge = new QLabel("LỄ TÂN", m_sidebarFrame);
  roleBadge->setAlignment(Qt::AlignCenter);
  roleBadge->setStyleSheet(
      "QLabel { background-color: #EFF6FF; color: #1E40AF; font-size: 14px; "
      "font-weight: 900; font-family: 'Segoe UI'; letter-spacing: 1.5px; "
      "padding: 10px 14px; border-radius: 10px; border: none; "
      "margin-top: 4px; margin-bottom: 14px; }");
  m_sidebarLayout->addWidget(roleBadge);

  m_btnOverview = new QPushButton("Tổng Quan", m_sidebarFrame);
  m_btnRegister = new QPushButton("Đăng Ký Khám", m_sidebarFrame);
  m_btnPatients = new QPushButton("Bệnh Nhân", m_sidebarFrame);
  m_btnRoomQueue = new QPushButton("Hàng Đợi", m_sidebarFrame);
  m_btnLeaveManage = new QPushButton("Nghỉ Phép", m_sidebarFrame);

  m_btnOverview->setCursor(Qt::PointingHandCursor);
  m_btnRegister->setCursor(Qt::PointingHandCursor);
  m_btnPatients->setCursor(Qt::PointingHandCursor);
  m_btnRoomQueue->setCursor(Qt::PointingHandCursor);
  m_btnLeaveManage->setCursor(Qt::PointingHandCursor);

  m_sidebarLayout->addWidget(m_btnOverview);
  m_sidebarLayout->addWidget(m_btnRegister);
  m_sidebarLayout->addWidget(m_btnPatients);
  m_sidebarLayout->addWidget(m_btnRoomQueue);
  m_sidebarLayout->addWidget(m_btnLeaveManage);
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
  connect(m_btnPatients, &QPushButton::clicked, this, [this]() {
    handlePatientFilterChanged();
    switchPage(2, m_btnPatients);
  });
  connect(m_btnRoomQueue, &QPushButton::clicked, this, [this]() {
    onRefreshRoomQueue();
    switchPage(3, m_btnRoomQueue);
  });
  connect(m_btnLeaveManage, &QPushButton::clicked, this, [this]() {
    onLeaveTabSelected();
    switchPage(4, m_btnLeaveManage);
  });

  connect(m_btnLogout, &QPushButton::clicked, this,
          &BaseDashboardWidget::logoutRequested);
}

void ReceptionDashboardWidget::switchPage(int index, QPushButton *activeBtn) {
  if (!m_stackedWidget)
    return;
  m_stackedWidget->setCurrentIndex(index);

  QPushButton *buttons[] = {m_btnOverview, m_btnRegister, m_btnPatients,
                            m_btnRoomQueue, m_btnLeaveManage};
  for (auto *btn : buttons) {
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
    refreshOverviewStats();
  }
}

void ReceptionDashboardWidget::buildOverviewPage() {
  m_overviewPage = new QWidget();
  m_overviewPage->setStyleSheet("background-color: #F8F9FA;");

  QVBoxLayout *layout = new QVBoxLayout(m_overviewPage);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(20);

  QLabel *lblTitle = new QLabel("TỔNG QUAN HÔM NAY", m_overviewPage);
  lblTitle->setStyleSheet(
      "font-size: 22px; font-weight: bold; color: #000000;");
  layout->addWidget(lblTitle);

  // Cards layout
  QHBoxLayout *cardsLayout = new QHBoxLayout();
  cardsLayout->setSpacing(16);

  // Card 1: Completed & Started
  QFrame *card1 = makeCard(m_overviewPage);
  QVBoxLayout *c1Layout = new QVBoxLayout(card1);
  c1Layout->setContentsMargins(16, 14, 16, 14);
  c1Layout->setSpacing(4);
  QLabel *c1Title = new QLabel("Lượt khám hoàn thành", card1);
  c1Title->setStyleSheet("color: #5F6368; font-size: 13px; font-weight: 500;");
  m_lblCompletedToday = new QLabel("0 ca", card1);
  m_lblCompletedToday->setStyleSheet(
      "color: #34A853; font-size: 22px; font-weight: bold;");
  m_lblStartedToday = new QLabel("Đang khám: 0 ca", card1);
  m_lblStartedToday->setStyleSheet(
      "color: #718096; font-size: 12px; font-weight: 500;");
  c1Layout->addWidget(c1Title);
  c1Layout->addWidget(m_lblCompletedToday);
  c1Layout->addWidget(m_lblStartedToday);
  cardsLayout->addWidget(card1);

  // Card 2: Waiting Patients & Wait Time
  QFrame *card2 = makeCard(m_overviewPage);
  QVBoxLayout *c2Layout = new QVBoxLayout(card2);
  c2Layout->setContentsMargins(16, 14, 16, 14);
  c2Layout->setSpacing(4);
  QLabel *c2Title = new QLabel("Bệnh nhân chờ khám", card2);
  c2Title->setStyleSheet("color: #5F6368; font-size: 13px; font-weight: 500;");
  m_lblCheckedInToday = new QLabel("0 bệnh nhân", card2);
  m_lblCheckedInToday->setStyleSheet(
      "color: #E67E22; font-size: 22px; font-weight: bold;");
  m_lblAvgWaitTime = new QLabel("Thời gian chờ TB: 0.0 phút", card2);
  m_lblAvgWaitTime->setStyleSheet(
      "color: #718096; font-size: 12px; font-weight: 500;");
  c2Layout->addWidget(c2Title);
  c2Layout->addWidget(m_lblCheckedInToday);
  c2Layout->addWidget(m_lblAvgWaitTime);
  cardsLayout->addWidget(card2);

  // Card 3: Scheduled & No-Show
  QFrame *card3 = makeCard(m_overviewPage);
  QVBoxLayout *c3Layout = new QVBoxLayout(card3);
  c3Layout->setContentsMargins(16, 14, 16, 14);
  c3Layout->setSpacing(4);
  QLabel *c3Title = new QLabel("Lịch hẹn còn lại hôm nay", card3);
  c3Title->setStyleSheet("color: #5F6368; font-size: 13px; font-weight: 500;");
  m_lblScheduledToday = new QLabel("0 ca", card3);
  m_lblScheduledToday->setStyleSheet(
      "color: #4B94F2; font-size: 22px; font-weight: bold;");
  m_lblNoShowCancelled = new QLabel("Hủy / Vắng mặt: 0 ca", card3);
  m_lblNoShowCancelled->setStyleSheet(
      "color: #718096; font-size: 12px; font-weight: 500;");
  c3Layout->addWidget(c3Title);
  c3Layout->addWidget(m_lblScheduledToday);
  c3Layout->addWidget(m_lblNoShowCancelled);
  cardsLayout->addWidget(card3);

  // Card 4: Total Visits & Active Rooms
  QFrame *card4 = makeCard(m_overviewPage);
  QVBoxLayout *c4Layout = new QVBoxLayout(card4);
  c4Layout->setContentsMargins(16, 14, 16, 14);
  c4Layout->setSpacing(4);
  QLabel *c4Title = new QLabel("Tổng lượt tiếp nhận", card4);
  c4Title->setStyleSheet("color: #5F6368; font-size: 13px; font-weight: 500;");
  m_lblTotalApptsToday = new QLabel("0 lượt", card4);
  m_lblTotalApptsToday->setStyleSheet(
      "color: #2B6CB0; font-size: 22px; font-weight: bold;");
  m_lblActiveRooms = new QLabel("Phòng đang hoạt động: 0 phòng", card4);
  m_lblActiveRooms->setStyleSheet(
      "color: #718096; font-size: 12px; font-weight: 500;");
  c4Layout->addWidget(c4Title);
  c4Layout->addWidget(m_lblTotalApptsToday);
  c4Layout->addWidget(m_lblActiveRooms);
  cardsLayout->addWidget(card4);

  layout->addLayout(cardsLayout);

  QLabel *lblSubTitle = new QLabel("DANH SÁCH HOẠT ĐỘNG GẦN ĐÂY", m_overviewPage);
  lblSubTitle->setStyleSheet(
      "font-size: 16px; font-weight: bold; color: #000000; margin-top: 20px;");
  layout->addWidget(lblSubTitle);

  m_recentActivityTable = new QTableWidget(0, 4, m_overviewPage);
  m_recentActivityTable->setHorizontalHeaderLabels(
      {"Thời gian", "Bệnh nhân", "Bác sĩ", "Trạng thái"});
  m_recentActivityTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_recentActivityTable->setStyleSheet(
      "QTableWidget { background-color: white; "
      "border-radius: 8px; border: 1px solid #EAEAEA; color: #333333; outline: "
      "none; }"
      "QTableWidget::item { outline: none; border: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }"
      "QHeaderView::section { background-color: #EFF6FF; color: #1E40AF; "
      "font-weight: bold; border: none; padding: 8px 10px; border-bottom: 2px solid #BFDBFE; }");
  m_recentActivityTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_recentActivityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_recentActivityTable->setFocusPolicy(Qt::NoFocus);
  layout->addWidget(m_recentActivityTable);
  layout->addStretch();
}

void ReceptionDashboardWidget::refreshRecentActivity() {
  if (!m_recentActivityTable || !m_appointmentService)
    return;

  auto appts =
      m_appointmentService->getAppointmentsByDate(QDate::currentDate());
  int showCount = qMin<int>(appts.size(), 10);

  m_recentActivityTable->setRowCount(showCount);
  for (int i = 0; i < showCount; ++i) {
    const auto &a = appts[i];
    QString timeStr = a.startTime;
    if (!a.endTime.isEmpty())
      timeStr += " - " + a.endTime;

    m_recentActivityTable->setItem(i, 0, new QTableWidgetItem(timeStr));
    m_recentActivityTable->setItem(i, 1, new QTableWidgetItem(a.patientName));
    m_recentActivityTable->setItem(i, 2, new QTableWidgetItem(a.doctorName));
    m_recentActivityTable->setItem(
        i, 3, new QTableWidgetItem(AppointmentStatusText::toVi(a.status)));
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
    QMessageBox::warning(this, "Lỗi",
                         "Không tìm thấy bệnh nhân. Vui lòng tạo bệnh nhân mới "
                         "trước khi đăng ký khám.");
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
  QLabel *lblTitle = new QLabel("ĐĂNG KÝ KHÁM BỆNH", contentWidget);
  lblTitle->setStyleSheet(
      "font-size: 22px; font-weight: bold; color: #000000;");

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
    connect(
        &dialog, &PatientRegistrationDialog::saved, this,
        [this](const QString &phone, const QString &citizenId,
               const QString &name) {
          Q_UNUSED(name);
          if (!m_patientService)
            return;
          auto patientOpt =
              m_patientService->getPatientByPhoneOrCitizenId(phone, citizenId);
          if (patientOpt.has_value()) {
            m_currentPatientId = patientOpt->patientId;
            if (m_txtPatientPhone)
              m_txtPatientPhone->setText(patientOpt->phone);
            if (m_txtPatientCitizenId)
              m_txtPatientCitizenId->setText(citizenId);

            QMessageBox::information(
                this, "Xác Nhận Thông Tin Bệnh Nhân",
                QString("Đã xác nhận bệnh nhân thành công!\n\n"
                        "Họ tên: %1\nMã bệnh nhân: %2\nSố điện thoại: "
                        "%3\nCCCD: %4\n\n"
                        "Vui lòng chọn Chuyên khoa và Bác sĩ bên dưới để hoàn "
                        "tất Đăng ký khám.")
                    .arg(patientOpt->fullName, patientOpt->patientCode,
                         patientOpt->phone, citizenId));
          }
          if (m_txtPatientPhone)
            m_txtPatientPhone->setReadOnly(true);
          if (m_txtPatientCitizenId)
            m_txtPatientCitizenId->setReadOnly(true);

          if (m_btnContinue) {
            m_btnContinue->setText("Đã xác nhận");
            m_btnContinue->setEnabled(false);
            m_btnContinue->setStyleSheet(
                "background-color: #EAEAEA; color: #999; padding: 10px 25px; "
                "border-radius: 6px; font-size: 15px; font-weight: bold;");
          }

          if (m_apptCard)
            m_apptCard->setVisible(true);
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

  QLabel *lblStep1 = new QLabel("Thông tin bệnh nhân", patientCard);
  lblStep1->setStyleSheet(
      "font-size: 18px; font-weight: bold; color: #4B94F2;");
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
  m_btnContinue = new QPushButton("Xác nhận", patientCard);
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

  QLabel *lblStep2 = new QLabel("Chọn khung giờ khám", m_apptCard);
  lblStep2->setStyleSheet(
      "font-size: 18px; font-weight: bold; color: #4B94F2;");
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
  QCalendarWidget *apptCalendar = new QCalendarWidget();
  apptCalendar->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
  apptCalendar->setMinimumSize(330, 250);
  m_dateEdit->setCalendarWidget(apptCalendar);
  m_dateEdit->setCalendarPopup(true);
  m_dateEdit->setStyleSheet(inputStyle);

  // Custom calendar theme
  m_dateEdit->calendarWidget()->setStyleSheet(
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
      "selection-background-color: #4B94F2; selection-color: white; font-size: "
      "13px; }"
      "QCalendarWidget QAbstractItemView:enabled { font-size: 13px; color: "
      "#333333; background-color: #FFFFFF; selection-background-color: "
      "#4B94F2; selection-color: #FFFFFF; }"
      "QCalendarWidget QAbstractItemView:disabled { color: #CCCCCC; }");

  colDate->addWidget(lblDate);
  colDate->addWidget(m_dateEdit);

  apptFieldsLayout->addLayout(colSpecialty);
  apptFieldsLayout->addLayout(colDate);
  apptFieldsLayout->addStretch();
  apptLayout->addLayout(apptFieldsLayout);

  QWidget *listWidget = new QWidget(m_apptCard);
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
  QLayoutItem *item;
  while ((item = m_doctorListLayout->takeAt(0)) != nullptr) {
    if (item->widget())
      item->widget()->deleteLater();
    delete item;
  }

  m_selectedDoctorId = -1;
  m_selectedTimeSlot = "";
  m_selectedSlotButton = nullptr;

  DoctorSearchCriteria allCriteria;
  allCriteria.onlyActive = true;
  auto allDoctors = m_staffService
                        ? m_staffService->searchDoctorsPaged(allCriteria).items
                        : QList<std::shared_ptr<SystemUser>>();

  // Rebuild specialty combo dynamically from active doctor list
  QString savedSpecialty = m_comboSpecialty->currentText();
  m_comboSpecialty->blockSignals(true);
  m_comboSpecialty->clear();
  m_comboSpecialty->addItem("Tất cả");

  QStringList knownSpecialties;
  for (const auto &doc : allDoctors) {
    auto docModel = std::dynamic_pointer_cast<Doctor>(doc);
    if (!docModel)
      continue;
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
  if (specialty == "Tất cả")
    specialty = "";

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
    QLabel *lblEmpty = new QLabel("Không có bác sĩ nào cho chuyên khoa này.");
    lblEmpty->setAlignment(Qt::AlignCenter);
    lblEmpty->setStyleSheet("color: #777; font-size: 14px; margin-top: 20px;");
    m_doctorListLayout->addWidget(lblEmpty);
  }

  for (const auto &doc : doctors) {
    auto docModel = std::dynamic_pointer_cast<Doctor>(doc);
    if (!docModel)
      continue;

    int doctorId = docModel->getAccountId();
    QFrame *docCard = new QFrame(m_doctorListLayout->parentWidget());
    docCard->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px "
                           "solid #EAEAEA; border-radius: 12px; }");
    QVBoxLayout *cardLayout = new QVBoxLayout(docCard);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    cardLayout->setSpacing(10);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *lblAvatar = new QLabel(docCard);
    lblAvatar->setFixedSize(60, 60);
    lblAvatar->setStyleSheet("background-color: #F1F3F4; border-radius: 30px;");

    QVBoxLayout *infoLayout = new QVBoxLayout();
    QLabel *lblName = new QLabel(docModel->getFullName(), docCard);
    lblName->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #1976D2; border: none;");

    auto profile = m_staffService->getOwnProfile(doctorId);
    QString shiftInfo = profile ? profile->shift : "Cả ngày";
    QString shiftDisplay = shiftInfo.isEmpty() ? "Không có" : shiftInfo;
    QLabel *lblDesc = new QLabel("Phòng Khám - Ca: " + shiftDisplay, docCard);
    lblDesc->setStyleSheet("color: #555; font-size: 13px; border: none;");

    QPushButton *btnInfo = new QPushButton("Thông tin bác sĩ >", docCard);
    btnInfo->setCursor(Qt::PointingHandCursor);
    btnInfo->setStyleSheet(
        "background-color: #E3F2FD; color: #1976D2; border-radius: 10px; "
        "padding: 4px 10px; font-size: 12px; border: none; text-align: left; "
        "max-width: 120px;");

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

    QFrame *hLine = new QFrame(docCard);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setStyleSheet(
        "border: none; border-top: 1px dashed #DDD; background: transparent;");
    cardLayout->addWidget(hLine);

    QLabel *lblDateHeader = new QLabel(
        date.toString("dd/MM/yyyy") + " - Ca: " + shiftDisplay, docCard);
    lblDateHeader->setStyleSheet(
        "color: #2E7D32; font-weight: bold; font-size: 14px; border: none; "
        "margin-top: 5px;");
    cardLayout->addWidget(lblDateHeader);

    QStringList timeSlots =
        m_appointmentService->getAvailableTimeSlots(doctorId, date);

    if (timeSlots.isEmpty()) {
      QString emptyMsg = "ĐÃ HẾT SỐ ĐẶT TRƯỚC";
      if (shiftInfo.isEmpty() || shiftInfo.trimmed().toUpper() == "NONE") {
        emptyMsg = "BÁC SĨ KHÔNG CÓ LỊCH KHÁM";
      }
      QLabel *lblFull = new QLabel(emptyMsg, docCard);
      lblFull->setAlignment(Qt::AlignCenter);
      lblFull->setStyleSheet("color: #999; font-size: 13px; font-style: "
                             "italic; border: none; margin: 10px;");
      cardLayout->addWidget(lblFull);
    } else {
      QGridLayout *gridLayout = new QGridLayout();
      gridLayout->setSpacing(10);
      int row = 0, col = 0;
      for (const QString &slot : timeSlots) {
        QPushButton *btnSlot = new QPushButton(slot, docCard);
        btnSlot->setCursor(Qt::PointingHandCursor);
        btnSlot->setStyleSheet(
            "QPushButton { background-color: #F5F5F5; color: #333; border: "
            "none; border-radius: 8px; padding: 10px; font-weight: bold; "
            "font-size: 14px; }");
        connect(btnSlot, &QPushButton::clicked, this,
                [this, doctorId, slot, btnSlot]() {
                  this->onTimeSlotClicked(doctorId, slot, btnSlot);
                });
        gridLayout->addWidget(btnSlot, row, col);
        col++;
        if (col > 1) {
          col = 0;
          row++;
        }
      }
      cardLayout->addLayout(gridLayout);
    }
    m_doctorListLayout->addWidget(docCard);
  }
  m_doctorListLayout->addStretch();
}

void ReceptionDashboardWidget::onTimeSlotClicked(int doctorId,
                                                 const QString &timeSlot,
                                                 QPushButton *btn) {
  if (m_selectedSlotButton) {
    m_selectedSlotButton->setStyleSheet(
        "QPushButton { background-color: #F5F5F5; color: #333; border: none; "
        "border-radius: 8px; padding: 10px; font-weight: bold; font-size: "
        "14px; }");
  }
  m_selectedDoctorId = doctorId;
  m_selectedTimeSlot = timeSlot;
  m_selectedSlotButton = btn;
  btn->setStyleSheet("QPushButton { background-color: #0B57D0; color: white; "
                     "border: none; border-radius: 8px; padding: 10px; "
                     "font-weight: bold; font-size: 14px; }");
}

void ReceptionDashboardWidget::buildPatientsPage() {
  m_patientsPage = new QWidget();
  m_patientsPage->setStyleSheet("background-color: #F8F9FA;");

  QVBoxLayout *pageLayout = new QVBoxLayout(m_patientsPage);
  pageLayout->setContentsMargins(20, 20, 20, 20);
  pageLayout->setSpacing(15);

  // Top header layout
  QHBoxLayout *topLayout = new QHBoxLayout();
  QLabel *lblTitle = new QLabel("DANH SÁCH BỆNH NHÂN", m_patientsPage);
  lblTitle->setStyleSheet(
      "font-size: 22px; font-weight: bold; color: #000000;");
  topLayout->addWidget(lblTitle);
  topLayout->addStretch();

  pageLayout->addLayout(topLayout);

  // Card filter
  QFrame *filterCard = makeCard(m_patientsPage);
  QHBoxLayout *filterLayout = new QHBoxLayout(filterCard);
  filterLayout->setContentsMargins(15, 12, 15, 12);
  filterLayout->setSpacing(12);

  m_txtPatientSearchKey = new QLineEdit(filterCard);
  m_txtPatientSearchKey->setPlaceholderText(
      "Tìm kiếm họ tên, mã BN, CCCD, SĐT...");
  m_txtPatientSearchKey->setStyleSheet(
      "QLineEdit { padding: 6px 12px; border: 1px solid #D1D5DB; "
      "border-radius: 6px; font-size: 13px; min-height: 32px; background: "
      "white; min-width: 250px; }"
      "QLineEdit:focus { border: 1px solid #2563EB; }");

  m_btnResetPatientFilters = new QPushButton("Đặt lại", filterCard);
  m_btnResetPatientFilters->setCursor(Qt::PointingHandCursor);
  m_btnResetPatientFilters->setStyleSheet(
      "QPushButton { background-color: #6B7280; color: white; border: none; "
      "border-radius: 6px; padding: 4px 12px; font-weight: bold; min-height: 28px; "
      "font-size: 12px; } QPushButton:hover { background-color: #4B5563; }");

  filterLayout->addWidget(m_txtPatientSearchKey);
  filterLayout->addWidget(m_btnResetPatientFilters);
  filterLayout->addStretch();
  pageLayout->addWidget(filterCard);

  // Table Card
  QFrame *tableCard = makeCard(m_patientsPage);
  QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
  cardLayout->setContentsMargins(0, 0, 0, 0);

  m_patientsTable = new QTableWidget(0, 4, tableCard);
  m_patientsTable->setHorizontalHeaderLabels(
      {"Mã BN", "Họ Tên", "Giới Tính", "Thao tác"});
  m_patientsTable->horizontalHeader()->setStretchLastSection(false);
  m_patientsTable->horizontalHeader()->setSectionResizeMode(0,
                                                            QHeaderView::Fixed);
  m_patientsTable->horizontalHeader()->resizeSection(0, 140);
  m_patientsTable->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::Stretch);
  m_patientsTable->horizontalHeader()->setSectionResizeMode(2,
                                                            QHeaderView::Fixed);
  m_patientsTable->horizontalHeader()->resizeSection(2, 120);
  m_patientsTable->horizontalHeader()->setSectionResizeMode(3,
                                                            QHeaderView::Fixed);
  m_patientsTable->horizontalHeader()->resizeSection(3, 300);

  m_patientsTable->verticalHeader()->setDefaultSectionSize(46);
  m_patientsTable->verticalHeader()->setVisible(false);
  m_patientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_patientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_patientsTable->setFocusPolicy(Qt::NoFocus);
  m_patientsTable->setStyleSheet(
      "QTableWidget { border: none; outline: none; gridline-color: #E2E8F0; "
      "font-size: 13px; background-color: white; color: #0F172A; }"
      "QTableWidget::item { outline: none; border: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }"
      "QHeaderView::section { background-color: #EFF6FF; padding: 10px; "
      "font-weight: bold; border: none; border-bottom: 2px solid #BFDBFE; "
      "color: #1E40AF; }");

  // Pagination layout
  QHBoxLayout *paginationLayout = new QHBoxLayout();
  paginationLayout->setContentsMargins(15, 10, 15, 15);

  m_btnPatientPrevPage = new QPushButton("Trang trước", tableCard);
  m_btnPatientPrevPage->setCursor(Qt::PointingHandCursor);
  m_btnPatientPrevPage->setStyleSheet(
      "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: "
      "6px; padding: 6px 12px; font-weight: bold; border: none; min-height: "
      "32px; }"
      "QPushButton:hover { background-color: #CBD5E1; }"
      "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }");

  m_lblPatientPageInfo = new QLabel("Trang 1 / 1", tableCard);
  m_lblPatientPageInfo->setStyleSheet(
      "background: transparent; border: none; font-size: 13px; font-weight: "
      "bold; color: #475569;");

  m_btnPatientNextPage = new QPushButton("Trang sau", tableCard);
  m_btnPatientNextPage->setCursor(Qt::PointingHandCursor);
  m_btnPatientNextPage->setStyleSheet(
      "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: "
      "6px; padding: 6px 12px; font-weight: bold; border: none; min-height: "
      "32px; }"
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

  connect(m_txtPatientSearchKey, &QLineEdit::textChanged, this,
          &ReceptionDashboardWidget::handlePatientFilterChanged);
  connect(m_btnResetPatientFilters, &QPushButton::clicked, this,
          &ReceptionDashboardWidget::handlePatientResetFilters);
  connect(m_btnPatientPrevPage, &QPushButton::clicked, this,
          &ReceptionDashboardWidget::handlePatientPrevPage);
  connect(m_btnPatientNextPage, &QPushButton::clicked, this,
          &ReceptionDashboardWidget::handlePatientNextPage);

  refreshPatientsTable();
}

void ReceptionDashboardWidget::handlePatientFilterChanged() {
  m_patientCurrentPage = 1;
  refreshPatientsTable();
}

void ReceptionDashboardWidget::handlePatientResetFilters() {
  if (m_txtPatientSearchKey)
    m_txtPatientSearchKey->clear();
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
  if (!m_patientsTable)
    return;
  m_patientsTable->setRowCount(0);

  PatientSearchCriteria criteria;
  if (m_txtPatientSearchKey) {
    criteria.searchKey = m_txtPatientSearchKey->text().trimmed();
  }
  criteria.type = PatientType::Outpatient;
  criteria.page = m_patientCurrentPage;
  criteria.pageSize = 10;

  auto pagedResult = m_patientService
                         ? m_patientService->searchPatientsPaged(criteria)
                         : PagedResult<PatientSearchResultDTO>();
  auto patients = pagedResult.items;

  m_patientTotalPages = qMax(1, pagedResult.totalPages());
  m_patientCurrentPage = qBound(1, pagedResult.page, m_patientTotalPages);

  if (m_lblPatientPageInfo) {
    m_lblPatientPageInfo->setText(
        QString("%1 / %2").arg(m_patientCurrentPage).arg(m_patientTotalPages));
  }
  if (m_btnPatientPrevPage)
    m_btnPatientPrevPage->setEnabled(m_patientCurrentPage > 1);
  if (m_btnPatientNextPage)
    m_btnPatientNextPage->setEnabled(m_patientCurrentPage <
                                     m_patientTotalPages);

  m_patientsTable->setRowCount(patients.size());
  for (int i = 0; i < patients.size(); ++i) {
    const auto &p = patients[i];
    m_patientsTable->setItem(i, 0, new QTableWidgetItem(p.patientCode));
    m_patientsTable->setItem(i, 1, new QTableWidgetItem(p.fullName));
    m_patientsTable->setItem(i, 2,
                             new QTableWidgetItem(GenderText::toVi(p.gender)));

    QWidget *actionWidget = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(4, 4, 4, 4);
    actionLayout->setSpacing(8);
    actionWidget->setStyleSheet("background: transparent;");

    QPushButton *btnHistory = new QPushButton("Lịch sử khám");
    btnHistory->setCursor(Qt::PointingHandCursor);
    btnHistory->setStyleSheet(
        "QPushButton { background-color: #1A73E8; color: white; border-radius: "
        "4px; padding: 5px 12px; font-weight: bold; font-size: 12px; "
        "min-width: 95px; }"
        "QPushButton:hover { background-color: #1557B0; }");

    QPushButton *btnContactInfo = new QPushButton("Xem thông tin liên lạc");
    btnContactInfo->setCursor(Qt::PointingHandCursor);
    btnContactInfo->setStyleSheet(
        "QPushButton { background-color: #0284C7; color: white; border-radius: "
        "4px; padding: 5px 12px; font-weight: bold; font-size: 12px; "
        "min-width: 140px; }"
        "QPushButton:hover { background-color: #0369A1; }");

    int pId = p.patientId;
    QString pName = p.fullName;
    connect(btnHistory, &QPushButton::clicked, this,
            [this, pId, pName]() { showPatientHistoryDialog(pId, pName); });

    connect(btnContactInfo, &QPushButton::clicked, this, [this, pId]() {
      PatientEditDialog dialog(pId, m_patientService, this);
      connect(&dialog, &PatientEditDialog::patientUpdated, this,
              [this]() { refreshPatientsTable(); });
      dialog.exec();
    });

    actionLayout->addWidget(btnHistory);
    actionLayout->addWidget(btnContactInfo);
    actionLayout->setAlignment(Qt::AlignCenter);
    m_patientsTable->setCellWidget(i, 3, actionWidget);
  }
}



void ReceptionDashboardWidget::buildRoomQueuePage() {
  m_roomQueuePage = new QWidget();
  m_roomQueuePage->setStyleSheet("background-color: #F8F9FA;");

  QVBoxLayout *layout = new QVBoxLayout(m_roomQueuePage);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(20);

  QLabel *lblTitle = new QLabel("QUẢN LÝ HÀNG ĐỢI PHÒNG KHÁM", m_roomQueuePage);
  lblTitle->setStyleSheet(
      "font-size: 22px; font-weight: bold; color: #000000;");
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
  if (!m_roomQueueLayout || !m_appointmentService)
    return;

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

  for (const auto &st : statuses) {
    auto *card = new RoomQueueWidget(
        st.roomId, st.roomNumber, st.doctorId, st.doctorName,
        st.currentTicketNumber, st.nextTicketNumber, st.waitingCount, this);
    connect(card, &RoomQueueWidget::clicked, this, [this](int rId, int dId) {
      auto items =
          m_appointmentService->getDoctorQueue(dId, QDate::currentDate());
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

void ReceptionDashboardWidget::showPatientHistoryDialog(
    int patientId, const QString &patientName) {
  QDialog dialog(this);
  dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                        Qt::WindowStaysOnTopHint);
  dialog.setStyleSheet("QDialog { background-color: #FFFFFF; border: 1px solid "
                       "#EAEAEA; border-radius: 12px; }");
  dialog.setMinimumSize(900, 500);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(15);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *lblTitle = new QLabel(QString("Lịch sử khám - %1").arg(patientName));
  lblTitle->setStyleSheet(
      "font-size: 20px; font-weight: bold; color: #202124;");

  QPushButton *btnClose = new QPushButton("✕");
  btnClose->setFixedSize(30, 30);
  btnClose->setCursor(Qt::PointingHandCursor);
  btnClose->setStyleSheet("QPushButton { border: none; font-size: 18px; "
                          "font-weight: bold; color: #5F6368; }"
                          "QPushButton:hover { color: #D32F2F; "
                          "background-color: #FEE2E2; border-radius: 15px; }");
  connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::accept);

  headerLayout->addWidget(lblTitle);
  headerLayout->addStretch();
  headerLayout->addWidget(btnClose);
  layout->addLayout(headerLayout);

  QTableWidget *table = new QTableWidget(0, 6, &dialog);
  table->setHorizontalHeaderLabels(
      {"Ngày khám", "Giờ khám", "Bác sĩ", "Phòng", "Trạng thái", "Thao tác"});
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->horizontalHeader()->setSectionResizeMode(
      5, QHeaderView::ResizeToContents);
  table->setStyleSheet(
      "QTableWidget { background-color: white; border-radius: 8px; border: 1px "
      "solid #EAEAEA; color: #333333; outline: none; }"
      "QHeaderView::section { background-color: #EFF6FF; font-weight: bold; "
      "border: none; padding: 10px; color: #1E40AF; border-bottom: 2px solid #BFDBFE; }"
      "QTableWidget::item { padding: 5px; border-bottom: 1px solid #EAEAEA; "
      "color: #333333; outline: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }");
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setFocusPolicy(Qt::NoFocus);

  auto appts = m_appointmentService->getPatientAppointments(patientId);
  table->setRowCount(appts.size());

  for (int i = 0; i < appts.size(); ++i) {
    const auto &a = appts[i];
    QString timeStr = a.startTime;
    if (!a.endTime.isEmpty())
      timeStr += " - " + a.endTime;

    table->setItem(i, 0, new QTableWidgetItem(a.appointmentDate));
    table->setItem(i, 1, new QTableWidgetItem(timeStr));
    table->setItem(i, 2, new QTableWidgetItem(a.doctorName));
    table->setItem(i, 3, new QTableWidgetItem(a.roomNumber));

    QString statusText = AppointmentStatusText::toVi(a.status);
    table->setItem(i, 4, new QTableWidgetItem(statusText));

    QDate apptDate = QDate::fromString(a.appointmentDate, "yyyy-MM-dd");
    QDate todayDate = QDate::currentDate();

    if (a.status == "SCHEDULED" && apptDate >= todayDate) {
      QWidget *widget = new QWidget();
      QHBoxLayout *l = new QHBoxLayout(widget);
      l->setContentsMargins(4, 4, 4, 4);
      l->setSpacing(8);

      QPushButton *btnCheckIn = nullptr;
      if (apptDate == todayDate) {
        btnCheckIn = new QPushButton("Check-in");
        btnCheckIn->setStyleSheet(
            "background-color: #34A853; color: white; border-radius: 4px; "
            "padding: 5px 10px; font-weight: bold;");
        btnCheckIn->setCursor(Qt::PointingHandCursor);
      }

      QPushButton *btnCancel = new QPushButton("Hủy");
      btnCancel->setStyleSheet(
          "background-color: #F44336; color: white; border-radius: 4px; "
          "padding: 5px 10px; font-weight: bold;");
      btnCancel->setCursor(Qt::PointingHandCursor);

      QString pName = patientName;
      QString dName = a.doctorName;
      QString rName = a.roomNumber;
      QString aDate = a.appointmentDate;
      QString tStr = timeStr;
      int apptId = a.appointmentId;
      QDialog *dlgPtr = &dialog;

      if (btnCheckIn) {
        connect(
            btnCheckIn, &QPushButton::clicked, &dialog,
            [this, dlgPtr, apptId, pName, dName, rName, aDate, tStr]() {
              QMessageBox confirmBox(dlgPtr);
              confirmBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                                        Qt::WindowStaysOnTopHint);
              confirmBox.setWindowTitle("Xác nhận");
              confirmBox.setText(
                  "Tiến hành Check-in và phát số thứ tự cho bệnh nhân?");
              confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
              confirmBox.setStyleSheet(
                  "QMessageBox { background-color: #FFFFFF; border: 2px solid "
                  "#4B94F2; border-radius: 8px; }"
                  "QLabel { color: #111827; font-size: 14px; font-weight: 500; "
                  "}"
                  "QPushButton { background-color: #4B94F2; color: white; "
                  "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                  "QPushButton:hover { background-color: #3b82f6; }");
              if (confirmBox.exec() == QMessageBox::Yes) {
                auto result = m_appointmentService->checkInPatient(apptId);
                if (result.second > 0) {
                  QMessageBox msgBox(dlgPtr);
                  msgBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                                        Qt::WindowStaysOnTopHint);
                  msgBox.setIcon(QMessageBox::Information);
                  msgBox.setWindowTitle("Check-in Thành công");

                  QString info =
                      QString("Bệnh nhân: %1\nNgày: %2  -  Giờ: %3\nBác sĩ: "
                              "%4\nPhòng: %5\nSỐ THỨ TỰ: %6")
                          .arg(pName)
                          .arg(aDate)
                          .arg(tStr)
                          .arg(dName)
                          .arg(rName)
                          .arg(result.second);

                  msgBox.setText("ĐÃ PHÁT SỐ THỨ TỰ THÀNH CÔNG\n\n" + info);
                  msgBox.setStyleSheet(
                      "QMessageBox { background-color: #FFFFFF; border: 2px "
                      "solid #059669; border-radius: 8px; }"
                      "QLabel { color: #111827; font-size: 14px; font-weight: "
                      "500; }"
                      "QPushButton { background-color: #059669; color: white; "
                      "border-radius: 4px; padding: 6px 16px; font-weight: "
                      "bold; }"
                      "QPushButton:hover { background-color: #047857; }");
                  msgBox.exec();
                  dlgPtr->accept();
                } else {
                  QMessageBox errBox(dlgPtr);
                  errBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                                        Qt::WindowStaysOnTopHint);
                  errBox.setIcon(QMessageBox::Warning);
                  errBox.setWindowTitle("Lỗi");
                  errBox.setText(result.first.isEmpty()
                                     ? "Không thể Check-in bệnh nhân."
                                     : result.first);
                  errBox.setStyleSheet(
                      "QMessageBox { background-color: #FFFFFF; border: 2px "
                      "solid #F44336; border-radius: 8px; }"
                      "QLabel { color: #111827; font-size: 14px; font-weight: "
                      "500; }"
                      "QPushButton { background-color: #F44336; color: white; "
                      "border-radius: 4px; padding: 6px 16px; font-weight: "
                      "bold; }"
                      "QPushButton:hover { background-color: #D32F2F; }");
                  errBox.exec();
                }
              }
            });
      }

      connect(
          btnCancel, &QPushButton::clicked, &dialog, [this, dlgPtr, apptId]() {
            QMessageBox confirmBox(dlgPtr);
            confirmBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                                      Qt::WindowStaysOnTopHint);
            confirmBox.setWindowTitle("Xác nhận");
            confirmBox.setText("Bạn có chắc chắn muốn hủy lịch hẹn này?");
            confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            confirmBox.setStyleSheet(
                "QMessageBox { background-color: #FFFFFF; border: 2px solid "
                "#F59E0B; border-radius: 8px; }"
                "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                "QPushButton { background-color: #F59E0B; color: white; "
                "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                "QPushButton:hover { background-color: #D97706; }");
            if (confirmBox.exec() == QMessageBox::Yes) {
              QString err = m_appointmentService->cancelAppointment(apptId);
              if (err.isEmpty()) {
                QMessageBox okBox(dlgPtr);
                okBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                                     Qt::WindowStaysOnTopHint);
                okBox.setIcon(QMessageBox::Information);
                okBox.setWindowTitle("Thành công");
                okBox.setText("Đã hủy lịch hẹn.");
                okBox.setStyleSheet(
                    "QMessageBox { background-color: #FFFFFF; border: 2px "
                    "solid #059669; border-radius: 8px; }"
                    "QLabel { color: #111827; font-size: 14px; font-weight: "
                    "500; }"
                    "QPushButton { background-color: #059669; color: white; "
                    "border-radius: 4px; padding: 6px 16px; font-weight: bold; "
                    "}"
                    "QPushButton:hover { background-color: #047857; }");
                okBox.exec();
                dlgPtr->accept();
              } else {
                QMessageBox errBox(dlgPtr);
                errBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                                      Qt::WindowStaysOnTopHint);
                errBox.setIcon(QMessageBox::Warning);
                errBox.setWindowTitle("Lỗi");
                errBox.setText(err);
                errBox.setStyleSheet(
                    "QMessageBox { background-color: #FFFFFF; border: 2px "
                    "solid #F44336; border-radius: 8px; }"
                    "QLabel { color: #111827; font-size: 14px; font-weight: "
                    "500; }"
                    "QPushButton { background-color: #F44336; color: white; "
                    "border-radius: 4px; padding: 6px 16px; font-weight: bold; "
                    "}"
                    "QPushButton:hover { background-color: #D32F2F; }");
                errBox.exec();
              }
            }
          });

      if (btnCheckIn)
        l->addWidget(btnCheckIn);
      l->addWidget(btnCancel);
      l->setAlignment(Qt::AlignCenter);
      table->setCellWidget(i, 5, widget);
    } else {
      table->removeCellWidget(i, 5);
      QTableWidgetItem *emptyItem = new QTableWidgetItem("-");
      emptyItem->setTextAlignment(Qt::AlignCenter);
      table->setItem(i, 5, emptyItem);
    }
  }

  layout->addWidget(table);
  dialog.exec();
}

void ReceptionDashboardWidget::buildLeaveManagePage() {
  m_leaveManagePage = new QWidget(this);
  m_leaveManagePage->setStyleSheet("background-color: #FFFFFF;");
  QVBoxLayout *layout = new QVBoxLayout(m_leaveManagePage);
  layout->setContentsMargins(40, 40, 40, 40);
  layout->setSpacing(20);

  QLabel *lblTitle = new QLabel("QUẢN LÝ NGHỈ PHÉP", m_leaveManagePage);
  lblTitle->setStyleSheet(
      "font-size: 22px; font-weight: bold; color: #000000; background: transparent; border: none;");
  layout->addWidget(lblTitle);

  m_leaveTabWidget = new QTabWidget(m_leaveManagePage);
  m_leaveTabWidget->setStyleSheet(
      "QTabWidget::pane { border: 1px solid #E2E8F0; border-radius: 12px; background-color: #FFFFFF; }"
      "QTabBar::tab { padding: 10px 22px; font-weight: bold; font-size: 13px; color: #475569; background-color: #F8FAFC; border: 1px solid #E2E8F0; border-bottom: none; border-top-left-radius: 8px; border-top-right-radius: 8px; margin-right: 2px; }"
      "QTabBar::tab:selected { color: #2563EB; background-color: #FFFFFF; border-bottom: 2px solid #FFFFFF; }");

  // --- Tab 1: Đăng ký nghỉ phép ---
  QWidget *tabRegister = new QWidget();
  tabRegister->setStyleSheet("background-color: #FFFFFF;");
  QVBoxLayout *layRegister = new QVBoxLayout(tabRegister);
  layRegister->setContentsMargins(20, 20, 20, 20);

  QFrame *formFrame = new QFrame(tabRegister);
  formFrame->setStyleSheet(
      "QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 12px; }"
      "QLabel { color: #0F172A; font-size: 13px; font-weight: 600; background-color: transparent; border: none; }"
  );
  QFormLayout *formLayout = new QFormLayout(formFrame);
  formLayout->setContentsMargins(24, 24, 24, 24);
  formLayout->setSpacing(16);

  m_lblLeaveBalance = new QLabel("Đang tải dữ liệu...", formFrame);
  m_lblLeaveBalance->setStyleSheet("color: #2563EB; font-weight: bold; font-size: 14px; background: transparent; border: none;");

  auto createCustomCalendar = []() {
    QCalendarWidget *cal = new QCalendarWidget();
    cal->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
    cal->setMinimumSize(330, 250);
    cal->setStyleSheet(
        "QCalendarWidget { background-color: #FFFFFF; color: #333333; }"
        "QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: "
        "#FFFFFF; border-bottom: 1px solid #EAEAEA; min-height: 36px; }"
        "QCalendarWidget QToolButton { color: #333333; font-weight: bold; "
        "background-color: transparent; border: none; padding: 4px 6px; "
        "margin: 1px; font-size: 13px; }"
        "QCalendarWidget QToolButton:hover { background-color: #E3F2FD; "
        "border-radius: 4px; }"
        "QCalendarWidget QMenu { background-color: #FFFFFF; color: #333333; }"
        "QCalendarWidget QSpinBox { background-color: #FFFFFF; color: #333333; "
        "selection-background-color: #4B94F2; selection-color: white; "
        "font-size: 13px; }"
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
      "QDateEdit { padding: 8px 12px; border: 1px solid #CBD5E1; border-radius: 6px; color: #0F172A; background-color: #FFFFFF; font-size: 13px; }"
      "QDateEdit:focus { border: 1px solid #2563EB; }");

  m_leaveEndDate = new QDateEdit(QDate::currentDate(), formFrame);
  m_leaveEndDate->setCalendarWidget(createCustomCalendar());
  m_leaveEndDate->setCalendarPopup(true);
  m_leaveEndDate->setMinimumDate(QDate::currentDate());
  m_leaveEndDate->setDisplayFormat("dd/MM/yyyy");
  m_leaveEndDate->setStyleSheet(
      "QDateEdit { padding: 8px 12px; border: 1px solid #CBD5E1; border-radius: 6px; color: #0F172A; background-color: #FFFFFF; font-size: 13px; }"
      "QDateEdit:focus { border: 1px solid #2563EB; }");

  m_txtLeaveReason = new QTextEdit(formFrame);
  m_txtLeaveReason->setFixedHeight(80);
  m_txtLeaveReason->setStyleSheet(
      "QTextEdit { padding: 8px 12px; border: 1px solid #CBD5E1; border-radius: 6px; color: #0F172A; background-color: #FFFFFF; font-size: 13px; }"
      "QTextEdit:focus { border: 1px solid #2563EB; }");

  formLayout->addRow("Quỹ phép còn lại:", m_lblLeaveBalance);
  formLayout->addRow("Từ ngày:", m_leaveStartDate);
  formLayout->addRow("Đến ngày:", m_leaveEndDate);
  formLayout->addRow("Lý do:", m_txtLeaveReason);

  QPushButton *btnSubmit = new QPushButton("Gửi yêu cầu", formFrame);
  btnSubmit->setCursor(Qt::PointingHandCursor);
  btnSubmit->setStyleSheet(
      "QPushButton { background-color: #2563EB; color: white; padding: 10px 24px; font-weight: bold; font-size: 13px; border-radius: 8px; border: none; min-height: 36px; }"
      "QPushButton:hover { background-color: #1D4ED8; }");

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  btnLayout->addWidget(btnSubmit);

  layRegister->addWidget(formFrame);
  layRegister->addLayout(btnLayout);
  layRegister->addStretch();

  m_leaveTabWidget->addTab(tabRegister, "Đăng ký nghỉ phép");

  // --- Tab 2: Lịch sử nghỉ phép ---
  QWidget *tabHistory = new QWidget();
  tabHistory->setStyleSheet("background-color: #FFFFFF;");
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
    "QTableWidget { border: 1px solid #E2E8F0; border-radius: 8px; color: #0F172A; background-color: #FFFFFF; outline: none; }"
    "QTableWidget::item { background-color: transparent; outline: none; border: none; color: #0F172A; padding: 8px; }"
    "QTableWidget::item:selected { background-color: #EFF6FF; color: #2563EB; font-weight: 600; }"
    "QHeaderView::section { background-color: #EFF6FF; color: #1E40AF; font-weight: bold; padding: 8px; border: none; border-bottom: 2px solid #BFDBFE; }");
  layHistory->addWidget(m_tableLeaveHistory);
  m_leaveTabWidget->addTab(tabHistory, "Lịch sử nghỉ phép");

  layout->addWidget(m_leaveTabWidget);

  connect(btnSubmit, &QPushButton::clicked, this,
          &ReceptionDashboardWidget::onSubmitLeaveRequest);

  connect(m_leaveTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
    if (index == 1) {
      loadLeaveHistory();
    }
  });

  m_stackedWidget->addWidget(m_leaveManagePage);
}

void ReceptionDashboardWidget::onLeaveTabSelected() {
  if (!m_currentUser || !m_staffService)
    return;

  if (auto sysUser = std::dynamic_pointer_cast<SystemUser>(m_currentUser)) {
    int staffId = sysUser->getAccountId();
    LeaveBalanceDTO balance =
        m_staffService->getLeaveBalance(staffId, QDate::currentDate().year());
    if (balance.totalDays > 0) {
      int remaining = balance.totalDays - balance.usedDays;
      m_lblLeaveBalance->setText(QString("%1 ngày").arg(remaining));
      m_lblLeaveBalance->setStyleSheet("background-color: transparent; color: #1A73E8; font-weight: bold;");
    } else {
      m_lblLeaveBalance->setText("Không thể lấy dữ liệu quỹ phép.");
      m_lblLeaveBalance->setStyleSheet("background-color: transparent; color: red;");
    }
  }
}

void ReceptionDashboardWidget::loadLeaveHistory() {
  if (!m_currentUser || !m_tableLeaveHistory || !m_staffService)
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

    QTableWidgetItem *statusItem =
        new QTableWidgetItem(LeaveStatusText::toVi(req.status));
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

void ReceptionDashboardWidget::onSubmitLeaveRequest() {
  if (!m_currentUser || !m_staffService)
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

  try {
    QString errorMsg =
        m_staffService->registerLeave(staffId, startDate, endDate, reason);
    if (errorMsg.isEmpty()) {
      int year = startDate.year();
      QMessageBox::information(
          this, "Thành công",
          "Đăng ký nghỉ phép thành công! Đơn của bạn đang ở trạng thái CHỜ "
          "DUYỆT.");

      LeaveBalanceDTO balance = m_staffService->getLeaveBalance(staffId, year);
      int remaining = balance.totalDays - balance.usedDays;
      m_lblLeaveBalance->setText(QString("%1 ngày").arg(remaining));

      m_txtLeaveReason->clear();
    } else {
      QMessageBox::warning(this, "Lỗi", errorMsg);
    }
  } catch (const std::exception &e) {
    QMessageBox::warning(this, "Lỗi", e.what());
  }
}
