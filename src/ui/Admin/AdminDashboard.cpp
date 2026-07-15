#include "AdminDashboard.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/Doctor.h"
#include "ManageDoctorsWidget.h"
#include "ManageNursesWidget.h"
#include "ManagePatientsWidget.h"
#include "ManageReceptionWidget.h"
#include "../../service/PatientService.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

AdminDashboardWidget::AdminDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<AppointmentService> appointmentService,
    QWidget *parent)
    : BaseDashboardWidget(user, staffService, patientService, appointmentService, parent), m_staffService(staffService),
      m_patientService(patientService), m_currentUser(user), m_manageDoctorsPage(nullptr),
      m_manageNursesPage(nullptr), m_managePatientsPage(nullptr),
      m_manageReceptionPage(nullptr) {
  initializeDashboard();
}

void AdminDashboardWidget::fillDashboardData() {
  if (m_currentUser && m_docNameLabel) {
    m_docNameLabel->setText("Admin: " + m_currentUser->getFullName());
  }

  // --- Sidebar Menu ---
  QPushButton *btnManageDoctors =
      new QPushButton("Quản lý Bác sĩ", m_sidebarFrame);
  QPushButton *btnManageNurses =
      new QPushButton("Quản lý Y tá", m_sidebarFrame);
  QPushButton *btnManagePatients =
      new QPushButton("Quản lý Bệnh nhân", m_sidebarFrame);
  QPushButton *btnManageReception =
      new QPushButton("Quản lý Lễ tân", m_sidebarFrame);

  auto setSidebarBtnStyle = [](QPushButton *btn) {
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedSize(260, 50);
    btn->setStyleSheet(
        "QPushButton { background-color: transparent; color: #6B7280; "
        "font-size: 15px; font-weight: bold; border-radius: 8px; border: none; "
        "text-align: left; padding-left: 15px; }"
        "QPushButton:hover { background-color: #F3F4F6; color: #111827; }");
  };

  setSidebarBtnStyle(btnManageDoctors);
  setSidebarBtnStyle(btnManageNurses);
  setSidebarBtnStyle(btnManagePatients);
  setSidebarBtnStyle(btnManageReception);

  // Default active style
  btnManageDoctors->setStyleSheet(
      "QPushButton { background-color: #EBF5FF; color: #4B94F2; font-size: "
      "15px; font-weight: bold; border-radius: 8px; border: none; text-align: "
      "left; padding-left: 15px; }");

  m_sidebarLayout->addWidget(btnManageDoctors);
  m_sidebarLayout->addWidget(btnManageNurses);
  m_sidebarLayout->addWidget(btnManagePatients);
  m_sidebarLayout->addWidget(btnManageReception);
  m_sidebarLayout->addStretch(); // Đẩy menu lên trên

  m_btnLogout = new QPushButton("Đăng xuất", m_sidebarFrame);
  m_btnLogout->setStyleSheet(
      "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; "
      "color: #D32F2F; border: none; border-radius: 0px; background-color: "
      "transparent; font-weight: bold; }"
      "QPushButton:hover { background-color: #FFEBEE; }");
  m_btnLogout->setCursor(Qt::PointingHandCursor);
  m_sidebarLayout->addWidget(m_btnLogout);
  connect(m_btnLogout, &QPushButton::clicked, this,
          &BaseDashboardWidget::logoutRequested);

  // --- Main Content ---
  m_mainContentWidget->setObjectName("MainContentWidget");
  m_stackedWidget = new QStackedWidget(m_mainContentWidget);
  m_stackedWidget->setObjectName("StackedWidget");
  m_stackedWidget->setStyleSheet(
      "QStackedWidget#StackedWidget > QWidget { background-color: #EEF2F6; }");
  m_mainContentLayout->addWidget(m_stackedWidget, 1);

  // Khởi tạo các trang quản lý bằng widget độc lập
  m_manageDoctorsPage = new ManageDoctorsWidget(m_staffService, this);
  m_manageNursesPage = new ManageNursesWidget(m_staffService, this);
  m_managePatientsPage = new ManagePatientsWidget(m_patientService, m_stackedWidget);
  m_manageReceptionPage = new ManageReceptionWidget(m_staffService, this);

  m_stackedWidget->addWidget(m_manageDoctorsPage);
  m_stackedWidget->addWidget(m_manageNursesPage);
  m_stackedWidget->addWidget(m_managePatientsPage);
  m_stackedWidget->addWidget(m_manageReceptionPage);

  // Mặc định hiển thị trang Quản lý Bác sĩ
  m_stackedWidget->setCurrentWidget(m_manageDoctorsPage);

  // Events cho menu
  auto setActiveMenu = [=](QPushButton *activeBtn) {
      QString activeStyle = "QPushButton { background-color: #EBF5FF; color: #4B94F2; font-size: 15px; font-weight: bold; border-radius: 8px; border: none; text-align: left; padding-left: 15px; }";
      QString inactiveStyle = "QPushButton { background-color: transparent; color: #6B7280; font-size: 15px; font-weight: bold; border-radius: 8px; border: none; text-align: left; padding-left: 15px; } QPushButton:hover { background-color: #F3F4F6; color: #111827; }";
      
      btnManageDoctors->setStyleSheet(activeBtn == btnManageDoctors ? activeStyle : inactiveStyle);
      btnManageNurses->setStyleSheet(activeBtn == btnManageNurses ? activeStyle : inactiveStyle);
      btnManagePatients->setStyleSheet(activeBtn == btnManagePatients ? activeStyle : inactiveStyle);
      btnManageReception->setStyleSheet(activeBtn == btnManageReception ? activeStyle : inactiveStyle);
  };

  connect(btnManageDoctors, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_manageDoctorsPage);
      m_manageDoctorsPage->loadDoctorsList();
      setActiveMenu(btnManageDoctors);
  });

  connect(btnManageNurses, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_manageNursesPage);
      m_manageNursesPage->loadNursesList();
      setActiveMenu(btnManageNurses);
  });

  connect(btnManagePatients, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_managePatientsPage);
      m_managePatientsPage->loadPatientsList();
      setActiveMenu(btnManagePatients);
  });

  connect(btnManageReception, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_manageReceptionPage);
      m_manageReceptionPage->loadReceptionList();
      setActiveMenu(btnManageReception);
  });
}
