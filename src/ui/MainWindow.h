#pragma once
#include <QMainWindow>
<<<<<<< Updated upstream
#include <QStackedWidget>
#include <memory>
#include "loginwidget.h"
#include "dashboard.h"
#include "../service/AuthService.h" 
=======
#include <QPushButton>
#include <QTableWidget>
>>>>>>> Stashed changes

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
<<<<<<< Updated upstream
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget* m_stackedWidget; 
    LoginWidget* m_loginWidget;  
    std::shared_ptr<AuthService>  m_authService; 
=======
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupSidebar();
    void setupMainContent();
    void setupHeader();
    void setupStatsGrid();
    void setupMiddleGrid();
    void setupTable();

    QWidget *centralWidget;
    QWidget *sidebar;
    QWidget *mainContent;
    QTableWidget *appointmentsTable;
>>>>>>> Stashed changes
};