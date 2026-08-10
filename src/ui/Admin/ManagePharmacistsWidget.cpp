#include "ManagePharmacistsWidget.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/CommonEnums.h"
#include "PharmacistRegistrationDialog.h"
#include <QCoreApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>


ManagePharmacistsWidget::ManagePharmacistsWidget(
    std::shared_ptr<StaffService> staffService, QWidget *parent)
    : QWidget(parent), m_staffService(staffService), m_tblPharmacists(nullptr) {
  buildUI();
}

QFrame *ManagePharmacistsWidget::makeCard(QWidget *parent) {
  QFrame *card = new QFrame(parent);
  card->setObjectName("DashboardCard");
  card->setStyleSheet("QFrame#DashboardCard {"
                      "   background-color: #FFFFFF;"
                      "   border: 1px solid #E5E7EB;"
                      "   border-radius: 12px;"
                      "}");
  return card;
}

void ManagePharmacistsWidget::buildUI() {
  QVBoxLayout *pageLayout = new QVBoxLayout(this);
  pageLayout->setContentsMargins(30, 30, 30, 30);
  pageLayout->setSpacing(20);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *lblPageTitle = new QLabel("QUẢN LÝ DƯỢC SĨ", this);
  lblPageTitle->setStyleSheet(
      "font-size: 22px; font-weight: bold; color: #000000;");
  headerLayout->addWidget(lblPageTitle);

  headerLayout->addStretch();

  QPushButton *btnAddNew = new QPushButton("Thêm Dược sĩ", this);
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

  m_txtSectionFilter = new QLineEdit(filterCard);
  m_txtSectionFilter->setPlaceholderText("Khu vực thuốc...");
  m_txtSectionFilter->setStyleSheet(
      "QLineEdit { padding: 6px 12px; border: 1px solid #D1D5DB; "
      "border-radius: 6px; font-size: 13px; min-height: 32px; background: "
      "white; }"
      "QLineEdit:focus { border: 1px solid #2563EB; }");
  m_txtSectionFilter->setFixedWidth(140);

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

  m_btnResetFilters = new QPushButton("Đặt lại", filterCard);
  m_btnResetFilters->setCursor(Qt::PointingHandCursor);
  m_btnResetFilters->setStyleSheet(
      "QPushButton { background-color: #6B7280; color: white; border: none; "
      "border-radius: 6px; padding: 4px 12px; font-weight: bold; min-height: 28px; "
      "font-size: 12px; } QPushButton:hover { background-color: #4B5563; }");

  filterLayout->addWidget(m_txtSearchKey);
  filterLayout->addWidget(m_txtSectionFilter);
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

  m_tblPharmacists = new QTableWidget(0, 4, tableCard);
  m_tblPharmacists->setHorizontalHeaderLabels(
      {"Mã NV", "Họ Tên", "Trạng thái", "Thao tác"});
  m_tblPharmacists->horizontalHeader()->setStretchLastSection(false);
  m_tblPharmacists->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::Fixed);
  m_tblPharmacists->horizontalHeader()->resizeSection(0, 150);
  m_tblPharmacists->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::Stretch);
  m_tblPharmacists->horizontalHeader()->setSectionResizeMode(
      2, QHeaderView::Fixed);
  m_tblPharmacists->horizontalHeader()->resizeSection(2, 150);
  m_tblPharmacists->horizontalHeader()->setSectionResizeMode(
      3, QHeaderView::Fixed);
  m_tblPharmacists->horizontalHeader()->resizeSection(3, 320);

  m_tblPharmacists->verticalHeader()->setDefaultSectionSize(46);
  m_tblPharmacists->verticalHeader()->setVisible(false);
  m_tblPharmacists->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tblPharmacists->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tblPharmacists->setFocusPolicy(Qt::NoFocus);
  m_tblPharmacists->setStyleSheet(
      "QTableWidget { border: none; outline: none; gridline-color: #E2E8F0; font-size: 13px; "
      "background-color: white; color: #0F172A; }"
      "QTableWidget::item { outline: none; border: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }"
      "QHeaderView::section { background-color: #EFF6FF; padding: 10px; "
      "font-weight: bold; border: none; border-bottom: 2px solid #BFDBFE; "
      "color: #1E40AF; }");

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

  cardLayout->addWidget(m_tblPharmacists);
  cardLayout->addLayout(paginationLayout);
  pageLayout->addWidget(tableCard);

  // Connections
  connect(btnAddNew, &QPushButton::clicked, this,
          &ManagePharmacistsWidget::showAddPharmacistDialog);
  connect(m_txtSearchKey, &QLineEdit::textChanged, this,
          &ManagePharmacistsWidget::handleFilterChanged);
  connect(m_txtSectionFilter, &QLineEdit::textChanged, this,
          &ManagePharmacistsWidget::handleFilterChanged);
  connect(m_cbDepartmentFilter,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ManagePharmacistsWidget::handleFilterChanged);
  connect(m_cbShiftFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ManagePharmacistsWidget::handleFilterChanged);
  connect(m_cbStatusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ManagePharmacistsWidget::handleFilterChanged);
  connect(m_btnResetFilters, &QPushButton::clicked, this,
          &ManagePharmacistsWidget::handleResetFilters);
  connect(m_btnPrevPage, &QPushButton::clicked, this,
          &ManagePharmacistsWidget::handlePrevPage);
  connect(m_btnNextPage, &QPushButton::clicked, this,
          &ManagePharmacistsWidget::handleNextPage);

  // Load data
  loadPharmacistsList();
}

