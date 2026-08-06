#include "NurseDashboard.h"
#include "dto/AppointmentDTOs.h"
#include "model/SystemUser.h"
#include "ui/view/Profile.h"


#include <QDate>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QCalendarWidget>
#include <QFormLayout>
#include <QTabWidget>
#include <QLocale>
#include <QStyle>


#include "repository/DatabaseManager.h"

NurseDashboardWidget::NurseDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<ServiceRequestService> serviceRequestService,
    std::shared_ptr<AppointmentService> appointmentService, QWidget *parent)
    : BaseDashboardWidget(user, staffService, parent),
      m_serviceRequestService(serviceRequestService),
      m_appointmentService(appointmentService) {

  // Read Nurse Room ID & Room Type from user profile if available
  if (m_staffService && m_currentUser) {
    auto profile = m_staffService->getOwnProfile(m_currentUser->getAccountId());
    if (auto nurseProfile = dynamic_cast<NurseProfileDTO *>(profile.get())) {
      m_nurseRoomId = nurseProfile->roomId;
      m_nurseRoomType = nurseProfile->roomType;
    }
  }

  if (m_nurseRoomType == RoomType::Unknown && m_nurseRoomId > 0) {
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(
        "SELECT room_type FROM rooms WHERE room_id = ?", {m_nurseRoomId});
    if (query.next()) {
      m_nurseRoomType = roomTypeFromString(query.value(0).toString());
    }
  }

  initializeDashboard();
}

QFrame *NurseDashboardWidget::makeCard(QWidget *parent) {
  QFrame *card = new QFrame(parent ? parent : this);
  card->setStyleSheet("QFrame { background-color: #FFFFFF; border-radius: 8px; "
                      "border: 1px solid #DFE1E6; }");
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
  shadow->setBlurRadius(10);
  shadow->setColor(QColor(0, 0, 0, 15));
  shadow->setOffset(0, 2);
  card->setGraphicsEffect(shadow);
  return card;
}

void NurseDashboardWidget::fillDashboardData() {
  buildSidebar();

  m_stackedWidget = new QStackedWidget(m_mainContentWidget);

  buildOverviewPage();

  if (m_nurseRoomType == RoomType::Lab) {
    buildLabQueuePage();
    m_stackedWidget->addWidget(m_overviewPage);
    m_stackedWidget->addWidget(m_labQueuePage);
    loadLabRooms();
  } else if (m_nurseRoomType == RoomType::Exam) {
    buildAppointmentsPage();
    m_stackedWidget->addWidget(m_overviewPage);
    m_stackedWidget->addWidget(m_appointmentsPage);
  } else {
    // Unassigned or General Nurse: Add all pages
    buildAppointmentsPage();
    buildLabQueuePage();
    m_stackedWidget->addWidget(m_overviewPage);
    m_stackedWidget->addWidget(m_appointmentsPage);
    m_stackedWidget->addWidget(m_labQueuePage);
    loadLabRooms();
  }

  buildLeaveManagePage();

  m_mainContentLayout->addWidget(m_stackedWidget);

  switchPage(0, m_btnOverview);
  updateOverviewData();
  if (m_nurseRoomType != RoomType::Lab) {
    updateAppointmentsTable();
  }
  if (m_nurseRoomType != RoomType::Exam) {
    updateQueueTable();
  }
}

void NurseDashboardWidget::buildSidebar() {
  QString roleBadgeText = "Y TÁ";
  if (m_nurseRoomType == RoomType::Lab) {
    roleBadgeText = "Y TÁ XÉT NGHIỆM";
  } else if (m_nurseRoomType == RoomType::Exam) {
    roleBadgeText = "Y TÁ PHÒNG KHÁM";
  }

  QLabel *roleBadge = new QLabel(roleBadgeText, m_sidebarFrame);
  roleBadge->setAlignment(Qt::AlignCenter);
  roleBadge->setStyleSheet(
      "QLabel { background-color: #EFF6FF; color: #1E40AF; font-size: 14px; "
      "font-weight: 900; font-family: 'Segoe UI'; letter-spacing: 1.5px; "
      "padding: 10px 14px; border-radius: 10px; border: none; "
      "margin-top: 4px; margin-bottom: 14px; }");
  m_sidebarLayout->addWidget(roleBadge);

  m_btnOverview = new QPushButton("Tổng Quan", m_sidebarFrame);
  m_btnOverview->setCursor(Qt::PointingHandCursor);
  m_sidebarLayout->addWidget(m_btnOverview);

  if (m_nurseRoomType == RoomType::Exam) {
    m_btnAppointments = new QPushButton("Lịch Hẹn Khám", m_sidebarFrame);
    m_btnAppointments->setCursor(Qt::PointingHandCursor);
    m_sidebarLayout->addWidget(m_btnAppointments);
  } else if (m_nurseRoomType == RoomType::Lab) {
    m_btnLabQueue = new QPushButton("Phòng Xét Nghiệm", m_sidebarFrame);
    m_btnLabQueue->setCursor(Qt::PointingHandCursor);
    m_sidebarLayout->addWidget(m_btnLabQueue);
  } else {
    // Unassigned / General: show both
    m_btnAppointments = new QPushButton("Lịch Hẹn Khám", m_sidebarFrame);
    m_btnAppointments->setCursor(Qt::PointingHandCursor);
    m_sidebarLayout->addWidget(m_btnAppointments);

    m_btnLabQueue = new QPushButton("Phòng Xét Nghiệm", m_sidebarFrame);
    m_btnLabQueue->setCursor(Qt::PointingHandCursor);
    m_sidebarLayout->addWidget(m_btnLabQueue);
  }

  m_btnLeaveManage = new QPushButton("Nghỉ Phép", m_sidebarFrame);
  m_btnLeaveManage->setCursor(Qt::PointingHandCursor);
  m_sidebarLayout->addWidget(m_btnLeaveManage);

  QPushButton *btnLogout = new QPushButton("Đăng Xuất", m_sidebarFrame);
  btnLogout->setCursor(Qt::PointingHandCursor);
  btnLogout->setStyleSheet(
      "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; "
      "color: #D32F2F; border: none; border-radius: 0px; background-color: "
      "transparent; font-weight: bold; }"
      "QPushButton:hover { background-color: #FFEBEE; }");

  m_sidebarLayout->addStretch();
  m_sidebarLayout->addWidget(btnLogout);
  m_sidebarLayout->addSpacing(30);

  connect(m_btnOverview, &QPushButton::clicked, this,
          [this]() { switchPage(0, m_btnOverview); });

  if (m_nurseRoomType == RoomType::Exam) {
    connect(m_btnAppointments, &QPushButton::clicked, this,
            [this]() { switchPage(1, m_btnAppointments); });
  } else if (m_nurseRoomType == RoomType::Lab) {
    connect(m_btnLabQueue, &QPushButton::clicked, this,
            [this]() { switchPage(1, m_btnLabQueue); });
  } else {
    connect(m_btnAppointments, &QPushButton::clicked, this,
            [this]() { switchPage(1, m_btnAppointments); });
    connect(m_btnLabQueue, &QPushButton::clicked, this,
            [this]() { switchPage(2, m_btnLabQueue); });
  }

  connect(m_btnLeaveManage, &QPushButton::clicked, this, [this]() {
    onLeaveTabSelected();
    int leaveIndex = m_stackedWidget->indexOf(m_leaveManagePage);
    switchPage(leaveIndex, m_btnLeaveManage);
  });

  connect(btnLogout, &QPushButton::clicked, this,
          &NurseDashboardWidget::logoutRequested);
}

