
#include "ui/MainWindow.h"
#include "ui/logindialog.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    LoginDialog w;
    w.show();

    MainWindow window;
    window.show();

    return app.exec();
}