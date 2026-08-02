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

    QPushButton *btnAddNew = new QPushButton("Thêm Y tá", this);
    btnAddNew->setCursor(Qt::PointingHandCursor);
    btnAddNew->setFixedSize(140, 40);
    btnAddNew->setStyleSheet(
        "QPushButton { background-color: #2563EB; color: white; font-size: 14px; "
        "font-weight: bold; border-radius: 6px; border: none; }"
        "QPushButton:hover { background-color: #1D4ED8; }");
    headerLayout->addWidget(btnAddNew);
    pageLayout->addLayout(headerLayout);

    // Card bao bọc Table
    QFrame *tableCard = makeCard(this);
    QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(0, 0, 0, 0);

    m_tblNurses = new QTableWidget(0, 7, tableCard);
    m_tblNurses->setHorizontalHeaderLabels(
        {"Mã YT", "Họ Tên", "Cấp bậc", "Chứng chỉ", "SĐT", "Trạng thái", "Thao tác"});
    m_tblNurses->horizontalHeader()->setStretchLastSection(true);
    m_tblNurses->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tblNurses->verticalHeader()->setDefaultSectionSize(46);
    m_tblNurses->verticalHeader()->setVisible(false);
    m_tblNurses->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblNurses->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblNurses->setStyleSheet(
        "QTableWidget { border: none; gridline-color: #E2E8F0; font-size: 13px; "
        "background-color: white; color: #0F172A; }"
        "QHeaderView::section { background-color: #F8FAFC; padding: 10px; "
        "font-weight: bold; border: none; border-bottom: 1px solid #E2E8F0; "
        "color: #1E293B; }");

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
    auto profile = m_staffService->getOwnProfile(nurse->getAccountId());
    auto nurseProfile = dynamic_cast<NurseProfileDTO*>(profile.get());
    if (nurseProfile) {
        NurseRegistrationDialog dialog(m_staffService, this);
        dialog.loadNurseData(nurseProfile);
        if (dialog.exec() == QDialog::Accepted) {
            loadNursesList();
        }
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể lấy thông tin chi tiết Y tá.");
    }
}

namespace {
constexpr int PAGE_SIZE = 20;
}

void ManageNursesWidget::loadNursesList() {
    if (!m_tblNurses || !m_staffService) return;
    m_tblNurses->setRowCount(0);

    NurseSearchCriteria criteria;
    criteria.onlyActive = true;
    criteria.includeDeleted = false;
    criteria.pageSize = PAGE_SIZE;

    auto nurses = m_staffService->searchNursesPaged(criteria).items;


    
    for (int i = 0; i < nurses.size(); ++i) {
        auto nurse = nurses[i];
        if (!nurse) continue;
        
        m_tblNurses->insertRow(i);
        
        QTableWidgetItem* itemCode = new QTableWidgetItem(nurse->getStaffCode());
        itemCode->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemName = new QTableWidgetItem(nurse->getFullName());
        itemName->setForeground(QBrush(QColor("#111827")));
        
        auto profile = m_staffService->getOwnProfile(nurse->getAccountId());
        auto nurseProfile = dynamic_cast<NurseProfileDTO*>(profile.get());

        QString nurseLevel = nurseProfile ? nurseProfile->nurseLevel : "---";
        QTableWidgetItem* itemLevel = new QTableWidgetItem(nurseLevel);
        itemLevel->setForeground(QBrush(QColor("#111827")));
        
        QString cert = nurseProfile ? nurseProfile->certification : "---";
        QTableWidgetItem* itemCert = new QTableWidgetItem(cert);
        itemCert->setForeground(QBrush(QColor("#111827")));
        
        QString phoneStr = profile ? profile->phoneNumber : "---";
        QTableWidgetItem* itemPhone = new QTableWidgetItem(phoneStr);
        itemPhone->setForeground(QBrush(QColor("#111827")));

        QTableWidgetItem *itemStatus =
            new QTableWidgetItem(nurse->isActive() ? "Hoạt động" : "Nghỉ việc");
        if (nurse->isActive()) {
            itemStatus->setForeground(QBrush(QColor("#059669")));
        } else {
            itemStatus->setForeground(QBrush(QColor("#DC2626")));
        }
        
        m_tblNurses->setItem(i, 0, itemCode);
        m_tblNurses->setItem(i, 1, itemName);
        m_tblNurses->setItem(i, 2, itemLevel);
        m_tblNurses->setItem(i, 3, itemCert);
        m_tblNurses->setItem(i, 4, itemPhone);
        m_tblNurses->setItem(i, 5, itemStatus);

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

        QPushButton *btnDeactivate = new QPushButton(nurse->isActive() ? "Vô hiệu hóa" : "Kích hoạt");
        btnDeactivate->setCursor(Qt::PointingHandCursor);
        if (nurse->isActive()) {
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
        m_tblNurses->setCellWidget(i, 6, actionWidget);

        connect(btnEdit, &QPushButton::clicked, this,
                [this, nurse]() { showEditNurseDialog(nurse); });

        connect(btnResetPwd, &QPushButton::clicked, this, [this, nurse]() {
            auto confirm = QMessageBox::question(this, "Xác nhận Reset Mật khẩu",
                QString("Bạn có chắc chắn muốn reset mật khẩu cho Y tá %1 (%2)?").arg(nurse->getFullName(), nurse->getStaffCode()),
                QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                auto res = m_staffService->resetPassword(nurse->getAccountId());
                if (res.result) {
                    QMessageBox::information(this, "Reset Mật khẩu thành công",
                        QString("Mật khẩu mới cho Y tá %1 (%2) là:\n\n%3\n\nTài khoản sẽ yêu cầu đổi mật khẩu khi đăng nhập lần tiếp theo.")
                        .arg(nurse->getFullName(), nurse->getStaffCode(), res.newPassword));
                } else {
                    QMessageBox::warning(this, "Lỗi", "Không thể reset mật khẩu cho Y tá này.");
                }
            }
        });

        connect(btnEdit, &QPushButton::clicked, this,
                [this, nurse]() { showEditNurseDialog(nurse); });
                
        connect(btnDeactivate, &QPushButton::clicked, this, [this, nurse]() {
            if (nurse->isActive()) {
                if (m_staffService->deactivateStaff(nurse->getAccountId())) {
                    loadNursesList();
                } else {
                    QMessageBox::warning(this, "Lỗi", "Không thể vô hiệu hóa Y tá này.");
                }
            } else {
                if (m_staffService->reactivateStaff(nurse->getAccountId())) {
                    loadNursesList();
                } else {
                    QMessageBox::warning(this, "Lỗi", "Không thể kích hoạt Y tá này.");
                }
            }
        });
    }
}
