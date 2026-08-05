#include "ManageNursesWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QCoreApplication>
#include "NurseRegistrationDialog.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/SystemUser.h"
#include "../../model/CommonEnums.h"

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

    // Filter Bar Card
    QFrame *filterCard = makeCard(this);
    QHBoxLayout *filterLayout = new QHBoxLayout(filterCard);
    filterLayout->setContentsMargins(15, 10, 15, 10);
    filterLayout->setSpacing(10);

    m_txtSearchKey = new QLineEdit(filterCard);
    m_txtSearchKey->setPlaceholderText("Mã NV, Họ tên...");
    m_txtSearchKey->setStyleSheet(
        "QLineEdit { padding: 6px 12px; border: 1px solid #D1D5DB; border-radius: 6px; font-size: 13px; min-height: 32px; background: white; }"
        "QLineEdit:focus { border: 1px solid #2563EB; }");
    m_txtSearchKey->setFixedWidth(160);

    m_cbLevelFilter = new QComboBox(filterCard);
    m_cbLevelFilter->setStyleSheet(
        "QComboBox { padding: 6px 12px; border: 1px solid #D1D5DB; border-radius: 6px; font-size: 13px; min-height: 32px; background: white; }"
        "QComboBox:focus { border: 1px solid #2563EB; }");
    m_cbLevelFilter->addItem("Tất cả cấp bậc", "");
    m_cbLevelFilter->addItems({"JUNIOR", "SENIOR", "HEAD"});

    m_cbDepartmentFilter = new QComboBox(filterCard);
    m_cbDepartmentFilter->setStyleSheet(
        "QComboBox { padding: 6px 12px; border: 1px solid #D1D5DB; border-radius: 6px; font-size: 13px; min-height: 32px; background: white; }"
        "QComboBox:focus { border: 1px solid #2563EB; }");
    m_cbDepartmentFilter->addItem("Tất cả khoa", -1);
    for (const auto& pair : DepartmentText::getList()) {
        int depId = pair.second.split(" - ").first().toInt();
        m_cbDepartmentFilter->addItem(pair.second, depId);
    }

    m_cbShiftFilter = new QComboBox(filterCard);
    m_cbShiftFilter->setStyleSheet(
        "QComboBox { padding: 6px 12px; border: 1px solid #D1D5DB; border-radius: 6px; font-size: 13px; min-height: 32px; background: white; }"
        "QComboBox:focus { border: 1px solid #2563EB; }");
    m_cbShiftFilter->addItem("Tất cả ca trực", "");
    for (const auto& pair : ShiftText::getList()) {
        m_cbShiftFilter->addItem(pair.second, pair.first);
    }

    m_cbStatusFilter = new QComboBox(filterCard);
    m_cbStatusFilter->setStyleSheet(
        "QComboBox { padding: 6px 12px; border: 1px solid #D1D5DB; border-radius: 6px; font-size: 13px; min-height: 32px; background: white; }"
        "QComboBox:focus { border: 1px solid #2563EB; }");
    m_cbStatusFilter->addItem("Đang làm việc", true);
    m_cbStatusFilter->addItem("Tất cả trạng thái", false);

    m_btnResetFilters = new QPushButton("Đặt lại", filterCard);
    m_btnResetFilters->setCursor(Qt::PointingHandCursor);
    m_btnResetFilters->setStyleSheet(
        "QPushButton { background-color: #EF4444; color: white; border-radius: 6px; padding: 6px 15px; font-weight: bold; border: none; min-height: 32px; }"
        "QPushButton:hover { background-color: #DC2626; }");

    QLabel *lblSearchIcon = new QLabel(filterCard);
#ifdef PROJECT_ROOT_DIR
    QString searchIconPath = QString::fromUtf8(PROJECT_ROOT_DIR) + "/assets/icons/search_icon.png";
#else
    QString searchIconPath = QCoreApplication::applicationDirPath() + "/assets/icons/search_icon.png";
