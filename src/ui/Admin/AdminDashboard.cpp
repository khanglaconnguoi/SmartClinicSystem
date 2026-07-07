#include "AdminDashboard.h"
#include "../../model/Doctor.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "../../repository/DatabaseManager.h"

AdminDashboardWidget::AdminDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService, QWidget *parent)
    : BaseDashboardWidget(parent), m_staffService(staffService),
      m_currentUser(user) {
  initializeDashboard();
}

void AdminDashboardWidget::fillDashboardData() {
  buildSidebar();

  if (m_currentUser && m_docNameLabel) {
    m_docNameLabel->setText("Admin: " + m_currentUser->getFullName());
  }

  m_mainContentWidget->setObjectName("MainContentWidget");
  m_stackedWidget = new QStackedWidget(m_mainContentWidget);
  m_stackedWidget->setObjectName("StackedWidget");
  m_stackedWidget->setStyleSheet(
      "QStackedWidget#StackedWidget > QWidget { background-color: #EEF2F6; }");
  m_mainContentLayout->addWidget(m_stackedWidget, 1);

  buildManageDoctorsPage();
  buildManageNursesPage();
  buildManagePatientsPage();
  buildAddDoctorPage();

  m_stackedWidget->setCurrentWidget(m_manageDoctorsPage);
}

void AdminDashboardWidget::buildSidebar() {
  QPushButton *btnManageDoctors =
      new QPushButton("Quản lý Bác sĩ", m_sidebarFrame);
  btnManageDoctors->setCursor(Qt::PointingHandCursor);

  QPushButton *btnManageNurses =
      new QPushButton("Quản lý Y tá", m_sidebarFrame);
  btnManageNurses->setCursor(Qt::PointingHandCursor);

  QPushButton *btnManagePatients =
      new QPushButton("Quản lý Bệnh nhân", m_sidebarFrame);
  btnManagePatients->setCursor(Qt::PointingHandCursor);

  QString defaultStyle =
      "QPushButton { background-color: transparent; color: #4B5563; "
      "font-weight: 500; text-align: left; padding: 10px 15px; border: none; "
      "border-radius: 8px; } QPushButton:hover { background-color: #E5E7EB; "
      "color: #111827; }";
  QString activeStyle =
      "QPushButton#activeMenu { background-color: #ACDEF2; color: #4B94F2; "
      "font-weight: bold; border-left: 4px solid #4B94F2; }";

  btnManageDoctors->setStyleSheet(defaultStyle + activeStyle);
  btnManageNurses->setStyleSheet(defaultStyle + activeStyle);
  btnManagePatients->setStyleSheet(defaultStyle + activeStyle);

  btnManageDoctors->setObjectName("activeMenu");

  m_sidebarLayout->addWidget(btnManageDoctors);
  m_sidebarLayout->addWidget(btnManageNurses);
  m_sidebarLayout->addWidget(btnManagePatients);
  m_sidebarLayout->addStretch();

  connect(btnManageDoctors, &QPushButton::clicked, this,
          &AdminDashboardWidget::showManageDoctorsList);
  connect(btnManageNurses, &QPushButton::clicked, this,
          &AdminDashboardWidget::showManageNursesList);
  connect(btnManagePatients, &QPushButton::clicked, this,
          &AdminDashboardWidget::showManagePatientsList);

  // UI logic for active menu visual update (basic)
  auto updateActiveMenu = [=](QPushButton *activeBtn) {
    btnManageDoctors->setObjectName("");
    btnManageNurses->setObjectName("");
    btnManagePatients->setObjectName("");
    activeBtn->setObjectName("activeMenu");
    btnManageDoctors->style()->unpolish(btnManageDoctors);
    btnManageDoctors->style()->polish(btnManageDoctors);
    btnManageNurses->style()->unpolish(btnManageNurses);
    btnManageNurses->style()->polish(btnManageNurses);
    btnManagePatients->style()->unpolish(btnManagePatients);
    btnManagePatients->style()->polish(btnManagePatients);
  };

  connect(btnManageDoctors, &QPushButton::clicked, this,
          [=]() { updateActiveMenu(btnManageDoctors); });
  connect(btnManageNurses, &QPushButton::clicked, this,
          [=]() { updateActiveMenu(btnManageNurses); });
  connect(btnManagePatients, &QPushButton::clicked, this,
          [=]() { updateActiveMenu(btnManagePatients); });
}

