#include "MainWindow.h"
#include "Admin/AdminDashboard.h"
#include "Doctor/DoctorDashboard.h"
#include "Doctor/PatientDashboard.h"
#include "Reception/ReceptionDashboard.h"
#include "Pharmacy/PharmacistDashboard.h"
#include "Nurse/NurseDashboard.h"
#include "model/CommonEnums.h"
#include "model/IAuthenticatable.h"
#include "model/SystemUser.h"
#include "model/CommonEnums.h"
#include "service/UserSession.h"
#include "service/PharmacyService.h"
#include "service/BillingService.h"
#include "service/ServiceRequestService.h"
#include "repository/MedicationRepository.h"
#include "repository/PrescriptionRepository.h"
#include <QDialog>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>

MainWindow::MainWindow(std::shared_ptr<AuthService> authService, 
                       std::shared_ptr<StaffService> staffService, 
                       std::shared_ptr<PatientService> patientService, 
                       std::shared_ptr<AppointmentService> appointmentService, 
                       std::shared_ptr<MedicalRecordService> medicalRecordService, 
                       std::shared_ptr<PharmacyService> pharmacyService, 
                       std::shared_ptr<BillingService> billingService,
                       std::shared_ptr<ServiceRequestService> serviceRequestService,
                       std::shared_ptr<AnalyticService> analyticService,
                       QWidget *parent) : 
  QMainWindow(parent), 
  m_authService(std::move(authService)), 
  m_staffService(std::move(staffService)), 
  m_patientService(std::move(patientService)), 
  m_appointmentService(std::move(appointmentService)), 
  m_medicalRecordService(std::move(medicalRecordService)), 
  m_pharmacyService(std::move(pharmacyService)),
  m_billingService(std::move(billingService)),
  m_serviceRequestService(std::move(serviceRequestService)),
  m_analyticService(std::move(analyticService)) {
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  setWindowTitle("Hệ thống Quản lý Phòng khám Thông minh");
  this->setMinimumSize(800, 500);

  QScreen *screen = QGuiApplication::primaryScreen();
  if (screen) {
    QRect screenGeometry = screen->geometry();

    int width = 1000; // Nếu vẫn hơi thiếu bạn tăng lên 850
    int height = 610; // Nếu vẫn hơi thiếu bạn tăng lên 550

    this->resize(width, height);
    this->move(screenGeometry.center() - this->rect().center());
  }

  m_stackedWidget = new QStackedWidget(this);
  setCentralWidget(m_stackedWidget);

  m_loginWidget = new LoginDialog(m_authService, m_staffService, this);
  m_stackedWidget->addWidget(m_loginWidget);

  connect(m_loginWidget, &LoginDialog::loginSucceeded, this,
          [this](std::shared_ptr<IAuthenticatable> user) {
            if (!user)
              return;

            UserSession::getInstance().setCurrentAccount(user);

            if (user->getAccountType() == AccountType::Staff) {
              auto staffUser = std::dynamic_pointer_cast<SystemUser>(user);
              if (staffUser) {
                if (staffUser->getRole() == UserRole::Doctor) {
                  switchToDoctorDashboard(user);
                } else if (staffUser->getRole() == UserRole::Admin) {
                  switchToAdminDashboard(user);
                } else if (staffUser->getRole() == UserRole::Nurse) {
                  switchToNurseDashboard(user);
                } else if (staffUser->getRole() == UserRole::Receptionist) {
                  switchToReceptionDashboard(user);
                } else if (staffUser->getRole() == UserRole::Pharmacist) {
                  switchToPharmacistDashboard(user);
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
    int width = screenGeometry.width();
    int height = screenGeometry.height();

    this->resize(width, height);
    this->move(screenGeometry.center() - this->rect().center());
  }

  this->showMaximized();
}

void MainWindow::switchToDoctorDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  auto dashboard = new DoctorDashboardWidget(user, m_staffService, m_patientService, m_appointmentService, m_medicalRecordService, m_pharmacyService, m_serviceRequestService, m_analyticService, this);
  registerDashboardPage(dashboard);
}

void MainWindow::switchToAdminDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  try {
    auto dashboard = new AdminDashboardWidget(
        user, m_staffService, m_patientService, m_appointmentService, m_analyticService, this);
    registerDashboardPage(dashboard);
  } catch (const std::exception &e) {
    QMessageBox::critical(this, "Lỗi", QString("Exception: %1").arg(e.what()));
  } catch (...) {
    QMessageBox::critical(this, "Lỗi",
                          "Unknown exception during AdminDashboard creation.");
  }
}

void MainWindow::switchToReceptionDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  auto dashboard = new ReceptionDashboardWidget(
      user, m_staffService, m_patientService, m_appointmentService,
      m_analyticService, this);
  registerDashboardPage(dashboard);
}

void MainWindow::switchToPatientDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  auto dashboard = new PatientDashboardWidget(
      user, m_staffService, m_patientService, m_appointmentService, this);
  registerDashboardPage(dashboard);
}

void MainWindow::switchToPharmacistDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  auto dashboard = new PharmacistDashboardWidget(
      user, m_staffService, m_medicalRecordService, m_pharmacyService, m_billingService, this);
  registerDashboardPage(dashboard);
}

void MainWindow::switchToNurseDashboard(
    std::shared_ptr<IAuthenticatable> user) {
  auto dashboard = new NurseDashboardWidget(
      user, m_staffService, m_serviceRequestService, this);
  registerDashboardPage(dashboard);
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
    UserSession::getInstance().clear();
    m_stackedWidget->setCurrentIndex(0);

    this->showNormal();

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
      QRect screenGeometry = screen->geometry();

      int width = 1000;
      int height = 610;

      this->resize(width, height);
      this->move(screenGeometry.center() - this->rect().center());
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
