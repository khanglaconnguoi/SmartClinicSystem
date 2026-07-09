#pragma once

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QStackedWidget>
#include <QMouseEvent>
#include <memory>
#include "ui/Patient.h"
#include "repository/StaffRepository.h"
#include "service/StaffService.h"

class IAuthenticatable;

class ClickableLabel : public QLabel {
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = nullptr) : QLabel(parent) {}

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override {
        emit clicked();
        QLabel::mousePressEvent(event);
    }
};

class BaseDashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit BaseDashboardWidget(std::shared_ptr<IAuthenticatable> user = nullptr, QWidget *parent = nullptr);
    virtual ~BaseDashboardWidget() override = default;

    // Hàm Template Method khởi tạo toàn bộ giao diện chuẩn theo theme bạn gửi
    void initializeDashboard();

signals:
    void logoutRequested();
private slots:
    void handleMenuChanged();
    void handleAvatarClicked();
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
    ClickableLabel* m_docNameLabel;
    QPushButton* m_docAvatarBtn;
    std::shared_ptr<IAuthenticatable> m_currentUser;
    // Bảng và các danh sách dùng chung
    QTableWidget* m_patientTable;   
    QStackedWidget* m_dynamicStackedWidget;
    QWidget* m_defaultDashboardView;
    Patient* m_patientPage;

private:
    QHBoxLayout* m_globalLayout;
    void setupSidebarFrame();
    void setupMainContentFrame();
};