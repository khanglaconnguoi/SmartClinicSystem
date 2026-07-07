#include "MainWindow.h"
#include "../model/IAuthenticatable.h"
#include "../model/SystemUser.h"
#include "Doctor/DoctorDashboard.h"
#include "Patient/PatientDashboard.h"
#include "Admin/AdminDashboard.h"
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Smart Clinic System");
  resize(1000, 600);
  m_authService = std::make_shared<AuthService>();
  auto staffRepo = std::make_shared<StaffRepository>();
  m_staffService = std::make_shared<StaffService>(staffRepo);

  m_stackedWidget = new QStackedWidget(this);
  setCentralWidget(m_stackedWidget);

  m_loginWidget = new LoginDialog(m_authService, this);
  m_stackedWidget->addWidget(m_loginWidget);

  connect(m_loginWidget, &LoginDialog::loginSucceeded, this,
          [this](std::shared_ptr<IAuthenticatable> user) {
            if (!user)
              return;

            if (user->getAccountType() == AccountType::Staff) {
              auto staffUser = std::dynamic_pointer_cast<SystemUser>(user);
              if (staffUser) {
                if (staffUser->getRole() == UserRole::Doctor) {
                  switchToDoctorDashboard(user);
                } else if (staffUser->getRole() == UserRole::Admin) {
                  switchToAdminDashboard(user);
                } else if (staffUser->getRole() == UserRole::Nurse) {
                  QMessageBox::information(this, "Thông báo", "Giao diện Điều dưỡng đang được phát triển.");
                } else if (staffUser->getRole() == UserRole::Receptionist) {
                  QMessageBox::information(this, "Thông báo", "Giao diện Lễ tân đang được phát triển.");
                }
              }
            } else if (user->getAccountType() == AccountType::Patient) {
              switchToPatientDashboard(user);
            }
          });
}

void MainWindow::registerDashboardPage(BaseDashboardWidget *page) {
  if (!page)
    return;

  while (m_stackedWidget->count() > 1) {
    QWidget *oldDash = m_stackedWidget->widget(1);
    m_stackedWidget->removeWidget(oldDash);
    oldDash->deleteLater();
  }

  int newIndex = m_stackedWidget->addWidget(page);

  connect(page, &BaseDashboardWidget::logoutRequested, this,
          &MainWindow::handleGlobalLogout);

  m_stackedWidget->setCurrentIndex(newIndex);
}

void MainWindow::switchToDoctorDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  auto *doctorDash = new DoctorDashboardWidget(user, this);
  registerDashboardPage(doctorDash);
}

void MainWindow::switchToAdminDashboard(std::shared_ptr<IAuthenticatable> user) {
  try {
    AdminDashboardWidget *adminDashboard = new AdminDashboardWidget(user, m_staffService, this);
    connect(adminDashboard, &BaseDashboardWidget::logoutRequested, this, &MainWindow::handleGlobalLogout);
    registerDashboardPage(adminDashboard);
    this->showMaximized();
  } catch (const std::exception& e) {
    QMessageBox::critical(this, "Error", QString("Exception: %1").arg(e.what()));
  } catch (...) {
    QMessageBox::critical(this, "Error", "Unknown exception during AdminDashboard creation.");
  }
}

void MainWindow::switchToPatientDashboard(std::shared_ptr<IAuthenticatable> user) {
  auto *patientDash = new PatientDashboardWidget(user, this);
  registerDashboardPage(patientDash);
  this->showMaximized();
}

void MainWindow::handleGlobalLogout() {
  QMessageBox msgBox(this);
  msgBox.setWindowTitle("Xác nhận đăng xuất");

  msgBox.setText("<p style='color: #202124; font-family: \"Arial\"; font-size: "
                 "15px; font-weight: 500; margin-left: 10px;'>"
                 "Bạn có chắc chắn muốn đăng xuất?</p>");
  msgBox.setIcon(QMessageBox::Question);

  QPushButton *yesButton = msgBox.addButton("Đồng Ý", QMessageBox::YesRole);
  QPushButton *noButton = msgBox.addButton("Hủy", QMessageBox::NoRole);

  yesButton->setCursor(Qt::PointingHandCursor);
  noButton->setCursor(Qt::PointingHandCursor);

  msgBox.setStyleSheet(
      "QMessageBox { "
      "   background-color: #FFFFFF; "
      "   border: 1px solid #EAEAEA; "
      "   border-radius: 12px; "
      "}"
      "QPushButton { "
      "   background-color: #4B94F2; "
      "   color: #FFFFFF; "
      "   font-family: 'Arial'; "
      "   font-size: 13px; "
      "   font-weight: bold; "
      "   border: none; "
      "   border-radius: 6px; "
      "   padding: 8px 20px; "
      "   min-width: 75px; "
      "}"
      "QPushButton:hover { background-color: #398CBF; }"
      "QPushButton:pressed { background-color: #62B7D9; }"
      "QPushButton[text=\"Hủy\"] { "
      "   background-color: #F1F3F4; "
      "   color: #5F6368; "
      "   border: 1px solid #DADCE0; "
      "}"
      "QPushButton[text=\"Hủy\"]:hover { background-color: #E8EAED; color: #3C4043; }"
      "QPushButton[text=\"Hủy\"]:pressed { background-color: #D2D6DE; }");

  msgBox.exec();

  if (msgBox.clickedButton() == yesButton) {
    m_stackedWidget->setCurrentIndex(0);

    this->showNormal();
    this->resize(1000, 600);
    m_loginWidget->clearFields();

    QWidget *currentDash = m_stackedWidget->widget(1);
    if (currentDash && currentDash != m_loginWidget) {
      m_stackedWidget->removeWidget(currentDash);
      currentDash->deleteLater();
    }
  }
}

MainWindow::~MainWindow() {}
