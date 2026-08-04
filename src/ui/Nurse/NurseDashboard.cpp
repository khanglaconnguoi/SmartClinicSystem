#include "NurseDashboard.h"
#include "ui/view/Profile.h"
#include "model/SystemUser.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QGraphicsDropShadowEffect>
#include <QDate>
#include <QDebug>

NurseDashboardWidget::NurseDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<AppointmentService> appointmentService,
    std::shared_ptr<ServiceRequestService> serviceRequestService,
    QWidget *parent)
    : BaseDashboardWidget(user, staffService, patientService, appointmentService, parent),
      m_serviceRequestService(serviceRequestService) {
    
    // Read Nurse Room ID from user profile if available
    if (m_baseStaffService && m_currentUser) {
        auto profile = m_baseStaffService->getOwnProfile(m_currentUser->getAccountId());
        if (auto nurseProfile = dynamic_cast<NurseProfileDTO*>(profile.get())) {
            m_nurseRoomId = nurseProfile->roomId;
        }
    }

    initializeDashboard();
}

QFrame *NurseDashboardWidget::makeCard(QWidget *parent) {
    QFrame *card = new QFrame(parent ? parent : this);
    card->setStyleSheet("QFrame { background-color: #FFFFFF; border-radius: 8px; border: 1px solid #DFE1E6; }");
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
    buildLabQueuePage();

    m_stackedWidget->addWidget(m_overviewPage);
    m_stackedWidget->addWidget(m_labQueuePage);

    m_mainContentLayout->addWidget(m_stackedWidget);

    loadLabRooms();
    switchPage(0, m_btnOverview);
    updateQueueTable();
}

void NurseDashboardWidget::buildSidebar() {
    m_btnOverview = new QPushButton("Tổng quan", m_sidebarFrame);
    m_btnLabQueue = new QPushButton("Phòng xét nghiệm", m_sidebarFrame);
    QPushButton *btnLogout = new QPushButton("Đăng xuất", m_sidebarFrame);

    m_btnOverview->setObjectName("activeBtn"); // Default active

    btnLogout->setCursor(Qt::PointingHandCursor);
    btnLogout->setStyleSheet(
        "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; font-weight: 600; "
        "border: none; border-radius: 6px; color: #DC2626; background-color: transparent; } "
        "QPushButton:hover { background-color: #FEE2E2; color: #991B1B; }"
    );

    m_sidebarLayout->addWidget(m_btnOverview);
    m_sidebarLayout->addWidget(m_btnLabQueue);
    m_sidebarLayout->addStretch();
    m_sidebarLayout->addWidget(btnLogout);

    connect(m_btnOverview, &QPushButton::clicked, this, [this]() {
        switchPage(0, m_btnOverview);
    });

    connect(m_btnLabQueue, &QPushButton::clicked, this, [this]() {
        switchPage(1, m_btnLabQueue);
    });

    connect(btnLogout, &QPushButton::clicked, this, &NurseDashboardWidget::logoutRequested);
}

