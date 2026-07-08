// #include <QApplication>
// #include <memory>

// #include "ui/Profile.h"
// #include "repository/DatabaseManager.h"
// #include "repository/StaffRepository.h"
// #include "service/StaffService.h"

// int main(int argc, char *argv[]) {
//     QApplication app(argc, argv);
    
//     // 1. Khởi tạo kết nối DB (DatabaseManager của bạn)
//     DatabaseManager::getInstance();

//     // 2. Khởi tạo các thành phần Dependency Injection
//     auto staffRepo = std::make_shared<StaffRepository>();
//     auto staffService = std::make_shared<StaffService>(staffRepo);

//     // 3. Khởi tạo cửa sổ Profile và tiêm (inject) StaffService vào
//     ProfileWidget profileWindow(staffService);
    
//     profileWindow.loadProfile(2); 
    
//     profileWindow.show();

//     return app.exec();
// }

#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}