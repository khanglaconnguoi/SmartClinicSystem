#include "ManageReceptionWidget.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/CommonEnums.h"
#include "../../model/SystemUser.h"
#include "ReceptionRegistrationDialog.h"
#include <QCoreApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>


ManageReceptionWidget::ManageReceptionWidget(
    std::shared_ptr<StaffService> staffService, QWidget *parent)
    : QWidget(parent), m_staffService(staffService), m_tblReception(nullptr) {
  buildUI();
}

QFrame *ManageReceptionWidget::makeCard(QWidget *parent) {
  QFrame *card = new QFrame(parent);
  card->setStyleSheet("QFrame { background-color: #FFFFFF; border-radius: "
                      "12px; border: 1px solid #E5E7EB; }");
  return card;
}

void ManageReceptionWidget::buildUI() {
  QVBoxLayout *pageLayout = new QVBoxLayout(this);
  pageLayout->setContentsMargins(30, 30, 30, 30);
  pageLayout->setSpacing(20);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *lblTitle = new QLabel("Quản lý Lễ tân");
  lblTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #111827;");
  headerLayout->addWidget(lblTitle);

  headerLayout->addStretch();

  QPushButton *btnAddNew = new QPushButton("Thêm Lễ tân");
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
      "QLineEdit { padding: 6px 12px; border: 1px solid #D1D5DB; "
      "border-radius: 6px; font-size: 13px; min-height: 32px; background: "
      "white; }"
      "QLineEdit:focus { border: 1px solid #2563EB; }");
  m_txtSearchKey->setFixedWidth(160);

  m_cbDepartmentFilter = new QComboBox(filterCard);
  m_cbDepartmentFilter->setStyleSheet(
      "QComboBox { padding: 6px 12px; border: 1px solid #D1D5DB; "
      "border-radius: 6px; font-size: 13px; min-height: 32px; background: "
      "white; }"
      "QComboBox:focus { border: 1px solid #2563EB; }");
  m_cbDepartmentFilter->addItem("Tất cả khoa", -1);
  for (const auto &pair : DepartmentText::getList()) {
    int depId = pair.second.split(" - ").first().toInt();
    m_cbDepartmentFilter->addItem(pair.second, depId);
  }

  m_cbShiftFilter = new QComboBox(filterCard);
  m_cbShiftFilter->setStyleSheet(
      "QComboBox { padding: 6px 12px; border: 1px solid #D1D5DB; "
      "border-radius: 6px; font-size: 13px; min-height: 32px; background: "
      "white; }"
      "QComboBox:focus { border: 1px solid #2563EB; }");
  m_cbShiftFilter->addItem("Tất cả ca trực", "");
  for (const auto &pair : ShiftText::getList()) {
    m_cbShiftFilter->addItem(pair.second, pair.first);
  }

  m_cbStatusFilter = new QComboBox(filterCard);
  m_cbStatusFilter->setStyleSheet(
      "QComboBox { padding: 6px 12px; border: 1px solid #D1D5DB; "
      "border-radius: 6px; font-size: 13px; min-height: 32px; background: "
      "white; }"
      "QComboBox:focus { border: 1px solid #2563EB; }");
  m_cbStatusFilter->addItem("Đang làm việc", true);
  m_cbStatusFilter->addItem("Tất cả trạng thái", false);

    QLabel *lblSearchIcon = new QLabel(filterCard);
    QPixmap searchPix(":/assets/icons/search_icon.png");
    if (searchPix.isNull()) {
#ifdef PROJECT_ROOT_DIR
        searchPix.load(QString::fromUtf8(PROJECT_ROOT_DIR) + "/assets/icons/search_icon.png");
#else
        searchPix.load(QCoreApplication::applicationDirPath() + "/assets/icons/search_icon.png");
#endif
    }

    if (!searchPix.isNull()) {
        lblSearchIcon->setPixmap(searchPix.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        lblSearchIcon->setText("Tìm kiếm:");
    }

  filterLayout->addWidget(m_txtSearchKey);
  filterLayout->addWidget(m_cbDepartmentFilter);
  filterLayout->addWidget(m_cbShiftFilter);
  filterLayout->addWidget(m_cbStatusFilter);
  filterLayout->addWidget(m_btnResetFilters);
  filterLayout->addStretch();
  pageLayout->addWidget(filterCard);

  // Table Card
  QFrame *tableCard = makeCard(this);
  QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
  cardLayout->setContentsMargins(0, 0, 0, 0);

  m_tblReception = new QTableWidget(tableCard);
  m_tblReception->setColumnCount(4);
  m_tblReception->setHorizontalHeaderLabels(
      {"Mã NV", "Họ Tên", "Trạng thái", "Thao tác"});
  m_tblReception->horizontalHeader()->setStretchLastSection(false);
  m_tblReception->horizontalHeader()->setSectionResizeMode(0,
                                                           QHeaderView::Fixed);
  m_tblReception->horizontalHeader()->resizeSection(0, 150);
  m_tblReception->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::Stretch);
  m_tblReception->horizontalHeader()->setSectionResizeMode(2,
                                                           QHeaderView::Fixed);
  m_tblReception->horizontalHeader()->resizeSection(2, 150);
  m_tblReception->horizontalHeader()->setSectionResizeMode(3,
                                                           QHeaderView::Fixed);
  m_tblReception->horizontalHeader()->resizeSection(3, 320);

  m_tblReception->verticalHeader()->setDefaultSectionSize(46);
  m_tblReception->verticalHeader()->setVisible(false);
  m_tblReception->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tblReception->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tblReception->setFocusPolicy(Qt::NoFocus);
  m_tblReception->setStyleSheet(
      "QTableWidget { border: none; outline: none; gridline-color: #E2E8F0; font-size: 13px; "
      "background-color: white; color: #0F172A; }"
      "QTableWidget::item { outline: none; border: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }"
      "QHeaderView::section { background-color: #F8FAFC; padding: 10px; "
      "font-weight: bold; border: none; border-bottom: 1px solid #E2E8F0; "
      "color: #1E293B; }");

  // Pagination layout
  QHBoxLayout *paginationLayout = new QHBoxLayout();
  paginationLayout->setContentsMargins(15, 10, 15, 15);

  m_btnPrevPage = new QPushButton("Trang trước", tableCard);
  m_btnPrevPage->setCursor(Qt::PointingHandCursor);
  m_btnPrevPage->setStyleSheet(
      "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: "
      "6px; padding: 6px 12px; font-weight: bold; border: none; min-height: "
      "32px; }"
      "QPushButton:hover { background-color: #CBD5E1; }"
      "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }");

  m_lblPageInfo = new QLabel(
      QString("%1 / %2").arg(m_currentPage).arg(m_totalPages), tableCard);
  m_lblPageInfo->setStyleSheet(
      "background: transparent; border: none; font-size: 13px; font-weight: "
      "bold; color: #475569;");

  m_btnNextPage = new QPushButton("Trang sau", tableCard);
  m_btnNextPage->setCursor(Qt::PointingHandCursor);
  m_btnNextPage->setStyleSheet(
      "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: "
      "6px; padding: 6px 12px; font-weight: bold; border: none; min-height: "
      "32px; }"
      "QPushButton:hover { background-color: #CBD5E1; }"
      "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }");

  paginationLayout->addWidget(m_btnPrevPage);
  paginationLayout->addStretch();
  paginationLayout->addWidget(m_lblPageInfo);
  paginationLayout->addStretch();
  paginationLayout->addWidget(m_btnNextPage);

  cardLayout->addWidget(m_tblReception);
  cardLayout->addLayout(paginationLayout);
  pageLayout->addWidget(tableCard);

  // Connections
  connect(btnAddNew, &QPushButton::clicked, this,
          &ManageReceptionWidget::showAddReceptionDialog);
  connect(m_txtSearchKey, &QLineEdit::textChanged, this,
          &ManageReceptionWidget::handleFilterChanged);
  connect(m_cbDepartmentFilter,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ManageReceptionWidget::handleFilterChanged);
  connect(m_cbShiftFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ManageReceptionWidget::handleFilterChanged);
  connect(m_cbStatusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ManageReceptionWidget::handleFilterChanged);
  connect(m_btnResetFilters, &QPushButton::clicked, this,
          &ManageReceptionWidget::handleResetFilters);
  connect(m_btnPrevPage, &QPushButton::clicked, this,
          &ManageReceptionWidget::handlePrevPage);
  connect(m_btnNextPage, &QPushButton::clicked, this,
          &ManageReceptionWidget::handleNextPage);

  loadReceptionList();
}