#endif
    QPixmap searchPix(searchIconPath);
    if (!searchPix.isNull()) {
        lblSearchIcon->setPixmap(searchPix.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        lblSearchIcon->setText("Tìm kiếm:");
    }

    filterLayout->addWidget(lblSearchIcon);
    filterLayout->addWidget(m_txtSearchKey);
    filterLayout->addWidget(m_cbLevelFilter);
    filterLayout->addWidget(m_cbDepartmentFilter);
    filterLayout->addWidget(m_cbShiftFilter);
    filterLayout->addWidget(m_cbStatusFilter);
    filterLayout->addWidget(m_btnResetFilters);
    filterLayout->addStretch();
    pageLayout->addWidget(filterCard);

    // Card bao bọc Table
    QFrame *tableCard = makeCard(this);
    QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(0, 0, 0, 0);

    m_tblNurses = new QTableWidget(0, 5, tableCard);
    m_tblNurses->setHorizontalHeaderLabels(
        {"Mã NV", "Họ Tên", "Cấp bậc", "Trạng thái", "Thao tác"});
    m_tblNurses->horizontalHeader()->setStretchLastSection(false);
    m_tblNurses->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_tblNurses->horizontalHeader()->resizeSection(0, 150);
    m_tblNurses->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tblNurses->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_tblNurses->horizontalHeader()->resizeSection(2, 180);
    m_tblNurses->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_tblNurses->horizontalHeader()->resizeSection(3, 150);
    m_tblNurses->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    m_tblNurses->horizontalHeader()->resizeSection(4, 300);

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

    // Pagination layout
    QHBoxLayout *paginationLayout = new QHBoxLayout();
    paginationLayout->setContentsMargins(15, 10, 15, 15);
    
    m_btnPrevPage = new QPushButton("Trang trước", tableCard);
    m_btnPrevPage->setCursor(Qt::PointingHandCursor);
    m_btnPrevPage->setStyleSheet(
        "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: 6px; padding: 6px 12px; font-weight: bold; border: none; min-height: 32px; }"
        "QPushButton:hover { background-color: #CBD5E1; }"
        "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }");

    m_lblPageInfo = new QLabel("Trang 1 / 1", tableCard);
    m_lblPageInfo->setStyleSheet("font-size: 13px; font-weight: bold; color: #475569;");

    m_btnNextPage = new QPushButton("Trang sau", tableCard);
    m_btnNextPage->setCursor(Qt::PointingHandCursor);
    m_btnNextPage->setStyleSheet(
        "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: 6px; padding: 6px 12px; font-weight: bold; border: none; min-height: 32px; }"
        "QPushButton:hover { background-color: #CBD5E1; }"
        "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }");

    paginationLayout->addWidget(m_btnPrevPage);
    paginationLayout->addStretch();
    paginationLayout->addWidget(m_lblPageInfo);
    paginationLayout->addStretch();
    paginationLayout->addWidget(m_btnNextPage);

    cardLayout->addWidget(m_tblNurses);
    cardLayout->addLayout(paginationLayout);
    pageLayout->addWidget(tableCard);

    // Connections
    connect(btnAddNew, &QPushButton::clicked, this, &ManageNursesWidget::showAddNurseDialog);
    connect(m_txtSearchKey, &QLineEdit::textChanged, this, &ManageNursesWidget::handleFilterChanged);
    connect(m_cbLevelFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManageNursesWidget::handleFilterChanged);
    connect(m_cbDepartmentFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManageNursesWidget::handleFilterChanged);
    connect(m_cbShiftFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManageNursesWidget::handleFilterChanged);
    connect(m_cbStatusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManageNursesWidget::handleFilterChanged);
    connect(m_btnResetFilters, &QPushButton::clicked, this, &ManageNursesWidget::handleResetFilters);
    connect(m_btnPrevPage, &QPushButton::clicked, this, &ManageNursesWidget::handlePrevPage);
    connect(m_btnNextPage, &QPushButton::clicked, this, &ManageNursesWidget::handleNextPage);

    // Load data
    loadNursesList();
}

namespace {
constexpr int PAGE_SIZE = 10;
}

