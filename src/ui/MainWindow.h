// MainWindow.h
#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <memory>
#include "view/LoginDialog.h"
#include "BaseDashboard.h"
#include "service/AuthService.h" 
#include "service/StaffService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<AuthService> authService, 
                        std::shared_ptr<StaffService> staffService, 
                        std::shared_ptr<PatientService> patientService,
                        std::shared_ptr<AppointmentService> appointmentService,
                        QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void handleGlobalLogout();

private:
    void registerDashboardPage(BaseDashboardWidget* page);
    void switchToDoctorDashboard(std::shared_ptr<IAuthenticatable> user);
    void switchToPatientDashboard(std::shared_ptr<IAuthenticatable> user);
    void switchToAdminDashboard(std::shared_ptr<IAuthenticatable> user);
    void switchToReceptionDashboard(std::shared_ptr<IAuthenticatable> user);

    QStackedWidget* m_stackedWidget;
    LoginDialog* m_loginWidget;
    std::shared_ptr<AuthService> m_authService;
    std::shared_ptr<StaffService> m_staffService;
    std::shared_ptr<PatientService> m_patientService;
    std::shared_ptr<AppointmentService> m_appointmentService;
};