void NurseDashboardWidget::buildOverviewPage() {
  m_overviewPage = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(m_overviewPage);
  mainLayout->setContentsMargins(20, 20, 20, 28);
  mainLayout->setSpacing(20);

  QString pageTitleText = "TỔNG QUAN HOẠT ĐỘNG Y TÁ";
  if (m_nurseRoomType == RoomType::Exam) {
    pageTitleText = "TỔNG QUAN PHÒNG KHÁM";
  } else if (m_nurseRoomType == RoomType::Lab) {
    pageTitleText = "TỔNG QUAN PHÒNG XÉT NGHIỆM";
  }

  QLabel *title = new QLabel(pageTitleText, m_overviewPage);
  title->setStyleSheet("font-size: 22px; font-weight: bold; color: #000000;");
  mainLayout->addWidget(title);

  // Stat Cards Grid
  QHBoxLayout *cardsLayout = new QHBoxLayout();
  cardsLayout->setSpacing(15);

  auto createStatCard = [this](const QString &titleText, QLabel *&valueLabel,
                               const QString &colorHex) -> QFrame * {
    QFrame *card = makeCard();
    QVBoxLayout *l = new QVBoxLayout(card);
    l->setContentsMargins(20, 15, 20, 15);
    QLabel *lblTitle = new QLabel(titleText, card);
    lblTitle->setStyleSheet(
        "font-size: 14px; font-weight: 600; color: #5E6C84; border: none;");
    valueLabel = new QLabel("0", card);
    valueLabel->setStyleSheet(
        QString("font-size: 28px; font-weight: bold; color: %1; border: none;")
            .arg(colorHex));
    l->addWidget(lblTitle);
    l->addWidget(valueLabel);
    return card;
  };

  if (m_nurseRoomType == RoomType::Exam) {
    cardsLayout->addWidget(
        createStatCard("Tổng ca hẹn hôm nay", m_lblTotalRequests, "#2563EB"));
    cardsLayout->addWidget(
        createStatCard("Chờ khám / Đã hẹn", m_lblPendingRequests, "#F59E0B"));
    cardsLayout->addWidget(
        createStatCard("Đã check-in", m_lblProcessingRequests, "#06B6D4"));
    cardsLayout->addWidget(
        createStatCard("Khám hoàn thành", m_lblCompletedRequests, "#10B981"));
  } else if (m_nurseRoomType == RoomType::Lab) {
    cardsLayout->addWidget(
        createStatCard("Tổng yêu cầu hôm nay", m_lblTotalRequests, "#2563EB"));
    cardsLayout->addWidget(
        createStatCard("Chờ xét nghiệm", m_lblPendingRequests, "#F59E0B"));
    cardsLayout->addWidget(
        createStatCard("Đang thực hiện", m_lblProcessingRequests, "#06B6D4"));
    cardsLayout->addWidget(
        createStatCard("Đã trả kết quả", m_lblCompletedRequests, "#10B981"));
  } else {
    cardsLayout->addWidget(
        createStatCard("Tổng lượt hôm nay", m_lblTotalRequests, "#2563EB"));
    cardsLayout->addWidget(
        createStatCard("Chờ xử lý", m_lblPendingRequests, "#F59E0B"));
    cardsLayout->addWidget(
        createStatCard("Đang thực hiện", m_lblProcessingRequests, "#06B6D4"));
    cardsLayout->addWidget(
        createStatCard("Đã hoàn thành", m_lblCompletedRequests, "#10B981"));
  }

  mainLayout->addLayout(cardsLayout);

  // Recent Requests Table Card
  QFrame *tableCard = makeCard();
  QVBoxLayout *tableCardLayout = new QVBoxLayout(tableCard);
  tableCardLayout->setContentsMargins(20, 20, 20, 20);

  QString tableTitleText = "DANH SÁCH HOẠT ĐỘNG GẦN ĐÂY";
  QStringList headers;
  if (m_nurseRoomType == RoomType::Exam) {
    tableTitleText = "DANH SÁCH LỊCH HẸN KHÁM HÔM NAY";
    headers = {"Thời gian", "Mã BN", "Bệnh nhân",
               "Bác sĩ",    "Phòng", "Trạng thái"};
  } else if (m_nurseRoomType == RoomType::Lab) {
    tableTitleText = "DANH SÁCH YÊU CẦU XÉT NGHIỆM GẦN ĐÂY";
    headers = {"STT", "Mã BN", "Bệnh nhân", "Dịch vụ", "Phòng", "Trạng thái"};
  } else {
    headers = {"STT", "Mã BN", "Bệnh nhân", "Chi tiết", "Phòng", "Trạng thái"};
  }

  QLabel *tableTitle = new QLabel(tableTitleText, tableCard);
  tableTitle->setStyleSheet(
      "font-size: 16px; font-weight: bold; color: #000000; border: none;");
  tableCardLayout->addWidget(tableTitle);

  m_overviewTable = new QTableWidget(tableCard);
  m_overviewTable->setColumnCount(headers.size());
  m_overviewTable->setHorizontalHeaderLabels(headers);
  m_overviewTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_overviewTable->verticalHeader()->setVisible(false);
  m_overviewTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_overviewTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_overviewTable->setFocusPolicy(Qt::NoFocus);
  m_overviewTable->setStyleSheet(
      "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: "
      "#F4F5F7; color: #172B4D; outline: none; } "
      "QHeaderView::section { background-color: #EFF6FF; color: #1E40AF; "
      "font-weight: bold; border: none; padding: 8px 10px; border-bottom: 2px solid #BFDBFE; } "
      "QTableWidget::item { color: #172B4D; background-color: #FFFFFF; "
      "padding: 6px; outline: none; } "
      "QTableWidget::item:focus { outline: none; border: none; } "
      "QTableWidget::item:selected { background-color: #DEEBFF; color: "
      "#0052CC; }");

  tableCardLayout->addWidget(m_overviewTable);
  mainLayout->addWidget(tableCard, 1);
}

