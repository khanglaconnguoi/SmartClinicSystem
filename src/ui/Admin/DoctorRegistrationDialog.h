#pragma once

#include "../../service/StaffService.h"
#include "../../service/AppointmentService.h"
#include "service/StaffService.h"
#include "dto/StaffDTOs.h"
#include "../view/AvatarPickerWidget.h"
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>

class DoctorRegistrationDialog : public QDialog {
  Q_OBJECT
public:
  explicit DoctorRegistrationDialog(std::shared_ptr<StaffService> staffService,
                                    std::shared_ptr<AppointmentService> appointmentService = nullptr,
                                    QWidget *parent = nullptr);
  ~DoctorRegistrationDialog() override = default;

  void loadDoctorData(DoctorProfileDTO *doctor);
  void setReadOnlyMode(bool readOnly);

private slots:
  void handleSave();

private:
  void setupUi();

  std::shared_ptr<StaffService> m_staffService;
  std::shared_ptr<AppointmentService> m_appointmentService;
  int m_editStaffId = -1;

  // Các trường form tạo bác sĩ
  AvatarPickerWidget *m_avatarPicker;
  QLineEdit *m_txtFullName;
  QLineEdit *m_txtCitizenId;
  QLineEdit *m_txtPhone;
  QComboBox *m_cbSpecialty;

  QComboBox *m_cbGender;
  QDateEdit *m_dtDateOfBirth;
  QLineEdit *m_txtEmail;
  QLineEdit *m_txtAddress;
  QComboBox *m_cbDepartment;
  QComboBox *m_cbRoom;
  QDateEdit *m_dtHireDate;
  QComboBox *m_cbShift;
  QLineEdit *m_txtLicenseNumber;
  QSpinBox *m_sbExperienceYears;
  QSpinBox *m_sbConsultationFee;
  QTextEdit *m_txtBio;

  QPushButton *m_btnSave;
  QPushButton *m_btnCancel;
  bool m_isReadOnly = false;
  QLabel *m_lblPageTitle = nullptr;
};

