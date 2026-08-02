#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>

#include "service/AuthService.h"
#include "service/StaffService.h"
#include "model/IAuthenticatable.h"

class LoginDialog : public QWidget
{
    Q_OBJECT

public:
    explicit LoginDialog(std::shared_ptr<AuthService> authService,
                         std::shared_ptr<StaffService> staffService,
                         QWidget *parent = nullptr);
    ~LoginDialog();

    void clearFields();

signals:
    void loginSucceeded(std::shared_ptr<IAuthenticatable> user);

private slots:
    void handleLogin();

private:
    void showPasswordChangeDialog(std::shared_ptr<IAuthenticatable> user);

    std::shared_ptr<AuthService> m_authService;
    std::shared_ptr<StaffService> m_staffService;

    // UI members
    QLabel      *lblLeftLogo;
    QLabel      *lblSlogan;
    QLineEdit   *txtAccount;
    QLineEdit   *txtPassword;
    QPushButton *btnLogin;
    QPushButton *btnForgot;
    QWidget     *rightContainer;
    QLabel      *lblHello;
    QLabel      *lblSubDetails;
    QLabel      *lblDoctor;
};

#endif // LOGINDIALOG_H
