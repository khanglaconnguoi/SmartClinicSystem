#include "ui/profile.h"
#include "repository/DatabaseManager.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    DatabaseManager::getInstance();
    ProfileWidget profileWindow;
    profileWindow.loadDoctorProfile("D2603");
    
    profileWindow.show();

    return app.exec();
}
/*#include "ui/Doctor/DoctorDashboard.h"
#include "repository/DatabaseManager.h"
#include <QApplication>
#include <memory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    DatabaseManager::getInstance();
    
    DoctorDashboardWidget dashboardWindow(nullptr);
    dashboardWindow.resize(1280, 800);
    dashboardWindow.show();

    return app.exec();
}
*/