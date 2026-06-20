#include "logindialog.h"
#include <QApplication>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox> 
#include "AuthService.h" 
LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(1000, 700);

    QString appPath = QApplication::applicationDirPath();

    btnClose = new QPushButton("x", this);
    btnClose->setGeometry(955, 15, 30, 30);
    btnClose->setStyleSheet("QPushButton { color: white; font-size: 18px; background: transparent; border: none; }"
                           "QPushButton:hover { color: #FFCDD2; }");
    connect(btnClose, &QPushButton::clicked, this, &LoginDialog::close);

    QWidget *cardWidget = new QWidget(this);
    cardWidget->setGeometry(40, 70, 920, 560);
    cardWidget->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 12px; }");

    QGraphicsDropShadowEffect *cardShadow = new QGraphicsDropShadowEffect(this);
    cardShadow->setBlurRadius(15);
    cardShadow->setColor(QColor(0, 0, 0, 60));
    cardShadow->setOffset(0, 5);
    cardWidget->setGraphicsEffect(cardShadow);

    QWidget *leftForm = new QWidget(cardWidget);
    leftForm->setGeometry(0, 100, 440, 460);
    leftForm->setStyleSheet("background: transparent;");

    lblLeftLogo = new QLabel(leftForm);
    lblLeftLogo->setGeometry(110, 10, 220, 110);
    lblLeftLogo->setPixmap(QPixmap(appPath + "/logo.png"));
    lblLeftLogo->setScaledContents(true);

    lblSlogan = new QLabel("NovaCare - Nâng cao sức khỏe", leftForm);
    lblSlogan->setGeometry(0, 130, 440, 20);
    lblSlogan->setAlignment(Qt::AlignCenter);
    lblSlogan->setStyleSheet("font-size: 13px; font-weight: bold; color: #757575;");

    txtAccount = new QLineEdit(leftForm);
    txtAccount->setGeometry(50, 180, 360, 48);
    txtAccount->setPlaceholderText("Nhập tài khoản");
    txtAccount->setStyleSheet("QLineEdit { font-size: 14px; padding-left: 45px; border: 1px solid #E0E0E0; border-radius: 6px; background-color: white; color: #333333; }"
                             "QLineEdit:focus { border: 1px solid #00969A; }");

    QLabel *icoUser = new QLabel("👤", txtAccount);
    icoUser->setGeometry(15, 14, 20, 20);
    icoUser->setStyleSheet("font-size: 16px; color: #00969A; border: none; background: transparent;");

    txtPassword = new QLineEdit(leftForm);
    txtPassword->setGeometry(50, 245, 360, 48);
    txtPassword->setPlaceholderText("Nhập mật khẩu");
    txtPassword->setEchoMode(QLineEdit::Password);

    QPushButton *btnEye = new QPushButton("👁️", txtPassword);
    btnEye->setGeometry(320, 9, 30, 30);
    btnEye->setCursor(Qt::PointingHandCursor);
    btnEye->setStyleSheet("QPushButton { background: transparent; border: none; font-size: 16px; }");

    connect(btnEye, &QPushButton::clicked, this, [=]() {
        if (txtPassword->echoMode() == QLineEdit::Password) {
            txtPassword->setEchoMode(QLineEdit::Normal);
            btnEye->setText("🙈");
        } else {
            txtPassword->setEchoMode(QLineEdit::Password);
            btnEye->setText("👁️");
        }
    });

    txtPassword->setStyleSheet(
        "QLineEdit { "
        "   font-size: 14px; "
        "   padding-left: 45px; "
        "   padding-right: 45px; "
        "   border: 1px solid #E0E0E0; "
        "   border-radius: 6px; "
        "   background-color: white; "
        "   color: #333333; "
        "} "
        "QLineEdit:focus { "
        "   border: 1px solid #00969A; "
        "}"
    );

    QLabel *icoLock = new QLabel("🔒︎", txtPassword);
    icoLock->setGeometry(15, 14, 20, 20);
    icoLock->setStyleSheet("font-size: 16px; color: #00969A; border: none; background: transparent;");

    btnLogin = new QPushButton("Đăng nhập", leftForm);
    btnLogin->setGeometry(50, 320, 360, 48);
    btnLogin->setStyleSheet(
        "QPushButton { background-color: #00969A; color: white; font-size: 16px; font-weight: bold; border-radius: 6px; border: none; }"
        "QPushButton:hover { background-color: #00838F; }");

    btnForgot = new QPushButton("Quên mật khẩu?", leftForm);
    btnForgot->setGeometry(50, 385, 360, 30);
    btnForgot->setStyleSheet(
        "QPushButton { color: #007A7E; font-size: 13px; background: transparent; border: none; }"
        "QPushButton:hover { color: #00969A; }");

    rightContainer = new QWidget(cardWidget);
    rightContainer->setGeometry(440, 0, 480, 560);
    rightContainer->setStyleSheet(
        "QWidget { "
        "   background-color: #E5F7F8; "
        "   border-top-right-radius: 12px; "
        "   border-bottom-right-radius: 12px; "
        "   border-top-left-radius: 0px; "
        "   border-bottom-left-radius: 0px; "
        "}"
    );

    lblHello = new QLabel("HELLO !", rightContainer);
    lblHello->setGeometry(40, 60, 400, 50);
    lblHello->setStyleSheet("font-size: 42px; font-weight: bold; color: #007A7E; background: transparent;");

    lblSubDetails = new QLabel("Please enter your details\nto continue", rightContainer);
    lblSubDetails->setGeometry(40, 120, 400, 50);
    lblSubDetails->setStyleSheet("font-size: 16px; color: #555555; background: transparent;");

    lblDoctor = new QLabel(rightContainer);
    lblDoctor->setGeometry(40, 160, 430, 400);
    lblDoctor->setPixmap(QPixmap(appPath + "/doctor.png"));
    lblDoctor->setScaledContents(true);
    lblDoctor->setStyleSheet("background: transparent;");
}

LoginDialog::~LoginDialog() {}

void LoginDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#00969A"));
    painter.drawRoundedRect(rect(), 0, 0);

    painter.setBrush(Qt::NoBrush);

    QPen pen1(QColor("#B2DFDB"), 4);
    painter.setPen(pen1);
    painter.setOpacity(0.4);
    painter.drawEllipse(480 - 30, 170 - 30, 120, 120);

    painter.setOpacity(0.3);
    QPen pen2(QColor("#B2DFDB"), 3);
    painter.setPen(pen2);
    painter.drawEllipse(480, 170, 60, 60);

    painter.setOpacity(0.4);
    painter.drawEllipse(960 - 120, 170 + 20, 80, 80);

    painter.setOpacity(0.2);
    QPen pen3(QColor("#00969A"), 2);
    painter.setPen(pen3);
    painter.drawEllipse(960 - 105, 170 + 45, 40, 40);

    painter.setOpacity(0.15);
    QPen pen4(QColor("#00969A"), 4);
    painter.setPen(pen4);
    painter.drawEllipse(480 - 40, 660 - 180, 160, 160);

    painter.setOpacity(1.0);
}