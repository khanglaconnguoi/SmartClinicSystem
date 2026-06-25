#include "MainWindow.h"
#include "Doctor/DoctorDashboard.h" 
#include "../model/IAuthenticatable.h"   
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {    
    setWindowTitle("Smart Clinic System");
    resize(1000, 600);
    m_authService = std::make_shared<AuthService>();

    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    m_loginWidget = new LoginWidget(this);
    m_stackedWidget->addWidget(m_loginWidget);

    connect(m_loginWidget, &LoginWidget::loginButtonClicked, this, [this]() {
        
        QString staffCode = m_loginWidget->getStaffCode();
        QString plainPassword = m_loginWidget->getPlainPassword();

        if (staffCode.isEmpty() || plainPassword.isEmpty()) {
            qDebug() << "khong duoc de trong";
            return;
        }

        std::optional<std::shared_ptr<IAuthenticatable>> authResult = m_authService->login(staffCode, plainPassword);

        if (authResult.has_value()) {
            qDebug() << "Thanh cong" << staffCode;
            
            std::shared_ptr<IAuthenticatable> loggedInUser = authResult.value();

            if (!loggedInUser) return;

            if (loggedInUser->getAccountType() == AccountType::Staff) {
                auto* doctorDash = new DoctorDashboardWidget(loggedInUser, this);
                registerDashboardPage(doctorDash);
                this->showMaximized();
            } 
            else if (loggedInUser->getAccountType() == AccountType::Patient) {
                // Sau này làm Dashboard bệnh nhân thì bạn thêm ở đây:
                // auto* patientDash = new PatientDashboardWidget(loggedInUser, this);
                // registerDashboardPage(patientDash);
                // this->showMaximized();
            }
            m_loginWidget->clearFields();
        } 
        else {
            qDebug() << "Sai mk";
            QMessageBox::critical(this, "Lỗi đăng nhập", "Tài khoản hoặc mật khẩu không chính xác!");
        }
    });
}

void MainWindow::registerDashboardPage(BaseDashboardWidget* page) {
    if (!page) return;

    int newIndex = m_stackedWidget->addWidget(page);
    
    connect(page, &BaseDashboardWidget::logoutRequested, this, &MainWindow::handleGlobalLogout);
    
    m_stackedWidget->setCurrentIndex(newIndex);
}

void MainWindow::handleGlobalLogout() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Xác nhận đăng xuất");
    
    msgBox.setText("<p style='color: #202124; font-family: \"Arial\"; font-size: 15px; font-weight: 500; margin-left: 10px;'>"
                   "Bạn có chắc chắn muốn đăng xuất?</p>");
    msgBox.setIcon(QMessageBox::Question);

    QPushButton *yesButton = msgBox.addButton("Đồng Ý", QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton("Hủy", QMessageBox::NoRole);

    yesButton->setCursor(Qt::PointingHandCursor);
    noButton->setCursor(Qt::PointingHandCursor);

    msgBox.setStyleSheet(
        "QMessageBox { "
        "   background-color: #FFFFFF; "
        "   border: 1px solid #EAEAEA; "
        "   border-radius: 12px; "
        "}"
        // Cấu hình nút "Đồng Ý" (Màu xanh ngọc chủ đạo #00966C giống logo của bạn)
        "QPushButton { "
        "   background-color: #00966C; "
        "   color: #FFFFFF; "
        "   font-family: 'Arial'; "
        "   font-size: 13px; "
        "   font-weight: bold; "
        "   border: none; "
        "   border-radius: 6px; "
        "   padding: 8px 20px; "
        "   min-width: 75px; "
        "}"
        "QPushButton:hover { background-color: #007D5A; }"
        "QPushButton:pressed { background-color: #005F44; }"
        
        // Cấu hình riêng cho nút "Hủy" (Màu xám nhạt nền nã để làm nền cho nút chính)
        "QPushButton[text=\"Hủy\"] { "
        "   background-color: #F1F3F4; "
        "   color: #5F6368; "
        "   border: 1px solid #DADCE0; "
        "}"
        "QPushButton[text=\"Hủy\"]:hover { background-color: #E8EAED; color: #3C4043; }"
        "QPushButton[text=\"Hủy\"]:pressed { background-color: #D2D6DE; }"
    );

    // Thực thi Dialog
    msgBox.exec();

    // Xử lý logic chuyển trang khi bấm "Đồng Ý"
    if (msgBox.clickedButton() == yesButton) {
        m_stackedWidget->setCurrentIndex(0);
        
        this->showNormal();
        this->resize(1000, 600); 
        
        QWidget* currentDash = m_stackedWidget->widget(1);
        if (currentDash && currentDash != m_loginWidget) {
            m_stackedWidget->removeWidget(currentDash);
            currentDash->deleteLater(); 
        }
    }
}

MainWindow::~MainWindow() {}