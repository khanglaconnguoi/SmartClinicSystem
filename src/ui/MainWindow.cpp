#include "MainWindow.h"
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {    
    setWindowTitle("Smart Clinic System");
    resize(400, 300);

    QLabel *label = new QLabel("Hello world", this);
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
}
