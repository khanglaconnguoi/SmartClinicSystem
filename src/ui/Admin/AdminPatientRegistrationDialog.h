#pragma once

#include <QDialog>
#include <memory>
#include "../../service/PatientService.h"

class QLineEdit;
class QComboBox;
class QDateEdit;
class QPushButton;

class AdminPatientRegistrationDialog : public QDialog {
    Q_OBJECT
public:
    explicit AdminPatientRegistrationDialog(std::shared_ptr<PatientService> patientService, QWidget* parent = nullptr);
    ~AdminPatientRegistrationDialog() override = default;

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
    QLineEdit *m_txtInsurance;

    QLineEdit *m_txtEmergencyContactName;
    QLineEdit *m_txtEmergencyContactPhone;

    QPushButton* m_btnSave;
    QPushButton* m_btnCancel;
};
