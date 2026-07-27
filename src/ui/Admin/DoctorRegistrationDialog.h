#pragma once

#include "../../service/StaffService.h"
#include <QDialog>
#include <memory>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QSpinBox;
class QTextEdit;
class QPushButton;
class QLabel;
struct DoctorProfileDTO;

class DoctorRegistrationDialog : public QDialog {
  Q_OBJECT
public:
  explicit DoctorRegistrationDialog(std::shared_ptr<StaffService> staffService,
                                    QWidget *parent = nullptr);
  ~DoctorRegistrationDialog() override = default;

  void loadDoctorData(DoctorProfileDTO *doctor);

private slots:
  void handleSave();

private:
  void setupUi();

  std::shared_ptr<StaffService> m_staffService;
  int m_editStaffId = -1;

  // Các trường form tạo bác sĩ
  QLineEdit *m_txtFullName;
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

  QPushButton *m_btnSave;
  QPushButton *m_btnCancel;
};