void NurseDashboardWidget::buildAppointmentsPage() {
  m_appointmentsPage = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(m_appointmentsPage);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(16);

  QFrame *tblCard = makeCard(m_appointmentsPage);
  QVBoxLayout *tblLayout = new QVBoxLayout(tblCard);
  tblLayout->setContentsMargins(20, 15, 20, 15);
  tblLayout->setSpacing(12);

  QHBoxLayout *topLayout = new QHBoxLayout();
  m_lblApptTitle = new QLabel("DANH SÁCH LỊCH HẸN KHÁM HÔM NAY", tblCard);
  m_lblApptTitle->setStyleSheet(
      "font-size: 16px; font-weight: bold; color: #000000; border: none;");
  topLayout->addWidget(m_lblApptTitle);

  QLabel *lblFilter = new QLabel("Ngày:", tblCard);
  lblFilter->setStyleSheet("font-weight: bold; color: #475569;");

  m_apptDateEdit = new QDateEdit(QDate::currentDate(), tblCard);
  m_apptDateEdit->setCalendarPopup(true);
  m_apptDateEdit->setDisplayFormat("dd/MM/yyyy");
  m_apptDateEdit->setStyleSheet(
      "QDateEdit { padding: 6px 12px; border: 1px solid #D1D5DB; "
      "border-radius: 6px; font-size: 13px; min-height: 32px; background: "
      "white; }"
      "QDateEdit:focus { border: 1px solid #2563EB; }");

  QPushButton *btnRefresh = new QPushButton("Tải lại", tblCard);
  btnRefresh->setCursor(Qt::PointingHandCursor);
  btnRefresh->setStyleSheet(
      "QPushButton { background-color: #2563EB; color: white; border: none; "
      "border-radius: 8px; padding: 6px 18px; font-weight: bold; min-height: 32px; "
      "font-size: 13px; }"
      "QPushButton:hover { background-color: #1D4ED8; }");

  topLayout->addStretch();
  topLayout->addWidget(lblFilter);
  topLayout->addWidget(m_apptDateEdit);
  topLayout->addSpacing(10);
  topLayout->addWidget(btnRefresh);
  tblLayout->addLayout(topLayout);

  m_appointmentsTable = new QTableWidget(0, 6, tblCard);
  m_appointmentsTable->setHorizontalHeaderLabels(
      {"Thời gian", "Bệnh nhân", "Bác sĩ", "Phòng", "Trạng thái", "Thao tác"});
  m_appointmentsTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_appointmentsTable->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::ResizeToContents);
  m_appointmentsTable->horizontalHeader()->setSectionResizeMode(
      5, QHeaderView::Fixed);
  m_appointmentsTable->horizontalHeader()->resizeSection(5, 200);

  m_appointmentsTable->verticalHeader()->setDefaultSectionSize(46);
  m_appointmentsTable->setStyleSheet(
      "QTableWidget { background-color: white; border-radius: 8px; border: 1px "
      "solid #EAEAEA; color: #333333; outline: none; }"
      "QHeaderView::section { background-color: #EFF6FF; color: #1E40AF; "
      "font-weight: bold; border: none; padding: 10px; border-bottom: 2px solid #BFDBFE; }"
      "QTableWidget::item { padding: 5px; border-bottom: 1px solid #EAEAEA; "
      "color: #333333; outline: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }");
  m_appointmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_appointmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_appointmentsTable->setFocusPolicy(Qt::NoFocus);

  tblLayout->addWidget(m_appointmentsTable);
  layout->addWidget(tblCard, 1);

  connect(m_apptDateEdit, &QDateEdit::dateChanged, this,
          [this](const QDate &) { updateAppointmentsTable(); });
  connect(btnRefresh, &QPushButton::clicked, this,
          &NurseDashboardWidget::updateAppointmentsTable);
}

