#pragma once
#include "../BaseDashboard.h"
#include <QDateEdit>
#include <QTextEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <memory>


class IAuthenticatable;
class QVBoxLayout;
class QHBoxLayout;
class QStackedWidget;
class QCalendarWidget;
class PatientWidget;

class DoctorDashboardWidget : public BaseDashboardWidget {
  Q_OBJECT

public:
  explicit DoctorDashboardWidget(
      std::shared_ptr<IAuthenticatable> user = nullptr,
      std::shared_ptr<StaffService> staffService = nullptr,
      std::shared_ptr<PatientService> patientService = nullptr,
      std::shared_ptr<AppointmentService> appointmentService = nullptr,
      QWidget *parent = nullptr);
  virtual ~DoctorDashboardWidget() override = default;

protected:
  virtual void fillDashboardData() override;

private:

  int m_currentExaminingRow = -1;

  QStackedWidget *m_stackedWidget = nullptr;
  QPushButton *m_btnDash = nullptr;
  QPushButton *m_btnPatients = nullptr;
  QPushButton *m_btnAppoint = nullptr;
  QPushButton *m_btnSetting = nullptr;
  QPushButton *m_btnLeaveManage = nullptr;

  QWidget *m_overviewPage = nullptr;
  PatientWidget *m_patientsPage = nullptr;
  QWidget *m_appointmentsPage = nullptr;
  QWidget *m_settingsPage = nullptr;
  QWidget *m_leaveManagePage = nullptr;
  class ClinicalExamWidget *m_clinicalExamPage = nullptr;

  // Leave Management UI components
  class QLabel *m_lblLeaveBalance = nullptr;
  class QDateEdit *m_leaveStartDate = nullptr;
  class QDateEdit *m_leaveEndDate = nullptr;
  class QTextEdit *m_txtLeaveReason = nullptr;
  
  QTabWidget *m_leaveTabWidget = nullptr;
  QTableWidget *m_tableLeaveHistory = nullptr;

  void buildSidebar();
  void buildOverviewPage();
  void buildPatientsPage();
  void buildAppointmentsPage();
  void buildSettingsPage();
  void buildLeaveManagePage();
  void buildClinicalExamPage();

  QFrame *makeCard(QWidget *parent = nullptr);

  void createDoctorCards(QWidget *parentPage, QVBoxLayout *pageLayout);
  void createDoctorCharts(QWidget *parentPage, QVBoxLayout *pageLayout);
  void createDoctorTable(QWidget *parentPage, QVBoxLayout *pageLayout);

  void switchPage(int index, QPushButton *activeBtn);
  void openClinicalExam(const QString &name, const QString &id,
                        const QString &time, const QString &specialty,
                        int rowIndex, bool isFromTodayList = true);
  void handlePatientExamFinished();
  void handleCallPatientRequested();
  void refreshAppointmentsTables();

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
  QTableWidget *m_appointmentsTable = nullptr;
  QTableWidget *m_patientTable = nullptr;
  bool m_isExaminingFromTodayList = true;
};