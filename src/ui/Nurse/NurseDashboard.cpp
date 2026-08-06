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
    std::shared_ptr<ServiceRequestService> serviceRequestService,
    QWidget *parent)
    : BaseDashboardWidget(user, staffService, parent),
      m_serviceRequestService(serviceRequestService) {
    
    // Read Nurse Room ID from user profile if available
    if (m_staffService && m_currentUser) {
        auto profile = m_staffService->getOwnProfile(m_currentUser->getAccountId());
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
    QLabel *roleHeader = new QLabel("Y TÁ", m_sidebarFrame);
    roleHeader->setStyleSheet(
        "font-size: 10px; font-weight: bold; color: #B0B8C4;"
        "letter-spacing: 2px; margin: 8px 4px 4px 4px;");

    m_btnOverview = new QPushButton("Tổng Quan", m_sidebarFrame);
    m_btnLabQueue = new QPushButton("Phòng Xét Nghiệm", m_sidebarFrame);
    QPushButton *btnLogout = new QPushButton("Đăng Xuất", m_sidebarFrame);

    m_btnOverview->setObjectName("activeBtn"); // Default active

    btnLogout->setCursor(Qt::PointingHandCursor);
    btnLogout->setStyleSheet(
        "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; "
        "color: #D32F2F; border: none; border-radius: 0px; background-color: "
        "transparent; font-weight: bold; }"
        "QPushButton:hover { background-color: #FFEBEE; }");

    m_sidebarLayout->addWidget(roleHeader);
    m_sidebarLayout->addWidget(m_btnOverview);
    m_sidebarLayout->addWidget(m_btnLabQueue);
    m_sidebarLayout->addStretch();
    m_sidebarLayout->addWidget(btnLogout);
    m_sidebarLayout->addSpacing(30);

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
    m_overviewTable->verticalHeader()->setVisible(false);
    m_overviewTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_overviewTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_overviewTable->setFocusPolicy(Qt::NoFocus);
    m_overviewTable->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: #F4F5F7; color: #172B4D; outline: none; } "
        "QHeaderView::section { background-color: #F4F5F7; color: #5E6C84; font-weight: bold; border: none; padding: 8px; } "
        "QTableWidget::item { color: #172B4D; background-color: #FFFFFF; padding: 6px; outline: none; } "
        "QTableWidget::item:focus { outline: none; border: none; } "
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
    m_lblRoomValue = new QLabel(filterCard);
    m_lblRoomValue->setStyleSheet("font-weight: bold; color: #0052CC; font-size: 14px; background-color: #DEEBFF; border-radius: 4px; padding: 6px 12px; border: none;");

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
    m_queueTable->setHorizontalHeaderLabels({"STT", "Mã BN", "Họ và tên", "Dịch vụ", "Bác sĩ chỉ định", "Trạng thái", "Thời gian"});
    m_queueTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_queueTable->verticalHeader()->setVisible(false);
    m_queueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_queueTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_queueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_queueTable->setFocusPolicy(Qt::NoFocus);
    m_queueTable->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: #F4F5F7; color: #172B4D; outline: none; } "
        "QHeaderView::section { background-color: #F4F5F7; color: #5E6C84; font-weight: bold; border: none; padding: 8px; } "
        "QTableWidget::item { color: #172B4D; background-color: #FFFFFF; padding: 6px; outline: none; } "
        "QTableWidget::item:focus { outline: none; border: none; } "
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

    // Active Patient Section
    QFrame *activePatientBox = new QFrame(actionCard);
    activePatientBox->setStyleSheet("QFrame { background-color: #F4F5F7; border: 1px solid #DFE1E6; border-radius: 6px; padding: 10px; } QLabel { border: none; background: transparent; }");
    QVBoxLayout *activeLayout = new QVBoxLayout(activePatientBox);
    activeLayout->setContentsMargins(10, 10, 10, 10);
    activeLayout->setSpacing(6);

    QLabel *activeTitle = new QLabel("BỆNH NHÂN HIỆN TẠI", activePatientBox);
    activeTitle->setStyleSheet("font-size: 11px; font-weight: bold; color: #5E6C84;");
    activeLayout->addWidget(activeTitle);

    auto createInfoRow = [activePatientBox](const QString &label, QLabel *&valLabel) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setSpacing(5);
        QLabel *lbl = new QLabel(label, activePatientBox);
        lbl->setStyleSheet("color: #5E6C84; font-size: 12px; font-weight: 500;");
        valLabel = new QLabel("-", activePatientBox);
        valLabel->setStyleSheet("color: #172B4D; font-size: 12px; font-weight: bold;");
        row->addWidget(lbl);
        row->addWidget(valLabel, 1, Qt::AlignRight);
        return row;
    };

    activeLayout->addLayout(createInfoRow("Mã BN:", m_lblActivePatientCode));
    activeLayout->addLayout(createInfoRow("Họ tên:", m_lblActivePatientName));
    activeLayout->addLayout(createInfoRow("Dịch vụ:", m_lblActivePatientService));
    activeLayout->addLayout(createInfoRow("Trạng thái:", m_lblActivePatientStatus));

    actionLayout->addWidget(activePatientBox);

    m_btnCheckIn = new QPushButton("Tiếp Nhận", actionCard);
    m_btnStartProcessing = new QPushButton("Bắt Đầu XN", actionCard);
    m_btnComplete = new QPushButton("Trả Kết Quả", actionCard);
    m_btnCancel = new QPushButton("Hủy Yêu Cầu", actionCard);

    QString actionBtnBase = "QPushButton { font-weight: bold; border-radius: 6px; padding: 10px; border: none; font-size: 14px; } ";
    m_btnCheckIn->setStyleSheet(actionBtnBase + "QPushButton { background-color: #FFAB00; color: white; } QPushButton:hover { background-color: #FF8F00; } QPushButton:disabled { background-color: #DFE1E6; color: #959595; }");
    m_btnStartProcessing->setStyleSheet(actionBtnBase + "QPushButton { background-color: #0065FF; color: white; } QPushButton:hover { background-color: #0052CC; } QPushButton:disabled { background-color: #DFE1E6; color: #959595; }");
    m_btnComplete->setStyleSheet(actionBtnBase + "QPushButton { background-color: #36B37E; color: white; } QPushButton:hover { background-color: #2D9D6F; } QPushButton:disabled { background-color: #DFE1E6; color: #959595; }");
    m_btnCancel->setStyleSheet(actionBtnBase + "QPushButton { background-color: #FF5630; color: white; } QPushButton:hover { background-color: #DE350B; } QPushButton:disabled { background-color: #DFE1E6; color: #959595; }");

    actionLayout->addWidget(m_btnCheckIn);
    actionLayout->addWidget(m_btnStartProcessing);
    actionLayout->addWidget(m_btnComplete);
    actionLayout->addWidget(m_btnCancel);
    actionLayout->addStretch();

    contentLayout->addWidget(actionCard, 1);
    mainLayout->addLayout(contentLayout, 1);

    // Connections
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
    if (m_nurseRoomId > 0) {
        m_lblRoomValue->setText(QString("Phòng %1").arg(m_nurseRoomId));
    } else {
        m_lblRoomValue->setText("Chưa phân công");
    }
}

