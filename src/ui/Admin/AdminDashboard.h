#pragma once

#include "ui/BaseDashboard.h"
#include "ui/Admin/ManageDoctorsWidget.h"
#include "ui/Admin/ManageNursesWidget.h"
#include "ui/Admin/ManagePatientsWidget.h"
#include "ui/Admin/ManageReceptionWidget.h"
#include "ui/Admin/ManagePharmacistsWidget.h"
#include "ui/Admin/ManageLeaveWidget.h"
#include "ui/Admin/AdminAnalyticsWidget.h"
#include "service/StaffService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"
#include "service/AnalyticService.h"
#include <QComboBox>
#include <QDateEdit>
#include <QFrame>
#include <QLineEdit>
#include <QStackedWidget>
#include <QPushButton>

class AdminDashboardWidget : public BaseDashboardWidget {
  Q_OBJECT

public:
  explicit AdminDashboardWidget(
      std::shared_ptr<IAuthenticatable> user = nullptr,
      std::shared_ptr<StaffService> staffService = nullptr,
      std::shared_ptr<PatientService> patientService = nullptr,
      std::shared_ptr<AppointmentService> appointmentService = nullptr,
      std::shared_ptr<AnalyticService> analyticService = nullptr,
      QWidget *parent = nullptr);
  ~AdminDashboardWidget() override = default;

protected:
  void fillDashboardData() override;

private:
  void buildSidebar();

  std::shared_ptr<StaffService> m_staffService;
  std::shared_ptr<PatientService> m_patientService;
  std::shared_ptr<AnalyticService> m_analyticService;


  // Các thành phần UI
  QStackedWidget *m_stackedWidget;
  ManageDoctorsWidget *m_manageDoctorsPage;
  ManageNursesWidget *m_manageNursesPage;
  ManagePatientsWidget *m_managePatientsPage;
  ManageReceptionWidget *m_manageReceptionPage;
  ManageLeaveWidget *m_manageLeavesPage;
  ManagePharmacistsWidget *m_managePharmacistsPage;
  AdminAnalyticsWidget *m_analyticsPage;

  QPushButton *m_btnManageLeaves;
  QPushButton *m_btnAnalytics;
};
