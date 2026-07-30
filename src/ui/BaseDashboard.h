#pragma once

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QMouseEvent>
#include <memory>
#include "model/IAuthenticatable.h"

#include "service/StaffService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"

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
    explicit BaseDashboardWidget(std::shared_ptr<IAuthenticatable> user = nullptr, std::shared_ptr<StaffService> staffService = nullptr, std::shared_ptr<PatientService> patientService = nullptr, std::shared_ptr<AppointmentService> appointmentService = nullptr, QWidget *parent = nullptr);
    virtual ~BaseDashboardWidget() override = default;

    void initializeDashboard();

signals:
    void logoutRequested();

private slots:
    void handleAvatarClicked();

protected:
    virtual void fillDashboardData() = 0;

    QFrame*       m_sidebarFrame   = nullptr;
    QVBoxLayout*  m_sidebarLayout  = nullptr;
    QLabel*       m_logoLabel      = nullptr;
    QPushButton*  m_btnLogout      = nullptr;

    QWidget*      m_mainContentWidget  = nullptr;
    QVBoxLayout*  m_mainContentLayout  = nullptr;
    QLineEdit*    m_searchInput        = nullptr;
    ClickableLabel* m_docNameLabel     = nullptr;
    QPushButton*  m_docAvatarBtn       = nullptr;
    std::shared_ptr<IAuthenticatable> m_currentUser;
    std::shared_ptr<StaffService> m_baseStaffService;
    std::shared_ptr<PatientService> m_basePatientService;
    std::shared_ptr<AppointmentService> m_baseAppointmentService;

private:
    QHBoxLayout* m_globalLayout = nullptr;
    void setupSidebarFrame();
    void setupMainContentFrame();
};