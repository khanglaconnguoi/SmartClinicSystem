#pragma once

#include "../BaseDashboard.h" // Đường dẫn trỏ tới lớp cha Base của bạn
#include <memory>

// Forward declaration các lớp cần thiết để tối ưu hóa thời gian biên dịch
class IAuthenticatable;
class QVBoxLayout;
class QHBoxLayout;

class QStackedWidget;
class QCalendarWidget;

class DoctorDashboardWidget : public BaseDashboardWidget {
    Q_OBJECT

public:
    explicit DoctorDashboardWidget(std::shared_ptr<IAuthenticatable> user, QWidget *parent = nullptr);
    virtual ~DoctorDashboardWidget() override = default;

protected:
    virtual void fillDashboardData() override;

private:
    std::shared_ptr<IAuthenticatable> m_currentUser;
    int m_currentExaminingRow = -1;

    // Sidebar buttons và stacked widget để quản lý các trang con
    QStackedWidget* m_stackedWidget   = nullptr;
    QPushButton*    m_btnDash         = nullptr;
    QPushButton*    m_btnPatients     = nullptr;
    QPushButton*    m_btnAppoint      = nullptr;
    QPushButton*    m_btnSetting      = nullptr;

    // Các trang con
    QWidget*            m_overviewPage     = nullptr;
    QWidget*            m_patientsPage     = nullptr;
    QWidget*            m_appointmentsPage = nullptr;
    QWidget*            m_settingsPage     = nullptr;
    class ClinicalExamWidget* m_clinicalExamPage = nullptr;

    // Hàm dựng layout từng trang con
    void buildSidebar();
    void buildOverviewPage();
    void buildPatientsPage();
    void buildAppointmentsPage();
    void buildSettingsPage();
    void buildClinicalExamPage();

    // Hàm helper vẽ các thành phần trong trang Overview
    void createDoctorCards(QWidget* parentPage, QVBoxLayout* pageLayout);
    void createDoctorCharts(QWidget* parentPage, QVBoxLayout* pageLayout);
    void createDoctorTable(QWidget* parentPage, QVBoxLayout* pageLayout);

    void switchPage(int index, QPushButton* activeBtn);
    void openClinicalExam(const QString& name, const QString& id, const QString& time, const QString& specialty, int rowIndex, bool isFromTodayList = true);
    void handlePatientExamFinished();
    void refreshAppointmentsTables();

public:
    struct ApptMeta {
        int appointmentId;
        int patientId;
        QString name;
        QString code;
        QString time;
        QString reason;
    };
private:
    QList<ApptMeta> m_rowApptMeta;
    QList<ApptMeta> m_apptPageMeta;
    QTableWidget*   m_appointmentsTable = nullptr;
    bool            m_isExaminingFromTodayList = true;
};