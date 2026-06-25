#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <memory>
#include "LoginWidget.h"
#include "BaseDashboard.h"
#include "../service/AuthService.h" 

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void handleGlobalLogout();

private:
    void registerDashboardPage(BaseDashboardWidget* page);

    QStackedWidget* m_stackedWidget;
    LoginWidget* m_loginWidget;
    std::shared_ptr<AuthService> m_authService;
};