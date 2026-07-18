#include "ManageNursesWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include "NurseRegistrationDialog.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/SystemUser.h"
ManageNursesWidget::ManageNursesWidget(std::shared_ptr<StaffService> staffService, QWidget* parent)
    : QWidget(parent), m_staffService(staffService), m_tblNurses(nullptr) {
    buildUI();
}

QFrame* ManageNursesWidget::makeCard(QWidget* parent) {
    QFrame* card = new QFrame(parent);
    card->setObjectName("DashboardCard");
    card->setStyleSheet(
        "QFrame#DashboardCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 12px;"
        "}"
    );
    return card;
}

void ManageNursesWidget::buildUI() {
    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(30, 30, 30, 30);
    pageLayout->setSpacing(20);

    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *lblPageTitle = new QLabel("Danh sách Y tá", this);
    lblPageTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #111827;");
    headerLayout->addWidget(lblPageTitle);

    headerLayout->addStretch();

    QPushButton *btnAddNew = new QPushButton("+ Thêm Y tá", this);
    btnAddNew->setCursor(Qt::PointingHandCursor);
    btnAddNew->setFixedSize(140, 40);
    btnAddNew->setStyleSheet(
        "QPushButton { background-color: #4B94F2; color: white; font-size: 14px; "
        "font-weight: bold; border-radius: 6px; border: none; }"
        "QPushButton:hover { background-color: #398CBF; }");
    headerLayout->addWidget(btnAddNew);
    pageLayout->addLayout(headerLayout);

    // Card bao bọc Table
    QFrame *tableCard = makeCard(this);
    QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(0, 0, 0, 0);

    m_tblNurses = new QTableWidget(0, 6, tableCard);
    m_tblNurses->setHorizontalHeaderLabels(
        {"Mã YT", "Họ Tên", "Cấp bậc", "Chứng chỉ", "SĐT", "Thao tác"});
    m_tblNurses->horizontalHeader()->setStretchLastSection(true);
    m_tblNurses->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tblNurses->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblNurses->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblNurses->setStyleSheet("QTableWidget { border: none; background-color: white; alternate-background-color: #F9FAFB; color: #111827; } QHeaderView::section { background-color: #F3F4F6; padding: 12px; font-weight: bold; border: none; border-bottom: 1px solid #E5E7EB; color: #111827; } QTableWidget::item { padding: 12px; border-bottom: 1px solid #E5E7EB; color: #111827; }");

    cardLayout->addWidget(m_tblNurses);
    pageLayout->addWidget(tableCard);

    connect(btnAddNew, &QPushButton::clicked, this, &ManageNursesWidget::showAddNurseDialog);

    // Load data
    loadNursesList();
}

void ManageNursesWidget::showAddNurseDialog() {
    NurseRegistrationDialog dialog(m_staffService, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadNursesList();
    }
}

void ManageNursesWidget::showEditNurseDialog(std::shared_ptr<SystemUser> nurse) {
    // Tạm thời chưa triển khai lấy NurseProfileDTO từ SystemUser
    QMessageBox::information(this, "Sửa Y tá", "Tính năng sửa Y tá đang được triển khai!");
}

void ManageNursesWidget::loadNursesList() {
    if (!m_tblNurses || !m_staffService) return;
    m_tblNurses->setRowCount(0);

    auto nurses = m_staffService->searchNurses("");
    
    for (int i = 0; i < nurses.size(); ++i) {
        auto nurse = nurses[i];
        if (!nurse) continue;
        
        m_tblNurses->insertRow(i);
        
        QTableWidgetItem* itemCode = new QTableWidgetItem(nurse->getStaffCode());
        itemCode->setData(Qt::UserRole, nurse->getAccountId());
        itemCode->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemName = new QTableWidgetItem(nurse->getFullName());
        itemName->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemLevel = new QTableWidgetItem("Khoa Cấp cứu"); // fake info as nurse system is incomplete
        itemLevel->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemCert = new QTableWidgetItem("Điều dưỡng trưởng");
        itemCert->setForeground(QBrush(QColor("#111827")));
        
        auto profile = m_staffService->getOwnProfile(nurse->getAccountId());
        QString phoneStr = profile ? profile->phoneNumber : "";
        QTableWidgetItem* itemPhone = new QTableWidgetItem(phoneStr);
        itemPhone->setForeground(QBrush(QColor("#111827")));
        
        m_tblNurses->setItem(i, 0, itemCode);
        m_tblNurses->setItem(i, 1, itemName);
        m_tblNurses->setItem(i, 2, itemLevel);
        m_tblNurses->setItem(i, 3, itemCert);
        m_tblNurses->setItem(i, 4, itemPhone);

        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setSpacing(8);
        QPushButton *btnEdit = new QPushButton("Sửa");
        btnEdit->setCursor(Qt::PointingHandCursor);
        btnEdit->setStyleSheet(
            "QPushButton { color: #4B94F2; border: 1px solid #4B94F2; padding: 4px "
            "8px; border-radius: 4px; background-color: white; } QPushButton:hover "
            "{ background-color: #EBF5FF; }");
        QPushButton *btnDelete = new QPushButton("Xóa");
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setStyleSheet(
            "QPushButton { color: #F43F5E; border: 1px solid #F43F5E; padding: 4px "
            "8px; border-radius: 4px; background-color: white; } QPushButton:hover "
            "{ background-color: #FFE4E6; }");
        actionLayout->addWidget(btnEdit);
        actionLayout->addWidget(btnDelete);
        actionLayout->setAlignment(Qt::AlignCenter);

        m_tblNurses->setCellWidget(i, 5, actionWidget);
    }
}