void NurseDashboardWidget::updateOverviewData() {
    if (!m_serviceRequestService) return;

    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    int roomId = m_nurseRoomId;

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

    int roomId = m_nurseRoomId;
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

    // UPDATE ACTIVE PATIENT SECTION
    auto allOnDateRequests = m_serviceRequestService->getRequestsByRoom(roomId, date);
    
    std::optional<ServiceRequestDTO> activeReq = std::nullopt;
    for (const auto &req : allOnDateRequests) {
        if (req.status == ServiceRequestStatus::CheckedIn || req.status == ServiceRequestStatus::Processing) {
            activeReq = req;
            break;
        }
    }

    if (activeReq.has_value()) {
        m_activeRequestId = activeReq->requestId;
        if (m_lblActivePatientCode) m_lblActivePatientCode->setText(activeReq->patientCode);
        if (m_lblActivePatientName) m_lblActivePatientName->setText(activeReq->patientName);
        if (m_lblActivePatientService) m_lblActivePatientService->setText(activeReq->serviceName);
        if (m_lblActivePatientStatus) m_lblActivePatientStatus->setText(serviceRequestStatusToVi(activeReq->status));

        // Enable buttons based on status
        if (m_btnCheckIn) m_btnCheckIn->setEnabled(false);
        
        if (activeReq->status == ServiceRequestStatus::CheckedIn) {
            if (m_btnStartProcessing) m_btnStartProcessing->setEnabled(true);
            if (m_btnComplete) m_btnComplete->setEnabled(false);
        } else if (activeReq->status == ServiceRequestStatus::Processing) {
            if (m_btnStartProcessing) m_btnStartProcessing->setEnabled(false);
            if (m_btnComplete) m_btnComplete->setEnabled(true);
        }
        if (m_btnCancel) m_btnCancel->setEnabled(true);
    } else {
        m_activeRequestId = -1;
        if (m_lblActivePatientCode) m_lblActivePatientCode->setText("-");
        if (m_lblActivePatientName) m_lblActivePatientName->setText("Chưa có");
        if (m_lblActivePatientService) m_lblActivePatientService->setText("-");
        if (m_lblActivePatientStatus) m_lblActivePatientStatus->setText("Trống");

        if (m_btnCheckIn) m_btnCheckIn->setEnabled(false);
        if (m_btnStartProcessing) m_btnStartProcessing->setEnabled(false);
        if (m_btnComplete) m_btnComplete->setEnabled(false);
        if (m_btnCancel) m_btnCancel->setEnabled(false);
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
    if (row < 0) return;
    QTableWidgetItem *item = m_queueTable->item(row, 0);
    if (item) {
        m_selectedRequestId = item->data(Qt::UserRole).toInt();
        
        // If there is no active patient, enable appropriate buttons based on selection
        if (m_activeRequestId <= 0) {
            if (m_btnCheckIn) m_btnCheckIn->setEnabled(true);
            if (m_btnCancel) m_btnCancel->setEnabled(true);
        }
    }
}

void NurseDashboardWidget::onCheckInClicked() {
    if (m_activeRequestId > 0) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng hoàn thành hoặc hủy yêu cầu hiện tại trước khi tiếp nhận bệnh nhân mới.");
        return;
    }
    if (m_selectedRequestId <= 0) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng chọn một yêu cầu trong danh sách để tiếp nhận.");
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
    int reqId = (m_activeRequestId > 0) ? m_activeRequestId : m_selectedRequestId;
    if (reqId <= 0) {
        QMessageBox::warning(this, "Thông báo", "Không có bệnh nhân nào đang xử lý.");
        return;
    }

    if (m_serviceRequestService->startProcessing(reqId)) {
        QMessageBox::information(this, "Thành công", "Đã chuyển trạng thái sang Đang Thực Hiện Xét Nghiệm.");
        updateQueueTable();
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể bắt đầu (yêu cầu phải ở trạng thái Đã Tiếp Nhận).");
    }
}