void AdminDashboardWidget::showManageDoctorsList() {
  m_stackedWidget->setCurrentWidget(m_manageDoctorsPage);
}

void AdminDashboardWidget::showManageNursesList() {
  m_stackedWidget->setCurrentWidget(m_manageNursesPage);
}

void AdminDashboardWidget::showManagePatientsList() {
  m_stackedWidget->setCurrentWidget(m_managePatientsPage);
}

void AdminDashboardWidget::showAddDoctorForm() {
  m_stackedWidget->setCurrentWidget(m_addDoctorPage);
}

void AdminDashboardWidget::buildManageDoctorsPage() {
  m_manageDoctorsPage = new QWidget(this);
  QVBoxLayout *pageLayout = new QVBoxLayout(m_manageDoctorsPage);
  pageLayout->setContentsMargins(30, 30, 30, 30);
  pageLayout->setSpacing(20);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *lblPageTitle = new QLabel("Danh sách Bác sĩ", m_manageDoctorsPage);
  lblPageTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #111827;");
  headerLayout->addWidget(lblPageTitle);

  headerLayout->addStretch();

  QPushButton *btnAddNew =
      new QPushButton("+ Thêm Bác sĩ", m_manageDoctorsPage);
  btnAddNew->setCursor(Qt::PointingHandCursor);
  btnAddNew->setFixedSize(140, 40);
  btnAddNew->setStyleSheet(
      "QPushButton { background-color: #4B94F2; color: white; font-size: 14px; "
      "font-weight: bold; border-radius: 6px; border: none; }"
      "QPushButton:hover { background-color: #398CBF; }");
  headerLayout->addWidget(btnAddNew);
  pageLayout->addLayout(headerLayout);

  // Card bao bọc Table
  QFrame *tableCard = makeCard(m_manageDoctorsPage);
  QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
  cardLayout->setContentsMargins(0, 0, 0, 0);

  m_tblDoctors = new QTableWidget(0, 6, tableCard);
  m_tblDoctors->setHorizontalHeaderLabels({"Mã NV", "Họ Tên", "Chuyên khoa",
                                           "Kinh nghiệm", "Phí khám",
                                           "Thao tác"});
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

  m_stackedWidget->addWidget(m_manageDoctorsPage);

  connect(btnAddNew, &QPushButton::clicked, this,
          &AdminDashboardWidget::showAddDoctorForm);

  // Load data
  loadDoctorsList();
}

