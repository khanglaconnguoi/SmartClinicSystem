#include "ManageLeaveWidget.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QPushButton>
#include "../../dto/StaffDTOs.h"
#include <QHeaderView>

ManageLeaveWidget::ManageLeaveWidget(std::shared_ptr<StaffService> staffService, std::shared_ptr<AppointmentService> appointmentService, QWidget *parent)
    : QWidget(parent), m_staffService(staffService), m_appointmentService(appointmentService) {
    setupUi();
}

QFrame* ManageLeaveWidget::makeCard(QWidget* parent) {
    QFrame* card = new QFrame(parent);
    card->setObjectName("CardFrame");
    card->setStyleSheet("#CardFrame { background-color: #FFFFFF; border-radius: 12px; }");
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 5);
    card->setGraphicsEffect(shadow);
    return card;
}

void ManageLeaveWidget::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    QLabel* lblTitle = new QLabel("Quản Lý Nghỉ Phép", this);
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #202124;");
    layout->addWidget(lblTitle);

    QTabWidget* tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #DADCE0; border-radius: 8px; background: white; }"
        "QTabBar::tab { padding: 10px 20px; font-weight: bold; color: #5F6368; background: #F1F3F4; border: 1px solid #DADCE0; border-bottom: none; border-top-left-radius: 8px; border-top-right-radius: 8px; margin-right: 2px; }"
        "QTabBar::tab:selected { color: #1A73E8; background: white; }"
    );
    
    // Tab 1: Duyệt đơn
    QWidget* tabApprove = new QWidget();
    QVBoxLayout* layApprove = new QVBoxLayout(tabApprove);
    layApprove->setContentsMargins(20, 20, 20, 20);
    
    m_tablePendingLeaves = new QTableWidget(tabApprove);
    m_tablePendingLeaves->setColumnCount(7);
    m_tablePendingLeaves->setHorizontalHeaderLabels({"Mã NV", "Họ tên", "Từ ngày", "Đến ngày", "Lý do", "Trạng thái", "Thao tác"});
    
    QHeaderView* header = m_tablePendingLeaves->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setSectionResizeMode(6, QHeaderView::Fixed); // Action column fixed size
    m_tablePendingLeaves->setColumnWidth(6, 160); // Enough for 2 buttons
    header->setStretchLastSection(false); // don't stretch the fixed action column
    // Let the reason column stretch instead
    header->setSectionResizeMode(4, QHeaderView::Stretch);
    
    m_tablePendingLeaves->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tablePendingLeaves->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tablePendingLeaves->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tablePendingLeaves->setStyleSheet(
        "QTableWidget { border: 1px solid #DADCE0; border-radius: 4px; color: #202124; background-color: white; }"
        "QHeaderView::section { background-color: #F1F3F4; color: #5F6368; font-weight: bold; padding: 4px; border: 1px solid #DADCE0; }"
    );
    layApprove->addWidget(m_tablePendingLeaves);
    
    tabWidget->addTab(tabApprove, "Danh sách Chờ duyệt");

    // Tab 2: Đăng ký nghỉ
    QWidget* tabRegister = new QWidget();
    QVBoxLayout* layRegister = new QVBoxLayout(tabRegister);
    layRegister->setContentsMargins(20, 20, 20, 20);

    QFrame* formFrame = makeCard(tabRegister);
    QFormLayout* formLayout = new QFormLayout(formFrame);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setSpacing(15);

    m_comboLeaveStaff = new QComboBox(formFrame);
    m_comboLeaveStaff->setStyleSheet("padding: 8px; border: 1px solid #DADCE0; border-radius: 4px;");
    
    StaffSearchCriteria criteria;
    criteria.onlyActive = true;
    auto staffList = m_staffService->searchStaff(criteria);
    m_comboLeaveStaff->addItem("--- Chọn nhân viên ---", -1);
    for (const auto& staff : staffList) {
        m_comboLeaveStaff->addItem(QString("%1 - %2 (%3)").arg(staff->getStaffCode(), staff->getFullName(), userRoleToVi(staff->getRole())), staff->getAccountId());
    }

    m_lblLeaveBalance = new QLabel("Vui lòng chọn nhân viên.", formFrame);
    m_lblLeaveBalance->setStyleSheet("color: #5F6368; font-style: italic;");

    m_leaveStartDate = new QDateEdit(QDate::currentDate(), formFrame);
    m_leaveStartDate->setCalendarPopup(true);
    m_leaveStartDate->setMinimumDate(QDate::currentDate());
    m_leaveStartDate->setStyleSheet("padding: 8px; border: 1px solid #DADCE0; border-radius: 4px;");

    m_leaveEndDate = new QDateEdit(QDate::currentDate(), formFrame);
    m_leaveEndDate->setCalendarPopup(true);
    m_leaveEndDate->setMinimumDate(QDate::currentDate());
    m_leaveEndDate->setStyleSheet("padding: 8px; border: 1px solid #DADCE0; border-radius: 4px;");

    m_txtLeaveReason = new QTextEdit(formFrame);
    m_txtLeaveReason->setFixedHeight(80);
    m_txtLeaveReason->setStyleSheet("padding: 8px; border: 1px solid #DADCE0; border-radius: 4px;");

    formLayout->addRow("Nhân viên:", m_comboLeaveStaff);
    formLayout->addRow("Quỹ phép:", m_lblLeaveBalance);
    formLayout->addRow("Từ ngày:", m_leaveStartDate);
    formLayout->addRow("Đến ngày:", m_leaveEndDate);
    formLayout->addRow("Lý do:", m_txtLeaveReason);

    QPushButton* btnSubmit = new QPushButton("Đăng ký Nghỉ phép", formFrame);
    btnSubmit->setCursor(Qt::PointingHandCursor);
    btnSubmit->setStyleSheet("background-color: #1A73E8; color: white; padding: 10px 20px; font-weight: bold; border-radius: 4px; border: none;");

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnSubmit);

    layRegister->addWidget(formFrame);
    layRegister->addLayout(btnLayout);
    layRegister->addStretch();

    tabWidget->addTab(tabRegister, "Đăng ký Nghỉ phép (Hộ)");

    layout->addWidget(tabWidget);

    connect(m_comboLeaveStaff, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManageLeaveWidget::onLeaveStaffSelected);
    connect(btnSubmit, &QPushButton::clicked, this, &ManageLeaveWidget::onSubmitLeaveRequest);
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
    
    for (const auto& req : pendingList) {
        int row = m_tablePendingLeaves->rowCount();
        m_tablePendingLeaves->insertRow(row);
        
        QTableWidgetItem* itemStaffCode = new QTableWidgetItem(req.staffCode);
        itemStaffCode->setData(Qt::UserRole, req.requestId); // Store ID here
        
        m_tablePendingLeaves->setItem(row, 0, itemStaffCode);
        m_tablePendingLeaves->setItem(row, 1, new QTableWidgetItem(req.fullName));
        m_tablePendingLeaves->setItem(row, 2, new QTableWidgetItem(req.startDate.toString("dd/MM/yyyy")));
        m_tablePendingLeaves->setItem(row, 3, new QTableWidgetItem(req.endDate.toString("dd/MM/yyyy")));
        m_tablePendingLeaves->setItem(row, 4, new QTableWidgetItem(req.reason));
        m_tablePendingLeaves->setItem(row, 5, new QTableWidgetItem(req.status));
        
        // Add action buttons
        QWidget* actionWidget = new QWidget();
        QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setSpacing(8);
        
        QPushButton* btnApprove = new QPushButton("Duyệt", actionWidget);
        btnApprove->setCursor(Qt::PointingHandCursor);
        btnApprove->setStyleSheet("background-color: #34A853; color: white; border-radius: 4px; font-weight: bold; padding: 4px 8px;");
        
        QPushButton* btnReject = new QPushButton("Từ chối", actionWidget);
        btnReject->setCursor(Qt::PointingHandCursor);
        btnReject->setStyleSheet("background-color: #EA4335; color: white; border-radius: 4px; font-weight: bold; padding: 4px 8px;");
        
        actionLayout->addWidget(btnApprove);
        actionLayout->addWidget(btnReject);
        
        m_tablePendingLeaves->setCellWidget(row, 6, actionWidget);
        
        int reqId = req.requestId;
        
        connect(btnApprove, &QPushButton::clicked, this, [this, reqId]() {
            QString err = m_staffService->processLeaveRequest(reqId, true, m_appointmentService);
            if (!err.isEmpty()) {
                QMessageBox::warning(this, "Lỗi", err);
            } else {
                QMessageBox::information(this, "Thành công", "Đã duyệt đơn nghỉ phép thành công.");
                loadPendingLeaves();
            }
        });
        
        connect(btnReject, &QPushButton::clicked, this, [this, reqId]() {
            QString err = m_staffService->processLeaveRequest(reqId, false, m_appointmentService);
            if (!err.isEmpty()) {
                QMessageBox::warning(this, "Lỗi", err);
            } else {
                QMessageBox::information(this, "Thành công", "Đã từ chối đơn nghỉ phép.");
                loadPendingLeaves();
            }
        });
    }
}



