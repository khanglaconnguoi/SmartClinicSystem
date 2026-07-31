#include "ManageReceptionWidget.h"
#include "ReceptionRegistrationDialog.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/SystemUser.h"
#include "../../model/CommonEnums.h"
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
    m_tblReception->verticalHeader()->setDefaultSectionSize(50);
    m_tblReception->verticalHeader()->setVisible(false);
    m_tblReception->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblReception->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblReception->setStyleSheet(
        "QTableWidget { border: none; gridline-color: #EAEAEA; font-size: 13px; "
        "background-color: white; }"
        "QHeaderView::section { background-color: #F8FAFC; padding: 8px; "
        "font-weight: bold; border: none; border-bottom: 1px solid #EAEAEA; "
        "color: #111827; }");

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
    auto profile = m_staffService->getOwnProfile(reception->getAccountId());
    if (profile) {
        ReceptionRegistrationDialog dialog(m_staffService, this);
        dialog.loadReceptionistData(profile.get());
        if (dialog.exec() == QDialog::Accepted) {
            loadReceptionList();
        }
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể lấy thông tin chi tiết Lễ tân.");
    }
}

namespace {
constexpr int PAGE_SIZE = 20;
}

void ManageReceptionWidget::loadReceptionList() {
    if (!m_tblReception || !m_staffService) return;
    m_tblReception->setRowCount(0);

    ReceptionistSearchCriteria criteria;
    criteria.onlyActive = true;
    criteria.includeDeleted = false;
    criteria.pageSize = PAGE_SIZE;

    auto receptionists = m_staffService->searchReceptionistsPaged(criteria).items;


    
    for (int i = 0; i < receptionists.size(); ++i) {
        auto receptionist = receptionists[i];
        if (!receptionist) continue;
        
        m_tblReception->insertRow(i);
        
        QTableWidgetItem* itemCode = new QTableWidgetItem(receptionist->getStaffCode());
        itemCode->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemName = new QTableWidgetItem(receptionist->getFullName());
        itemName->setForeground(QBrush(QColor("#111827")));
        
        auto profile = m_staffService->getOwnProfile(receptionist->getAccountId());
        QString shiftStr = ShiftText::toVi(profile ? profile->shift : "");
        QString phoneStr = profile ? profile->phoneNumber : "---";
        
        QTableWidgetItem* itemShift = new QTableWidgetItem(shiftStr);
        itemShift->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemPhone = new QTableWidgetItem(phoneStr);
        itemPhone->setForeground(QBrush(QColor("#111827")));

        QTableWidgetItem *itemStatus =
            new QTableWidgetItem(receptionist->isActive() ? "Hoạt động" : "Nghỉ việc");
        if (receptionist->isActive()) {
            itemStatus->setForeground(QBrush(QColor("#059669")));
        } else {
            itemStatus->setForeground(QBrush(QColor("#DC2626")));
        }
        
        m_tblReception->setItem(i, 0, itemCode);
        m_tblReception->setItem(i, 1, itemName);
        m_tblReception->setItem(i, 2, itemShift);
        m_tblReception->setItem(i, 3, itemPhone);
        m_tblReception->setItem(i, 4, itemStatus);

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

        QPushButton *btnDeactivate = new QPushButton(receptionist->isActive() ? "Vô hiệu hóa" : "Kích hoạt");
        btnDeactivate->setCursor(Qt::PointingHandCursor);
        if (receptionist->isActive()) {
            btnDeactivate->setStyleSheet(
                "QPushButton { color: #D93025; border: 1px solid #D93025; padding: 4px "
                "8px; border-radius: 4px; background-color: white; } QPushButton:hover "
                "{ background-color: #FCE8E6; }");
        } else {
            btnDeactivate->setStyleSheet(
                "QPushButton { color: #059669; border: 1px solid #059669; padding: 4px "
                "8px; border-radius: 4px; background-color: white; } QPushButton:hover "
                "{ background-color: #D1FAE5; }");
        }

        actionLayout->addWidget(btnEdit);
        actionLayout->addWidget(btnDeactivate);
        m_tblReception->setCellWidget(i, 5, actionWidget);

        connect(btnEdit, &QPushButton::clicked, this,
                [this, receptionist]() { showEditReceptionDialog(receptionist); });
                
        connect(btnDeactivate, &QPushButton::clicked, this, [this, receptionist]() {
            if (receptionist->isActive()) {
                if (m_staffService->deactivateStaff(receptionist->getAccountId())) {
                    loadReceptionList();
                } else {
                    QMessageBox::warning(this, "Lỗi", "Không thể vô hiệu hóa Lễ tân này.");
                }
            } else {
                if (m_staffService->reactivateStaff(receptionist->getAccountId())) {
                    loadReceptionList();
                } else {
                    QMessageBox::warning(this, "Lỗi", "Không thể kích hoạt Lễ tân này.");
                }
            }
        });
    }
}
