#include "AdminDashboard.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/Doctor.h"
#include "ManageDoctorsWidget.h"
#include "ManageNursesWidget.h"
#include "ManagePatientsWidget.h"
#include "ManageReceptionWidget.h"
#include "ManagePharmacistsWidget.h"
#include "ManageLeaveWidget.h"
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
    std::shared_ptr<AnalyticService> analyticService,
    QWidget *parent)
    : BaseDashboardWidget(user, staffService, parent),
      m_patientService(patientService), m_appointmentService(appointmentService), m_analyticService(analyticService), m_manageDoctorsPage(nullptr),
      m_manageNursesPage(nullptr),
      m_manageReceptionPage(nullptr), m_managePharmacistsPage(nullptr), m_analyticsPage(nullptr) {
  initializeDashboard();
}

void AdminDashboardWidget::fillDashboardData() {
  if (m_currentUser && m_nameLabel) {
    m_nameLabel->setText("Admin: " + m_currentUser->getFullName());
  }

  // --- Sidebar Menu ---
  m_btnAnalytics =
      new QPushButton("Thống kê & Báo cáo", m_sidebarFrame);
  m_btnManageDoctors =
      new QPushButton("Quản lý Bác sĩ", m_sidebarFrame);
  m_btnManageNurses =
      new QPushButton("Quản lý Y tá", m_sidebarFrame);
  m_btnManageReception =
      new QPushButton("Quản lý Lễ tân", m_sidebarFrame);
  m_btnManagePharmacists =
      new QPushButton("Quản lý Dược sĩ", m_sidebarFrame);
  m_btnManageLeaves =
      new QPushButton("Quản lý Nghỉ phép", m_sidebarFrame);

  m_btnAnalytics->setCursor(Qt::PointingHandCursor);
  m_btnManageDoctors->setCursor(Qt::PointingHandCursor);
  m_btnManageNurses->setCursor(Qt::PointingHandCursor);
  m_btnManageReception->setCursor(Qt::PointingHandCursor);
  m_btnManagePharmacists->setCursor(Qt::PointingHandCursor);
  m_btnManageLeaves->setCursor(Qt::PointingHandCursor);

  m_btnAnalytics->setObjectName("activeBtn"); // Default active

  m_sidebarLayout->addWidget(m_btnAnalytics);
  m_sidebarLayout->addWidget(m_btnManageDoctors);
  m_sidebarLayout->addWidget(m_btnManageNurses);
  m_sidebarLayout->addWidget(m_btnManageReception);
  m_sidebarLayout->addWidget(m_btnManagePharmacists);
  m_sidebarLayout->addWidget(m_btnManageLeaves);
  m_sidebarLayout->addStretch();

  m_btnLogout = new QPushButton("Đăng xuất", m_sidebarFrame);
  m_btnLogout->setStyleSheet(
      "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; "
      "color: #DC2626; border: none; border-radius: 8px; background-color: "
      "transparent; font-weight: bold; }"
      "QPushButton:hover { background-color: #FEE2E2; }");
  m_btnLogout->setCursor(Qt::PointingHandCursor);
  m_sidebarLayout->addStretch(1);
  m_sidebarLayout->addWidget(m_btnLogout);
  m_sidebarLayout->setContentsMargins(10, 10, 10, 65);
  connect(m_btnLogout, &QPushButton::clicked, this,
          &BaseDashboardWidget::logoutRequested);

  // --- Main Content ---
  m_mainContentWidget->setObjectName("MainContentWidget");
  m_stackedWidget = new QStackedWidget(m_mainContentWidget);
  m_stackedWidget->setObjectName("StackedWidget");
  m_stackedWidget->setStyleSheet(
      "QStackedWidget#StackedWidget > QWidget { background-color: #F8FAFC; }");
  m_mainContentLayout->addWidget(m_stackedWidget, 1);

  // Khởi tạo các trang quản lý bằng widget độc lập
  m_analyticsPage = new AdminAnalyticsWidget(m_analyticService, this);
  m_manageDoctorsPage = new ManageDoctorsWidget(m_staffService, m_appointmentService, this);
  m_manageNursesPage = new ManageNursesWidget(m_staffService, this);
  m_manageReceptionPage = new ManageReceptionWidget(m_staffService, this);
  m_managePharmacistsPage = new ManagePharmacistsWidget(m_staffService, this);
  m_manageLeavesPage = new ManageLeaveWidget(m_staffService, m_appointmentService, this);

  m_stackedWidget->addWidget(m_analyticsPage);
  m_stackedWidget->addWidget(m_manageDoctorsPage);
  m_stackedWidget->addWidget(m_manageNursesPage);
  m_stackedWidget->addWidget(m_manageReceptionPage);
  m_stackedWidget->addWidget(m_managePharmacistsPage);
  m_stackedWidget->addWidget(m_manageLeavesPage);

  // Mặc định hiển thị trang Thống kê & Báo cáo
  m_stackedWidget->setCurrentWidget(m_analyticsPage);

  // Events cho menu
  auto setActiveMenu = [=](QPushButton *activeBtn) {
      QPushButton* buttons[] = { m_btnAnalytics, m_btnManageDoctors, m_btnManageNurses, m_btnManageReception, m_btnManagePharmacists, m_btnManageLeaves };
      for (auto* btn : buttons) {
          if (btn) {
              btn->setObjectName("");
              btn->style()->unpolish(btn);
              btn->style()->polish(btn);
          }
      }
      if (activeBtn) {
          activeBtn->setObjectName("activeBtn");
          activeBtn->style()->unpolish(activeBtn);
          activeBtn->style()->polish(activeBtn);
      }
  };

  connect(m_btnAnalytics, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_analyticsPage);
      m_analyticsPage->loadAnalyticsData();
      setActiveMenu(m_btnAnalytics);
  });

  connect(m_btnManageDoctors, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_manageDoctorsPage);
      m_manageDoctorsPage->loadDoctorsList();
      setActiveMenu(m_btnManageDoctors);
  });

  connect(m_btnManageNurses, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_manageNursesPage);
      m_manageNursesPage->loadNursesList();
      setActiveMenu(m_btnManageNurses);
  });

  connect(m_btnManageReception, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_manageReceptionPage);
      m_manageReceptionPage->loadReceptionList();
      setActiveMenu(m_btnManageReception);
  });
  
  connect(m_btnManagePharmacists, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_managePharmacistsPage);
      m_managePharmacistsPage->loadPharmacistsList();
      setActiveMenu(m_btnManagePharmacists);
  });

  connect(m_btnManageLeaves, &QPushButton::clicked, this, [=]() {
      m_stackedWidget->setCurrentWidget(m_manageLeavesPage);
      setActiveMenu(m_btnManageLeaves);
  });
}