void ManageLeaveWidget::onLeaveStaffSelected(int /*index*/) {
    int staffId = m_comboLeaveStaff->currentData().toInt();
    if (staffId <= 0) {
        m_lblLeaveBalance->setText("Vui lòng chọn nhân viên.");
        return;
    }

    LeaveBalanceDTO balance = m_staffService->getLeaveBalance(staffId, QDate::currentDate().year());
    m_lblLeaveBalance->setText(QString("Đã dùng: %1 / %2 ngày (Còn lại: %3 ngày)")
                               .arg(balance.usedDays)
                               .arg(balance.totalDays)
                               .arg(balance.totalDays - balance.usedDays));
}

void ManageLeaveWidget::onSubmitLeaveRequest() {
    int staffId = m_comboLeaveStaff->currentData().toInt();
    if (staffId <= 0) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng chọn nhân viên.");
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

    QString successMsg = "Đăng ký nghỉ phép thành công! Đơn sẽ được chuyển sang trạng thái Chờ duyệt.";
    QMessageBox::information(this, "Thành công", successMsg);
    
    // Reset form
    m_txtLeaveReason->clear();
    m_leaveStartDate->setDate(QDate::currentDate());
    m_leaveEndDate->setDate(QDate::currentDate());
    onLeaveStaffSelected(m_comboLeaveStaff->currentIndex());
}