void AdminDashboardWidget::buildManageNursesPage() {
  m_manageNursesPage = new QWidget(this);
  QVBoxLayout *pageLayout = new QVBoxLayout(m_manageNursesPage);
  pageLayout->setContentsMargins(30, 30, 30, 30);
  pageLayout->setSpacing(20);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *lblPageTitle = new QLabel("Danh sách Y tá", m_manageNursesPage);
  lblPageTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #111827;");
  headerLayout->addWidget(lblPageTitle);

  headerLayout->addStretch();

  QPushButton *btnAddNew = new QPushButton("+ Thêm Y tá", m_manageNursesPage);
  btnAddNew->setCursor(Qt::PointingHandCursor);
  btnAddNew->setFixedSize(140, 40);
  btnAddNew->setStyleSheet(
      "QPushButton { background-color: #4B94F2; color: white; font-size: 14px; "
      "font-weight: bold; border-radius: 6px; border: none; }"
      "QPushButton:hover { background-color: #398CBF; }");
  headerLayout->addWidget(btnAddNew);
  pageLayout->addLayout(headerLayout);

  // Card bao bọc Table
  QFrame *tableCard = makeCard(m_manageNursesPage);
  QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
  cardLayout->setContentsMargins(0, 0, 0, 0);

  m_tblNurses = new QTableWidget(0, 6, tableCard);
  m_tblNurses->setHorizontalHeaderLabels(
      {"Mã NV", "Họ Tên", "Cấp bậc", "Chứng chỉ", "SĐT", "Thao tác"});
  m_tblNurses->horizontalHeader()->setStretchLastSection(true);
  m_tblNurses->horizontalHeader()->setSectionResizeMode(1,
                                                        QHeaderView::Stretch);
  m_tblNurses->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tblNurses->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tblNurses->setStyleSheet(
      "QTableWidget { border: none; gridline-color: #EAEAEA; font-size: 13px; "
      "background-color: white; }"
      "QHeaderView::section { background-color: #F8FAFC; padding: 8px; "
      "font-weight: bold; border: none; border-bottom: 1px solid #EAEAEA; "
      "color: #111827; }");

  cardLayout->addWidget(m_tblNurses);
  pageLayout->addWidget(tableCard);
  m_stackedWidget->addWidget(m_manageNursesPage);

  // Dummy data for Nurse UI
  for (int i = 0; i < 3; ++i) {
    int row = m_tblNurses->rowCount();
    m_tblNurses->insertRow(row);

    QTableWidgetItem *itemCode =
        new QTableWidgetItem(QString("N100%1").arg(i + 1));
    itemCode->setForeground(QBrush(QColor("#111827")));
    QTableWidgetItem *itemName =
        new QTableWidgetItem(QString("Y tá Trần Thị B%1").arg(i + 1));
    itemName->setForeground(QBrush(QColor("#111827")));
    QTableWidgetItem *itemLevel = new QTableWidgetItem("Y tá trưởng");
    itemLevel->setForeground(QBrush(QColor("#111827")));
    QTableWidgetItem *itemCert = new QTableWidgetItem("Chứng chỉ hành nghề Y");
    itemCert->setForeground(QBrush(QColor("#111827")));
    QTableWidgetItem *itemPhone = new QTableWidgetItem("0912345678");
    itemPhone->setForeground(QBrush(QColor("#111827")));

    m_tblNurses->setItem(row, 0, itemCode);
    m_tblNurses->setItem(row, 1, itemName);
    m_tblNurses->setItem(row, 2, itemLevel);
    m_tblNurses->setItem(row, 3, itemCert);
    m_tblNurses->setItem(row, 4, itemPhone);

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
    m_tblNurses->setCellWidget(row, 5, actionWidget);
  }
}

void AdminDashboardWidget::buildManagePatientsPage() {
  m_managePatientsPage = new QWidget(this);
  QVBoxLayout *pageLayout = new QVBoxLayout(m_managePatientsPage);
  pageLayout->setContentsMargins(30, 30, 30, 30);
  pageLayout->setSpacing(20);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *lblPageTitle =
      new QLabel("Danh sách Bệnh nhân", m_managePatientsPage);
  lblPageTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #111827;");
  headerLayout->addWidget(lblPageTitle);

  headerLayout->addStretch();

  QPushButton *btnAddNew =
      new QPushButton("+ Thêm Bệnh nhân", m_managePatientsPage);
  btnAddNew->setCursor(Qt::PointingHandCursor);
  btnAddNew->setFixedSize(160, 40);
  btnAddNew->setStyleSheet(
      "QPushButton { background-color: #4B94F2; color: white; font-size: 14px; "
      "font-weight: bold; border-radius: 6px; border: none; }"
      "QPushButton:hover { background-color: #398CBF; }");
  headerLayout->addWidget(btnAddNew);
  pageLayout->addLayout(headerLayout);

  // Card bao bọc Table
  QFrame *tableCard = makeCard(m_managePatientsPage);
  QVBoxLayout *cardLayout = new QVBoxLayout(tableCard);
  cardLayout->setContentsMargins(0, 0, 0, 0);

  m_tblPatients = new QTableWidget(0, 6, tableCard);
  m_tblPatients->setHorizontalHeaderLabels(
      {"Mã BN", "Họ Tên", "Giới tính", "Ngày sinh", "SĐT", "Thao tác"});
  m_tblPatients->horizontalHeader()->setStretchLastSection(true);
  m_tblPatients->horizontalHeader()->setSectionResizeMode(1,
                                                          QHeaderView::Stretch);
  m_tblPatients->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tblPatients->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tblPatients->setStyleSheet(
      "QTableWidget { border: none; gridline-color: #EAEAEA; font-size: 13px; "
      "background-color: white; }"
      "QHeaderView::section { background-color: #F8FAFC; padding: 8px; "
      "font-weight: bold; border: none; border-bottom: 1px solid #EAEAEA; "
      "color: #111827; }");

  cardLayout->addWidget(m_tblPatients);
  pageLayout->addWidget(tableCard);
  m_stackedWidget->addWidget(m_managePatientsPage);

  // Load data from DB
  loadPatientsList();
}

