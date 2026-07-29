#include "ManageDoctorsWidget.h"
#include "../../dto/StaffDTOs.h"
#include "DoctorRegistrationDialog.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ManageDoctorsWidget::ManageDoctorsWidget(
    std::shared_ptr<StaffService> staffService, QWidget *parent)
    : QWidget(parent), m_staffService(staffService), m_tblDoctors(nullptr) {
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
  QLabel *lblPageTitle = new QLabel("Danh sách Bác sĩ", this);
  lblPageTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #111827;");
  headerLayout->addWidget(lblPageTitle);

  headerLayout->addStretch();

  QPushButton *btnAddNew = new QPushButton("+ Thêm Bác sĩ", this);
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

  m_tblDoctors = new QTableWidget(0, 6, tableCard);
  m_tblDoctors->setHorizontalHeaderLabels(
      {"Mã BS", "Họ Tên", "Chuyên khoa", "SĐT", "Trạng thái", "Thao tác"});
  m_tblDoctors->horizontalHeader()->setStretchLastSection(true);
  m_tblDoctors->horizontalHeader()->setSectionResizeMode(1,
                                                         QHeaderView::Stretch);
  m_tblDoctors->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tblDoctors->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tblDoctors->setStyleSheet(
      "QTableWidget { border: none; gridline-color: #EAEAEA; font-size: 13px; "
      "background-color: white; }"
      "QHeaderView::section { background-color: #F8FAFC; padding: 8px; "
      "font-weight: bold; border: none; border-bottom: 1px solid #EAEAEA; "
      "color: #111827; }");

  cardLayout->addWidget(m_tblDoctors);
  pageLayout->addWidget(tableCard);

  connect(btnAddNew, &QPushButton::clicked, this,
          &ManageDoctorsWidget::showAddDoctorDialog);

  // Load data
  loadDoctorsList();
}

void ManageDoctorsWidget::loadDoctorsList() {
  if (!m_tblDoctors || !m_staffService)
    return;
  m_tblDoctors->setRowCount(0);

  DoctorSearchCriteria criteria;
  criteria.onlyActive = true;
  criteria.includeDeleted = false;
  criteria.pageSize = 0; // Load all for UI table display

  QList<std::shared_ptr<SystemUser>> doctors =
      m_staffService->searchDoctorsPaged(criteria).items;


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

    QTableWidgetItem *itemPhone = new QTableWidgetItem("---");
    itemPhone->setForeground(QBrush(QColor("#111827")));

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
    m_tblDoctors->setItem(row, 3, itemPhone);
    m_tblDoctors->setItem(row, 4, itemStatus);

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
        "QPushButton { color: #D93025; border: 1px solid #D93025; padding: 4px "
        "8px; border-radius: 4px; background-color: white; } QPushButton:hover "
        "{ background-color: #FCE8E6; }");

    actionLayout->addWidget(btnEdit);
    actionLayout->addWidget(btnDelete);
    m_tblDoctors->setCellWidget(row, 5, actionWidget);

    connect(btnEdit, &QPushButton::clicked, this,
            [this, doc]() { showEditDoctorDialog(doc); });
  }
}

void ManageDoctorsWidget::showAddDoctorDialog() {
  DoctorRegistrationDialog dialog(m_staffService, this);
  if (dialog.exec() == QDialog::Accepted) {
    loadDoctorsList();
  }
}

void ManageDoctorsWidget::showEditDoctorDialog(std::shared_ptr<Doctor> doc) {
  auto profile = m_staffService->getOwnProfile(doc->getAccountId());
  auto doctorProfile = dynamic_cast<DoctorProfileDTO *>(profile.get());
  if (doctorProfile) {
    DoctorRegistrationDialog dialog(m_staffService, this);
    dialog.loadDoctorData(doctorProfile);
    if (dialog.exec() == QDialog::Accepted) {
      loadDoctorsList();
    }
  } else {
    QMessageBox::warning(this, "Lỗi",
                         "Không thể lấy thông tin chi tiết bác sĩ.");
  }
}
