#pragma once

#include "../BaseDashboard.h"
#include <QString>
#include <memory>

class IAuthenticatable;
class StaffService;
class PatientService;
class AppointmentService;
class AnalyticService;

class QStackedWidget;
class QPushButton;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QDateEdit;
class QTableWidget;
class QLabel;
class QTextEdit;
class QLineEdit;
class QTabWidget;
class QGridLayout;
class QFrame;
class QScrollArea;

class ReceptionDashboardWidget : public BaseDashboardWidget {
  Q_OBJECT

public:
  explicit ReceptionDashboardWidget(
      std::shared_ptr<IAuthenticatable> user = nullptr,
      std::shared_ptr<StaffService> staffService = nullptr,
      std::shared_ptr<PatientService> patientService = nullptr,
      std::shared_ptr<AppointmentService> appointmentService = nullptr,
      std::shared_ptr<AnalyticService> analyticService = nullptr,
      QWidget *parent = nullptr);
  virtual ~ReceptionDashboardWidget() override = default;

protected:
  virtual void fillDashboardData() override;

private:

  std::shared_ptr<PatientService> m_patientService;
  std::shared_ptr<AppointmentService> m_appointmentService;
  std::shared_ptr<AnalyticService> m_analyticService;

  // Sidebar buttons and stacked widget
  QStackedWidget *m_stackedWidget = nullptr;
  QPushButton *m_btnOverview = nullptr;
  QPushButton *m_btnRegister = nullptr;
  QPushButton *m_btnPatients = nullptr;
  QPushButton *m_btnRoomQueue = nullptr;
  QPushButton *m_btnLeaveManage = nullptr;

  // Pages
  QWidget *m_overviewPage = nullptr;
  QWidget *m_registerPage = nullptr;
  QWidget *m_patientsPage = nullptr;
  QWidget *m_roomQueuePage = nullptr;
  QWidget *m_leaveManagePage = nullptr;

  // Leave Management UI
  QLabel *m_lblLeaveBalance = nullptr;
  QDateEdit *m_leaveStartDate = nullptr;
  QDateEdit *m_leaveEndDate = nullptr;
  QTextEdit *m_txtLeaveReason = nullptr;
  QTabWidget *m_leaveTabWidget = nullptr;
  QTableWidget *m_tableLeaveHistory = nullptr;

  // Room Queue Page UI
  class QGridLayout *m_roomQueueLayout = nullptr;

  // Registration UI elements
  class QFrame *m_apptCard = nullptr;
  QLineEdit *m_txtPatientPhone = nullptr;
  QLineEdit *m_txtPatientCitizenId = nullptr;
  QPushButton *m_btnContinue = nullptr;
  QPushButton *m_btnConfirm = nullptr;
  QPushButton *m_btnCancel = nullptr;
  int m_currentPatientId = -1;

  QComboBox *m_comboSpecialty = nullptr;
  QDateEdit *m_dateEdit = nullptr;

  // New UI Elements for Doctor Cards
  class QScrollArea *m_scrollArea = nullptr;
  QVBoxLayout *m_doctorListLayout = nullptr;

  // State variables for selection
  int m_selectedDoctorId = -1;
  QString m_selectedTimeSlot = "";
  QPushButton *m_selectedSlotButton = nullptr;

  // Overview UI elements
  class QLabel *m_lblCompletedToday = nullptr;
  class QLabel *m_lblStartedToday = nullptr;
  class QLabel *m_lblCheckedInToday = nullptr;
  class QLabel *m_lblAvgWaitTime = nullptr;
  class QLabel *m_lblScheduledToday = nullptr;
  class QLabel *m_lblNoShowCancelled = nullptr;
  class QLabel *m_lblTotalApptsToday = nullptr;
  class QLabel *m_lblActiveRooms = nullptr;
  QTableWidget *m_recentActivityTable = nullptr;

  // Management UI
  QTableWidget *m_patientsTable = nullptr;
  QLineEdit *m_txtPatientSearchKey = nullptr;
  QPushButton *m_btnResetPatientFilters = nullptr;
  QPushButton *m_btnPatientPrevPage = nullptr;
  QPushButton *m_btnPatientNextPage = nullptr;
  class QLabel *m_lblPatientPageInfo = nullptr;
  int m_patientCurrentPage = 1;
  int m_patientTotalPages = 1;

  // Build functions
  void buildSidebar();
  void buildOverviewPage();
  void refreshRecentActivity();
  void buildRegisterPage();
  void buildPatientsPage();
  void refreshPatientsTable();
  void handlePatientFilterChanged();
  void handlePatientResetFilters();
  void handlePatientPrevPage();
  void handlePatientNextPage();
  void buildRoomQueuePage();
  void buildLeaveManagePage();
  void onPatientTypeSelected(bool isNew);

  class QFrame *makeCard(QWidget *parent = nullptr);

  // Helpers
  void clearLayout(QLayout *layout);
  void switchPage(int index, QPushButton *activeBtn);
  void refreshOverviewStats();
  void loadAvailableDoctors();
  void loadDoctorTimeSlots(int docId, const QString &date);
  void showPatientHistoryDialog(int patientId, const QString &patientName);

  void onLeaveTabSelected();
  void loadLeaveHistory();
  void onSubmitLeaveRequest();

private slots:
  void onContinueClicked();
  void onConfirmClicked();

  void updateDoctorList();
  void onRefreshRoomQueue();
  void onTimeSlotClicked(int doctorId, const QString &timeSlot,
                         QPushButton *btn);
};
