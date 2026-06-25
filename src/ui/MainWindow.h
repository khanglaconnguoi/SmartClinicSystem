#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <memory>
#include "loginwidget.h"
#include "dashboard.h"
#include "../service/AuthService.h" 

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget* m_stackedWidget; 
    LoginWidget* m_loginWidget;  
    std::shared_ptr<AuthService>  m_authService; 
};