namespace {
constexpr int PAGE_SIZE = 10;
}

void ManagePharmacistsWidget::loadPharmacistsList() {
  if (!m_tblPharmacists || !m_staffService)
    return;
  m_tblPharmacists->setRowCount(0);

  PharmacistSearchCriteria criteria;
  criteria.searchKey = m_txtSearchKey->text().trimmed();
  criteria.pharmacySection = m_txtSectionFilter->text().trimmed();
  criteria.departmentId = m_cbDepartmentFilter->currentData().toInt();
  criteria.shift = m_cbShiftFilter->currentData().toString();
  criteria.onlyActive = m_cbStatusFilter->currentData().toBool();
  criteria.includeDeleted = false;
  criteria.page = m_currentPage;
  criteria.pageSize = PAGE_SIZE;

  auto result = m_staffService->searchPharmacistsPaged(criteria);
  QList<std::shared_ptr<SystemUser>> pharmacists = result.items;

  m_totalPages = result.totalPages();
  if (m_totalPages < 1)
    m_totalPages = 1;
  m_lblPageInfo->setText(
      QString("%1 / %2").arg(m_currentPage).arg(m_totalPages));
  m_btnPrevPage->setEnabled(result.hasPrev());
  m_btnNextPage->setEnabled(result.hasNext());

  for (int i = 0; i < pharmacists.size(); ++i) {
    auto doc = std::dynamic_pointer_cast<Pharmacist>(pharmacists[i]);
    if (!doc)
      continue;

    int row = m_tblPharmacists->rowCount();
    m_tblPharmacists->insertRow(row);

    QTableWidgetItem *itemCode = new QTableWidgetItem(doc->getStaffCode());
    itemCode->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemName = new QTableWidgetItem(doc->getFullName());
    itemName->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemStatus =
        new QTableWidgetItem(doc->isActive() ? "Hoạt động" : "Nghỉ việc");
    if (doc->isActive()) {
      itemStatus->setForeground(QBrush(QColor("#059669")));
    } else {
      itemStatus->setForeground(QBrush(QColor("#DC2626")));
    }

    m_tblPharmacists->setItem(row, 0, itemCode);
    m_tblPharmacists->setItem(row, 1, itemName);
    m_tblPharmacists->setItem(row, 2, itemStatus);

    QWidget *actionWidget = new QWidget();
    actionWidget->setStyleSheet("background: transparent;");
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(4, 4, 4, 4);
    actionLayout->setSpacing(8);

    QPushButton *btnEdit = new QPushButton("Xem chi tiết");
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setStyleSheet(
        "QPushButton { background-color: #2563EB; color: white; border: none; padding: 6px "
        "12px; border-radius: 6px; font-weight: bold; "
        "} QPushButton:hover "
        "{ background-color: #1D4ED8; }");

    QPushButton *btnResetPwd = new QPushButton("Reset MK");
    btnResetPwd->setCursor(Qt::PointingHandCursor);
    btnResetPwd->setStyleSheet(
        "QPushButton { background-color: #D97706; color: white; border: none; padding: 6px "
        "12px; border-radius: 6px; font-weight: bold; "
        "} QPushButton:hover "
        "{ background-color: #B45309; }");

    QPushButton *btnDeactivate =
        new QPushButton(doc->isActive() ? "Vô hiệu hóa" : "Kích hoạt");
    btnDeactivate->setCursor(Qt::PointingHandCursor);
    if (doc->isActive()) {
      btnDeactivate->setStyleSheet(
          "QPushButton { background-color: #DC2626; color: white; border: none; padding: "
          "6px "
          "12px; border-radius: 6px; font-weight: "
          "bold; } QPushButton:hover "
          "{ background-color: #B91C1C; }");
    } else {
      btnDeactivate->setStyleSheet(
          "QPushButton { background-color: #16A34A; color: white; border: none; padding: "
          "6px "
          "12px; border-radius: 6px; font-weight: "
          "bold; } QPushButton:hover "
          "{ background-color: #15803D; }");
    }

    actionLayout->addWidget(btnEdit);
    actionLayout->addWidget(btnResetPwd);
    actionLayout->addWidget(btnDeactivate);
    m_tblPharmacists->setCellWidget(row, 3, actionWidget);

    connect(btnEdit, &QPushButton::clicked, this,
            [this, doc]() { showEditPharmacistDialog(doc); });

    connect(btnResetPwd, &QPushButton::clicked, this, [this, doc]() {
      auto confirm = QMessageBox::question(
          this, "Xác nhận Reset Mật khẩu",
          QString("Bạn có chắc chắn muốn reset mật khẩu cho Dược sĩ %1 (%2)?")
              .arg(doc->getFullName(), doc->getStaffCode()),
          QMessageBox::Yes | QMessageBox::No);
      if (confirm == QMessageBox::Yes) {
        auto res = m_staffService->resetPassword(doc->getAccountId());
        if (res.result) {
          QDialog successDialog(this);
          successDialog.setWindowTitle("Reset Mật khẩu thành công");
          successDialog.setMinimumWidth(380);
          successDialog.setStyleSheet(
              "QDialog { background-color: #FFFFFF; border-radius: 8px; }"
              "QLabel { color: #1F2937; font-size: 14px; }"
              "QLineEdit { background-color: #F9FAFB; border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 14px; font-weight: bold; color: #111827; }"
              "QPushButton { background-color: #2563EB; color: white; font-weight: bold; min-width: 100px; min-height: 35px; border-radius: 6px; border: none; font-size: 14px; }"
              "QPushButton:hover { background-color: #1D4ED8; }"
          );

          QVBoxLayout *dlgLayout = new QVBoxLayout(&successDialog);
          dlgLayout->setSpacing(15);
          dlgLayout->setContentsMargins(24, 24, 24, 20);

          QLabel *lblTitle = new QLabel(QString("Reset mật khẩu cho Dược sĩ %1 thành công!").arg(doc->getFullName()), &successDialog);
          lblTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #111827;");
          lblTitle->setWordWrap(true);
          dlgLayout->addWidget(lblTitle);

          QFormLayout *formLayout = new QFormLayout();
          formLayout->setSpacing(10);

          QLineEdit *txtStaffCode = new QLineEdit(doc->getStaffCode(), &successDialog);
          txtStaffCode->setReadOnly(true);
          QLabel *lblCode = new QLabel("Mã nhân viên:", &successDialog);
          lblCode->setStyleSheet("font-weight: 600; color: #374151;");
          formLayout->addRow(lblCode, txtStaffCode);

          QLineEdit *txtPassword = new QLineEdit(res.newPassword, &successDialog);
          txtPassword->setReadOnly(true);
          QLabel *lblPass = new QLabel("Mật khẩu mới:", &successDialog);
          lblPass->setStyleSheet("font-weight: 600; color: #374151;");
          formLayout->addRow(lblPass, txtPassword);

          dlgLayout->addLayout(formLayout);

          QLabel *lblNote = new QLabel("Tài khoản sẽ yêu cầu đổi mật khẩu khi đăng nhập lần tiếp theo.", &successDialog);
          lblNote->setStyleSheet("font-size: 13px; color: #6B7280; font-style: italic;");
          lblNote->setWordWrap(true);
          dlgLayout->addWidget(lblNote);

          QHBoxLayout *btnLayout = new QHBoxLayout();
          btnLayout->addStretch();
          QPushButton *btnOk = new QPushButton("OK", &successDialog);
          btnOk->setCursor(Qt::PointingHandCursor);
          connect(btnOk, &QPushButton::clicked, &successDialog, &QDialog::accept);
          btnLayout->addWidget(btnOk);
          btnLayout->addStretch();

          dlgLayout->addLayout(btnLayout);

          successDialog.exec();
        } else {
          QMessageBox::warning(this, "Lỗi",
                               "Không thể reset mật khẩu cho Dược sĩ này.");
        }
      }
    });

    connect(btnDeactivate, &QPushButton::clicked, this, [this, doc]() {
      if (doc->isActive()) {
        if (m_staffService->deactivateStaff(doc->getAccountId())) {
          loadPharmacistsList();
        } else {
          QMessageBox::warning(this, "Lỗi",
                               "Không thể vô hiệu hóa Dược sĩ này.");
        }
      } else {
        if (m_staffService->reactivateStaff(doc->getAccountId())) {
          loadPharmacistsList();
        } else {
          QMessageBox::warning(this, "Lỗi", "Không thể kích hoạt Dược sĩ này.");
        }
      }
    });
  }
}

