#include "LoginDialog.h"
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include "service/AuthService.h"
#include "model/IAuthenticatable.h"
#include "service/UserSession.h"

LoginDialog::LoginDialog(std::shared_ptr<AuthService> authService, QWidget *parent)
    : QWidget(parent), m_authService(authService)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #4B94F2;");

    QString appPath = QApplication::applicationDirPath();

    QWidget *cardWidget = new QWidget(this);
    cardWidget->setGeometry(40, 20, 920, 560);
    cardWidget->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 12px; }");

    QGraphicsDropShadowEffect *cardShadow = new QGraphicsDropShadowEffect(this);
    cardShadow->setBlurRadius(15);
    cardShadow->setColor(QColor(0, 0, 0, 30));
    cardShadow->setOffset(0, 4);
    cardWidget->setGraphicsEffect(cardShadow);

    // =========================================================
    // FORM BÊN TRÁI
    // =========================================================
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
    txtAccount->setStyleSheet(
        "QLineEdit { font-size: 14px; padding-left: 45px; border: 1px solid #E0E0E0; border-radius: 6px; background-color: white; color: #333333; }"
        "QLineEdit:focus { border: 1px solid #4B94F2; }"
    );

    QLabel *icoUser = new QLabel("👤", txtAccount);
    icoUser->setGeometry(15, 14, 20, 20);
    icoUser->setStyleSheet("font-size: 16px; color: #4B94F2; border: none; background: transparent;");

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
        "QLineEdit { font-size: 14px; padding-left: 45px; padding-right: 45px; border: 1px solid #E0E0E0; border-radius: 6px; background-color: white; color: #333333; }"
        "QLineEdit:focus { border: 1px solid #4B94F2; }"
    );

    QLabel *icoLock = new QLabel("🔒︎", txtPassword);
    icoLock->setGeometry(15, 14, 20, 20);
    icoLock->setStyleSheet("font-size: 16px; color: #4B94F2; border: none; background: transparent;");

    btnLogin = new QPushButton("Đăng nhập", leftForm);
    btnLogin->setGeometry(50, 320, 360, 48);
    btnLogin->setCursor(Qt::PointingHandCursor);
    btnLogin->setStyleSheet(
        "QPushButton { background-color: #4B94F2; color: white; font-size: 16px; font-weight: bold; border-radius: 6px; border: none; }"
        "QPushButton:hover { background-color: #357AE8; }"
        "QPushButton:pressed { background-color: #245DC1; }"
    );

    connect(btnLogin,   &QPushButton::clicked,        this, &LoginDialog::handleLogin);
    connect(txtPassword, &QLineEdit::returnPressed,   this, &LoginDialog::handleLogin);

    btnForgot = new QPushButton("Quên mật khẩu?", leftForm);
    btnForgot->setGeometry(50, 385, 360, 30);
    btnForgot->setStyleSheet(
        "QPushButton { color: #357AE8; font-size: 13px; background: transparent; border: none; }"
        "QPushButton:hover { color: #4B94F2; text-decoration: underline; }"
    );

    // =========================================================
    // KHUNG CHÀO ĐÓN BÊN PHẢI
    // =========================================================
    rightContainer = new QWidget(cardWidget);
    rightContainer->setGeometry(440, 0, 480, 560);
    rightContainer->setStyleSheet(
        "QWidget { "
        "   background-color: #F0F6FF; "
        "   border-top-right-radius: 12px; "
        "   border-bottom-right-radius: 12px; "
        "   border-top-left-radius: 0px; "
        "   border-bottom-left-radius: 0px; "
        "}"
    );

    lblHello = new QLabel("XIN CHÀO !", rightContainer);
    lblHello->setGeometry(40, 60, 400, 50);
    lblHello->setStyleSheet("font-size: 42px; font-weight: bold; color: #357AE8; background: transparent;");

    lblSubDetails = new QLabel("Vui lòng nhập thông tin tài khoản\nđể tiếp tục làm việc", rightContainer);
    lblSubDetails->setGeometry(40, 120, 400, 50);
    lblSubDetails->setStyleSheet("font-size: 16px; color: #555555; background: transparent;");

    lblDoctor = new QLabel(rightContainer);
    lblDoctor->setGeometry(40, 160, 430, 380);
    lblDoctor->setPixmap(QPixmap(appPath + "/doctor.png"));
    lblDoctor->setScaledContents(true);
    lblDoctor->setStyleSheet("background: transparent;");
}

LoginDialog::~LoginDialog() {}

void LoginDialog::handleLogin()
{
    QString username = txtAccount->text().trimmed();
    QString password = txtPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng nhập đầy đủ thông tin!");
        return;
    }

    if (!m_authService->login(username, password).isLoginSuccess) {
        QMessageBox::critical(this, "Lỗi đăng nhập", "Tài khoản hoặc mật khẩu không chính xác.");
    } else {
        emit loginSucceeded(UserSession::getInstance().getCurrentAccount());
    }
}

void LoginDialog::clearFields()
{
    if (txtAccount)  txtAccount->clear();
    if (txtPassword) txtPassword->clear();
    if (txtAccount)  txtAccount->setFocus();
}