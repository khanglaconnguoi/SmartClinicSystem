#include <QDebug>

#include "BaseDashboard.h"
#include "view/Profile.h"
#include "service/StaffService.h"
#include "service/UserSession.h"
#include "view/Profile.h"


BaseDashboardWidget::BaseDashboardWidget(std::shared_ptr<IAuthenticatable> user, std::shared_ptr<StaffService> staffService, QWidget *parent)
    : QWidget(parent), m_sidebarFrame(nullptr), m_mainContentWidget(nullptr), m_currentUser(user ? user : UserSession::getInstance().getCurrentAccount()), m_staffService(std::move(staffService))
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

#include <QCoreApplication>

    m_logoLabel = new QLabel(m_sidebarFrame);
    QPixmap logoPix;
    bool logoLoaded = false;
#ifdef PROJECT_ROOT_DIR
    QString defaultPath = QString::fromUtf8(PROJECT_ROOT_DIR) + "/assets/images/logo.png";
    logoLoaded = logoPix.load(defaultPath);
#endif
    if (!logoLoaded) {
        QString fallbackPath = QCoreApplication::applicationDirPath() + "/assets/images/logo.png";
        logoLoaded = logoPix.load(fallbackPath);
    }
    if (!logoLoaded) {
        logoLoaded = logoPix.load("assets/images/logo.png");
    }
    if (!logoPix.isNull()) {
        QPixmap scaledLogo = logoPix.scaledToWidth(170, Qt::SmoothTransformation);
        m_logoLabel->setPixmap(scaledLogo);
        m_logoLabel->setAlignment(Qt::AlignCenter);
        m_logoLabel->setStyleSheet("margin-top: 5px; margin-bottom: 15px; background: transparent; border: none;");
    } else {
        m_logoLabel->setText("SMART CLINIC SYSTEM");
        m_logoLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2563EB; margin-bottom: 20px;");
    }
    m_sidebarLayout->addWidget(m_logoLabel);

    m_globalLayout->addWidget(m_sidebarFrame);
}

#include <QPainter>
#include <QPainterPath>

static QPixmap getCircularPixmap(const QPixmap &src, int size) {
    if (src.isNull()) return QPixmap();
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    qreal borderWidth = 1.5;
    
    QPainterPath clipPath;
    clipPath.addEllipse(borderWidth, borderWidth, size - 2 * borderWidth, size - 2 * borderWidth);
    
    painter.save();
    painter.setClipPath(clipPath);
    
    int innerSize = size - 2 * borderWidth;
    QPixmap scaled = src.scaled(innerSize, innerSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    int x = (scaled.width() - innerSize) / 2;
    int y = (scaled.height() - innerSize) / 2;
    painter.drawPixmap(borderWidth, borderWidth, scaled, x, y, innerSize, innerSize);
    painter.restore();
    
    QPen pen(QColor("#2563EB"), borderWidth);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(borderWidth / 2.0, borderWidth / 2.0, size - borderWidth, size - borderWidth);
    
    return result;
}

void BaseDashboardWidget::setupMainContentFrame() {
    if (m_mainContentWidget) return;

    m_mainContentWidget = new QWidget(this);
    m_mainContentLayout = new QVBoxLayout(m_mainContentWidget);
    m_mainContentLayout->setContentsMargins(30, 25, 30, 28);
    m_mainContentLayout->setSpacing(25);

    QHBoxLayout* topbarLayout = new QHBoxLayout();
    QHBoxLayout* userInfoLayout = new QHBoxLayout();
    userInfoLayout->setSpacing(10); 

    m_nameLabel = new ClickableLabel(m_mainContentWidget);
    m_nameLabel->setText(m_currentUser ? m_currentUser->getFullName().toUpper() : "LOADING...");
    m_nameLabel->setCursor(Qt::PointingHandCursor);
    m_nameLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #1E293B; font-family: 'Segoe UI', sans-serif; letter-spacing: 0.5px;");

    m_avatarBtn = new QPushButton(m_mainContentWidget);
    m_avatarBtn->setFixedSize(36, 36);
    m_avatarBtn->setCursor(Qt::PointingHandCursor);

    if (m_currentUser && !m_currentUser->getAvatar().isNull()) {
        QPixmap circularPix = getCircularPixmap(m_currentUser->getAvatar(), 36);
        m_avatarBtn->setIcon(QIcon(circularPix));
        m_avatarBtn->setIconSize(QSize(36, 36));
        m_avatarBtn->setText("");
        m_avatarBtn->setStyleSheet(
            "QPushButton { "
            "   background-color: transparent; "
            "   border: none; "
            "   padding: 0px; "
            "   margin: 0px; "
            "}"
            "QPushButton:hover { "
            "   background-color: rgba(0, 0, 0, 0.05); "
            "   border-radius: 18px; "
            "}"
        );
    } else {
        QString initial = m_currentUser ? m_currentUser->getFullName().left(1).toUpper() : "U";
        m_avatarBtn->setText(initial);
        m_avatarBtn->setStyleSheet(
            "QPushButton { "
            "   background-color: #EFF6FF; "
            "   color: #2563EB; "
            "   border: 1.5px solid #2563EB; "
            "   border-radius: 18px; "
            "   font-size: 15px; "
            "   font-weight: bold; "
            "   padding: 0px; "
            "   margin: 0px; "
            "}"
            "QPushButton:hover { "
            "   background-color: #DBEAFE; "
            "}"
        );
    }

    QObject::connect(m_nameLabel, &ClickableLabel::clicked, this, &BaseDashboardWidget::handleAvatarClicked);
    QObject::connect(m_avatarBtn, &QPushButton::clicked, this, &BaseDashboardWidget::handleAvatarClicked);

    userInfoLayout->addWidget(m_nameLabel);
    userInfoLayout->addWidget(m_avatarBtn);

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
    if (!m_staffService) {
        qWarning() << "Thiếu StaffService để hiển thị hồ sơ.";
        return;
    }

    auto* profileDialog = new ProfileWidget(m_staffService, this);
    profileDialog->setAttribute(Qt::WA_DeleteOnClose);
    profileDialog->loadProfile(m_currentUser->getAccountId());
    profileDialog->show();
    profileDialog->raise();
    profileDialog->activateWindow();
}