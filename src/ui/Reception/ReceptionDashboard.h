#pragma once

#include "../BaseDashboard.h"
#include <QString>
#include <memory>

class IAuthenticatable;
class StaffService;
class QStackedWidget;
class QPushButton;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QDateEdit;
class QTableWidget;

class ReceptionDashboardWidget : public BaseDashboardWidget {
  Q_OBJECT

public:
  explicit ReceptionDashboardWidget(
      std::shared_ptr<IAuthenticatable> user = nullptr,
      std::shared_ptr<StaffService> staffService = nullptr,
      std::shared_ptr<PatientService> patientService = nullptr,
      std::shared_ptr<AppointmentService> appointmentService = nullptr,
      QWidget *parent = nullptr);
  virtual ~ReceptionDashboardWidget() override = default;

protected:
  virtual void fillDashboardData() override;

private:

  std::shared_ptr<StaffService> m_staffService;

  // Sidebar buttons and stacked widget
  QStackedWidget *m_stackedWidget = nullptr;
  QPushButton *m_btnOverview = nullptr;
  QPushButton *m_btnRegister = nullptr;
  QPushButton *m_btnPatients = nullptr;
  QPushButton *m_btnManageAppts = nullptr;
  QPushButton *m_btnRoomQueue = nullptr;

  // Pages
  QWidget *m_overviewPage = nullptr;
  QWidget *m_registerPage = nullptr;
  QWidget *m_patientsPage = nullptr;
  QWidget *m_appointmentsPage = nullptr;
  QWidget *m_roomQueuePage = nullptr;

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
  class QLabel *m_lblRevenue = nullptr;
  class QLabel *m_lblPatientNum = nullptr;

  // Management UI
  QTableWidget *m_appointmentsTable = nullptr;
  QDateEdit *m_apptDateEdit = nullptr;

  // Build functions
  void buildSidebar();
  void buildOverviewPage();
  void buildRegisterPage();
  void buildPatientsPage();
  void buildAppointmentsPage();
  void buildRoomQueuePage();
  void onPatientTypeSelected(bool isNew);

  class QFrame *makeCard(QWidget *parent = nullptr);

  // Helpers
  void clearLayout(QLayout *layout);
  void switchPage(int index, QPushButton *activeBtn);
  void loadAvailableDoctors();
  void loadDoctorTimeSlots(int docId, const QString &date);
  void updateAppointmentsTable();
  void showPatientHistoryDialog(int patientId, const QString &patientName);

private slots:
  void onContinueClicked();
  void onConfirmClicked();

  void updateDoctorList();
  void onRefreshRoomQueue();
  void onTimeSlotClicked(int doctorId, const QString &timeSlot,
                         QPushButton *btn);
};
