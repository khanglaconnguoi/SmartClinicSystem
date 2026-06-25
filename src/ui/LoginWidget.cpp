/**
 * @file    LoginWidget.cpp
 * @brief   Vẽ chi tiết bố cục giao diện Login chia đôi
 * @author  [Tên của bạn]
 */

#include "LoginWidget.h" // Nhớ include đúng tên file viết hoa PascalCase
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    this->setWindowTitle("Hospital Manager - Login");
    this->resize(1000, 600); 
    setupUi();
}

LoginWidget::~LoginWidget() {
    // Qt tự quản lý bộ nhớ nên để trống
}

void LoginWidget::setupUi() {
    // 1. Layout tổng nằm ngang chia đôi màn hình
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // =================================================================
    // [PHẦN BÊN TRÁI] - BANNER NỀN XANH NGỌC
    // =================================================================
    auto* leftBanner = new QLabel(this);
    leftBanner->setObjectName("leftBanner");
    leftBanner->setStyleSheet(
        "QLabel#leftBanner {"
        "   background-color: #e2f4f2;" 
        "   background-image: url('src/resources/doctor_banner.png');"
        "   background-repeat: no-repeat;"
        "   background-position: center;"
        "}"
    );
    
    // =================================================================
    // [PHẦN BÊN PHẢI] - FORM ĐĂNG NHẬP (NỀN TRẮNG SÁNG)
    // =================================================================
    auto* rightContainer = new QWidget(this);
    rightContainer->setStyleSheet(
        "QWidget {"
        "   background-color: #ffffff;" 
        "   color: #333333;"
        "}"
        "QLabel {"
        "   color: #333333;"
        "   background: transparent;"
        "}"
    );

    auto* formLayout = new QVBoxLayout(rightContainer);
    formLayout->setContentsMargins(60, 40, 60, 40);
    formLayout->setSpacing(15);

    // Tiêu đề LOGO Hospital
    auto* logoLabel = new QLabel("<font color='#2ec4b6'><b>NovaCare</b></font> <font color='#333333'>Clinic</font>", this);
    logoLabel->setStyleSheet("font-size: 24px; font-family: 'Arial'; font-weight: bold; background: transparent;");
    logoLabel->setAlignment(Qt::AlignCenter);

    // Ô nhập Username
    auto* userTitle = new QLabel("Username or E-mail", this);
    m_usernameInput = new QLineEdit(this);
    m_usernameInput->setPlaceholderText("bnpthinh2518@clc.fitus.edu.vn");
    m_usernameInput->setStyleSheet(
        "padding: 8px; border: 1px solid #cccccc; border-radius: 4px; background-color: #ffffff; color: #333333;"
    );

    // Ô nhập Password
    auto* passTitle = new QLabel("Password", this);
    m_passwordInput = new QLineEdit(this);
    m_passwordInput->setEchoMode(QLineEdit::Password); 
    m_passwordInput->setStyleSheet(
        "padding: 8px; border: 1px solid #cccccc; border-radius: 4px; background-color: #ffffff; color: #333333;"
    ); 
    
    QPushButton* togglePasswordBtn = new QPushButton("Xem", m_passwordInput);
    
    togglePasswordBtn->setCursor(Qt::PointingHandCursor); 
    togglePasswordBtn->setStyleSheet(
        "QPushButton {"
        "   border: none;"             
        "   background: transparent;"  
        "   color: #2ec4b6;"          
        "   font-weight: bold;"
        "   padding-right: 10px;"    
        "}"
        "QPushButton:hover { color: #24a195; }" 
    );

    // 3. Đặt nút bấm này nằm nép về phía bên phải bên trong ô Password
    QHBoxLayout* passwordLayout = new QHBoxLayout(m_passwordInput);
    passwordLayout->setContentsMargins(0, 0, 0, 0); // Xóa viền layout con
    passwordLayout->addStretch();                   // Đẩy nút bấm về tận cùng bên phải
    passwordLayout->addWidget(togglePasswordBtn);   // Nhét nút vào
    m_passwordInput->setLayout(passwordLayout);
    m_passwordInput->setTextMargins(0, 0, 50, 0); 

    //Nối signal để bấm vào hiện chữ
    connect(togglePasswordBtn, &QPushButton::clicked, this, [this, togglePasswordBtn]() {
        if (m_passwordInput->echoMode() == QLineEdit::Password) {
            //Hiện mật khẩu
            m_passwordInput->setEchoMode(QLineEdit::Normal);   
            togglePasswordBtn->setText("Ẩn");                  
        } else {
            //Tắt hiện chữ
            m_passwordInput->setEchoMode(QLineEdit::Password);
            togglePasswordBtn->setText("Xem");                
        }
    });

    // Nút đăng nhập Log in
    m_loginButton = new QPushButton("Log in", this);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #5b86e5; color: white; border: none; border-radius: 15px; padding: 10px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #4a75d4; }"
    );

    // Hiệu ứng đổ bóng cho nút login
    auto* shadowEffect = new QGraphicsDropShadowEffect(m_loginButton);
    shadowEffect->setBlurRadius(10);
    shadowEffect->setColor(QColor(91, 134, 229, 100));
    shadowEffect->setOffset(0, 4);
    m_loginButton->setGraphicsEffect(shadowEffect);
    
    // 🌟 KẾT NỐI SỰ KIỆN: Bấm Enter hoặc Click nút đều chuyển tiếp ra Signal tổng của bạn
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWidget::loginButtonClicked);
    connect(m_passwordInput, &QLineEdit::returnPressed, this, &LoginWidget::loginButtonClicked);

    //Nút ForgotPassword
    m_forgotPasswordLabel = new QLabel("<a href='#' style='color: #2ec4b6; text-decoration: none; background: transparent;'>Forget Password?</a>", this);
    m_forgotPasswordLabel->setAlignment(Qt::AlignCenter);

    //Vẽ các layout
    formLayout->addWidget(logoLabel);
    formLayout->addSpacing(30);
    formLayout->addWidget(userTitle);
    formLayout->addWidget(m_usernameInput);
    formLayout->addWidget(passTitle);
    formLayout->addWidget(m_passwordInput);
    formLayout->addSpacing(10);
    formLayout->addWidget(m_loginButton);
    formLayout->addSpacing(10);
    formLayout->addWidget(m_forgotPasswordLabel);
    formLayout->addStretch();

    // Ghép hai nửa Trái - Phải vào layout tổng
    mainLayout->addWidget(leftBanner, 5);
    mainLayout->addWidget(rightContainer, 5);
}

// 🌟 THÊM HÀM TIỆN ÍCH DỌN DẸP CHỮ KHI LOGOUT QUAY TRỞ LẠI
void LoginWidget::clearFields() {
    if (m_usernameInput) m_usernameInput->clear();
    if (m_passwordInput) m_passwordInput->clear();
    if (m_usernameInput) m_usernameInput->setFocus();
}