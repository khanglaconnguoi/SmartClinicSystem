#include "LoginDialog.h"
#include <QApplication>
#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include "service/AuthService.h"
#include "model/IAuthenticatable.h"
#include "model/SystemUser.h"
#include "service/UserSession.h"
#include "service/Validation.h"

LoginDialog::LoginDialog(std::shared_ptr<AuthService> authService,
                         std::shared_ptr<StaffService> staffService,
                         QWidget *parent)
    : QWidget(parent), m_authService(authService), m_staffService(staffService)
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
    leftForm->setGeometry(0, 50, 440, 460);
    leftForm->setStyleSheet("background: transparent;");

    lblLeftLogo = new QLabel(leftForm);
    lblLeftLogo->setGeometry(110, 10, 220, 110);
    QPixmap logoPix;
    bool logoLoaded = false;
#ifdef PROJECT_ROOT_DIR
    QString defaultPath = QString::fromUtf8(PROJECT_ROOT_DIR) + "/assets/images/logo.png";
    logoLoaded = logoPix.load(defaultPath);
#endif
    if (!logoLoaded) {
        QString fallbackPath = QApplication::applicationDirPath() + "/assets/images/logo.png";
        logoLoaded = logoPix.load(fallbackPath);
    }
    if (!logoLoaded) {
        logoLoaded = logoPix.load("assets/images/logo.png");
    }
    if (!logoPix.isNull()) {
        lblLeftLogo->setPixmap(logoPix);
        lblLeftLogo->setScaledContents(true);
    } else {
        lblLeftLogo->setText("SMART CLINIC SYSTEM");
        lblLeftLogo->setAlignment(Qt::AlignCenter);
        lblLeftLogo->setStyleSheet("font-size: 20px; font-weight: bold; color: #2563EB;");
    }

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
        "   background-color: #FFFFFF; "
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

    LoginResult result = m_authService->login(username, password);
    if (!result.isLoginSuccess) {
        QMessageBox::critical(this, "Lỗi đăng nhập", "Tài khoản hoặc mật khẩu không chính xác.");
        return;
    }

    auto currentUser = UserSession::getInstance().getCurrentAccount();
    if (result.mustChangePassword) {
        showPasswordChangeDialog(currentUser);
        return;
    }

    emit loginSucceeded(currentUser);
}

void LoginDialog::showPasswordChangeDialog(std::shared_ptr<IAuthenticatable> user)
{
    if (!m_staffService || !user) {
        emit loginSucceeded(user);
        return;
    }

    auto staffUser = std::dynamic_pointer_cast<SystemUser>(user);
    if (!staffUser) {
        emit loginSucceeded(user);
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Đổi mật khẩu lần đầu");
    dialog.setModal(true);
    dialog.resize(460, 320);
    dialog.setStyleSheet(
        "QDialog { background-color: #FFFFFF; }"
        "QLabel { color: #111827; }"
        "QLineEdit { padding: 10px 12px; border: 1px solid #D1D5DB; border-radius: 8px; background-color: #FFFFFF; color: #111827; }"
        "QLineEdit:focus { border: 1px solid #4B94F2; }"
        "QPushButton { padding: 8px 16px; border-radius: 8px; font-weight: 600; }"
    );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(14);
    layout->setContentsMargins(24, 24, 24, 24);

    QLabel *titleLabel = new QLabel("Bạn cần đổi mật khẩu trước AAA khi tiếp tục", &dialog);
    titleLabel->setStyleSheet("font-size: 17px; font-weight: bold; color: #111827;");
    layout->addWidget(titleLabel);

    QLabel *hintLabel = new QLabel("Vui lòng nhập mật khẩu mới có ít nhất 8 ký tự, gồm chữ hoa, chữ thường, số và ký tự đặc biệt.", &dialog);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet("font-size: 12px; color: #6B7280; line-height: 1.4;");
    layout->addWidget(hintLabel);

    QLineEdit *newPasswordEdit = new QLineEdit(&dialog);
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setPlaceholderText("Mật khẩu mới");
    layout->addWidget(newPasswordEdit);

    QLineEdit *confirmPasswordEdit = new QLineEdit(&dialog);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("Nhập lại mật khẩu mới");
    layout->addWidget(confirmPasswordEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton *cancelButton = new QPushButton("Hủy", &dialog);
    cancelButton->setStyleSheet("QPushButton { background-color: #F3F4F6; color: #374151; border: 1px solid #E5E7EB; }"
                                "QPushButton:hover { background-color: #E5E7EB; }");
    QPushButton *confirmButton = new QPushButton("Xác nhận", &dialog);
    confirmButton->setStyleSheet("QPushButton { background-color: #4B94F2; color: white; border: none; }"
                                 "QPushButton:hover { background-color: #357AE8; }");
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);
    layout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(confirmButton, &QPushButton::clicked, &dialog, [&]() {
        QString newPassword = newPasswordEdit->text();
        QString confirmPassword = confirmPasswordEdit->text();

        if (newPassword.isEmpty() || confirmPassword.isEmpty()) {
            QMessageBox::warning(&dialog, "Thông báo", "Vui lòng nhập đầy đủ mật khẩu mới.");
            return;
        }

        if (newPassword != confirmPassword) {
            QMessageBox::warning(&dialog, "Thông báo", "Mật khẩu xác nhận không khớp.");
            return;
        }

        QString validationError = Validation::validatePlainPassword(newPassword);
        if (!validationError.isEmpty()) {
            QMessageBox::warning(&dialog, "Thông báo", validationError);
            return;
        }

        if (!m_staffService->changePassword(staffUser->getAccountId(), newPassword)) {
            QMessageBox::critical(&dialog, "Lỗi", "Không thể đổi mật khẩu. Vui lòng thử lại.");
            return;
        }

        dialog.accept();
    });
    connect(confirmPasswordEdit, &QLineEdit::returnPressed, confirmButton, &QPushButton::click);

    if (dialog.exec() == QDialog::Accepted) {
        emit loginSucceeded(user);
    }
}

void LoginDialog::clearFields()
{
    if (txtAccount)  txtAccount->clear();
    if (txtPassword) txtPassword->clear();
    if (txtAccount)  txtAccount->setFocus();
}