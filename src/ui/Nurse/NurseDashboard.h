#pragma once

#include "ui/BaseDashboard.h"
#include "model/IAuthenticatable.h"
#include "service/StaffService.h"
#include "service/ServiceRequestService.h"
#include "service/AppointmentService.h"
#include <QString>
#include <QStackedWidget>
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <memory>

class NurseDashboardWidget : public BaseDashboardWidget {
    Q_OBJECT

public:
    explicit NurseDashboardWidget(
        std::shared_ptr<IAuthenticatable> user = nullptr,
        std::shared_ptr<StaffService> staffService = nullptr,
        std::shared_ptr<ServiceRequestService> serviceRequestService = nullptr,
        std::shared_ptr<AppointmentService> appointmentService = nullptr,
        QWidget *parent = nullptr);

    virtual ~NurseDashboardWidget() override = default;

protected:
    virtual void fillDashboardData() override;

private:
    std::shared_ptr<ServiceRequestService> m_serviceRequestService;
    std::shared_ptr<AppointmentService> m_appointmentService;

    // Sidebar & Navigation
    QStackedWidget *m_stackedWidget = nullptr;
    QPushButton *m_btnOverview = nullptr;
    QPushButton *m_btnAppointments = nullptr;
    QPushButton *m_btnLabQueue = nullptr;
    QPushButton *m_btnLeaveManage = nullptr;

    // Pages
    QWidget *m_overviewPage = nullptr;
    QWidget *m_appointmentsPage = nullptr;
    QWidget *m_labQueuePage = nullptr;
    QWidget *m_leaveManagePage = nullptr;

    // Leave Management Elements
    QLabel *m_lblLeaveBalance = nullptr;
    QDateEdit *m_leaveStartDate = nullptr;
    QDateEdit *m_leaveEndDate = nullptr;
    QTextEdit *m_txtLeaveReason = nullptr;
    QTabWidget *m_leaveTabWidget = nullptr;
    QTableWidget *m_tableLeaveHistory = nullptr;

    // Appointments Elements
    QLabel *m_lblApptTitle = nullptr;
    QDateEdit *m_apptDateEdit = nullptr;
    QTableWidget *m_appointmentsTable = nullptr;

    // Overview Elements
    QLabel *m_lblTotalRequests = nullptr;
    QLabel *m_lblPendingRequests = nullptr;
    QLabel *m_lblProcessingRequests = nullptr;
    QLabel *m_lblCompletedRequests = nullptr;
    QTableWidget *m_overviewTable = nullptr;

    // Lab Queue Elements
    QLabel *m_lblRoomValue = nullptr;
    QComboBox *m_comboStatusFilter = nullptr;
    QDateEdit *m_queueDateEdit = nullptr;
    QTableWidget *m_queueTable = nullptr;

    QPushButton *m_btnCheckIn = nullptr;
    QPushButton *m_btnStartProcessing = nullptr;
    QPushButton *m_btnComplete = nullptr;
    QPushButton *m_btnCancel = nullptr;

    // Selected Request State
    int m_selectedRequestId = -1;
    int m_nurseRoomId = -1;
    RoomType m_nurseRoomType = RoomType::Unknown;
    int m_activeRequestId = -1;

    // Active Patient Info Labels
    QLabel *m_lblActivePatientName = nullptr;
    QLabel *m_lblActivePatientCode = nullptr;
    QLabel *m_lblActivePatientService = nullptr;
    QLabel *m_lblActivePatientStatus = nullptr;

    // UI Construction
    void buildSidebar();
    void buildOverviewPage();
    void buildAppointmentsPage();
    void buildLabQueuePage();
    void buildLeaveManagePage();

    QFrame *makeCard(QWidget *parent = nullptr);
    void updateOverviewData();
    void updateAppointmentsTable();
    void updateQueueTable();
    void loadLabRooms();
    void switchPage(int index, QPushButton* activeBtn);

    void onLeaveTabSelected();
    void loadLeaveHistory();
    void onSubmitLeaveRequest();

private slots:
    void onStatusFilterChanged(int index);
    void onDateChanged(const QDate &date);
    void onQueueTableRowSelected(int row, int col);
    void onCheckInClicked();
    void onStartProcessingClicked();
    void onCompleteClicked();
    void onCancelClicked();
};