void ManagePharmacistsWidget::handleFilterChanged() {
  m_currentPage = 1;
  loadPharmacistsList();
}

void ManagePharmacistsWidget::handleResetFilters() {
  m_txtSearchKey->clear();
  m_txtSectionFilter->clear();
  m_cbDepartmentFilter->setCurrentIndex(0);
  m_cbShiftFilter->setCurrentIndex(0);
  m_cbStatusFilter->setCurrentIndex(0);
  m_currentPage = 1;
  loadPharmacistsList();
}

void ManagePharmacistsWidget::handlePrevPage() {
  if (m_currentPage > 1) {
    m_currentPage--;
    loadPharmacistsList();
  }
}

void ManagePharmacistsWidget::handleNextPage() {
  if (m_currentPage < m_totalPages) {
    m_currentPage++;
    loadPharmacistsList();
  }
}

void ManagePharmacistsWidget::showAddPharmacistDialog() {
  PharmacistRegistrationDialog dialog(m_staffService, this);
  if (dialog.exec() == QDialog::Accepted) {
    loadPharmacistsList();
  }
}

void ManagePharmacistsWidget::showEditPharmacistDialog(
    std::shared_ptr<Pharmacist> doc) {
  auto profile = m_staffService->getOwnProfile(doc->getAccountId());
  auto pharmacistProfile = dynamic_cast<PharmacistProfileDTO *>(profile.get());
  if (pharmacistProfile) {
    PharmacistRegistrationDialog dialog(m_staffService, this);
    dialog.loadPharmacistData(pharmacistProfile);
    if (dialog.exec() == QDialog::Accepted) {
      loadPharmacistsList();
    }
  } else {
    QMessageBox::warning(this, "Lỗi",
                         "Không thể lấy thông tin chi tiết dược sĩ.");
  }
}
