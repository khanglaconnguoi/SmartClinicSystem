#include "ManageDoctorsWidget.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/CommonEnums.h"
#include "DoctorRegistrationDialog.h"
#include <QCoreApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>

ManageDoctorsWidget::ManageDoctorsWidget(
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<AppointmentService> appointmentService, QWidget *parent)
    : QWidget(parent), m_staffService(staffService),
      m_appointmentService(appointmentService), m_tblDoctors(nullptr) {
  buildUI();
}

QFrame *ManageDoctorsWidget::makeCard(QWidget *parent) {
  QFrame *card = new QFrame(parent);
  card->setObjectName("DashboardCard");
  card->setStyleSheet("QFrame#DashboardCard {"
                      "   background-color: #FFFFFF;"
                      "   border: 1px solid #E5E7EB;"
                      "   border-radius: 12px;"
                      "}");
  return card;
}

void ManageDoctorsWidget::buildUI() {
  QVBoxLayout *pageLayout = new QVBoxLayout(this);
  pageLayout->setContentsMargins(30, 30, 30, 30);
  pageLayout->setSpacing(20);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *lblPageTitle = new QLabel("QUẢN LÝ BÁC SĨ", this);
  lblPageTitle->setStyleSheet(
      "font-size: 22px; font-weight: bold; color: #000000;");
  headerLayout->addWidget(lblPageTitle);

  headerLayout->addStretch();

  QPushButton *btnAddNew = new QPushButton("Thêm Bác sĩ", this);
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

  m_cbSpecialtyFilter = new QComboBox(filterCard);
  m_cbSpecialtyFilter->setStyleSheet(
      "QComboBox { padding: 6px 12px; border: 1px solid #D1D5DB; "
      "border-radius: 6px; font-size: 13px; min-height: 32px; background: "
      "white; }"
      "QComboBox:focus { border: 1px solid #2563EB; }");
  m_cbSpecialtyFilter->addItem("Tất cả chuyên khoa", "");
  m_cbSpecialtyFilter->addItems(
      {"Nội khoa", "Ngoại khoa", "Nhi khoa", "Da liễu", "Răng Hàm Mặt"});

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
  filterLayout->addWidget(m_cbSpecialtyFilter);
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

  m_tblDoctors = new QTableWidget(0, 5, tableCard);
  m_tblDoctors->setHorizontalHeaderLabels(
      {"Mã NV", "Họ Tên", "Chuyên khoa", "Trạng thái", "Thao tác"});
  m_tblDoctors->horizontalHeader()->setStretchLastSection(false);
  m_tblDoctors->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  m_tblDoctors->horizontalHeader()->resizeSection(0, 150);
  m_tblDoctors->horizontalHeader()->setSectionResizeMode(1,
                                                         QHeaderView::Stretch);
  m_tblDoctors->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
  m_tblDoctors->horizontalHeader()->resizeSection(2, 180);
  m_tblDoctors->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
  m_tblDoctors->horizontalHeader()->resizeSection(3, 150);
  m_tblDoctors->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
  m_tblDoctors->horizontalHeader()->resizeSection(4, 320);

  m_tblDoctors->verticalHeader()->setDefaultSectionSize(46);
  m_tblDoctors->verticalHeader()->setVisible(false);
  m_tblDoctors->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tblDoctors->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tblDoctors->setFocusPolicy(Qt::NoFocus);
  m_tblDoctors->setStyleSheet(
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

  cardLayout->addWidget(m_tblDoctors);
  cardLayout->addLayout(paginationLayout);
  pageLayout->addWidget(tableCard);

  // Connections
  connect(btnAddNew, &QPushButton::clicked, this,
          &ManageDoctorsWidget::showAddDoctorDialog);
  connect(m_txtSearchKey, &QLineEdit::textChanged, this,
          &ManageDoctorsWidget::handleFilterChanged);
  connect(m_cbSpecialtyFilter,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ManageDoctorsWidget::handleFilterChanged);
  connect(m_cbDepartmentFilter,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ManageDoctorsWidget::handleFilterChanged);
  connect(m_cbShiftFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ManageDoctorsWidget::handleFilterChanged);
  connect(m_cbStatusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ManageDoctorsWidget::handleFilterChanged);
  connect(m_btnResetFilters, &QPushButton::clicked, this,
          &ManageDoctorsWidget::handleResetFilters);
  connect(m_btnPrevPage, &QPushButton::clicked, this,
          &ManageDoctorsWidget::handlePrevPage);
  connect(m_btnNextPage, &QPushButton::clicked, this,
          &ManageDoctorsWidget::handleNextPage);

  // Load data
  loadDoctorsList();
}

namespace {
constexpr int PAGE_SIZE = 10;
}

void ManageDoctorsWidget::loadDoctorsList() {
  if (!m_tblDoctors || !m_staffService)
    return;
  m_tblDoctors->setRowCount(0);

  DoctorSearchCriteria criteria;
  criteria.searchKey = m_txtSearchKey->text().trimmed();

  criteria.specialty = m_cbSpecialtyFilter->currentText();
  if (criteria.specialty == "Tất cả chuyên khoa") {
    criteria.specialty = "";
  }

  criteria.departmentId = m_cbDepartmentFilter->currentData().toInt();
  criteria.shift = m_cbShiftFilter->currentData().toString();
  criteria.onlyActive = m_cbStatusFilter->currentData().toBool();
  criteria.includeDeleted = false;
  criteria.page = m_currentPage;
  criteria.pageSize = PAGE_SIZE;

  auto result = m_staffService->searchDoctorsPaged(criteria);
  QList<std::shared_ptr<SystemUser>> doctors = result.items;

  m_totalPages = result.totalPages();
  if (m_totalPages < 1)
    m_totalPages = 1;
  m_lblPageInfo->setText(
      QString("%1 / %2").arg(m_currentPage).arg(m_totalPages));
  m_btnPrevPage->setEnabled(result.hasPrev());
  m_btnNextPage->setEnabled(result.hasNext());

  for (int i = 0; i < doctors.size(); ++i) {
    auto doc = std::dynamic_pointer_cast<Doctor>(doctors[i]);
    if (!doc)
      continue;

    int row = m_tblDoctors->rowCount();
    m_tblDoctors->insertRow(row);

    QTableWidgetItem *itemCode = new QTableWidgetItem(doc->getStaffCode());
    itemCode->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemName = new QTableWidgetItem(doc->getFullName());
    itemName->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemSpec = new QTableWidgetItem(doc->getSpecialty());
    itemSpec->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemStatus =
        new QTableWidgetItem(doc->isActive() ? "Hoạt động" : "Nghỉ việc");
    if (doc->isActive()) {
      itemStatus->setForeground(QBrush(QColor("#059669")));
    } else {
      itemStatus->setForeground(QBrush(QColor("#DC2626")));
    }

    m_tblDoctors->setItem(row, 0, itemCode);
    m_tblDoctors->setItem(row, 1, itemName);
    m_tblDoctors->setItem(row, 2, itemSpec);
    m_tblDoctors->setItem(row, 3, itemStatus);

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
        new QPushButton(doc->isActive() ? "Vô hiệu hóa" : "Kích hoạt");
    btnDeactivate->setCursor(Qt::PointingHandCursor);
    if (doc->isActive()) {
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
    m_tblDoctors->setCellWidget(row, 4, actionWidget);

    connect(btnEdit, &QPushButton::clicked, this,
            [this, doc]() { showEditDoctorDialog(doc); });

    connect(btnResetPwd, &QPushButton::clicked, this, [this, doc]() {
      auto confirm = QMessageBox::question(
          this, "Xác nhận Reset Mật khẩu",
          QString("Bạn có chắc chắn muốn reset mật khẩu cho Bác sĩ %1 (%2)?")
              .arg(doc->getFullName(), doc->getStaffCode()),
          QMessageBox::Yes | QMessageBox::No);
      if (confirm == QMessageBox::Yes) {
        auto res = m_staffService->resetPassword(doc->getAccountId());
        if (res.result) {
          QMessageBox::information(
              this, "Reset Mật khẩu thành công",
              QString("Mật khẩu mới cho Bác sĩ %1 (%2) là:\n\n%3\n\nTài khoản "
                      "sẽ yêu cầu đổi mật khẩu khi đăng nhập lần tiếp theo.")
                  .arg(doc->getFullName(), doc->getStaffCode(),
                       res.newPassword));
        } else {
          QMessageBox::warning(this, "Lỗi",
                               "Không thể reset mật khẩu cho Bác sĩ này.");
        }
      }
    });

    connect(btnDeactivate, &QPushButton::clicked, this, [this, doc]() {
      if (doc->isActive()) {
        if (m_staffService->deactivateStaff(doc->getAccountId())) {
          loadDoctorsList();
        } else {
          QMessageBox::warning(this, "Lỗi",
                               "Không thể vô hiệu hóa Bác sĩ này.");
        }
      } else {
        if (m_staffService->reactivateStaff(doc->getAccountId())) {
          loadDoctorsList();
        } else {
          QMessageBox::warning(this, "Lỗi", "Không thể kích hoạt Bác sĩ này.");
        }
      }
    });
  }
}

void ManageDoctorsWidget::handleFilterChanged() {
  m_currentPage = 1;
  loadDoctorsList();
}

void ManageDoctorsWidget::handleResetFilters() {
  m_txtSearchKey->clear();
  m_cbSpecialtyFilter->setCurrentIndex(0);
  m_cbDepartmentFilter->setCurrentIndex(0);
  m_cbShiftFilter->setCurrentIndex(0);
  m_cbStatusFilter->setCurrentIndex(0);
  m_currentPage = 1;
  loadDoctorsList();
}

void ManageDoctorsWidget::handlePrevPage() {
  if (m_currentPage > 1) {
    m_currentPage--;
    loadDoctorsList();
  }
}

void ManageDoctorsWidget::handleNextPage() {
  if (m_currentPage < m_totalPages) {
    m_currentPage++;
    loadDoctorsList();
  }
}

void ManageDoctorsWidget::showAddDoctorDialog() {
  DoctorRegistrationDialog dialog(m_staffService, m_appointmentService, this);
  if (dialog.exec() == QDialog::Accepted) {
    loadDoctorsList();
  }
}

void ManageDoctorsWidget::showEditDoctorDialog(std::shared_ptr<Doctor> doc) {
  auto profile = m_staffService->getOwnProfile(doc->getAccountId());
  auto doctorProfile = dynamic_cast<DoctorProfileDTO *>(profile.get());
  if (doctorProfile) {
    DoctorRegistrationDialog dialog(m_staffService, m_appointmentService, this);
    dialog.loadDoctorData(doctorProfile);
    if (dialog.exec() == QDialog::Accepted) {
      loadDoctorsList();
    }
  } else {
    QMessageBox::warning(this, "Lỗi",
                         "Không thể lấy thông tin chi tiết bác sĩ.");
  }
}
