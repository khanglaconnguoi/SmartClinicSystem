//#include "ui/MainWindow.h"
#include "ui/logindialog.h"
#include "service/AuthService.h"
#include <QApplication>
#include <memory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 1. Khởi tạo dịch vụ xác thực AuthService
    auto authService = std::make_shared<AuthService>();

    LoginDialog w(authService);
    w.show();

    //MainWindow window;
    // window.show();

    return app.exec();
}