void AdminDashboardWidget::loadDoctorsList() {
  if (!m_tblDoctors)
    return;
  m_tblDoctors->setRowCount(0);

  QList<std::shared_ptr<SystemUser>> doctors =
      m_staffService->searchDoctors("", "", -1, "", true, false);

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

    QTableWidgetItem *itemSpecialty = new QTableWidgetItem(doc->getSpecialty());
    itemSpecialty->setForeground(QBrush(QColor("#111827")));

    QTableWidgetItem *itemExp = new QTableWidgetItem(
        QString::number(doc->getExperienceYears()) + " năm");
    itemExp->setForeground(QBrush(QColor("#111827")));

    QLocale locale(QLocale::Vietnamese, QLocale::Vietnam);
    QString feeStr = locale.toCurrencyString(doc->getConsultationFee(), "VND");
    QTableWidgetItem *itemFee = new QTableWidgetItem(feeStr);
    itemFee->setForeground(QBrush(QColor("#111827")));

    m_tblDoctors->setItem(row, 0, itemCode);
    m_tblDoctors->setItem(row, 1, itemName);
    m_tblDoctors->setItem(row, 2, itemSpecialty);
    m_tblDoctors->setItem(row, 3, itemExp);
    m_tblDoctors->setItem(row, 4, itemFee);

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
  }
}

void AdminDashboardWidget::loadPatientsList() {
    if (!m_tblPatients) return;
    m_tblPatients->setRowCount(0);

    DatabaseManager& db = DatabaseManager::getInstance();
    // Assuming table structure: patient_code, full_name, gender, date_of_birth, phone_number, is_deleted
    QSqlQuery query = db.selectQuery("SELECT patient_code, full_name, gender, date_of_birth, phone_number FROM patients WHERE is_deleted = 0");

    while (query.next()) {
        QString code = query.value(0).toString();
        QString name = query.value(1).toString();
        QString gender = query.value(2).toString();
        QString dob = query.value(3).toString();
        QString phone = query.value(4).toString();

        if (gender == "MALE") gender = "Nam";
        else if (gender == "FEMALE") gender = "Nữ";
        else if (gender == "OTHER") gender = "Khác";

        int row = m_tblPatients->rowCount();
        m_tblPatients->insertRow(row);

        QTableWidgetItem* itemCode = new QTableWidgetItem(code);
        itemCode->setForeground(QBrush(QColor("#111827")));
        QTableWidgetItem* itemName = new QTableWidgetItem(name);
        itemName->setForeground(QBrush(QColor("#111827")));
        QTableWidgetItem* itemGender = new QTableWidgetItem(gender);
        itemGender->setForeground(QBrush(QColor("#111827")));
        QTableWidgetItem* itemDob = new QTableWidgetItem(dob);
        itemDob->setForeground(QBrush(QColor("#111827")));
        QTableWidgetItem* itemPhone = new QTableWidgetItem(phone);
        itemPhone->setForeground(QBrush(QColor("#111827")));

        m_tblPatients->setItem(row, 0, itemCode);
        m_tblPatients->setItem(row, 1, itemName);
        m_tblPatients->setItem(row, 2, itemGender);
        m_tblPatients->setItem(row, 3, itemDob);
        m_tblPatients->setItem(row, 4, itemPhone);

        QWidget* actionWidget = new QWidget();
        QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setSpacing(8);
        QPushButton* btnEdit = new QPushButton("Sửa");
        btnEdit->setCursor(Qt::PointingHandCursor);
        btnEdit->setStyleSheet("QPushButton { color: #4B94F2; border: 1px solid #4B94F2; padding: 4px 8px; border-radius: 4px; background-color: white; } QPushButton:hover { background-color: #EBF5FF; }");
        QPushButton* btnDelete = new QPushButton("Xóa");
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setStyleSheet("QPushButton { color: #D93025; border: 1px solid #D93025; padding: 4px 8px; border-radius: 4px; background-color: white; } QPushButton:hover { background-color: #FCE8E6; }");
        actionLayout->addWidget(btnEdit);
        actionLayout->addWidget(btnDelete);
        m_tblPatients->setCellWidget(row, 5, actionWidget);
    }
}

