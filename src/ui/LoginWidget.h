/**
 * @file    LoginWidget.h
 * @brief   Giao diện màn hình đăng nhập hệ thống (Front-end)
 * @author  [Tên của bạn]
 */

#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QAction>

class LoginWidget : public QWidget {
    Q_OBJECT 

public:
    explicit LoginWidget(QWidget* parent = nullptr);
    ~LoginWidget() override;
    QString getStaffCode() const {return m_usernameInput->text().trimmed();}
    QString getPlainPassword() const{return m_passwordInput->text().trimmed();}
    void clearFields();

signals:
    void loginButtonClicked();
    
private:
    QLineEdit* m_usernameInput; 
    QLineEdit* m_passwordInput;
    QPushButton* m_loginButton;
    QLabel* m_forgotPasswordLabel;

    void setupUi(); 
};