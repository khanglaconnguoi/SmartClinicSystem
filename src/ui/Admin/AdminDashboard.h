#pragma once

#include "../../service/StaffService.h"
#include "../BaseDashboard.h"
#include <QComboBox>
#include <QDateEdit>
#include <QFrame>
#include <QLineEdit>
#include <QStackedWidget>

#include <QHeaderView>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextEdit>

class AdminDashboardWidget : public BaseDashboardWidget {
  Q_OBJECT

public:
  explicit AdminDashboardWidget(std::shared_ptr<IAuthenticatable> user,
                                std::shared_ptr<StaffService> staffService,
                                QWidget *parent = nullptr);
  ~AdminDashboardWidget() override = default;

protected:
  void fillDashboardData() override;

private slots:
  void handleAddDoctor();
  void showAddDoctorForm();
  void showManageDoctorsList();
  void showManageNursesList();
  void showManagePatientsList();

private:
  void buildSidebar();
  void buildManageDoctorsPage();
  void buildManageNursesPage();
  void buildManagePatientsPage();
  void buildAddDoctorPage();
  void loadDoctorsList();
  void loadPatientsList();

  QFrame *makeCard(QWidget *parent);

  std::shared_ptr<StaffService> m_staffService;
  std::shared_ptr<IAuthenticatable> m_currentUser;

  // Các thành phần UI
  QStackedWidget *m_stackedWidget;
  QWidget *m_addDoctorPage;
  QWidget *m_manageDoctorsPage;
  QWidget *m_manageNursesPage;
  QWidget *m_managePatientsPage;
  QTableWidget *m_tblDoctors;
  QTableWidget *m_tblNurses;
  QTableWidget *m_tblPatients;

  // Các trường form tạo bác sĩ
  QLineEdit *m_txtFullName;
  QLineEdit *m_txtPassword;
  QLineEdit *m_txtStaffCode;
  QLineEdit *m_txtCitizenId;
  QLineEdit *m_txtPhone;
  QComboBox *m_cbSpecialty;

  QComboBox *m_cbGender;
  QDateEdit *m_dtDateOfBirth;
  QLineEdit *m_txtEmail;
  QLineEdit *m_txtAddress;
  QComboBox *m_cbDepartment;
  QDateEdit *m_dtHireDate;
  QComboBox *m_cbShift;
  QLineEdit *m_txtLicenseNumber;
  QSpinBox *m_sbExperienceYears;
  QSpinBox *m_sbConsultationFee;
  QTextEdit *m_txtBio;
};
