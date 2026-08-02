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
    
    QPushButton* btnAddNew = new QPushButton("Thêm Lễ tân");
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
    m_tblReception->verticalHeader()->setDefaultSectionSize(46);
    m_tblReception->verticalHeader()->setVisible(false);
    m_tblReception->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblReception->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblReception->setStyleSheet(
        "QTableWidget { border: none; gridline-color: #E2E8F0; font-size: 13px; "
        "background-color: white; color: #0F172A; }"
        "QHeaderView::section { background-color: #F8FAFC; padding: 10px; "
        "font-weight: bold; border: none; border-bottom: 1px solid #E2E8F0; "
        "color: #1E293B; }");

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

        QPushButton *btnResetPwd = new QPushButton("Reset MK");
        btnResetPwd->setCursor(Qt::PointingHandCursor);
        btnResetPwd->setStyleSheet(
            "QPushButton { color: #D97706; border: 1px solid #D97706; padding: 4px "
            "8px; border-radius: 4px; background-color: white; font-weight: bold; } QPushButton:hover "
            "{ background-color: #FEF3C7; }");

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
        actionLayout->addWidget(btnResetPwd);
        actionLayout->addWidget(btnDeactivate);
        m_tblReception->setCellWidget(i, 5, actionWidget);

        connect(btnEdit, &QPushButton::clicked, this,
                [this, receptionist]() { showEditReceptionDialog(receptionist); });

        connect(btnResetPwd, &QPushButton::clicked, this, [this, receptionist]() {
            auto confirm = QMessageBox::question(this, "Xác nhận Reset Mật khẩu",
                QString("Bạn có chắc chắn muốn reset mật khẩu cho Lễ tân %1 (%2)?").arg(receptionist->getFullName(), receptionist->getStaffCode()),
                QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                auto res = m_staffService->resetPassword(receptionist->getAccountId());
                if (res.result) {
                    QMessageBox::information(this, "Reset Mật khẩu thành công",
                        QString("Mật khẩu mới cho Lễ tân %1 (%2) là:\n\n%3\n\nTài khoản sẽ yêu cầu đổi mật khẩu khi đăng nhập lần tiếp theo.")
                        .arg(receptionist->getFullName(), receptionist->getStaffCode(), res.newPassword));
                } else {
                    QMessageBox::warning(this, "Lỗi", "Không thể reset mật khẩu cho Lễ tân này.");
                }
            }
        });
                
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