void ManageNursesWidget::loadNursesList() {
    if (!m_tblNurses || !m_staffService) return;
    m_tblNurses->setRowCount(0);

    NurseSearchCriteria criteria;
    criteria.searchKey = m_txtSearchKey->text().trimmed();
    criteria.nurseLevel = m_cbLevelFilter->currentText();
    if (criteria.nurseLevel == "Tất cả cấp bậc") {
        criteria.nurseLevel = "";
    }
    criteria.departmentId = m_cbDepartmentFilter->currentData().toInt();
    criteria.shift = m_cbShiftFilter->currentData().toString();
    criteria.onlyActive = m_cbStatusFilter->currentData().toBool();
    criteria.includeDeleted = false;
    criteria.page = m_currentPage;
    criteria.pageSize = PAGE_SIZE;

    auto result = m_staffService->searchNursesPaged(criteria);
    auto nurses = result.items;

    m_totalPages = result.totalPages();
    if (m_totalPages < 1) m_totalPages = 1;
    m_lblPageInfo->setText(QString("%1 / %2").arg(m_currentPage).arg(m_totalPages));
    m_btnPrevPage->setEnabled(result.hasPrev());
    m_btnNextPage->setEnabled(result.hasNext());

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
        m_tblNurses->setItem(i, 3, itemStatus);

        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setSpacing(8);

        QPushButton *btnEdit = new QPushButton("Sửa");
        btnEdit->setCursor(Qt::PointingHandCursor);
        btnEdit->setStyleSheet(
            "QPushButton { color: #2563EB; border: 1px solid #2563EB; padding: 6px "
            "12px; border-radius: 6px; background-color: white; font-weight: bold; } QPushButton:hover "
            "{ background-color: #EFF6FF; }");

        QPushButton *btnResetPwd = new QPushButton("Reset MK");
        btnResetPwd->setCursor(Qt::PointingHandCursor);
        btnResetPwd->setStyleSheet(
            "QPushButton { color: #D97706; border: 1px solid #D97706; padding: 6px "
            "12px; border-radius: 6px; background-color: white; font-weight: bold; } QPushButton:hover "
            "{ background-color: #FEF3C7; }");

        QPushButton *btnDeactivate = new QPushButton(nurse->isActive() ? "Vô hiệu hóa" : "Kích hoạt");
        btnDeactivate->setCursor(Qt::PointingHandCursor);
        if (nurse->isActive()) {
            btnDeactivate->setStyleSheet(
                "QPushButton { color: #DC2626; border: 1px solid #DC2626; padding: 6px "
                "12px; border-radius: 6px; background-color: white; font-weight: bold; } QPushButton:hover "
                "{ background-color: #FEE2E2; }");
        } else {
            btnDeactivate->setStyleSheet(
                "QPushButton { color: #22C55E; border: 1px solid #22C55E; padding: 6px "
                "12px; border-radius: 6px; background-color: white; font-weight: bold; } QPushButton:hover "
                "{ background-color: #DCFCE7; }");
        }

        actionLayout->addWidget(btnEdit);
        actionLayout->addWidget(btnResetPwd);
        actionLayout->addWidget(btnDeactivate);
        m_tblNurses->setCellWidget(i, 4, actionWidget);

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

void ManageNursesWidget::handleFilterChanged() {
    m_currentPage = 1;
    loadNursesList();
}

void ManageNursesWidget::handleResetFilters() {
    m_txtSearchKey->clear();
    m_cbLevelFilter->setCurrentIndex(0);
    m_cbDepartmentFilter->setCurrentIndex(0);
    m_cbShiftFilter->setCurrentIndex(0);
    m_cbStatusFilter->setCurrentIndex(0);
    m_currentPage = 1;
    loadNursesList();
}

void ManageNursesWidget::handlePrevPage() {
    if (m_currentPage > 1) {
        m_currentPage--;
        loadNursesList();
    }
}

void ManageNursesWidget::handleNextPage() {
    if (m_currentPage < m_totalPages) {
        m_currentPage++;
        loadNursesList();
    }
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
