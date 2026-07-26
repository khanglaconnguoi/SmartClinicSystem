#pragma once

#include <QDialog>

#include <memory>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QPushButton;
class PatientService;

class PatientRegistrationDialog : public QDialog {
    Q_OBJECT
public:
    explicit PatientRegistrationDialog(std::shared_ptr<PatientService> patientService, QWidget* parent = nullptr);
    ~PatientRegistrationDialog() = default;

signals:
    void cancelled();
    void saved(QString phone, QString name);

private:
    std::shared_ptr<PatientService> m_patientService;
    void setupUi();

    QLineEdit* m_txtFullName;
    QDateEdit* m_dateOfBirth;
    QComboBox* m_cboGender;
    QLineEdit* m_txtCitizenId;
    QLineEdit* m_txtPhone;
    QLineEdit* m_txtEmail;
    QLineEdit* m_txtAddress;
    QComboBox* m_cboBloodType;
    QComboBox* m_cboPatientType;
    // Các trường Bảo hiểm y tế
    QComboBox *m_cbInsuranceType;
    QLineEdit *m_txtInsuranceProvider;
    QLineEdit *m_txtInsurancePolicy;
    class QDoubleSpinBox *m_spinInsuranceCoverage;
    QDateEdit *m_dateInsuranceFrom;
    QDateEdit *m_dateInsuranceTo;
    QLineEdit* m_txtEmergencyName;
    QLineEdit* m_txtEmergencyPhone;

    QPushButton* m_btnSave;
    QPushButton* m_btnCancel;
};
