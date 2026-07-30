#pragma once

#include "service/PatientService.h"
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <memory>

class PatientRegistrationDialog : public QDialog {
  Q_OBJECT
public:
  explicit PatientRegistrationDialog(
      std::shared_ptr<PatientService> patientService,
      QWidget *parent = nullptr);
  ~PatientRegistrationDialog() override = default;

signals:
  void cancelled();
  void saved(QString phone, QString name);

private slots:
  void handleSave();

private:
  void setupUi();

  std::shared_ptr<PatientService> m_patientService;

  // Các trường form tạo bệnh nhân
  QComboBox *m_cbPatientType;

  QLineEdit *m_txtFullName;
  QLineEdit *m_txtCitizenId;
  QLineEdit *m_txtPhone;
  QComboBox *m_cbGender;
  QDateEdit *m_dtDateOfBirth;
  QLineEdit *m_txtEmail;
  QLineEdit *m_txtAddress;

  QComboBox *m_cbBloodType;
  QLineEdit *m_txtAllergies;

  // Các trường Bảo hiểm y tế
  QComboBox *m_cbInsuranceType;
  QLineEdit *m_txtInsuranceProvider;
  QLineEdit *m_txtInsurancePolicy;
  QDoubleSpinBox *m_spinInsuranceCoverage;
  QDateEdit *m_dateInsuranceFrom;
  QDateEdit *m_dateInsuranceTo;

  QLineEdit *m_txtEmergencyContactName;
  QLineEdit *m_txtEmergencyContactPhone;

  QPushButton *m_btnSave;
  QPushButton *m_btnCancel;
};