void NurseDashboardWidget::updateAppointmentsTable() {
  if (!m_appointmentsTable || !m_appointmentService)
    return;

  if (m_lblApptTitle) {
    m_lblApptTitle->setText("DANH SÁCH LỊCH HẸN KHÁM HÔM NAY");
  }

  m_appointmentsTable->setRowCount(0);
  QDate date = m_apptDateEdit->date();
  auto allAppts = m_appointmentService->getAppointmentsByDate(date);

  // Filter appointments for Nurse's assigned room (m_nurseRoomId)
  QList<AppointmentRecordDTO> appts;
  for (const auto &a : allAppts) {
    if (m_nurseRoomId <= 0 || a.roomId == m_nurseRoomId) {
      appts.append(a);
    }
  }

  m_appointmentsTable->setRowCount(appts.size());
  for (int i = 0; i < appts.size(); ++i) {
    const auto &a = appts[i];
    QString timeStr = a.startTime;
    if (!a.endTime.isEmpty())
      timeStr += " - " + a.endTime;

    m_appointmentsTable->setItem(i, 0, new QTableWidgetItem(timeStr));
    m_appointmentsTable->setItem(i, 1, new QTableWidgetItem(a.patientName));
    m_appointmentsTable->setItem(i, 2, new QTableWidgetItem(a.doctorName));
    m_appointmentsTable->setItem(i, 3, new QTableWidgetItem(a.roomNumber));

    QString statusText = AppointmentStatusText::toVi(a.status);
    m_appointmentsTable->setItem(i, 4, new QTableWidgetItem(statusText));

    QDate apptDate = QDate::fromString(a.appointmentDate, "yyyy-MM-dd");
    QDate todayDate = QDate::currentDate();

    if (a.status == "SCHEDULED" && apptDate >= todayDate) {
      QWidget *widget = new QWidget();
      widget->setStyleSheet("background-color: transparent;");
      QHBoxLayout *l = new QHBoxLayout(widget);
      l->setContentsMargins(4, 4, 4, 4);
      l->setSpacing(8);

      QPushButton *btnCheckIn = nullptr;
      if (apptDate == todayDate) {
        btnCheckIn = new QPushButton("Check-in");
        btnCheckIn->setStyleSheet(
            "background-color: #2563EB; color: white; border-radius: 4px; "
            "padding: 5px 10px; font-weight: bold;");
        btnCheckIn->setCursor(Qt::PointingHandCursor);
      }

      QPushButton *btnCancel = new QPushButton("Hủy hẹn");
      btnCancel->setStyleSheet(
          "background-color: #EF4444; color: white; border-radius: 4px; "
          "padding: 5px 10px; font-weight: bold;");
      btnCancel->setCursor(Qt::PointingHandCursor);

      QString pName = a.patientName;
      QString dName = a.doctorName;
      QString rName = a.roomNumber;
      QString aDate = a.appointmentDate;
      QString tStr = timeStr;
      int apptId = a.appointmentId;

      if (btnCheckIn) {
        connect(
            btnCheckIn, &QPushButton::clicked, this,
            [this, apptId, pName, dName, rName, aDate, tStr]() {
              QMessageBox confirmBox(this);
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
                  QMessageBox msgBox(this);
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
                  updateAppointmentsTable();
                } else {
                  QMessageBox errBox(this);
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

      connect(btnCancel, &QPushButton::clicked, this, [this, apptId]() {
        QMessageBox confirmBox(this);
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
            QMessageBox okBox(this);
            okBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                                 Qt::WindowStaysOnTopHint);
            okBox.setIcon(QMessageBox::Information);
            okBox.setWindowTitle("Thành công");
            okBox.setText("Hủy lịch hẹn thành công.");
            okBox.setStyleSheet(
                "QMessageBox { background-color: #FFFFFF; border: 2px solid "
                "#059669; border-radius: 8px; }"
                "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                "QPushButton { background-color: #059669; color: white; "
                "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                "QPushButton:hover { background-color: #047857; }");
            okBox.exec();
            updateAppointmentsTable();
          } else {
            QMessageBox errBox(this);
            errBox.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                                  Qt::WindowStaysOnTopHint);
            errBox.setIcon(QMessageBox::Warning);
            errBox.setWindowTitle("Lỗi");
            errBox.setText(err);
            errBox.setStyleSheet(
                "QMessageBox { background-color: #FFFFFF; border: 2px solid "
                "#F44336; border-radius: 8px; }"
                "QLabel { color: #111827; font-size: 14px; font-weight: 500; }"
                "QPushButton { background-color: #F44336; color: white; "
                "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
                "QPushButton:hover { background-color: #D32F2F; }");
            errBox.exec();
          }
        }
      });

      if (btnCheckIn)
        l->addWidget(btnCheckIn);
      l->addWidget(btnCancel);
      l->setAlignment(Qt::AlignCenter);
      m_appointmentsTable->setCellWidget(i, 5, widget);
    } else {
      m_appointmentsTable->setItem(i, 5, new QTableWidgetItem("--"));
    }
  }
}

void NurseDashboardWidget::buildLabQueuePage() {
  m_labQueuePage = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(m_labQueuePage);
  mainLayout->setContentsMargins(20, 20, 20, 28);
  mainLayout->setSpacing(15);

  // Top Filter Bar Card
  QFrame *filterCard = makeCard();
  QHBoxLayout *filterLayout = new QHBoxLayout(filterCard);
  filterLayout->setContentsMargins(15, 12, 15, 12);
  filterLayout->setSpacing(15);

  QString filterInputStyle =
      "QComboBox, QDateEdit { background-color: #FFFFFF; color: #172B4D; "
      "border: 1px solid #DFE1E6; border-radius: 6px; padding: 6px 12px; "
      "font-size: 13px; min-height: 28px; } "
      "QComboBox QAbstractItemView { background-color: #FFFFFF; color: "
      "#172B4D; selection-background-color: #DEEBFF; selection-color: #0052CC; "
      "}";

  QLabel *lblRoom = new QLabel("Phòng xét nghiệm:", filterCard);
  lblRoom->setStyleSheet("font-weight: 600; color: #172B4D; border: none;");
  m_lblRoomValue = new QLabel(filterCard);
  m_lblRoomValue->setStyleSheet(
      "font-weight: bold; color: #0052CC; font-size: 14px; background-color: "
      "#DEEBFF; border-radius: 4px; padding: 6px 12px; border: none;");

  QLabel *lblStatus = new QLabel("Trạng thái:", filterCard);
  lblStatus->setStyleSheet("font-weight: 600; color: #172B4D; border: none;");
  m_comboStatusFilter = new QComboBox(filterCard);
  m_comboStatusFilter->addItem("Tất cả", -1);
  for (const auto &info : serviceRequestStatusList) {
    m_comboStatusFilter->addItem(info.viText, static_cast<int>(info.value));
  }
  m_comboStatusFilter->setStyleSheet(filterInputStyle);

  QLabel *lblDate = new QLabel("Ngày:", filterCard);
  lblDate->setStyleSheet("font-weight: 600; color: #172B4D; border: none;");
  m_queueDateEdit = new QDateEdit(QDate::currentDate(), filterCard);
  m_queueDateEdit->setCalendarPopup(true);
  m_queueDateEdit->setDisplayFormat("dd/MM/yyyy");
  m_queueDateEdit->setStyleSheet(filterInputStyle);

  QPushButton *btnRefresh = new QPushButton("Tải lại", filterCard);
  btnRefresh->setCursor(Qt::PointingHandCursor);
  btnRefresh->setStyleSheet(
      "QPushButton { background-color: #2563EB; color: white; border: none; "
      "border-radius: 8px; padding: 6px 18px; font-weight: bold; min-height: 32px; "
      "font-size: 13px; } QPushButton:hover { background-color: #1D4ED8; }");

  filterLayout->addWidget(lblRoom);
  filterLayout->addWidget(m_lblRoomValue);
  filterLayout->addWidget(lblStatus);
  filterLayout->addWidget(m_comboStatusFilter);
  filterLayout->addWidget(lblDate);
  filterLayout->addWidget(m_queueDateEdit);
  filterLayout->addWidget(btnRefresh);
  filterLayout->addStretch();

  mainLayout->addWidget(filterCard);

  // Table + Action Panel Horizontal Layout
  QHBoxLayout *contentLayout = new QHBoxLayout();
  contentLayout->setSpacing(15);

  // Table Card
  QFrame *tableCard = makeCard();
  QVBoxLayout *tableLayout = new QVBoxLayout(tableCard);
  tableLayout->setContentsMargins(15, 15, 15, 15);

  m_queueTable = new QTableWidget(tableCard);
  m_queueTable->setColumnCount(7);
  m_queueTable->setHorizontalHeaderLabels({"STT", "Mã BN", "Họ và tên",
                                           "Dịch vụ", "Bác sĩ chỉ định",
                                           "Trạng thái", "Thời gian"});
  m_queueTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_queueTable->verticalHeader()->setVisible(false);
  m_queueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_queueTable->setSelectionMode(QAbstractItemView::SingleSelection);
  m_queueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_queueTable->setFocusPolicy(Qt::NoFocus);
  m_queueTable->setStyleSheet(
      "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: "
      "#F4F5F7; color: #172B4D; outline: none; } "
      "QHeaderView::section { background-color: #EFF6FF; color: #1E40AF; "
      "font-weight: bold; border: none; padding: 8px 10px; border-bottom: 2px solid #BFDBFE; } "
      "QTableWidget::item { color: #172B4D; background-color: #FFFFFF; "
      "padding: 6px; outline: none; } "
      "QTableWidget::item:focus { outline: none; border: none; } "
      "QTableWidget::item:selected { background-color: #DEEBFF; color: "
      "#0052CC; }");

  tableLayout->addWidget(m_queueTable);
  contentLayout->addWidget(tableCard, 3);

  // Right Action Panel Card
  QFrame *actionCard = makeCard();
  actionCard->setFixedWidth(240);
  QVBoxLayout *actionLayout = new QVBoxLayout(actionCard);
  actionLayout->setContentsMargins(15, 20, 15, 20);
  actionLayout->setSpacing(12);

  QLabel *actionTitle = new QLabel("THAO TÁC HÀNG ĐỢI", actionCard);
  actionTitle->setStyleSheet(
      "font-size: 16px; font-weight: bold; color: #000000; border: none;");
  actionLayout->addWidget(actionTitle);

  // Active Patient Section
  QFrame *activePatientBox = new QFrame(actionCard);
  activePatientBox->setStyleSheet(
      "QFrame { background-color: #F4F5F7; border: 1px solid #DFE1E6; "
      "border-radius: 6px; padding: 10px; } QLabel { border: none; background: "
      "transparent; }");
  QVBoxLayout *activeLayout = new QVBoxLayout(activePatientBox);
  activeLayout->setContentsMargins(10, 10, 10, 10);
  activeLayout->setSpacing(6);

  QLabel *activeTitle = new QLabel("BỆNH NHÂN HIỆN TẠI", activePatientBox);
  activeTitle->setStyleSheet(
      "font-size: 11px; font-weight: bold; color: #5E6C84;");
  activeLayout->addWidget(activeTitle);

  auto createInfoRow = [activePatientBox](const QString &label,
                                          QLabel *&valLabel) {
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(5);
    QLabel *lbl = new QLabel(label, activePatientBox);
    lbl->setStyleSheet("color: #5E6C84; font-size: 12px; font-weight: 500;");
    valLabel = new QLabel("-", activePatientBox);
    valLabel->setStyleSheet(
        "color: #172B4D; font-size: 12px; font-weight: bold;");
    row->addWidget(lbl);
    row->addWidget(valLabel, 1, Qt::AlignRight);
    return row;
  };

  activeLayout->addLayout(createInfoRow("Mã BN:", m_lblActivePatientCode));
  activeLayout->addLayout(createInfoRow("Họ tên:", m_lblActivePatientName));
  activeLayout->addLayout(createInfoRow("Dịch vụ:", m_lblActivePatientService));
  activeLayout->addLayout(
      createInfoRow("Trạng thái:", m_lblActivePatientStatus));

  actionLayout->addWidget(activePatientBox);

  m_btnCheckIn = new QPushButton("Tiếp Nhận", actionCard);
  m_btnStartProcessing = new QPushButton("Bắt Đầu XN", actionCard);
  m_btnComplete = new QPushButton("Trả Kết Quả", actionCard);
  m_btnCancel = new QPushButton("Hủy Yêu Cầu", actionCard);

  QString actionBtnBase =
      "QPushButton { font-weight: bold; border-radius: 6px; padding: 10px; "
      "border: none; font-size: 14px; } ";
  m_btnCheckIn->setStyleSheet(
      actionBtnBase +
      "QPushButton { background-color: #FFAB00; color: white; } "
      "QPushButton:hover { background-color: #FF8F00; } QPushButton:disabled { "
      "background-color: #DFE1E6; color: #959595; }");
  m_btnStartProcessing->setStyleSheet(
      actionBtnBase +
      "QPushButton { background-color: #0065FF; color: white; } "
      "QPushButton:hover { background-color: #0052CC; } QPushButton:disabled { "
      "background-color: #DFE1E6; color: #959595; }");
  m_btnComplete->setStyleSheet(
      actionBtnBase +
      "QPushButton { background-color: #36B37E; color: white; } "
      "QPushButton:hover { background-color: #2D9D6F; } QPushButton:disabled { "
      "background-color: #DFE1E6; color: #959595; }");
  m_btnCancel->setStyleSheet(
      actionBtnBase +
      "QPushButton { background-color: #FF5630; color: white; } "
      "QPushButton:hover { background-color: #DE350B; } QPushButton:disabled { "
      "background-color: #DFE1E6; color: #959595; }");

  actionLayout->addWidget(m_btnCheckIn);
  actionLayout->addWidget(m_btnStartProcessing);
  actionLayout->addWidget(m_btnComplete);
  actionLayout->addWidget(m_btnCancel);
  actionLayout->addStretch();

  contentLayout->addWidget(actionCard, 1);
  mainLayout->addLayout(contentLayout, 1);

  // Connections
  connect(m_comboStatusFilter,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &NurseDashboardWidget::onStatusFilterChanged);
  connect(m_queueDateEdit, &QDateEdit::dateChanged, this,
          &NurseDashboardWidget::onDateChanged);
  connect(btnRefresh, &QPushButton::clicked, this,
          &NurseDashboardWidget::updateQueueTable);
  connect(m_queueTable, &QTableWidget::cellClicked, this,
          &NurseDashboardWidget::onQueueTableRowSelected);

  connect(m_btnCheckIn, &QPushButton::clicked, this,
          &NurseDashboardWidget::onCheckInClicked);
  connect(m_btnStartProcessing, &QPushButton::clicked, this,
          &NurseDashboardWidget::onStartProcessingClicked);
  connect(m_btnComplete, &QPushButton::clicked, this,
          &NurseDashboardWidget::onCompleteClicked);
  connect(m_btnCancel, &QPushButton::clicked, this,
          &NurseDashboardWidget::onCancelClicked);
}

void NurseDashboardWidget::loadLabRooms() {
  if (m_nurseRoomId > 0) {
    m_lblRoomValue->setText(QString("Phòng %1").arg(m_nurseRoomId));
  } else {
    m_lblRoomValue->setText("Chưa phân công");
  }
}

void NurseDashboardWidget::updateOverviewData() {
  if (m_nurseRoomType == RoomType::Exam) {
    if (!m_appointmentService)
      return;
    QDate today = QDate::currentDate();
    auto allAppts = m_appointmentService->getAppointmentsByDate(today);
    QList<AppointmentRecordDTO> appts;
    int scheduled = 0, checkedIn = 0, completed = 0;
    for (const auto &a : allAppts) {
      if (m_nurseRoomId <= 0 || a.roomId == m_nurseRoomId) {
        appts.append(a);
        if (a.status == "SCHEDULED")
          scheduled++;
        else if (a.status == "CHECKED_IN" || a.status == "IN_PROGRESS")
          checkedIn++;
        else if (a.status == "COMPLETED")
          completed++;
      }
    }
    if (m_lblTotalRequests)
      m_lblTotalRequests->setText(QString::number(appts.size()));
    if (m_lblPendingRequests)
      m_lblPendingRequests->setText(QString::number(scheduled));
    if (m_lblProcessingRequests)
      m_lblProcessingRequests->setText(QString::number(checkedIn));
    if (m_lblCompletedRequests)
      m_lblCompletedRequests->setText(QString::number(completed));

    if (m_overviewTable) {
      m_overviewTable->setRowCount(0);
      int rowCount = qMin(appts.size(), 15);
      for (int i = 0; i < rowCount; ++i) {
        const auto &a = appts[i];
        m_overviewTable->insertRow(i);
        m_overviewTable->setItem(i, 0, new QTableWidgetItem(a.startTime));
        m_overviewTable->setItem(i, 1, new QTableWidgetItem(a.patientCode));
        m_overviewTable->setItem(i, 2, new QTableWidgetItem(a.patientName));
        m_overviewTable->setItem(i, 3, new QTableWidgetItem(a.doctorName));
        m_overviewTable->setItem(i, 4, new QTableWidgetItem(a.roomNumber));
        m_overviewTable->setItem(
            i, 5, new QTableWidgetItem(AppointmentStatusText::toVi(a.status)));
      }
    }
    return;
  }

  if (!m_serviceRequestService)
    return;

  QString today = QDate::currentDate().toString("yyyy-MM-dd");
  int roomId = m_nurseRoomId;

  auto all = m_serviceRequestService->getRequestsByRoom(roomId, today);

  int pending = 0, processing = 0, completed = 0;
  for (const auto &req : all) {
    if (req.status == ServiceRequestStatus::Pending)
      pending++;
    else if (req.status == ServiceRequestStatus::CheckedIn ||
             req.status == ServiceRequestStatus::Processing)
      processing++;
    else if (req.status == ServiceRequestStatus::Completed)
      completed++;
  }

  if (m_lblTotalRequests)
    m_lblTotalRequests->setText(QString::number(all.size()));
  if (m_lblPendingRequests)
    m_lblPendingRequests->setText(QString::number(pending));
  if (m_lblProcessingRequests)
    m_lblProcessingRequests->setText(QString::number(processing));
  if (m_lblCompletedRequests)
    m_lblCompletedRequests->setText(QString::number(completed));

  // Populate Overview Table
  if (m_overviewTable) {
    m_overviewTable->setRowCount(0);
    int rowCount = qMin(all.size(), 15);
    for (int i = 0; i < rowCount; ++i) {
      const auto &req = all[i];
      m_overviewTable->insertRow(i);
      m_overviewTable->setItem(
          i, 0,
          new QTableWidgetItem(req.ticketNumber.has_value()
                                   ? QString::number(req.ticketNumber.value())
                                   : "-"));
      m_overviewTable->setItem(i, 1, new QTableWidgetItem(req.patientCode));
      m_overviewTable->setItem(i, 2, new QTableWidgetItem(req.patientName));
      m_overviewTable->setItem(i, 3, new QTableWidgetItem(req.serviceName));
      m_overviewTable->setItem(i, 4, new QTableWidgetItem(req.roomName));
      m_overviewTable->setItem(
          i, 5, new QTableWidgetItem(serviceRequestStatusToVi(req.status)));
    }
  }
}

void NurseDashboardWidget::updateQueueTable() {
  if (!m_serviceRequestService)
    return;

  int roomId = m_nurseRoomId;
  QString date = m_queueDateEdit
                     ? m_queueDateEdit->date().toString("yyyy-MM-dd")
                     : QDate::currentDate().toString("yyyy-MM-dd");

  std::optional<ServiceRequestStatus> statusOpt = std::nullopt;
  if (m_comboStatusFilter && m_comboStatusFilter->currentData().toInt() != -1) {
    statusOpt = static_cast<ServiceRequestStatus>(
        m_comboStatusFilter->currentData().toInt());
  }

  auto requests =
      m_serviceRequestService->getRequestsByRoom(roomId, date, statusOpt);

  m_queueTable->setRowCount(0);
  m_selectedRequestId = -1;

  for (int i = 0; i < requests.size(); ++i) {
    const auto &req = requests[i];
    m_queueTable->insertRow(i);

    QTableWidgetItem *itemTicket = new QTableWidgetItem(
        req.ticketNumber.has_value() ? QString::number(req.ticketNumber.value())
                                     : "Chưa có");
    QTableWidgetItem *itemCode = new QTableWidgetItem(req.patientCode);
    QTableWidgetItem *itemName = new QTableWidgetItem(req.patientName);
    QTableWidgetItem *itemService = new QTableWidgetItem(req.serviceName);
    QTableWidgetItem *itemDoctor = new QTableWidgetItem(req.doctorName);
    QTableWidgetItem *itemStatus =
        new QTableWidgetItem(serviceRequestStatusToVi(req.status));
    QTableWidgetItem *itemTime =
        new QTableWidgetItem(req.prescribedAt.toString("hh:mm dd/MM"));

    // Store request_id in item data
    itemTicket->setData(Qt::UserRole, req.requestId);

    m_queueTable->setItem(i, 0, itemTicket);
    m_queueTable->setItem(i, 1, itemCode);
    m_queueTable->setItem(i, 2, itemName);
    m_queueTable->setItem(i, 3, itemService);
    m_queueTable->setItem(i, 4, itemDoctor);
    m_queueTable->setItem(i, 5, itemStatus);
    m_queueTable->setItem(i, 6, itemTime);
  }

  // UPDATE ACTIVE PATIENT SECTION
  auto allOnDateRequests =
      m_serviceRequestService->getRequestsByRoom(roomId, date);

  std::optional<ServiceRequestDTO> activeReq = std::nullopt;
  for (const auto &req : allOnDateRequests) {
    if (req.status == ServiceRequestStatus::CheckedIn ||
        req.status == ServiceRequestStatus::Processing) {
      activeReq = req;
      break;
    }
  }

  if (activeReq.has_value()) {
    m_activeRequestId = activeReq->requestId;
    if (m_lblActivePatientCode)
      m_lblActivePatientCode->setText(activeReq->patientCode);
    if (m_lblActivePatientName)
      m_lblActivePatientName->setText(activeReq->patientName);
    if (m_lblActivePatientService)
      m_lblActivePatientService->setText(activeReq->serviceName);
    if (m_lblActivePatientStatus)
      m_lblActivePatientStatus->setText(
          serviceRequestStatusToVi(activeReq->status));

    // Enable buttons based on status
    if (m_btnCheckIn)
      m_btnCheckIn->setEnabled(false);

    if (activeReq->status == ServiceRequestStatus::CheckedIn) {
      if (m_btnStartProcessing)
        m_btnStartProcessing->setEnabled(true);
      if (m_btnComplete)
        m_btnComplete->setEnabled(false);
    } else if (activeReq->status == ServiceRequestStatus::Processing) {
      if (m_btnStartProcessing)
        m_btnStartProcessing->setEnabled(false);
      if (m_btnComplete)
        m_btnComplete->setEnabled(true);
    }
    if (m_btnCancel)
      m_btnCancel->setEnabled(true);
  } else {
    m_activeRequestId = -1;
    if (m_lblActivePatientCode)
      m_lblActivePatientCode->setText("-");
    if (m_lblActivePatientName)
      m_lblActivePatientName->setText("Chưa có");
    if (m_lblActivePatientService)
      m_lblActivePatientService->setText("-");
    if (m_lblActivePatientStatus)
      m_lblActivePatientStatus->setText("Trống");

    if (m_btnCheckIn)
      m_btnCheckIn->setEnabled(false);
    if (m_btnStartProcessing)
      m_btnStartProcessing->setEnabled(false);
    if (m_btnComplete)
      m_btnComplete->setEnabled(false);
    if (m_btnCancel)
      m_btnCancel->setEnabled(false);
  }
}

void NurseDashboardWidget::onStatusFilterChanged(int index) {
  Q_UNUSED(index);
  updateQueueTable();
}

void NurseDashboardWidget::onDateChanged(const QDate &date) {
  Q_UNUSED(date);
  updateQueueTable();
}

void NurseDashboardWidget::onQueueTableRowSelected(int row, int col) {
  Q_UNUSED(col);
  if (row < 0)
    return;
  QTableWidgetItem *item = m_queueTable->item(row, 0);
  if (item) {
    m_selectedRequestId = item->data(Qt::UserRole).toInt();

    // If there is no active patient, enable appropriate buttons based on
    // selection
    if (m_activeRequestId <= 0) {
      if (m_btnCheckIn)
        m_btnCheckIn->setEnabled(true);
      if (m_btnCancel)
        m_btnCancel->setEnabled(true);
    }
  }
}

void NurseDashboardWidget::onCheckInClicked() {
  if (m_activeRequestId > 0) {
    QMessageBox::warning(this, "Thông báo",
                         "Vui lòng hoàn thành hoặc hủy yêu cầu hiện tại trước "
                         "khi tiếp nhận bệnh nhân mới.");
    return;
  }
  if (m_selectedRequestId <= 0) {
    QMessageBox::warning(
        this, "Thông báo",
        "Vui lòng chọn một yêu cầu trong danh sách để tiếp nhận.");
    return;
  }

  int ticketNum = 0;
  if (m_serviceRequestService->checkIn(m_selectedRequestId, ticketNum)) {
    QMessageBox::information(
        this, "Thành công",
        QString("Đã tiếp nhận bệnh nhân! Số thứ tự STT: %1").arg(ticketNum));
    updateQueueTable();
  } else {
    QMessageBox::warning(
        this, "Lỗi",
        "Không thể tiếp nhận yêu cầu này (có thể đã tiếp nhận hoặc đã hủy).");
  }
}

void NurseDashboardWidget::onStartProcessingClicked() {
  int reqId = (m_activeRequestId > 0) ? m_activeRequestId : m_selectedRequestId;
  if (reqId <= 0) {
    QMessageBox::warning(this, "Thông báo",
                         "Không có bệnh nhân nào đang xử lý.");
    return;
  }

  if (m_serviceRequestService->startProcessing(reqId)) {
    QMessageBox::information(
        this, "Thành công",
        "Đã chuyển trạng thái sang Đang Thực Hiện Xét Nghiệm.");
    updateQueueTable();
  } else {
    QMessageBox::warning(
        this, "Lỗi",
        "Không thể bắt đầu (yêu cầu phải ở trạng thái Đã Tiếp Nhận).");
  }
}

void NurseDashboardWidget::onCompleteClicked() {
  int reqId = (m_activeRequestId > 0) ? m_activeRequestId : m_selectedRequestId;
  if (reqId <= 0) {
    QMessageBox::warning(this, "Thông báo",
                         "Không có bệnh nhân nào đang xử lý.");
    return;
  }

  bool ok = false;
  QString resultNote =
      QInputDialog::getMultiLineText(this, "Nhập Kết Quả Xét Nghiệm",
                                     "Kết quả xét nghiệm / Ghi chú:", "", &ok);

  if (!ok)
    return; // User cancelled input

  if (m_serviceRequestService->completeProcessing(reqId, resultNote)) {
    QMessageBox::information(this, "Thành công",
                             "Đã hoàn thành và trả kết quả xét nghiệm!");
    updateQueueTable();
  } else {
    QMessageBox::warning(
        this, "Lỗi",
        "Không thể hoàn thành (yêu cầu phải ở trạng thái Đang Thực Hiện).");
  }
}

void NurseDashboardWidget::onCancelClicked() {
  int reqId = (m_activeRequestId > 0) ? m_activeRequestId : m_selectedRequestId;
  if (reqId <= 0) {
    QMessageBox::warning(this, "Thông báo",
                         "Vui lòng chọn một yêu cầu trong danh sách hoặc có "
                         "bệnh nhân đang xử lý để hủy.");
    return;
  }

  auto reply = QMessageBox::question(
      this, "Xác nhận", "Bạn có chắc chắn muốn hủy yêu cầu xét nghiệm này?",
      QMessageBox::Yes | QMessageBox::No);
  if (reply != QMessageBox::Yes)
    return;

  if (m_serviceRequestService->cancelRequest(reqId)) {
    QMessageBox::information(this, "Thành công", "Đã hủy yêu cầu xét nghiệm.");
    updateQueueTable();
  } else {
    QMessageBox::warning(this, "Lỗi", "Không thể hủy yêu cầu này.");
  }
}

void NurseDashboardWidget::switchPage(int index, QPushButton *activeBtn) {
  if (!m_stackedWidget)
    return;
  m_stackedWidget->setCurrentIndex(index);

  QPushButton *buttons[] = {m_btnOverview, m_btnAppointments, m_btnLabQueue, m_btnLeaveManage};
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
    updateOverviewData();
  } else if (m_nurseRoomType == RoomType::Exam) {
    if (index == 1)
      updateAppointmentsTable();
  } else if (m_nurseRoomType == RoomType::Lab) {
    if (index == 1)
      updateQueueTable();
  } else {
    if (index == 1)
      updateAppointmentsTable();
    else if (index == 2)
      updateQueueTable();
  }
}

void NurseDashboardWidget::buildLeaveManagePage() {
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
          &NurseDashboardWidget::onSubmitLeaveRequest);

  connect(m_leaveTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
    if (index == 1) {
      loadLeaveHistory();
    }
  });

  m_stackedWidget->addWidget(m_leaveManagePage);
}

void NurseDashboardWidget::onLeaveTabSelected() {
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

void NurseDashboardWidget::loadLeaveHistory() {
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

void NurseDashboardWidget::onSubmitLeaveRequest() {
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
