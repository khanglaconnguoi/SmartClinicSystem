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
    setupSidebarFrame();
    setupMainContentFrame();

    // Khởi tạo và đổ dữ liệu vào các thành phần UI
    fillDashboardData();
}

void BaseDashboardWidget::setupSidebarFrame() {
    m_sidebarFrame = new QFrame(this);
    m_sidebarFrame->setObjectName("Sidebar");
    m_sidebarFrame->setFixedWidth(240); 
    
    m_sidebarFrame->setStyleSheet(
        "QFrame#Sidebar { background-color: #FFFFFF; border-right: 1px solid #EAEAEA; }"
        "QFrame#Sidebar QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; border: none; border-radius: 0px; color: #5F6368; background: transparent; font-weight: 500; }"
        "QFrame#Sidebar QPushButton:hover { background-color: #F1F3F4; color: #202124; }"
        "QFrame#Sidebar QPushButton#activeBtn, QFrame#Sidebar QPushButton#activeMenu, QFrame#Sidebar QPushButton:checked { background-color: #E8F0FE; color: #4B94F2; font-weight: bold; border-left: 4px solid #4B94F2; }"
    );

    m_sidebarLayout = new QVBoxLayout(m_sidebarFrame);
    m_sidebarLayout->setContentsMargins(15, 30, 15, 30);
    m_sidebarLayout->setSpacing(10);

    m_logoLabel = new QLabel("Nova Care Clinic", m_sidebarFrame);
    m_logoLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #4B94F2; margin-bottom: 20px;");
    m_sidebarLayout->addWidget(m_logoLabel);

    m_globalLayout->addWidget(m_sidebarFrame);
}

void BaseDashboardWidget::setupMainContentFrame() {
    m_mainContentWidget = new QWidget(this);
    m_mainContentLayout = new QVBoxLayout(m_mainContentWidget);
    m_mainContentLayout->setContentsMargins(30, 25, 30, 25);
    m_mainContentLayout->setSpacing(25);

    QHBoxLayout* topbarLayout = new QHBoxLayout();

    m_searchInput = new QLineEdit(m_mainContentWidget);
    m_searchInput->setPlaceholderText("🔍 Search for patients, doctors and etc...");
    m_searchInput->setFixedWidth(350);
    m_searchInput->setStyleSheet("padding: 10px 15px; border: 1px solid #DADCE0; border-radius: 8px; background-color: #FFFFFF; font-size: 13px; color: #333333;");
    
    QHBoxLayout* userInfoLayout = new QHBoxLayout();
    userInfoLayout->setSpacing(10); 

    m_docNameLabel = new QLabel("Loading...", m_mainContentWidget);
    m_docNameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #3C4043; font-family: 'Arial';");

    m_docAvatarBtn = new QPushButton(m_mainContentWidget);
    m_docAvatarBtn->setFixedSize(36, 36);
    m_docAvatarBtn->setCursor(Qt::PointingHandCursor);
    m_docAvatarBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; padding: 0px; margin: 0px; }");

    userInfoLayout->addWidget(m_docNameLabel);
    userInfoLayout->addWidget(m_docAvatarBtn);

    topbarLayout->addWidget(m_searchInput);
    topbarLayout->addStretch();
    topbarLayout->addLayout(userInfoLayout); 

    m_mainContentLayout->addLayout(topbarLayout);

    m_globalLayout->addWidget(m_mainContentWidget, 1);
}