QFrame *AdminDashboardWidget::makeCard(QWidget *parent) {
  QFrame *card = new QFrame(parent);
  card->setObjectName("DashboardCard");
  card->setStyleSheet(
      "QFrame#DashboardCard {"
      "   background-color: #FFFFFF;"
      "   border: 1px solid #E5E7EB;"
      "   border-radius: 12px;"
      "}"
      "QLabel { border: none; background: transparent; }"
      "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px "
      "10px; font-size: 13px; color: #111827; background: #FFFFFF; }"
      "QLineEdit:focus { border: 1px solid #4B94F2; }"
      "QComboBox { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px "
      "10px; font-size: 13px; color: #111827; background: #FFFFFF; }"
      "QComboBox:focus { border: 1px solid #4B94F2; }");
  return card;
}

void AdminDashboardWidget::buildAddDoctorPage() {
  m_addDoctorPage = new QWidget(this);
  QVBoxLayout *pageLayout = new QVBoxLayout(m_addDoctorPage);
  pageLayout->setContentsMargins(30, 30, 30, 30);
  pageLayout->setSpacing(20);

  QLabel *lblPageTitle = new QLabel("Thêm mới Bác sĩ", m_addDoctorPage);
  lblPageTitle->setStyleSheet(
      "font-size: 24px; font-weight: bold; color: #111827;");
  pageLayout->addWidget(lblPageTitle);

  QFrame *formCard = makeCard(m_addDoctorPage);
  QGridLayout *gridLayout = new QGridLayout(formCard);
  gridLayout->setContentsMargins(24, 24, 24, 24);
  gridLayout->setSpacing(16);

  // Row 0/1: Họ tên & Ngày sinh
  QLabel *lblName = new QLabel("Họ và Tên (*)", formCard);
  lblName->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151;");
  m_txtFullName = new QLineEdit(formCard);
  m_txtFullName->setPlaceholderText("VD: Nguyễn Văn A");

  QLabel *lblDob = new QLabel("Ngày sinh (*)", formCard);
  lblDob->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151;");
  m_dtDateOfBirth = new QDateEdit(QDate(1990, 1, 1), formCard);
  m_dtDateOfBirth->setCalendarPopup(true);
  m_dtDateOfBirth->setStyleSheet("QDateEdit { border: 1px solid #D1D5DB; "
                                 "border-radius: 6px; padding: 6px; }");

  gridLayout->addWidget(lblName, 0, 0);
  gridLayout->addWidget(m_txtFullName, 1, 0);
  gridLayout->addWidget(lblDob, 0, 1);
  gridLayout->addWidget(m_dtDateOfBirth, 1, 1);

  // Row 2/3: CMND & Giới tính
  QLabel *lblCitizen = new QLabel("Số CMND / CCCD (*)", formCard);
  lblCitizen->setStyleSheet(
      "font-size: 13px; font-weight: bold; color: #374151;");
  m_txtCitizenId = new QLineEdit(formCard);
  m_txtCitizenId->setPlaceholderText("Nhập 12 chữ số hợp lệ");

  QLabel *lblGender = new QLabel("Giới tính", formCard);
  lblGender->setStyleSheet(
      "font-size: 13px; font-weight: bold; color: #374151;");
  m_cbGender = new QComboBox(formCard);
  m_cbGender->addItems({"Nam", "Nữ", "Khác"});

  gridLayout->addWidget(lblCitizen, 2, 0);
  gridLayout->addWidget(m_txtCitizenId, 3, 0);
  gridLayout->addWidget(lblGender, 2, 1);
  gridLayout->addWidget(m_cbGender, 3, 1);

  // Row 4/5: SĐT & Email
  QLabel *lblPhone = new QLabel("Số điện thoại (*)", formCard);
  lblPhone->setStyleSheet(
      "font-size: 13px; font-weight: bold; color: #374151;");
  m_txtPhone = new QLineEdit(formCard);
  m_txtPhone->setPlaceholderText("VD: 0901234567");

  QLabel *lblEmail = new QLabel("Email", formCard);
  lblEmail->setStyleSheet(
      "font-size: 13px; font-weight: bold; color: #374151;");
  m_txtEmail = new QLineEdit(formCard);
  m_txtEmail->setPlaceholderText("VD: doctor@example.com");

  gridLayout->addWidget(lblPhone, 4, 0);
  gridLayout->addWidget(m_txtPhone, 5, 0);
  gridLayout->addWidget(lblEmail, 4, 1);
  gridLayout->addWidget(m_txtEmail, 5, 1);

  // Row 6/7: Địa chỉ & Phòng ban
  QLabel *lblAddress = new QLabel("Địa chỉ", formCard);
  lblAddress->setStyleSheet(
      "font-size: 13px; font-weight: bold; color: #374151;");
  m_txtAddress = new QLineEdit(formCard);
  m_txtAddress->setPlaceholderText("VD: TP Hồ Chí Minh");

  QLabel *lblDept = new QLabel("Phòng ban", formCard);
  lblDept->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151;");
  m_cbDepartment = new QComboBox(formCard);
  m_cbDepartment->addItems({"1 - Khoa Khám Bệnh", "2 - Khoa Nội",
                            "3 - Khoa Ngoại", "4 - Khoa Sản", "5 - Khoa Nhi"});

  gridLayout->addWidget(lblAddress, 6, 0);
  gridLayout->addWidget(m_txtAddress, 7, 0);
  gridLayout->addWidget(lblDept, 6, 1);
  gridLayout->addWidget(m_cbDepartment, 7, 1);

  // Row 8/9: Mã NV & Mật khẩu
  QLabel *lblCode = new QLabel("Mã nhân viên", formCard);
  lblCode->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151;");
  m_txtStaffCode = new QLineEdit(formCard);
  m_txtStaffCode->setPlaceholderText("Để trống sẽ tự động tạo");

  QLabel *lblPass = new QLabel("Mật khẩu (*)", formCard);
  lblPass->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151;");
  m_txtPassword = new QLineEdit(formCard);
  m_txtPassword->setEchoMode(QLineEdit::Password);
  m_txtPassword->setPlaceholderText("Mật khẩu đăng nhập");

  gridLayout->addWidget(lblCode, 8, 0);
  gridLayout->addWidget(m_txtStaffCode, 9, 0);
  gridLayout->addWidget(lblPass, 8, 1);
  gridLayout->addWidget(m_txtPassword, 9, 1);

  // Row 10/11: Chuyên khoa & Ca làm
  QLabel *lblSpecialty = new QLabel("Chuyên khoa", formCard);
  lblSpecialty->setStyleSheet(
      "font-size: 13px; font-weight: bold; color: #374151;");
  m_cbSpecialty = new QComboBox(formCard);
  m_cbSpecialty->addItems(
      {"Nội khoa", "Ngoại khoa", "Nhi khoa", "Da liễu", "Răng Hàm Mặt"});

  QLabel *lblShift = new QLabel("Ca làm việc", formCard);
  lblShift->setStyleSheet(
      "font-size: 13px; font-weight: bold; color: #374151;");
  m_cbShift = new QComboBox(formCard);
  m_cbShift->addItems({"Sáng", "Chiều", "Tối"});

  gridLayout->addWidget(lblSpecialty, 10, 0);
  gridLayout->addWidget(m_cbSpecialty, 11, 0);
  gridLayout->addWidget(lblShift, 10, 1);
  gridLayout->addWidget(m_cbShift, 11, 1);

  // Row 12/13: Ngày vào làm & Số CCHN
  QLabel *lblHireDate = new QLabel("Ngày vào làm", formCard);
  lblHireDate->setStyleSheet(
      "font-size: 13px; font-weight: bold; color: #374151;");
  m_dtHireDate = new QDateEdit(QDate::currentDate(), formCard);
  m_dtHireDate->setCalendarPopup(true);
  m_dtHireDate->setStyleSheet("QDateEdit { border: 1px solid #D1D5DB; "
                              "border-radius: 6px; padding: 6px; }");

  QLabel *lblLicense = new QLabel("Số chứng chỉ hành nghề", formCard);
  lblLicense->setStyleSheet(
      "font-size: 13px; font-weight: bold; color: #374151;");
  m_txtLicenseNumber = new QLineEdit(formCard);

  gridLayout->addWidget(lblHireDate, 12, 0);
  gridLayout->addWidget(m_dtHireDate, 13, 0);
  gridLayout->addWidget(lblLicense, 12, 1);
  gridLayout->addWidget(m_txtLicenseNumber, 13, 1);

  // Row 14/15: Số năm KN & Phí khám
  QLabel *lblExp = new QLabel("Số năm kinh nghiệm", formCard);
  lblExp->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151;");
  m_sbExperienceYears = new QSpinBox(formCard);
  m_sbExperienceYears->setRange(0, 50);
  m_sbExperienceYears->setStyleSheet("QSpinBox { border: 1px solid #D1D5DB; "
                                     "border-radius: 6px; padding: 6px; }");

  QLabel *lblFee = new QLabel("Phí khám (VNĐ)", formCard);
  lblFee->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151;");
  m_sbConsultationFee = new QSpinBox(formCard);
  m_sbConsultationFee->setRange(0, 10000000);
  m_sbConsultationFee->setSingleStep(50000);
  m_sbConsultationFee->setValue(150000);
  m_sbConsultationFee->setStyleSheet("QSpinBox { border: 1px solid #D1D5DB; "
                                     "border-radius: 6px; padding: 6px; }");

  gridLayout->addWidget(lblExp, 14, 0);
  gridLayout->addWidget(m_sbExperienceYears, 15, 0);
  gridLayout->addWidget(lblFee, 14, 1);
  gridLayout->addWidget(m_sbConsultationFee, 15, 1);

  // Row 16/17: Tiểu sử
  QLabel *lblBio = new QLabel("Tiểu sử / Giới thiệu", formCard);
  lblBio->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151;");
  m_txtBio = new QTextEdit(formCard);
  m_txtBio->setStyleSheet(
      "QTextEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: "
      "6px; background: #FFFFFF; font-size: 13px; }");
  m_txtBio->setFixedHeight(60);

  gridLayout->addWidget(lblBio, 16, 0, 1, 2);
  gridLayout->addWidget(m_txtBio, 17, 0, 1, 2);

  pageLayout->addWidget(formCard);

  // Nút Lưu và Trở lại
  QHBoxLayout *btnLayout = new QHBoxLayout();

  QPushButton *btnBack = new QPushButton("Trở lại", m_addDoctorPage);
  btnBack->setCursor(Qt::PointingHandCursor);
  btnBack->setFixedSize(140, 40);
  btnBack->setStyleSheet("QPushButton { background-color: #F3F4F6; color: "
                         "#4B5563; font-size: 14px; font-weight: bold; "
                         "border-radius: 6px; border: 1px solid #D1D5DB; }"
                         "QPushButton:hover { background-color: #E5E7EB; }");
  btnLayout->addWidget(btnBack);

  btnLayout->addStretch();
  QPushButton *btnSave = new QPushButton("Lưu tài khoản", m_addDoctorPage);
  btnSave->setCursor(Qt::PointingHandCursor);
  btnSave->setFixedSize(140, 40);
  btnSave->setStyleSheet(
      "QPushButton { background-color: #4B94F2; color: white; font-size: 14px; "
      "font-weight: bold; border-radius: 6px; border: none; }"
      "QPushButton:hover { background-color: #398CBF; }");
  btnLayout->addWidget(btnSave);

  pageLayout->addLayout(btnLayout);
  pageLayout->addStretch();

  m_stackedWidget->addWidget(m_addDoctorPage);
  connect(btnSave, &QPushButton::clicked, this,
          &AdminDashboardWidget::handleAddDoctor);
  connect(btnBack, &QPushButton::clicked, this,
          &AdminDashboardWidget::showManageDoctorsList);
}

