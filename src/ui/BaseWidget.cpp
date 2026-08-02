#include <QDebug>

#include "BaseDashboard.h"
#include "view/Profile.h"
#include "service/StaffService.h"
#include "service/UserSession.h"
#include "view/Profile.h"


BaseDashboardWidget::BaseDashboardWidget(std::shared_ptr<IAuthenticatable> user, std::shared_ptr<StaffService> staffService, std::shared_ptr<PatientService> patientService, std::shared_ptr<AppointmentService> appointmentService, QWidget *parent)
    : QWidget(parent), m_sidebarFrame(nullptr), m_mainContentWidget(nullptr), m_currentUser(user ? user : UserSession::getInstance().getCurrentAccount()), m_baseStaffService(std::move(staffService)), m_basePatientService(std::move(patientService)), m_baseAppointmentService(std::move(appointmentService))
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
    if (m_sidebarFrame) {
        m_globalLayout->removeWidget(m_sidebarFrame);
        m_sidebarFrame->deleteLater();
        m_sidebarFrame = nullptr;
    }

    m_sidebarFrame = new QFrame(this);
    m_sidebarFrame->setObjectName("Sidebar");
    m_sidebarFrame->setFixedWidth(240); 
    
    m_sidebarFrame->setStyleSheet(
        "QFrame#Sidebar { background-color: #FFFFFF; border-right: 1px solid #E2E8F0; }"
        "QFrame#Sidebar QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; border: none; border-radius: 8px; color: #475569; background: transparent; font-weight: 600; }"
        "QFrame#Sidebar QPushButton:hover { background-color: #F1F5F9; color: #1E293B; }"
        "QFrame#Sidebar QPushButton#activeBtn, QFrame#Sidebar QPushButton#activeMenu, QFrame#Sidebar QPushButton:checked { background-color: #EFF6FF; color: #2563EB; font-weight: bold; border-left: 4px solid #2563EB; }"
    );

    m_sidebarLayout = new QVBoxLayout(m_sidebarFrame);
    m_sidebarLayout->setContentsMargins(15, 30, 15, 30);
    m_sidebarLayout->setSpacing(10);

    m_logoLabel = new QLabel("Nova Care Clinic", m_sidebarFrame);
    m_logoLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2563EB; margin-bottom: 20px;");
    m_sidebarLayout->addWidget(m_logoLabel);

    m_globalLayout->addWidget(m_sidebarFrame);
}

void BaseDashboardWidget::setupMainContentFrame() {
    if (m_mainContentWidget) return;

    m_mainContentWidget = new QWidget(this);
    m_mainContentLayout = new QVBoxLayout(m_mainContentWidget);
    m_mainContentLayout->setContentsMargins(30, 25, 30, 28);
    m_mainContentLayout->setSpacing(25);

    QHBoxLayout* topbarLayout = new QHBoxLayout();

    m_searchInput = new QLineEdit(m_mainContentWidget);
    m_searchInput->setPlaceholderText("Nhập từ khóa tìm kiếm (bệnh nhân, bác sĩ...)...");
    m_searchInput->setFixedWidth(350);
    m_searchInput->setStyleSheet("padding: 10px 15px; border: 1px solid #CBD5E1; border-radius: 8px; background-color: #FFFFFF; font-size: 13px; color: #0F172A;");
    
    QHBoxLayout* userInfoLayout = new QHBoxLayout();
    userInfoLayout->setSpacing(10); 

    m_docNameLabel = new ClickableLabel(m_mainContentWidget);
    m_docNameLabel->setText(m_currentUser ? m_currentUser->getFullName() : "Loading...");
    m_docNameLabel->setCursor(Qt::PointingHandCursor);
    m_docNameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #1E293B; font-family: 'Arial';");

    m_docAvatarBtn = new QPushButton(m_mainContentWidget);
    m_docAvatarBtn->setFixedSize(36, 36);
    m_docAvatarBtn->setCursor(Qt::PointingHandCursor);
    if (m_currentUser && !m_currentUser->getAvatar().isNull()) {
        m_docAvatarBtn->setIcon(QIcon(m_currentUser->getAvatar()));
        m_docAvatarBtn->setIconSize(QSize(36, 36));
    }
    m_docAvatarBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #EFF6FF; "
        "   color: #2563EB; "
        "   border: 1px solid #DBEAFE; "
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

    m_globalLayout->addWidget(m_mainContentWidget, 1);
}

void BaseDashboardWidget::handleAvatarClicked() {
    if (!m_currentUser) {
        qWarning() << "Không có thông tin người dùng hiện tại để xem hồ sơ.";
        return;
    }
    if (!m_baseStaffService) {
        qWarning() << "Thiếu StaffService để hiển thị hồ sơ.";
        return;
    }

    auto* profileDialog = new ProfileWidget(m_baseStaffService, this);
    profileDialog->setAttribute(Qt::WA_DeleteOnClose);
    profileDialog->loadProfile(m_currentUser->getAccountId());
    profileDialog->show();
    profileDialog->raise();
    profileDialog->activateWindow();
}