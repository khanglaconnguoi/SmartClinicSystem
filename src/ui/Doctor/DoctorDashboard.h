#pragma once
#include <QCalendarWidget>
#include <QDateEdit>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <memory>

#include "model/IAuthenticatable.h"
#include "service/AppointmentService.h"
#include "service/MedicalRecordService.h"
#include "service/PatientService.h"
#include "service/PharmacyService.h"
#include "service/StaffService.h"
#include "service/ServiceRequestService.h"
#include "ui/BaseDashboard.h"
#include "ui/Doctor/ClinicalExamWidget.h"
#include "ui/Doctor/PatientWidget.h"


class DoctorDashboardWidget : public BaseDashboardWidget {
  Q_OBJECT

public:
    explicit DoctorDashboardWidget(
        std::shared_ptr<IAuthenticatable> user = nullptr, 
        std::shared_ptr<StaffService> staffService = nullptr, 
        std::shared_ptr<PatientService> patientService = nullptr, 
        std::shared_ptr<AppointmentService> appointmentService = nullptr, 
        std::shared_ptr<MedicalRecordService> medicalRecordService = nullptr, 
        std::shared_ptr<PharmacyService> pharmacyService = nullptr, 
        std::shared_ptr<ServiceRequestService> serviceRequestService = nullptr,
        QWidget *parent = nullptr
    );
    virtual ~DoctorDashboardWidget() override = default;

protected:
  virtual void fillDashboardData() override;

private:
    std::shared_ptr<PatientService> m_patientService = nullptr;
    std::shared_ptr<AppointmentService> m_appointmentService = nullptr;
    std::shared_ptr<MedicalRecordService> m_medicalRecordService = nullptr;
    int m_currentExaminingRow = -1;
    std::shared_ptr<PharmacyService> m_pharmacyService = nullptr;
    std::shared_ptr<ServiceRequestService> m_serviceRequestService = nullptr;

    QStackedWidget* m_stackedWidget = nullptr;

    QPushButton* m_btnDash = nullptr;
    QPushButton* m_btnPatients = nullptr;
    QPushButton* m_btnAppoint = nullptr;
    QPushButton* m_btnSetting = nullptr;
    QPushButton* m_btnLeaveManage = nullptr;

    QWidget* m_overviewPage = nullptr;
    PatientWidget* m_patientsPage = nullptr;
    QWidget* m_appointmentsPage = nullptr;
    QWidget* m_settingsPage = nullptr;
    QWidget* m_leaveManagePage = nullptr;
    ClinicalExamWidget* m_clinicalExamPage = nullptr;

    // Leave Management UI components
    QLabel* m_lblLeaveBalance = nullptr;
    QDateEdit* m_leaveStartDate = nullptr;
    QDateEdit* m_leaveEndDate = nullptr;
    QTextEdit* m_txtLeaveReason = nullptr;

    QTabWidget* m_leaveTabWidget = nullptr;
    QTableWidget* m_tableLeaveHistory = nullptr;

    void buildSidebar();
    void buildOverviewPage();
    void buildPatientsPage();
    void buildAppointmentsPage();
    void buildSettingsPage();
    void buildLeaveManagePage();
    void buildClinicalExamPage();

    QFrame* makeCard(QWidget* parent = nullptr);

    void createDoctorCards(QWidget* parentPage, QVBoxLayout* pageLayout);
    void createDoctorCharts(QWidget* parentPage, QVBoxLayout* pageLayout);
    void createDoctorTable(QWidget* parentPage, QVBoxLayout* pageLayout);

    void switchPage(int index, QPushButton* activeBtn);
    void openClinicalExam(const QString& name,
            const QString& id,
            const QString& time,
            const QString& specialty,
            int rowIndex,
            bool isFromTodayList = true);
    void openClinicalExamWithIds(int patientId,
            int appointmentId,
            const QString& name,
            const QString& code,
            const QString& time,
            const QString& reason);
    void handlePatientExamFinished();
    void handleCallPatientRequested(int apptId = -1);
    void refreshAppointmentsTables();
    void refreshOverviewCards();

    QLabel* m_lblCardAppointments = nullptr;
    QLabel* m_lblCardRevenue = nullptr;

   private slots:
    void onLeaveTabSelected();
    void loadLeaveHistory();
    void onSubmitLeaveRequest();

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
    QTableWidget* m_appointmentsTable = nullptr;
    QTableWidget* m_patientTable = nullptr;
    QDateEdit* m_apptDateFilter = nullptr;
    bool m_isExaminingFromTodayList = true;
};