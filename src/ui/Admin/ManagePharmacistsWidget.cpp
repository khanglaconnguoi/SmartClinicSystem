#include "ManagePharmacistsWidget.h"
#include "../../dto/StaffDTOs.h"
#include "PharmacistRegistrationDialog.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

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
  QLabel *lblPageTitle = new QLabel("Danh sách Dược sĩ", this);
  lblPageTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #111827;");
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

  // Card bao bọc Table
  QFrame *tableCard = makeCard(this);
  QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
  cardLayout->setContentsMargins(0, 0, 0, 0);

  m_tblPharmacists = new QTableWidget(0, 7, tableCard);
  m_tblPharmacists->setHorizontalHeaderLabels(
      {"Mã DS", "Họ Tên", "Khu vực NT", "Số CCHN", "SĐT", "Trạng thái", "Thao tác"});
  m_tblPharmacists->horizontalHeader()->setStretchLastSection(true);
  m_tblPharmacists->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_tblPharmacists->verticalHeader()->setDefaultSectionSize(46);
  m_tblPharmacists->verticalHeader()->setVisible(false);
  m_tblPharmacists->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tblPharmacists->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tblPharmacists->setStyleSheet(
      "QTableWidget { border: none; gridline-color: #E2E8F0; font-size: 13px; "
      "background-color: white; color: #0F172A; }"
      "QHeaderView::section { background-color: #F8FAFC; padding: 10px; "
      "font-weight: bold; border: none; border-bottom: 1px solid #E2E8F0; "
      "color: #1E293B; }");

  cardLayout->addWidget(m_tblPharmacists);
  pageLayout->addWidget(tableCard);

  connect(btnAddNew, &QPushButton::clicked, this,
          &ManagePharmacistsWidget::showAddPharmacistDialog);

  // Load data
  loadPharmacistsList();
}

namespace {
constexpr int PAGE_SIZE = 20;
}

void ManagePharmacistsWidget::loadPharmacistsList() {
  if (!m_tblPharmacists || !m_staffService)
    return;
  m_tblPharmacists->setRowCount(0);

  PharmacistSearchCriteria criteria;
  criteria.onlyActive = true;
  criteria.includeDeleted = false;
  criteria.pageSize = PAGE_SIZE;

  QList<std::shared_ptr<SystemUser>> pharmacists =
      m_staffService->searchPharmacistsPaged(criteria).items;

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

    QTableWidgetItem *itemSection = new QTableWidgetItem(doc->getPharmacySection());
    itemSection->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemLicense = new QTableWidgetItem(doc->getLicenseNumber());
    itemLicense->setForeground(QBrush(QColor("#111827")));
    
    QTableWidgetItem *itemPhone = new QTableWidgetItem("---");
    itemPhone->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemStatus =
        new QTableWidgetItem(doc->isActive() ? "Hoạt động" : "Nghỉ việc");
    if (doc->isActive()) {
      itemStatus->setForeground(QBrush(QColor("#059669")));
    } else {
      itemStatus->setForeground(QBrush(QColor("#DC2626")));
    }

    m_tblPharmacists->setItem(row, 0, itemCode);
    m_tblPharmacists->setItem(row, 1, itemName);
    m_tblPharmacists->setItem(row, 2, itemSection);
    m_tblPharmacists->setItem(row, 3, itemLicense);
    m_tblPharmacists->setItem(row, 4, itemPhone);
    m_tblPharmacists->setItem(row, 5, itemStatus);

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

    QPushButton *btnDeactivate = new QPushButton(doc->isActive() ? "Vô hiệu hóa" : "Kích hoạt");
    btnDeactivate->setCursor(Qt::PointingHandCursor);
    if (doc->isActive()) {
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
    m_tblPharmacists->setCellWidget(row, 6, actionWidget);

    connect(btnEdit, &QPushButton::clicked, this,
            [this, doc]() { showEditPharmacistDialog(doc); });

    connect(btnResetPwd, &QPushButton::clicked, this, [this, doc]() {
        auto confirm = QMessageBox::question(this, "Xác nhận Reset Mật khẩu",
            QString("Bạn có chắc chắn muốn reset mật khẩu cho Dược sĩ %1 (%2)?").arg(doc->getFullName(), doc->getStaffCode()),
            QMessageBox::Yes | QMessageBox::No);
        if (confirm == QMessageBox::Yes) {
            auto res = m_staffService->resetPassword(doc->getAccountId());
            if (res.result) {
                QMessageBox::information(this, "Reset Mật khẩu thành công",
                    QString("Mật khẩu mới cho Dược sĩ %1 (%2) là:\n\n%3\n\nTài khoản sẽ yêu cầu đổi mật khẩu khi đăng nhập lần tiếp theo.")
                    .arg(doc->getFullName(), doc->getStaffCode(), res.newPassword));
            } else {
                QMessageBox::warning(this, "Lỗi", "Không thể reset mật khẩu cho Dược sĩ này.");
            }
        }
    });
            
    connect(btnDeactivate, &QPushButton::clicked, this, [this, doc]() {
        if (doc->isActive()) {
            if (m_staffService->deactivateStaff(doc->getAccountId())) {
                loadPharmacistsList();
            } else {
                QMessageBox::warning(this, "Lỗi", "Không thể vô hiệu hóa Dược sĩ này.");
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

void ManagePharmacistsWidget::showAddPharmacistDialog() {
  PharmacistRegistrationDialog dialog(m_staffService, this);
  if (dialog.exec() == QDialog::Accepted) {
    loadPharmacistsList();
  }
}

void ManagePharmacistsWidget::showEditPharmacistDialog(std::shared_ptr<Pharmacist> doc) {
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