void NurseDashboardWidget::buildOverviewPage() {
    m_overviewPage = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_overviewPage);
    mainLayout->setContentsMargins(20, 20, 20, 28);
    mainLayout->setSpacing(20);

    // Title
    QLabel *title = new QLabel("Tổng Quan Hoạt Động Y Tá", m_overviewPage);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #172B4D;");
    mainLayout->addWidget(title);

    // Stat Cards Grid
    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(15);

    auto createStatCard = [this](const QString &titleText, QLabel *&valueLabel, const QString &colorHex) -> QFrame* {
        QFrame *card = makeCard();
        QVBoxLayout *l = new QVBoxLayout(card);
        l->setContentsMargins(20, 15, 20, 15);
        QLabel *lblTitle = new QLabel(titleText, card);
        lblTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #5E6C84; border: none;");
        valueLabel = new QLabel("0", card);
        valueLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1; border: none;").arg(colorHex));
        l->addWidget(lblTitle);
        l->addWidget(valueLabel);
        return card;
    };

    cardsLayout->addWidget(createStatCard("Tổng yêu cầu hôm nay", m_lblTotalRequests, "#0052CC"));
    cardsLayout->addWidget(createStatCard("Chờ tiếp nhận", m_lblPendingRequests, "#FFAB00"));
    cardsLayout->addWidget(createStatCard("Đang xét nghiệm", m_lblProcessingRequests, "#0065FF"));
    cardsLayout->addWidget(createStatCard("Đã hoàn thành", m_lblCompletedRequests, "#36B37E"));

    mainLayout->addLayout(cardsLayout);

    // Recent Requests Table Card
    QFrame *tableCard = makeCard();
    QVBoxLayout *tableCardLayout = new QVBoxLayout(tableCard);
    tableCardLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *tableTitle = new QLabel("Danh Sách Yêu Cầu Gần Đây", tableCard);
    tableTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #172B4D; border: none;");
    tableCardLayout->addWidget(tableTitle);

    m_overviewTable = new QTableWidget(tableCard);
    m_overviewTable->setColumnCount(6);
    m_overviewTable->setHorizontalHeaderLabels({"STT", "Mã BN", "Bệnh nhân", "Dịch vụ", "Phòng", "Trạng thái"});
    m_overviewTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_overviewTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_overviewTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_overviewTable->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: #F4F5F7; color: #172B4D; } "
        "QHeaderView::section { background-color: #F4F5F7; color: #5E6C84; font-weight: bold; border: none; padding: 8px; } "
        "QTableWidget::item { color: #172B4D; background-color: #FFFFFF; padding: 6px; } "
        "QTableWidget::item:selected { background-color: #DEEBFF; color: #0052CC; }"
    );

    tableCardLayout->addWidget(m_overviewTable);
    mainLayout->addWidget(tableCard, 1);
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
        "QComboBox, QDateEdit { background-color: #FFFFFF; color: #172B4D; border: 1px solid #DFE1E6; border-radius: 6px; padding: 6px 12px; font-size: 13px; min-height: 28px; } "
        "QComboBox QAbstractItemView { background-color: #FFFFFF; color: #172B4D; selection-background-color: #DEEBFF; selection-color: #0052CC; }";

    QLabel *lblRoom = new QLabel("Phòng xét nghiệm:", filterCard);
    lblRoom->setStyleSheet("font-weight: 600; color: #172B4D; border: none;");
    m_comboRooms = new QComboBox(filterCard);
    m_comboRooms->setMinimumWidth(180);
    m_comboRooms->setStyleSheet(filterInputStyle);

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

    QPushButton *btnRefresh = new QPushButton(" Tải lại", filterCard);
    btnRefresh->setStyleSheet("QPushButton { background-color: #DEEBFF; color: #0052CC; font-weight: bold; border: none; border-radius: 4px; padding: 6px 14px; } QPushButton:hover { background-color: #B3D4FF; }");

    filterLayout->addWidget(lblRoom);
    filterLayout->addWidget(m_comboRooms);
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
    m_queueTable->setHorizontalHeaderLabels({"STT Ticket", "Mã BN", "Họ tên bệnh nhân", "Dịch vụ", "Bác sĩ chỉ định", "Trạng thái", "Thời gian"});
    m_queueTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_queueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_queueTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_queueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_queueTable->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: #F4F5F7; color: #172B4D; } "
        "QHeaderView::section { background-color: #F4F5F7; color: #5E6C84; font-weight: bold; border: none; padding: 8px; } "
        "QTableWidget::item { color: #172B4D; background-color: #FFFFFF; padding: 6px; } "
        "QTableWidget::item:selected { background-color: #DEEBFF; color: #0052CC; }"
    );

    tableLayout->addWidget(m_queueTable);
    contentLayout->addWidget(tableCard, 3);

    // Right Action Panel Card
    QFrame *actionCard = makeCard();
    actionCard->setFixedWidth(240);
    QVBoxLayout *actionLayout = new QVBoxLayout(actionCard);
    actionLayout->setContentsMargins(15, 20, 15, 20);
    actionLayout->setSpacing(12);

    QLabel *actionTitle = new QLabel("Thao Tác Hàng Đợi", actionCard);
    actionTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #172B4D; border: none;");
    actionLayout->addWidget(actionTitle);

    m_btnCheckIn = new QPushButton("1. Tiếp Nhận", actionCard);
    m_btnStartProcessing = new QPushButton("2. Bắt Đầu XN", actionCard);
    m_btnComplete = new QPushButton("3. Trả Kết Quả", actionCard);
    m_btnCancel = new QPushButton("Hủy Yêu Cầu", actionCard);

    QString actionBtnBase = "QPushButton { font-weight: bold; border-radius: 6px; padding: 10px; border: none; font-size: 14px; } ";
    m_btnCheckIn->setStyleSheet(actionBtnBase + "QPushButton { background-color: #FFAB00; color: white; } QPushButton:hover { background-color: #FF8F00; }");
    m_btnStartProcessing->setStyleSheet(actionBtnBase + "QPushButton { background-color: #0065FF; color: white; } QPushButton:hover { background-color: #0052CC; }");
    m_btnComplete->setStyleSheet(actionBtnBase + "QPushButton { background-color: #36B37E; color: white; } QPushButton:hover { background-color: #2D9D6F; }");
    m_btnCancel->setStyleSheet(actionBtnBase + "QPushButton { background-color: #FF5630; color: white; } QPushButton:hover { background-color: #DE350B; }");

    actionLayout->addWidget(m_btnCheckIn);
    actionLayout->addWidget(m_btnStartProcessing);
    actionLayout->addWidget(m_btnComplete);
    actionLayout->addWidget(m_btnCancel);
    actionLayout->addStretch();

    contentLayout->addWidget(actionCard, 1);
    mainLayout->addLayout(contentLayout, 1);

    // Connections
    connect(m_comboRooms, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NurseDashboardWidget::onRoomChanged);
    connect(m_comboStatusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NurseDashboardWidget::onStatusFilterChanged);
    connect(m_queueDateEdit, &QDateEdit::dateChanged, this, &NurseDashboardWidget::onDateChanged);
    connect(btnRefresh, &QPushButton::clicked, this, &NurseDashboardWidget::updateQueueTable);
    connect(m_queueTable, &QTableWidget::cellClicked, this, &NurseDashboardWidget::onQueueTableRowSelected);

    connect(m_btnCheckIn, &QPushButton::clicked, this, &NurseDashboardWidget::onCheckInClicked);
    connect(m_btnStartProcessing, &QPushButton::clicked, this, &NurseDashboardWidget::onStartProcessingClicked);
    connect(m_btnComplete, &QPushButton::clicked, this, &NurseDashboardWidget::onCompleteClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &NurseDashboardWidget::onCancelClicked);
}

