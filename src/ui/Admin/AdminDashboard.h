#pragma once

#include "../../service/StaffService.h"
#include "../../model/Doctor.h"
#include "../BaseDashboard.h"
#include <QComboBox>
#include <QDateEdit>
#include <QFrame>
#include <QLineEdit>
#include <QStackedWidget>

#include <QStackedWidget>

class ManageDoctorsWidget;
class ManageNursesWidget;
class ManagePatientsWidget;
class ManageReceptionWidget;

class AdminDashboardWidget : public BaseDashboardWidget {
  Q_OBJECT

public:
  explicit AdminDashboardWidget(std::shared_ptr<IAuthenticatable> user,
                                std::shared_ptr<StaffService> staffService,
                                QWidget *parent = nullptr);
  ~AdminDashboardWidget() override = default;

protected:
  void fillDashboardData() override;

private:
  void buildSidebar();

  std::shared_ptr<StaffService> m_staffService;
  std::shared_ptr<IAuthenticatable> m_currentUser;

  // Các thành phần UI
  QStackedWidget *m_stackedWidget;
  ManageDoctorsWidget *m_manageDoctorsPage;
  ManageNursesWidget *m_manageNursesPage;
  ManagePatientsWidget *m_managePatientsPage;
  ManageReceptionWidget *m_manageReceptionPage;
};