void ManageReceptionWidget::showAddReceptionDialog() {
  ReceptionRegistrationDialog dialog(m_staffService, this);
  if (dialog.exec() == QDialog::Accepted) {
    loadReceptionList();
  }
}

void ManageReceptionWidget::showEditReceptionDialog(
    std::shared_ptr<SystemUser> reception) {
  auto profile = m_staffService->getOwnProfile(reception->getAccountId());
  if (profile) {
    ReceptionRegistrationDialog dialog(m_staffService, this);
    dialog.loadReceptionistData(profile.get());
    if (dialog.exec() == QDialog::Accepted) {
      loadReceptionList();
    }
  } else {
    QMessageBox::warning(this, "Lỗi",
                         "Không thể lấy thông tin chi tiết Lễ tân.");
  }
}

namespace {
constexpr int PAGE_SIZE = 10;
}

void ManageReceptionWidget::loadReceptionList() {
  if (!m_tblReception || !m_staffService)
    return;
  m_tblReception->setRowCount(0);

  ReceptionistSearchCriteria criteria;
  criteria.searchKey = m_txtSearchKey->text().trimmed();
  criteria.departmentId = m_cbDepartmentFilter->currentData().toInt();
  criteria.shift = m_cbShiftFilter->currentData().toString();
  criteria.onlyActive = m_cbStatusFilter->currentData().toBool();
  criteria.includeDeleted = false;
  criteria.page = m_currentPage;
  criteria.pageSize = PAGE_SIZE;

  auto result = m_staffService->searchReceptionistsPaged(criteria);
  auto receptionists = result.items;

  m_totalPages = result.totalPages();
  if (m_totalPages < 1)
    m_totalPages = 1;
  m_lblPageInfo->setText(
      QString("%1 / %2").arg(m_currentPage).arg(m_totalPages));
  m_btnPrevPage->setEnabled(result.hasPrev());
  m_btnNextPage->setEnabled(result.hasNext());

  for (int i = 0; i < receptionists.size(); ++i) {
    auto receptionist = receptionists[i];
    if (!receptionist)
      continue;

    m_tblReception->insertRow(i);

    QTableWidgetItem *itemCode =
        new QTableWidgetItem(receptionist->getStaffCode());
    itemCode->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemName =
        new QTableWidgetItem(receptionist->getFullName());
    itemName->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemStatus = new QTableWidgetItem(
        receptionist->isActive() ? "Hoạt động" : "Nghỉ việc");
    if (receptionist->isActive()) {
      itemStatus->setForeground(QBrush(QColor("#059669")));
    } else {
      itemStatus->setForeground(QBrush(QColor("#DC2626")));
    }

    m_tblReception->setItem(i, 0, itemCode);
    m_tblReception->setItem(i, 1, itemName);
    m_tblReception->setItem(i, 2, itemStatus);

    QWidget *actionWidget = new QWidget();
    actionWidget->setStyleSheet("background: transparent;");
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(4, 4, 4, 4);
    actionLayout->setSpacing(8);

    QPushButton *btnEdit = new QPushButton("Xem chi tiết");
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setStyleSheet(
        "QPushButton { color: #2563EB; border: 1px solid #2563EB; padding: 6px "
        "12px; border-radius: 6px; background-color: white; font-weight: bold; "
        "} QPushButton:hover "
        "{ background-color: #EFF6FF; }");

    QPushButton *btnResetPwd = new QPushButton("Reset MK");
    btnResetPwd->setCursor(Qt::PointingHandCursor);
    btnResetPwd->setStyleSheet(
        "QPushButton { color: #D97706; border: 1px solid #D97706; padding: 6px "
        "12px; border-radius: 6px; background-color: white; font-weight: bold; "
        "} QPushButton:hover "
        "{ background-color: #FEF3C7; }");

    QPushButton *btnDeactivate =
        new QPushButton(receptionist->isActive() ? "Vô hiệu hóa" : "Kích hoạt");
    btnDeactivate->setCursor(Qt::PointingHandCursor);
    if (receptionist->isActive()) {
      btnDeactivate->setStyleSheet(
          "QPushButton { color: #DC2626; border: 1px solid #DC2626; padding: "
          "6px "
          "12px; border-radius: 6px; background-color: white; font-weight: "
          "bold; } QPushButton:hover "
          "{ background-color: #FEE2E2; }");
    } else {
      btnDeactivate->setStyleSheet(
          "QPushButton { color: #22C55E; border: 1px solid #22C55E; padding: "
          "6px "
          "12px; border-radius: 6px; background-color: white; font-weight: "
          "bold; } QPushButton:hover "
          "{ background-color: #DCFCE7; }");
    }

    actionLayout->addWidget(btnEdit);
    actionLayout->addWidget(btnResetPwd);
    actionLayout->addWidget(btnDeactivate);
    m_tblReception->setCellWidget(i, 3, actionWidget);

    connect(btnEdit, &QPushButton::clicked, this,
            [this, receptionist]() { showEditReceptionDialog(receptionist); });

    connect(btnResetPwd, &QPushButton::clicked, this, [this, receptionist]() {
      auto confirm = QMessageBox::question(
          this, "Xác nhận Reset Mật khẩu",
          QString("Bạn có chắc chắn muốn reset mật khẩu cho Lễ tân %1 (%2)?")
              .arg(receptionist->getFullName(), receptionist->getStaffCode()),
          QMessageBox::Yes | QMessageBox::No);
      if (confirm == QMessageBox::Yes) {
        auto res = m_staffService->resetPassword(receptionist->getAccountId());
        if (res.result) {
          QMessageBox::information(
              this, "Reset Mật khẩu thành công",
              QString("Mật khẩu mới cho Lễ tân %1 (%2) là:\n\n%3\n\nTài khoản "
                      "sẽ yêu cầu đổi mật khẩu khi đăng nhập lần tiếp theo.")
                  .arg(receptionist->getFullName(),
                       receptionist->getStaffCode(), res.newPassword));
        } else {
          QMessageBox::warning(this, "Lỗi",
                               "Không thể reset mật khẩu cho Lễ tân này.");
        }
      }
    });

    connect(btnDeactivate, &QPushButton::clicked, this, [this, receptionist]() {
      if (receptionist->isActive()) {
        if (m_staffService->deactivateStaff(receptionist->getAccountId())) {
          loadReceptionList();
        } else {
          QMessageBox::warning(this, "Lỗi",
                               "Không thể vô hiệu hóa Lễ tân này.");
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

void ManageReceptionWidget::handleFilterChanged() {
  m_currentPage = 1;
  loadReceptionList();
}

void ManageReceptionWidget::handleResetFilters() {
  m_txtSearchKey->clear();
  m_cbDepartmentFilter->setCurrentIndex(0);
  m_cbShiftFilter->setCurrentIndex(0);
  m_cbStatusFilter->setCurrentIndex(0);
  m_currentPage = 1;
  loadReceptionList();
}

void ManageReceptionWidget::handlePrevPage() {
  if (m_currentPage > 1) {
    m_currentPage--;
    loadReceptionList();
  }
}

void ManageReceptionWidget::handleNextPage() {
  if (m_currentPage < m_totalPages) {
    m_currentPage++;
    loadReceptionList();
  }
}