void NurseDashboardWidget::loadLabRooms() {
    if (!m_baseAppointmentService) return;
    m_comboRooms->clear();
    m_comboRooms->addItem("--- Tất cả phòng ---", -1);

    auto rooms = m_baseAppointmentService->getRoomsByType("LAB");
    int selectIdx = 0;

    for (int i = 0; i < rooms.size(); ++i) {
        m_comboRooms->addItem(rooms[i].second, rooms[i].first);
        if (rooms[i].first == m_nurseRoomId) {
            selectIdx = i + 1; // +1 because item 0 is "--- Tất cả phòng ---"
        }
    }

    if (selectIdx > 0) {
        m_comboRooms->setCurrentIndex(selectIdx);
    }
}

void NurseDashboardWidget::updateOverviewData() {
    if (!m_serviceRequestService) return;

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    int roomId = m_comboRooms ? m_comboRooms->currentData().toInt() : -1;

    auto all = m_serviceRequestService->getRequestsByRoom(roomId, today);

    int pending = 0, processing = 0, completed = 0;
    for (const auto &req : all) {
        if (req.status == ServiceRequestStatus::Pending) pending++;
        else if (req.status == ServiceRequestStatus::CheckedIn || req.status == ServiceRequestStatus::Processing) processing++;
        else if (req.status == ServiceRequestStatus::Completed) completed++;
    }

    m_lblTotalRequests->setText(QString::number(all.size()));
    m_lblPendingRequests->setText(QString::number(pending));
    m_lblProcessingRequests->setText(QString::number(processing));
    m_lblCompletedRequests->setText(QString::number(completed));

    // Populate Overview Table
    m_overviewTable->setRowCount(0);
    int rowCount = qMin(all.size(), 15);
    for (int i = 0; i < rowCount; ++i) {
        const auto &req = all[i];
        m_overviewTable->insertRow(i);
        m_overviewTable->setItem(i, 0, new QTableWidgetItem(req.ticketNumber.has_value() ? QString::number(req.ticketNumber.value()) : "-"));
        m_overviewTable->setItem(i, 1, new QTableWidgetItem(req.patientCode));
        m_overviewTable->setItem(i, 2, new QTableWidgetItem(req.patientName));
        m_overviewTable->setItem(i, 3, new QTableWidgetItem(req.serviceName));
        m_overviewTable->setItem(i, 4, new QTableWidgetItem(req.roomName));
        m_overviewTable->setItem(i, 5, new QTableWidgetItem(serviceRequestStatusToVi(req.status)));
    }
}

