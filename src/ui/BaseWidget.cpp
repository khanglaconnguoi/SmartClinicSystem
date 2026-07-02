#include "BaseDashboard.h"

BaseDashboardWidget::BaseDashboardWidget(QWidget *parent)
    : QWidget(parent), m_sidebarFrame(nullptr), m_mainContentWidget(nullptr) 
{
    m_globalLayout = new QHBoxLayout(this);
    m_globalLayout->setContentsMargins(0, 0, 0, 0);
    m_globalLayout->setSpacing(0);
    this->setStyleSheet("background-color: #F8F9FA;");
}       

void BaseDashboardWidget::initializeDashboard() {
    // 1. Sao chép y chang cấu trúc Sidebar từ code của bạn
    setupSidebarFrame();
    
    // 2. Sao chép y chang cấu trúc Main Content từ code của bạn
    setupMainContentFrame();

    // 🌟 SAU KHI DỰNG KHUÔN XONG -> GỌI LỚP CON ĐỂ VẼ HÌNH / ĐỔ DỮ LIỆU THỰC TẾ
    fillDashboardData();
}

void BaseDashboardWidget::setupSidebarFrame() {
    m_sidebarFrame = new QFrame(this);
    m_sidebarFrame->setObjectName("Sidebar");
    m_sidebarFrame->setFixedWidth(240); 
    
    m_sidebarFrame->setStyleSheet(
        "QFrame#Sidebar { background-color: #FFFFFF; border-right: 1px solid #EAEAEA; }"
        "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; border: none; border-radius: 8px; color: #5F6368; background: transparent; font-weight: 500; }"
        "QPushButton:hover { background-color: #F1F3F4; color: #00966C; }"
        "QPushButton#activeBtn { background-color: #E6F5F0; color: #00966C; font-weight: bold; }"
    );

    m_sidebarLayout = new QVBoxLayout(m_sidebarFrame);
    m_sidebarLayout->setContentsMargins(15, 30, 15, 30);
    m_sidebarLayout->setSpacing(10);

    m_logoLabel = new QLabel("Nova Care Clinic", m_sidebarFrame);
    m_logoLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #00966C; margin-bottom: 20px;");
    m_sidebarLayout->addWidget(m_logoLabel);

    m_btnDash = new QPushButton("📊 Dashboard", m_sidebarFrame);
    m_btnDash->setObjectName("activeBtn"); 
    m_btnPatients = new QPushButton("👥 Patients", m_sidebarFrame);
    m_btnAppoint = new QPushButton("📅 Appointments", m_sidebarFrame);
    m_btnSetting = new QPushButton("⚙️ Settings", m_sidebarFrame);
    m_sidebarLayout->addWidget(m_btnDash);
    m_sidebarLayout->addWidget(m_btnPatients);
    m_sidebarLayout->addWidget(m_btnAppoint);
    m_sidebarLayout->addWidget(m_btnSetting);

    m_sidebarLayout->addStretch();

    m_btnLogout = new QPushButton("🚪 Log Out", m_sidebarFrame);
    m_btnLogout->setStyleSheet("color: #D93025;"); 
    m_sidebarLayout->addWidget(m_btnLogout);

    connect(m_btnLogout, &QPushButton::clicked, this, &BaseDashboardWidget::logoutRequested);

    m_globalLayout->addWidget(m_sidebarFrame);
}

void BaseDashboardWidget::setupMainContentFrame() {
    m_mainContentWidget = new QWidget(this);
    m_mainContentLayout = new QVBoxLayout(m_mainContentWidget);
    m_mainContentLayout->setContentsMargins(30, 25, 30, 25);
    m_mainContentLayout->setSpacing(25);

    // Thanh Topbar tĩnh
    QHBoxLayout* topbarLayout = new QHBoxLayout();

    m_searchInput = new QLineEdit(m_mainContentWidget);
    m_searchInput->setPlaceholderText("🔍 Search for patients, doctors and etc...");
    m_searchInput->setFixedWidth(350);
    m_searchInput->setStyleSheet("padding: 10px 15px; border: 1px solid #DADCE0; border-radius: 8px; background-color: #FFFFFF; font-size: 13px; color: #333333;");
    
    // 🌟 CHỖ NÀY ĐÂY: Phải có chữ "QHBoxLayout*" để khởi tạo biến nhé bạn!
    QHBoxLayout* userInfoLayout = new QHBoxLayout();
    userInfoLayout->setSpacing(10); 

    // Khởi tạo Text hiển thị tên bác sĩ
    m_docNameLabel = new QLabel("Loading...", m_mainContentWidget);
    m_docNameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #3C4043; font-family: 'Arial';");

    // Khởi tạo Button tròn chứa Avatar cá nhân
    m_docAvatarBtn = new QPushButton(m_mainContentWidget);
    m_docAvatarBtn->setFixedSize(36, 36);
    m_docAvatarBtn->setCursor(Qt::PointingHandCursor);
    m_docAvatarBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; padding: 0px; margin: 0px; }");

    // Thả chữ và nút bấm vào cụm Layout con
    userInfoLayout->addWidget(m_docNameLabel);
    userInfoLayout->addWidget(m_docAvatarBtn);

    // Xếp các thành phần lên thanh Topbar lớn
    topbarLayout->addWidget(m_searchInput);
    topbarLayout->addStretch();
    topbarLayout->addLayout(userInfoLayout); // Hút cụm user sang tận cùng bên phải

    // Đẩy thanh Topbar vào Layout tổng bên phải
    m_mainContentLayout->addLayout(topbarLayout);

    m_globalLayout->addWidget(m_mainContentWidget, 1);
}