#include "MainWindow.h"
#include "model/IAuthenticatable.h"
#include "model/SystemUser.h"
#include "repository/StaffRepository.h"
#include "Admin/AdminDashboard.h"
#include "Doctor/DoctorDashboard.h"
#include "Patient/PatientDashboard.h"
#include "Reception/ReceptionDashboard.h"
#include <QDialog>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Hệ thống Quản lý Phòng khám Thông minh");
  this->setFixedSize(1000, 600);
  
  QScreen *screen = QGuiApplication::primaryScreen();
  if (screen) {
      this->move(screen->geometry().center() - this->rect().center());
  }
  
  auto staffRepo = std::make_shared<StaffRepository>();
  m_authService = std::make_shared<AuthService>(staffRepo);
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
                  QMessageBox::information(
                      this, "Thông báo",
                      "Giao diện Điều dưỡng đang được phát triển.");
                } else if (staffUser->getRole() == UserRole::Receptionist) {
                  switchToReceptionDashboard(user);
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

  this->setMinimumSize(1000, 600);
  this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

  QScreen *screen = QGuiApplication::primaryScreen();
  if (screen) {
    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width() * 0.85;
    int height = screenGeometry.height() * 0.85;

    // preventing further resize.
    this->setFixedSize(width, height);

    // Center the window on the screen
    this->move(screenGeometry.center() - this->rect().center());
  }

  this->showNormal();
}

void MainWindow::switchToDoctorDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  auto *doctorDash = new DoctorDashboardWidget(user, this);
  registerDashboardPage(doctorDash);
}

void MainWindow::switchToAdminDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  try {
    AdminDashboardWidget *adminDashboard =
        new AdminDashboardWidget(user, m_staffService, this);
    registerDashboardPage(adminDashboard);
  } catch (const std::exception &e) {
    QMessageBox::critical(this, "Lỗi",
                          QString("Exception: %1").arg(e.what()));
  } catch (...) {
    QMessageBox::critical(this, "Lỗi",
                          "Unknown exception during AdminDashboard creation.");
  }
}

void MainWindow::switchToReceptionDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  auto *receptionDash = new ReceptionDashboardWidget(user, m_staffService, this);
  registerDashboardPage(receptionDash);
}

void MainWindow::switchToPatientDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  auto *patientDash = new PatientDashboardWidget(user, this);
  registerDashboardPage(patientDash);
}

void MainWindow::handleGlobalLogout() {
  QDialog dialog(this);
  dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  dialog.setAttribute(Qt::WA_TranslucentBackground);
  dialog.setFixedSize(350, 150);

  QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  QFrame *frame = new QFrame(&dialog);
  frame->setStyleSheet("QFrame { "
                       "   background-color: #FFFFFF; "
                       "   border-radius: 12px; "
                       "   border: 1px solid #EAEAEA; "
                       "}");
  QVBoxLayout *frameLayout = new QVBoxLayout(frame);
  frameLayout->setContentsMargins(20, 20, 20, 20);
  frameLayout->setSpacing(15);

  QLabel *lblTitle = new QLabel("Xác nhận đăng xuất", frame);
  lblTitle->setStyleSheet(
      "font-size: 16px; font-weight: bold; color: #111827; border: none;");
  frameLayout->addWidget(lblTitle);

  QLabel *lblMessage = new QLabel("Bạn có chắc chắn muốn đăng xuất?", frame);
  lblMessage->setStyleSheet("font-size: 14px; color: #4B5563; border: none;");
  frameLayout->addWidget(lblMessage);

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  QPushButton *btnCancel = new QPushButton("Hủy", frame);
  btnCancel->setCursor(Qt::PointingHandCursor);
  btnCancel->setStyleSheet("QPushButton { "
                           "   background-color: #F1F3F4; color: #5F6368; "
                           "   border: none; border-radius: 6px; padding: 8px "
                           "20px; font-weight: bold; font-size: 13px; "
                           "}"
                           "QPushButton:hover { background-color: #E8EAED; }");

  QPushButton *btnConfirm = new QPushButton("Đồng ý", frame);
  btnConfirm->setCursor(Qt::PointingHandCursor);
  btnConfirm->setStyleSheet("QPushButton { "
                            "   background-color: #D32F2F; color: #FFFFFF; "
                            "   border: none; border-radius: 6px; padding: 8px "
                            "20px; font-weight: bold; font-size: 13px; "
                            "}"
                            "QPushButton:hover { background-color: #B71C1C; }");

  btnLayout->addWidget(btnCancel);
  btnLayout->addWidget(btnConfirm);
  frameLayout->addLayout(btnLayout);

  mainLayout->addWidget(frame);

  connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
  connect(btnConfirm, &QPushButton::clicked, &dialog, &QDialog::accept);

  if (dialog.exec() == QDialog::Accepted) {
    m_stackedWidget->setCurrentIndex(0);

    this->showNormal();
    this->setFixedSize(1000, 600);
    
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        this->move(screen->geometry().center() - this->rect().center());
    }
    
    m_loginWidget->clearFields();

    QWidget *currentDash = m_stackedWidget->widget(1);
    if (currentDash && currentDash != m_loginWidget) {
      m_stackedWidget->removeWidget(currentDash);
      currentDash->deleteLater();
    }
  }
}

MainWindow::~MainWindow() {}