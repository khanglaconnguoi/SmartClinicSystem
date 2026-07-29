#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>

// Forward declarations
class AuthService;
class IAuthenticatable;

class LoginDialog : public QWidget
{
    Q_OBJECT

public:
    explicit LoginDialog(std::shared_ptr<AuthService> authService, QWidget *parent = nullptr);
    ~LoginDialog();

    void clearFields();

signals:
    void loginSucceeded(std::shared_ptr<IAuthenticatable> user);

private slots:
    void handleLogin();

private:
    std::shared_ptr<AuthService> m_authService;

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