void NurseDashboardWidget::onCompleteClicked() {
    int reqId = (m_activeRequestId > 0) ? m_activeRequestId : m_selectedRequestId;
    if (reqId <= 0) {
        QMessageBox::warning(this, "Thông báo", "Không có bệnh nhân nào đang xử lý.");
        return;
    }

    bool ok = false;
    QString resultNote = QInputDialog::getMultiLineText(
        this, "Nhập Kết Quả Xét Nghiệm",
        "Kết quả xét nghiệm / Ghi chú:", "", &ok
    );

    if (!ok) return; // User cancelled input

    if (m_serviceRequestService->completeProcessing(reqId, resultNote)) {
        QMessageBox::information(this, "Thành công", "Đã hoàn thành và trả kết quả xét nghiệm!");
        updateQueueTable();
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể hoàn thành (yêu cầu phải ở trạng thái Đang Thực Hiện).");
    }
}

void NurseDashboardWidget::onCancelClicked() {
    int reqId = (m_activeRequestId > 0) ? m_activeRequestId : m_selectedRequestId;
    if (reqId <= 0) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng chọn một yêu cầu trong danh sách hoặc có bệnh nhân đang xử lý để hủy.");
        return;
    }

    auto reply = QMessageBox::question(this, "Xác nhận", "Bạn có chắc chắn muốn hủy yêu cầu xét nghiệm này?", QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    if (m_serviceRequestService->cancelRequest(reqId)) {
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