void NurseDashboardWidget::updateQueueTable() {
    if (!m_serviceRequestService) return;

    int roomId = m_comboRooms ? m_comboRooms->currentData().toInt() : -1;
    QString date = m_queueDateEdit ? m_queueDateEdit->date().toString("yyyy-MM-dd") : QDate::currentDate().toString("yyyy-MM-dd");

    std::optional<ServiceRequestStatus> statusOpt = std::nullopt;
    if (m_comboStatusFilter && m_comboStatusFilter->currentData().toInt() != -1) {
        statusOpt = static_cast<ServiceRequestStatus>(m_comboStatusFilter->currentData().toInt());
    }

    auto requests = m_serviceRequestService->getRequestsByRoom(roomId, date, statusOpt);

    m_queueTable->setRowCount(0);
    m_selectedRequestId = -1;

    for (int i = 0; i < requests.size(); ++i) {
        const auto &req = requests[i];
        m_queueTable->insertRow(i);

        QTableWidgetItem *itemTicket = new QTableWidgetItem(req.ticketNumber.has_value() ? QString::number(req.ticketNumber.value()) : "Chưa có");
        QTableWidgetItem *itemCode = new QTableWidgetItem(req.patientCode);
        QTableWidgetItem *itemName = new QTableWidgetItem(req.patientName);
        QTableWidgetItem *itemService = new QTableWidgetItem(req.serviceName);
        QTableWidgetItem *itemDoctor = new QTableWidgetItem(req.doctorName);
        QTableWidgetItem *itemStatus = new QTableWidgetItem(serviceRequestStatusToVi(req.status));
        QTableWidgetItem *itemTime = new QTableWidgetItem(req.prescribedAt.toString("hh:mm dd/MM"));

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
}

void NurseDashboardWidget::onRoomChanged(int index) {
    Q_UNUSED(index);
    updateQueueTable();
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
    if (row < 0) return;
    QTableWidgetItem *item = m_queueTable->item(row, 0);
    if (item) {
        m_selectedRequestId = item->data(Qt::UserRole).toInt();
    }
}

void NurseDashboardWidget::onCheckInClicked() {
    if (m_selectedRequestId <= 0) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng chọn một yêu cầu trong danh sách.");
        return;
    }

    int ticketNum = 0;
    if (m_serviceRequestService->checkIn(m_selectedRequestId, ticketNum)) {
        QMessageBox::information(this, "Thành công", QString("Đã tiếp nhận bệnh nhân! Số thứ tự STT: %1").arg(ticketNum));
        updateQueueTable();
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể tiếp nhận yêu cầu này (có thể đã tiếp nhận hoặc đã hủy).");
    }
}

void NurseDashboardWidget::onStartProcessingClicked() {
    if (m_selectedRequestId <= 0) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng chọn một yêu cầu trong danh sách.");
        return;
    }

    if (m_serviceRequestService->startProcessing(m_selectedRequestId)) {
        QMessageBox::information(this, "Thành công", "Đã chuyển trạng thái sang Đang Thực Hiện Xét Nghiệm.");
        updateQueueTable();
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể bắt đầu (yêu cầu phải ở trạng thái Đã Tiếp Nhận).");
    }
}

void NurseDashboardWidget::onCompleteClicked() {
    if (m_selectedRequestId <= 0) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng chọn một yêu cầu trong danh sách.");
        return;
    }

    bool ok = false;
    QString resultNote = QInputDialog::getMultiLineText(
        this, "Nhập Kết Quả Xét Nghiệm",
        "Kết quả xét nghiệm / Ghi chú:", "", &ok
    );

    if (!ok) return; // User cancelled input

    if (m_serviceRequestService->completeProcessing(m_selectedRequestId, resultNote)) {
        QMessageBox::information(this, "Thành công", "Đã hoàn thành và trả kết quả xét nghiệm!");
        updateQueueTable();
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể hoàn thành (yêu cầu phải ở trạng thái Đang Thực Hiện).");
    }
}

void NurseDashboardWidget::onCancelClicked() {
    if (m_selectedRequestId <= 0) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng chọn một yêu cầu trong danh sách.");
        return;
    }

    auto reply = QMessageBox::question(this, "Xác nhận", "Bạn có chắc chắn muốn hủy yêu cầu xét nghiệm này?", QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    if (m_serviceRequestService->cancelRequest(m_selectedRequestId)) {
        QMessageBox::information(this, "Thành công", "Đã hủy yêu cầu xét nghiệm.");
        updateQueueTable();
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể hủy yêu cầu này.");
    }
}

void NurseDashboardWidget::switchPage(int index, QPushButton* activeBtn) {
    if (!m_stackedWidget) return;
    m_stackedWidget->setCurrentIndex(index);

    QPushButton* buttons[] = { m_btnOverview, m_btnLabQueue };
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
        updateOverviewData();
    } else if (index == 1) {
        updateQueueTable();
    }
}
