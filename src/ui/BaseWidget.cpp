#include "BaseDashboard.h"
#include "Profile.h"
#include <QDebug>

BaseDashboardWidget::BaseDashboardWidget(std::shared_ptr<IAuthenticatable> user, QWidget *parent)
    : QWidget(parent), m_sidebarFrame(nullptr), m_mainContentWidget(nullptr), m_currentUser(user)
{
    m_globalLayout = new QHBoxLayout(this);
    m_globalLayout->setContentsMargins(0, 0, 0, 0);
    m_globalLayout->setSpacing(0);
    this->setStyleSheet("background-color: #F8F9FA;");
}       

void BaseDashboardWidget::initializeDashboard() {
    setupSidebarFrame();
    setupMainContentFrame();
    fillDashboardData();
}

void BaseDashboardWidget::setupSidebarFrame() {
    // Nếu khung sidebar đã tồn tại từ trước (do lớp con hoặc luồng khác tạo), dọn dẹp nó để tránh nhân đôi
    if (m_sidebarFrame) {
        m_globalLayout->removeWidget(m_sidebarFrame);
        m_sidebarFrame->deleteLater();
        m_sidebarFrame = nullptr;
    }

    m_sidebarFrame = new QFrame(this);
    m_sidebarFrame->setObjectName("Sidebar");
    m_sidebarFrame->setFixedWidth(240); 
    
    m_sidebarFrame->setStyleSheet(
        "QFrame#Sidebar { background-color: #FFFFFF; border-right: 1px solid #EAEAEA; }"
        "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; border: none; border-radius: 6px; color: #5F6368; background: transparent; font-weight: 500; }"
        "QPushButton:hover { background-color: #F0F4FA; color: #4B94F2; }"
        "QPushButton#activeBtn { background-color: #E8F0FE; color: #4B94F2; font-weight: bold; }"
    );

    m_sidebarLayout = new QVBoxLayout(m_sidebarFrame);
    m_sidebarLayout->setContentsMargins(15, 30, 15, 30);
    m_sidebarLayout->setSpacing(10);

    m_logoLabel = new QLabel("Phòng Khám Nova Care", m_sidebarFrame);
    m_logoLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #4B94F2; margin-bottom: 20px;");
    m_sidebarLayout->addWidget(m_logoLabel);

    m_btnDash = new QPushButton("📊 Tổng quan", m_sidebarFrame);
    m_btnDash->setObjectName("activeBtn"); 
    m_btnPatients = new QPushButton("👥 Bệnh nhân", m_sidebarFrame);
    m_btnAppoint = new QPushButton("📅 Lịch hẹn", m_sidebarFrame);
    m_btnSetting = new QPushButton("⚙️ Cài đặt", m_sidebarFrame);
    
    m_sidebarLayout->addWidget(m_btnDash);
    m_sidebarLayout->addWidget(m_btnPatients);
    m_sidebarLayout->addWidget(m_btnAppoint);
    m_sidebarLayout->addWidget(m_btnSetting);

    m_sidebarLayout->addStretch();

    m_btnLogout = new QPushButton("🚪 Đăng xuất", m_sidebarFrame);
    m_btnLogout->setStyleSheet(
        "QPushButton { color: #D93025; font-weight: bold; border-radius: 6px; }"
        "QPushButton:hover { background-color: #FCE8E6; }"
    ); 
    m_sidebarLayout->addWidget(m_btnLogout);

    connect(m_btnDash, &QPushButton::clicked, this, &BaseDashboardWidget::handleMenuChanged);
    connect(m_btnPatients, &QPushButton::clicked, this, &BaseDashboardWidget::handleMenuChanged);
    connect(m_btnLogout, &QPushButton::clicked, this, &BaseDashboardWidget::logoutRequested);

    m_globalLayout->addWidget(m_sidebarFrame);
}