void AdminDashboardWidget::handleAddDoctor() {
  if (!m_staffService) {
    QMessageBox::critical(this, "Lỗi", "Service không khả dụng.");
    return;
  }

  // 1. Lấy dữ liệu cơ bản
  QString fullName = m_txtFullName->text().trimmed();
  QString password = m_txtPassword->text();
  QString staffCode = m_txtStaffCode->text().trimmed();
  QString citizenId = m_txtCitizenId->text().trimmed();
  QString phone = m_txtPhone->text().trimmed();
  QString email = m_txtEmail->text().trimmed();
  QString address = m_txtAddress->text().trimmed();

  if (fullName.isEmpty() || password.isEmpty() || citizenId.isEmpty() ||
      phone.isEmpty()) {
    QMessageBox::warning(this, "Thiếu thông tin",
                         "Vui lòng nhập đầy đủ các trường bắt buộc (*)");
    return;
  }

  // 2. Enum / Date fields
  Gender gender = (m_cbGender->currentText() == "Nam")  ? Gender::Male
                  : (m_cbGender->currentText() == "Nữ") ? Gender::Female
                                                        : Gender::Other;

  QDate dob = m_dtDateOfBirth->date();
  QDate hireDate = m_dtHireDate->date();
  QString shift = m_cbShift->currentText();
  QString specialty = m_cbSpecialty->currentText();

  // Department: map từ combobox index. Ví dụ "1 - Khoa Khám Bệnh" -> 1
  int departmentId =
      m_cbDepartment->currentIndex() + 1; // Giả định index 0 -> Dept 1

  // 3. Thông tin y khoa
  QString licenseNumber = m_txtLicenseNumber->text().trimmed();
  int experienceYears = m_sbExperienceYears->value();
  int consultationFee = m_sbConsultationFee->value();
  QString bio = m_txtBio->toPlainText().trimmed();

  // Mặc định avatar trống cho lúc khởi tạo
  QPixmap avatar;

  // 4. Gọi Service
  bool success = m_staffService->hireNewDoctor(
      staffCode, password, fullName, avatar, gender, dob, citizenId, phone,
      email, address, departmentId, hireDate, shift, specialty, licenseNumber,
      experienceYears, consultationFee, bio);

  if (success) {
    QMessageBox::information(this, "Thành công",
                             "Tạo tài khoản Bác sĩ thành công!");
    // Clear fields sau khi tạo
    m_txtFullName->clear();
    m_txtPassword->clear();
    m_txtStaffCode->clear();
    m_txtCitizenId->clear();
    m_txtPhone->clear();
    m_txtEmail->clear();
    m_txtAddress->clear();
    m_txtLicenseNumber->clear();
    m_txtBio->clear();
    showManageDoctorsList();
  } else {
    QMessageBox::critical(
        this, "Thất bại",
        "Tạo tài khoản thất bại! Vui lòng kiểm tra lại thông tin (CMND, SĐT "
        "không hợp lệ hoặc mật khẩu quá ngắn).");
  }
}
