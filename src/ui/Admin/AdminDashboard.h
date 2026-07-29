#pragma once

#include "../../model/Doctor.h"
#include "../../service/PatientService.h"
#include "../../service/StaffService.h"
#include "../BaseDashboard.h"
#include <QComboBox>
#include <QDateEdit>
#include <QFrame>
#include <QLineEdit>
#include <QStackedWidget>

class ManageDoctorsWidget;
class ManageNursesWidget;
class ManagePatientsWidget;
class ManageReceptionWidget;
class ManageLeaveWidget;

class AdminDashboardWidget : public BaseDashboardWidget {
  Q_OBJECT

public:
  explicit AdminDashboardWidget(
      std::shared_ptr<IAuthenticatable> user = nullptr,
      std::shared_ptr<StaffService> staffService = nullptr,
      std::shared_ptr<PatientService> patientService = nullptr,
      std::shared_ptr<AppointmentService> appointmentService = nullptr,
      QWidget *parent = nullptr);
  ~AdminDashboardWidget() override = default;

protected:
  void fillDashboardData() override;

private:
  void buildSidebar();

  std::shared_ptr<StaffService> m_staffService;
  std::shared_ptr<PatientService> m_patientService;


  // Các thành phần UI
  QStackedWidget *m_stackedWidget;
  ManageDoctorsWidget *m_manageDoctorsPage;
  ManageNursesWidget *m_manageNursesPage;
  ManagePatientsWidget *m_managePatientsPage;
  ManageReceptionWidget *m_manageReceptionPage;
  ManageLeaveWidget *m_manageLeavesPage;

  QPushButton *m_btnManageLeaves;
};
