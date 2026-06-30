#pragma once

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QDate>

class BaseDashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit BaseDashboardWidget(QWidget *parent = nullptr);
    virtual ~BaseDashboardWidget() override = default;

    // Hàm Template Method khởi tạo toàn bộ giao diện chuẩn
    void initializeDashboard();

signals:
    void logoutRequested();

protected:
    // Hook: lớp con override để vẽ nội dung + sidebar buttons riêng
    virtual void fillDashboardData() = 0;

    // ----- Sidebar (khung + layout + logo + nút logout) -----
    // Các nút menu cụ thể do lớp con tự tạo trong fillDashboardData()
    QFrame*       m_sidebarFrame   = nullptr;
    QVBoxLayout*  m_sidebarLayout  = nullptr;
    QLabel*       m_logoLabel      = nullptr;
    QPushButton*  m_btnLogout      = nullptr;

    // ----- Main Content -----
    QWidget*      m_mainContentWidget  = nullptr;
    QVBoxLayout*  m_mainContentLayout  = nullptr;
    QLineEdit*    m_searchInput        = nullptr;
    QLabel*       m_docNameLabel       = nullptr;
    QPushButton*  m_docAvatarBtn       = nullptr;
    QTableWidget* m_patientTable       = nullptr;

private:
    QHBoxLayout* m_globalLayout = nullptr;
    void setupSidebarFrame();
    void setupMainContentFrame();
};