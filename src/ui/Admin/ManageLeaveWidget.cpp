#include "ManageLeaveWidget.h"
#include "../../dto/StaffDTOs.h"
#include <QCalendarWidget>
#include <QCompleter>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>


ManageLeaveWidget::ManageLeaveWidget(
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<AppointmentService> appointmentService, QWidget *parent)
    : QWidget(parent), m_staffService(staffService),
      m_appointmentService(appointmentService) {
  setupUi();
}

QFrame *ManageLeaveWidget::makeCard(QWidget *parent) {
  QFrame *card = new QFrame(parent);
  card->setObjectName("CardFrame");
  card->setStyleSheet(
      "#CardFrame { background-color: #FFFFFF; border-radius: 12px; }");
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
  shadow->setBlurRadius(15);
  shadow->setColor(QColor(0, 0, 0, 15));
  shadow->setOffset(0, 5);
  card->setGraphicsEffect(shadow);
  return card;
}

void ManageLeaveWidget::setupUi() {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(40, 40, 40, 40);
  layout->setSpacing(20);

  QLabel *lblTitle = new QLabel("Quản Lý Nghỉ Phép", this);
  lblTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #202124;");
  layout->addWidget(lblTitle);

  QTabWidget *tabWidget = new QTabWidget(this);
  tabWidget->setStyleSheet(
      "QTabWidget::pane { border: 1px solid #E2E8F0; border-radius: 8px; "
      "background: white; }"
      "QTabBar::tab { padding: 10px 20px; font-weight: bold; color: #475569; "
      "background: #F8FAFC; border: 1px solid #E2E8F0; border-bottom: none; "
      "border-top-left-radius: 8px; border-top-right-radius: 8px; "
      "margin-right: 2px; }"
      "QTabBar::tab:selected { color: #2563EB; background: white; "
      "border-bottom: 2px solid #2563EB; }");

  // Tab 1: Duyệt đơn
  QWidget *tabApprove = new QWidget();
  QVBoxLayout *layApprove = new QVBoxLayout(tabApprove);
  layApprove->setContentsMargins(20, 20, 20, 20);

  m_tablePendingLeaves = new QTableWidget(tabApprove);
  m_tablePendingLeaves->setColumnCount(7);
  m_tablePendingLeaves->setHorizontalHeaderLabels({"Mã NV", "Họ tên", "Từ ngày",
                                                   "Đến ngày", "Lý do",
                                                   "Trạng thái", "Thao tác"});

  QHeaderView *header = m_tablePendingLeaves->horizontalHeader();
  header->setStretchLastSection(false);
  header->setSectionResizeMode(0, QHeaderView::Fixed);
  m_tablePendingLeaves->setColumnWidth(0, 120); // Mã NV
  header->setSectionResizeMode(1, QHeaderView::Fixed);
  m_tablePendingLeaves->setColumnWidth(1, 180); // Họ tên
  header->setSectionResizeMode(2, QHeaderView::Fixed);
  m_tablePendingLeaves->setColumnWidth(2, 120); // Từ ngày
  header->setSectionResizeMode(3, QHeaderView::Fixed);
  m_tablePendingLeaves->setColumnWidth(3, 120);          // Đến ngày
  header->setSectionResizeMode(4, QHeaderView::Stretch); // Lý do
  header->setSectionResizeMode(5, QHeaderView::Fixed);
  m_tablePendingLeaves->setColumnWidth(5, 130); // Trạng thái
  header->setSectionResizeMode(6, QHeaderView::Fixed);
  m_tablePendingLeaves->setColumnWidth(6, 180); // Thao tác
  m_tablePendingLeaves->verticalHeader()->setDefaultSectionSize(46);
  m_tablePendingLeaves->verticalHeader()->setVisible(false);

  m_tablePendingLeaves->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tablePendingLeaves->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tablePendingLeaves->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tablePendingLeaves->setFocusPolicy(Qt::NoFocus);
  m_tablePendingLeaves->setStyleSheet(
      "QTableWidget { border: 1px solid #E2E8F0; border-radius: 4px; color: "
      "#0F172A; background-color: white; outline: none; }"
      "QTableWidget::item { outline: none; border: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }"
      "QHeaderView::section { background-color: #F8FAFC; color: #1E293B; "
      "font-weight: bold; padding: 8px; border: 1px solid #E2E8F0; }");
  layApprove->addWidget(m_tablePendingLeaves);

  tabWidget->addTab(tabApprove, "Danh sách Chờ duyệt");

  // Tab 2: Đăng ký nghỉ
  QWidget *tabRegister = new QWidget();
  QVBoxLayout *layRegister = new QVBoxLayout(tabRegister);
  layRegister->setContentsMargins(20, 20, 20, 20);

  QFrame *formFrame = makeCard(tabRegister);
  QFormLayout *formLayout = new QFormLayout(formFrame);
  formLayout->setContentsMargins(20, 20, 20, 20);
  formLayout->setSpacing(15);

  m_comboLeaveStaff = new QComboBox(formFrame);
  m_comboLeaveStaff->setEditable(true);
  m_comboLeaveStaff->setInsertPolicy(QComboBox::NoInsert);
  if (m_comboLeaveStaff->completer()) {
    m_comboLeaveStaff->completer()->setFilterMode(Qt::MatchContains);
    m_comboLeaveStaff->completer()->setCaseSensitivity(Qt::CaseInsensitive);
  }
  if (m_comboLeaveStaff->lineEdit()) {
    m_comboLeaveStaff->lineEdit()->setPlaceholderText(
        "Nhập Mã NV hoặc Họ tên để tìm kiếm...");
  }
  m_comboLeaveStaff->setStyleSheet(
      "QComboBox { background: transparent; border: 1px solid #CBD5E1; "
      "border-radius: 6px; padding: 8px 12px; font-size: 13px; color: #0F172A; "
      "}"
      "QComboBox QAbstractItemView { background-color: #FFFFFF; color: "
      "#0F172A; selection-background-color: #2563EB; selection-color: white; }"
      "QLineEdit { background: transparent; color: #0F172A; border: none; "
      "font-size: 13px; }");

  StaffSearchCriteria criteria;
  criteria.onlyActive = true;
  criteria.pageSize = 100;
  auto staffList = m_staffService->searchStaffPaged(criteria).items;

  m_comboLeaveStaff->addItem("--- Tìm kiếm / Chọn nhân viên ---", -1);
  for (const auto &staff : staffList) {
    m_comboLeaveStaff->addItem(QString("%1 - %2 (%3)")
                                   .arg(staff->getStaffCode(),
                                        staff->getFullName(),
                                        userRoleToVi(staff->getRole())),
                               staff->getAccountId());
  }

  m_lblLeaveBalance =
      new QLabel("Vui lòng nhập hoặc chọn nhân viên.", formFrame);
  m_lblLeaveBalance->setStyleSheet(
      "color: #5F6368; font-style: italic; background: transparent;");

  auto createCustomCalendar = []() {
    QCalendarWidget *cal = new QCalendarWidget();
    cal->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
    cal->setMinimumSize(340, 260);
    cal->setStyleSheet(
        "QCalendarWidget { background-color: #FFFFFF; color: #1E293B; }"
        "QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: "
        "#F8FAFC; border-bottom: 1px solid #E2E8F0; min-height: 38px; }"
        "QCalendarWidget QToolButton { color: #1E293B; font-weight: bold; "
        "background-color: transparent; border: none; padding: 4px 8px; "
        "margin: 2px; font-size: 13px; border-radius: 4px; }"
        "QCalendarWidget QToolButton:hover { background-color: #EFF6FF; color: "
        "#2563EB; }"
        "QCalendarWidget QMenu { background-color: #FFFFFF; color: #1E293B; "
        "border: 1px solid #E2E8F0; }"
        "QCalendarWidget QSpinBox { background-color: #FFFFFF; color: #1E293B; "
        "selection-background-color: #2563EB; selection-color: white; "
        "font-size: 13px; }"
        "QCalendarWidget QAbstractItemView:enabled { font-size: 13px; color: "
        "#1E293B; background-color: #FFFFFF; selection-background-color: "
        "#2563EB; selection-color: #FFFFFF; outline: none; }"
        "QCalendarWidget QAbstractItemView:disabled { color: #94A3B8; }");
    return cal;
  };

  QString transparentInputStyle =
      "QDateEdit, QTextEdit { background: transparent; border: 1px solid "
      "#CBD5E1; border-radius: 6px; padding: 8px 12px; font-size: 13px; color: "
      "#0F172A; }"
      "QDateEdit:focus, QTextEdit:focus { border: 1px solid #2563EB; }";

  m_leaveStartDate = new QDateEdit(QDate::currentDate(), formFrame);
  m_leaveStartDate->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
  m_leaveStartDate->setCalendarWidget(createCustomCalendar());
  m_leaveStartDate->setCalendarPopup(true);
  m_leaveStartDate->setMinimumDate(QDate::currentDate());
  m_leaveStartDate->setDisplayFormat("dd/MM/yyyy");
  m_leaveStartDate->setStyleSheet(transparentInputStyle);

  m_leaveEndDate = new QDateEdit(QDate::currentDate(), formFrame);
  m_leaveEndDate->setLocale(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
  m_leaveEndDate->setCalendarWidget(createCustomCalendar());
  m_leaveEndDate->setCalendarPopup(true);
  m_leaveEndDate->setMinimumDate(QDate::currentDate());
  m_leaveEndDate->setDisplayFormat("dd/MM/yyyy");
  m_leaveEndDate->setStyleSheet(transparentInputStyle);

  m_txtLeaveReason = new QTextEdit(formFrame);
  m_txtLeaveReason->setFixedHeight(80);
  m_txtLeaveReason->setStyleSheet(transparentInputStyle);

  formLayout->addRow("Tìm nhân viên:", m_comboLeaveStaff);
  formLayout->addRow("Quỹ phép:", m_lblLeaveBalance);
  formLayout->addRow("Từ ngày:", m_leaveStartDate);
  formLayout->addRow("Đến ngày:", m_leaveEndDate);
  formLayout->addRow("Lý do:", m_txtLeaveReason);

  QPushButton *btnSubmit = new QPushButton("Đăng ký Nghỉ phép", formFrame);
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

  tabWidget->addTab(tabRegister, "Đăng ký Nghỉ phép (Hộ)");

  layout->addWidget(tabWidget);

  connect(m_comboLeaveStaff,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ManageLeaveWidget::onLeaveStaffSelected);
  if (m_comboLeaveStaff->lineEdit()) {
    connect(m_comboLeaveStaff->lineEdit(), &QLineEdit::textChanged, this,
            [this](const QString &) {
              onLeaveStaffSelected(m_comboLeaveStaff->currentIndex());
            });
  }
  connect(btnSubmit, &QPushButton::clicked, this,
          &ManageLeaveWidget::onSubmitLeaveRequest);
  connect(tabWidget, &QTabWidget::currentChanged, this, [=](int index) {
    if (index == 0) {
      loadPendingLeaves();
    }
  });

  loadPendingLeaves();
}

void ManageLeaveWidget::loadPendingLeaves() {
  m_tablePendingLeaves->setRowCount(0);
  auto pendingList = m_staffService->getPendingLeaveRequests();

  for (const auto &req : pendingList) {
    int row = m_tablePendingLeaves->rowCount();
    m_tablePendingLeaves->insertRow(row);

    QTableWidgetItem *itemStaffCode = new QTableWidgetItem(req.staffCode);
    itemStaffCode->setData(Qt::UserRole, req.requestId); // Store ID here

    m_tablePendingLeaves->setItem(row, 0, itemStaffCode);
    m_tablePendingLeaves->setItem(row, 1, new QTableWidgetItem(req.fullName));
    m_tablePendingLeaves->setItem(
        row, 2, new QTableWidgetItem(req.startDate.toString("dd/MM/yyyy")));
    m_tablePendingLeaves->setItem(
        row, 3, new QTableWidgetItem(req.endDate.toString("dd/MM/yyyy")));
    m_tablePendingLeaves->setItem(row, 4, new QTableWidgetItem(req.reason));
    m_tablePendingLeaves->setItem(row, 5, new QTableWidgetItem(req.status));

    // Add action buttons
    QWidget *actionWidget = new QWidget();
    actionWidget->setStyleSheet("background: transparent;");
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(4, 4, 4, 4);
    actionLayout->setSpacing(8);

    QPushButton *btnApprove = new QPushButton("Duyệt", actionWidget);
    btnApprove->setCursor(Qt::PointingHandCursor);
    btnApprove->setStyleSheet(
        "background-color: #34A853; color: white; border-radius: 4px; "
        "font-weight: bold; padding: 4px 8px;");

    QPushButton *btnReject = new QPushButton("Từ chối", actionWidget);
    btnReject->setCursor(Qt::PointingHandCursor);
    btnReject->setStyleSheet(
        "background-color: #EA4335; color: white; border-radius: 4px; "
        "font-weight: bold; padding: 4px 8px;");

    actionLayout->addWidget(btnApprove);
    actionLayout->addWidget(btnReject);

    m_tablePendingLeaves->setCellWidget(row, 6, actionWidget);

    int reqId = req.requestId;

    connect(btnApprove, &QPushButton::clicked, this, [this, reqId]() {
      QString err = m_staffService->processLeaveRequest(reqId, true,
                                                        m_appointmentService);
      if (!err.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", err);
      } else {
        QMessageBox::information(this, "Thành công",
                                 "Đã duyệt đơn nghỉ phép thành công.");
        loadPendingLeaves();
      }
    });

    connect(btnReject, &QPushButton::clicked, this, [this, reqId]() {
      QString err = m_staffService->processLeaveRequest(reqId, false,
                                                        m_appointmentService);
      if (!err.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", err);
      } else {
        QMessageBox::information(this, "Thành công",
                                 "Đã từ chối đơn nghỉ phép.");
        loadPendingLeaves();
      }
    });
  }
}

void ManageLeaveWidget::onLeaveStaffSelected(int /*index*/) {
  int staffId = m_comboLeaveStaff->currentData().toInt();
  if (staffId <= 0) {
    QString currentText = m_comboLeaveStaff->currentText().trimmed();
    if (!currentText.isEmpty()) {
      for (int i = 0; i < m_comboLeaveStaff->count(); ++i) {
        if (m_comboLeaveStaff->itemData(i).toInt() > 0 &&
            m_comboLeaveStaff->itemText(i).contains(currentText,
                                                    Qt::CaseInsensitive)) {
          staffId = m_comboLeaveStaff->itemData(i).toInt();
          break;
        }
      }
    }
  }

  if (staffId <= 0) {
    m_lblLeaveBalance->setText("Vui lòng nhập hoặc chọn nhân viên hợp lệ.");
    return;
  }

  LeaveBalanceDTO balance =
      m_staffService->getLeaveBalance(staffId, QDate::currentDate().year());
  m_lblLeaveBalance->setText(QString("Đã dùng: %1 / %2 ngày (Còn lại: %3 ngày)")
                                 .arg(balance.usedDays)
                                 .arg(balance.totalDays)
                                 .arg(balance.totalDays - balance.usedDays));
}

void ManageLeaveWidget::onSubmitLeaveRequest() {
  int staffId = m_comboLeaveStaff->currentData().toInt();
  if (staffId <= 0) {
    QString currentText = m_comboLeaveStaff->currentText().trimmed();
    if (!currentText.isEmpty()) {
      for (int i = 0; i < m_comboLeaveStaff->count(); ++i) {
        if (m_comboLeaveStaff->itemData(i).toInt() > 0 &&
            m_comboLeaveStaff->itemText(i).contains(currentText,
                                                    Qt::CaseInsensitive)) {
          staffId = m_comboLeaveStaff->itemData(i).toInt();
          break;
        }
      }
    }
  }

  if (staffId <= 0) {
    QMessageBox::warning(this, "Lỗi",
                         "Vui lòng nhập hoặc chọn nhân viên hợp lệ.");
    return;
  }

  QDate start = m_leaveStartDate->date();
  QDate end = m_leaveEndDate->date();
  QString reason = m_txtLeaveReason->toPlainText().trimmed();

  QString err = m_staffService->registerLeave(staffId, start, end, reason);
  if (!err.isEmpty()) {
    QMessageBox::warning(this, "Lỗi", err);
    return;
  }

  QString successMsg = "Đăng ký nghỉ phép thành công! Đơn sẽ được chuyển sang "
                       "trạng thái Chờ duyệt.";
  QMessageBox::information(this, "Thành công", successMsg);

  // Reset form
  m_txtLeaveReason->clear();
  m_leaveStartDate->setDate(QDate::currentDate());
  m_leaveEndDate->setDate(QDate::currentDate());
  onLeaveStaffSelected(m_comboLeaveStaff->currentIndex());
}