void BaseDashboardWidget::setupMainContentFrame() {
    if (m_mainContentWidget) return;

    m_mainContentWidget = new QWidget(this);
    m_mainContentLayout = new QVBoxLayout(m_mainContentWidget);
    m_mainContentLayout->setContentsMargins(30, 25, 30, 25);
    m_mainContentLayout->setSpacing(25);

    QHBoxLayout* topbarLayout = new QHBoxLayout();

    m_searchInput = new QLineEdit(m_mainContentWidget);
    m_searchInput->setPlaceholderText("🔍 Tìm kiếm bệnh nhân, bác sĩ, lịch hẹn...");
    m_searchInput->setFixedWidth(350);
    m_searchInput->setStyleSheet("padding: 10px 15px; border: 1px solid #DADCE0; border-radius: 8px; background-color: #FFFFFF; font-size: 13px; color: #333333;");
    
    QHBoxLayout* userInfoLayout = new QHBoxLayout();
    userInfoLayout->setSpacing(10); 

    m_docNameLabel = new ClickableLabel(m_mainContentWidget);
    m_docNameLabel->setText("Đang tải...");
    m_docNameLabel->setCursor(Qt::PointingHandCursor);
    m_docNameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #3C4043; font-family: 'Arial';");

    m_docAvatarBtn = new QPushButton(m_mainContentWidget);
    m_docAvatarBtn->setFixedSize(36, 36);
    m_docAvatarBtn->setCursor(Qt::PointingHandCursor);
    m_docAvatarBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #E8F0FE; "
        "   color: #4B94F2; "
        "   border: none; "
        "   border-radius: 18px; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   padding: 0px; "
        "   margin: 0px; "
        "}"
    );

    QObject::connect(m_docNameLabel, &ClickableLabel::clicked, this, &BaseDashboardWidget::handleAvatarClicked);
    QObject::connect(m_docAvatarBtn, &QPushButton::clicked, this, &BaseDashboardWidget::handleAvatarClicked);

    userInfoLayout->addWidget(m_docNameLabel);
    userInfoLayout->addWidget(m_docAvatarBtn);

    topbarLayout->addWidget(m_searchInput);
    topbarLayout->addStretch();
    topbarLayout->addLayout(userInfoLayout); 

    m_mainContentLayout->addLayout(topbarLayout);

    m_dynamicStackedWidget = new QStackedWidget(m_mainContentWidget);
    
    m_defaultDashboardView = new QWidget(this);
    m_mainContentLayout = new QVBoxLayout(m_defaultDashboardView); 
    m_mainContentLayout->setContentsMargins(0, 0, 0, 0);
    m_mainContentLayout->setSpacing(25);
    m_defaultDashboardView->setLayout(m_mainContentLayout);
    
    m_patientPage = new Patient(this);

    m_dynamicStackedWidget->addWidget(m_defaultDashboardView);
    m_dynamicStackedWidget->addWidget(m_patientPage);

    QVBoxLayout* finalMainLayout = qobject_cast<QVBoxLayout*>(m_mainContentWidget->layout());
    if (finalMainLayout) {
        finalMainLayout->addWidget(m_dynamicStackedWidget, 1);
    }

    m_globalLayout->addWidget(m_mainContentWidget, 1);
}

void BaseDashboardWidget::handleAvatarClicked() {
    if (!m_currentUser) {
        qWarning() << "Không có thông tin người dùng hiện tại để xem hồ sơ.";
        return;
    }

    auto staffService = std::make_shared<StaffService>(std::make_shared<StaffRepository>());
    auto* profileDialog = new ProfileWidget(staffService, this);
    profileDialog->setAttribute(Qt::WA_DeleteOnClose);
    profileDialog->loadProfile(m_currentUser->getAccountId());
    profileDialog->show();
    profileDialog->raise();
    profileDialog->activateWindow();
}

void BaseDashboardWidget::handleMenuChanged() {
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton) return;

    m_btnDash->setObjectName("");
    m_btnPatients->setObjectName("");
    m_btnDash->setStyle(m_btnDash->style());
    m_btnPatients->setStyle(m_btnPatients->style());

    clickedButton->setObjectName("activeBtn");
    clickedButton->setStyle(clickedButton->style());

    if (clickedButton == m_btnDash) {
        m_dynamicStackedWidget->setCurrentWidget(m_defaultDashboardView);
    } 
    else if (clickedButton == m_btnPatients) {
        m_dynamicStackedWidget->setCurrentWidget(m_patientPage);
    }
}