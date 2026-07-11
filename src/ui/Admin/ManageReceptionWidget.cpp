#include "ManageReceptionWidget.h"
#include "ReceptionRegistrationDialog.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/SystemUser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>

ManageReceptionWidget::ManageReceptionWidget(std::shared_ptr<StaffService> staffService, QWidget* parent)
    : QWidget(parent), m_staffService(staffService), m_tblReception(nullptr) {
    buildUI();
}

void ManageReceptionWidget::buildUI() {
    QVBoxLayout* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(30, 30, 30, 30);
    pageLayout->setSpacing(20);

    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* lblTitle = new QLabel("Quản lý Lễ tân");
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #111827;");
    headerLayout->addWidget(lblTitle);
    
    headerLayout->addStretch();
    
    QPushButton* btnAddNew = new QPushButton("+ Thêm Lễ tân");
    btnAddNew->setCursor(Qt::PointingHandCursor);
    btnAddNew->setStyleSheet("QPushButton { background-color: #2563EB; color: white; border-radius: 6px; padding: 8px 16px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: #1D4ED8; }");
    headerLayout->addWidget(btnAddNew);
    
    pageLayout->addLayout(headerLayout);

    // Table Card
    QFrame* tableCard = makeCard(this);
    QVBoxLayout* cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(0, 0, 0, 0);

    m_tblReception = new QTableWidget(tableCard);
    m_tblReception->setColumnCount(6);
    m_tblReception->setHorizontalHeaderLabels({"Mã NV", "Họ Tên", "Ca làm việc", "SĐT", "Trạng thái", "Thao tác"});
    m_tblReception->horizontalHeader()->setStretchLastSection(true);
    m_tblReception->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tblReception->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblReception->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblReception->setAlternatingRowColors(true);
    m_tblReception->setStyleSheet("QTableWidget { border: none; background-color: white; alternate-background-color: #F9FAFB; color: #111827; } QHeaderView::section { background-color: #F3F4F6; padding: 12px; font-weight: bold; border: none; border-bottom: 1px solid #E5E7EB; color: #111827; } QTableWidget::item { padding: 12px; border-bottom: 1px solid #E5E7EB; color: #111827; }");

    cardLayout->addWidget(m_tblReception);
    pageLayout->addWidget(tableCard);

    connect(btnAddNew, &QPushButton::clicked, this, &ManageReceptionWidget::showAddReceptionDialog);

    loadReceptionList();
}

QFrame* ManageReceptionWidget::makeCard(QWidget* parent) {
    QFrame* card = new QFrame(parent);
    card->setStyleSheet("QFrame { background-color: #FFFFFF; border-radius: 12px; border: 1px solid #E5E7EB; }");
    return card;
}

void ManageReceptionWidget::showAddReceptionDialog() {
    ReceptionRegistrationDialog dialog(m_staffService, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadReceptionList();
    }
}

void ManageReceptionWidget::showEditReceptionDialog(std::shared_ptr<SystemUser> reception) {
    QMessageBox::information(this, "Sửa Lễ tân", "Tính năng sửa Lễ tân đang được triển khai!");
}

void ManageReceptionWidget::loadReceptionList() {
    if (!m_tblReception || !m_staffService) return;
    m_tblReception->setRowCount(0);

    auto receptionists = m_staffService->searchReceptionists("");
    
    for (int i = 0; i < receptionists.size(); ++i) {
        auto receptionist = receptionists[i];
        if (!receptionist) continue;
        
        m_tblReception->insertRow(i);
        
        QTableWidgetItem* itemCode = new QTableWidgetItem(receptionist->getStaffCode());
        itemCode->setData(Qt::UserRole, receptionist->getAccountId());
        
        m_tblReception->setItem(i, 0, itemCode);
        m_tblReception->setItem(i, 1, new QTableWidgetItem(receptionist->getFullName()));
        
        auto profile = m_staffService->getOwnProfile(receptionist->getAccountId());
        QString shiftStr = profile ? profile->shift : "Cả ngày";
        QString phoneStr = profile ? profile->phoneNumber : "";
        
        m_tblReception->setItem(i, 2, new QTableWidgetItem(shiftStr));
        
        m_tblReception->setItem(i, 3, new QTableWidgetItem(phoneStr));
        m_tblReception->setItem(i, 4, new QTableWidgetItem(receptionist->isActive() ? "Hoạt động" : "Nghỉ việc"));
        
        QPushButton* btnEdit = new QPushButton("Sửa");
        btnEdit->setStyleSheet("QPushButton { color: #2563EB; font-weight: bold; background: transparent; border: none; } QPushButton:hover { text-decoration: underline; }");
        btnEdit->setCursor(Qt::PointingHandCursor);
        
        connect(btnEdit, &QPushButton::clicked, this, [this, receptionist]() {
            showEditReceptionDialog(receptionist);
        });
        
        QWidget* actionWidget = new QWidget();
        QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(0, 0, 0, 0);
        actionLayout->addWidget(btnEdit);
        actionLayout->setAlignment(Qt::AlignCenter);
        
        m_tblReception->setCellWidget(i, 5, actionWidget);
    }
}
