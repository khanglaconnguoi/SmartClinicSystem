#include "MainWindow.h"
#include <QLabel>
#include <QDebug>
#include <optional>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {    
    setWindowTitle("Smart Clinic System");
    resize(1000, 600);
    m_authService = std::make_shared<AuthService>();

    // 2. Khởi tạo bộ quản lý lật trang QStackedWidget
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // 3. Khởi tạo giao diện Login Front-end và thêm vào trang index 0
    m_loginWidget = new LoginWidget(this);
    m_stackedWidget->addWidget(m_loginWidget);

    auto* dashboard = new Dashboard;
    m_stackedWidget->addWidget(dashboard);

    connect(m_loginWidget, &LoginWidget::loginButtonClicked, this, [this]() {
        
        // Bước A: Đứng ở MainWindow thò tay sang màn Login bốc dữ liệu chữ QString ra ngoài
        QString staffCode = m_loginWidget->getStaffCode();
        QString plainPassword = m_loginWidget->getPlainPassword();

        // Kiểm tra chặn nếu gõ rỗng
        if (staffCode.isEmpty() || plainPassword.isEmpty()) {
            qDebug() << "khong duoc de trong";
            return;
        }

        // Bước B: Chuyển tiếp tham số QString vừa bốc được vào đúng hàm login của class AuthService
        std::optional<std::shared_ptr<IAuthenticatable>> authResult = m_authService->login(staffCode, plainPassword);

        // Bước C: Kiểm tra kết quả bóc tách từ std::optional
        if (authResult.has_value()) {
            qDebug() << "Thanh cong" << staffCode;
            
            // Lật trang vèo sang màn hình Dashboard chính (Index 1)!
            m_stackedWidget->setCurrentIndex(1); 
        } 
        else {
            qDebug() << "Sai mk";
        }
    });
}

MainWindow::~MainWindow() {}

