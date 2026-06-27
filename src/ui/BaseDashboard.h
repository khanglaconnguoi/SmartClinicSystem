#pragma once

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>

class BaseDashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit BaseDashboardWidget(QWidget *parent = nullptr);
    virtual ~BaseDashboardWidget() override = default;

    // Hàm Template Method khởi tạo toàn bộ giao diện chuẩn theo theme bạn gửi
    void initializeDashboard();

signals:
    void logoutRequested();

protected:
    // Hàm thuần ảo để lớp con (Bác sĩ) nhảy vào cấu hình dữ liệu/vẽ hình riêng
    virtual void fillDashboardData() = 0;

    // Các linh kiện dùng chung ở Sidebar
    QFrame* m_sidebarFrame;
    QVBoxLayout* m_sidebarLayout;
    QLabel* m_logoLabel;
    QPushButton* m_btnDash;
    QPushButton* m_btnPatients;
    QPushButton* m_btnAppoint;
    QPushButton* m_btnSetting;
    QPushButton* m_btnLogout;

    // Các linh kiện dùng chung ở Vùng nội dung chính (Main Content)
    QWidget* m_mainContentWidget;
    QVBoxLayout* m_mainContentLayout;
    QLineEdit* m_searchInput;
    QLabel* m_docNameLabel;
    QPushButton* m_docAvatarBtn;
    // Bảng và các danh sách dùng chung
    QTableWidget* m_patientTable;   

private:
    QHBoxLayout* m_globalLayout;
    void setupSidebarFrame();
    void setupMainContentFrame();
};