#pragma once

#include <QDialog>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QPushButton;

class PatientRegistrationDialog : public QDialog {
    Q_OBJECT
public:
    explicit PatientRegistrationDialog(QWidget* parent = nullptr);
    ~PatientRegistrationDialog() = default;

signals:
    void cancelled();
    void saved(QString phone, QString name);

private:
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
    QLineEdit* m_txtEmergencyName;
    QLineEdit* m_txtEmergencyPhone;

    QPushButton* m_btnSave;
    QPushButton* m_btnCancel;
};
