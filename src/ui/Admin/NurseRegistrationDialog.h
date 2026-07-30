#pragma once

#include "service/StaffService.h"
#include "dto/StaffDTOs.h"
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <memory>

class NurseRegistrationDialog : public QDialog {
    Q_OBJECT
public:
    explicit NurseRegistrationDialog(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~NurseRegistrationDialog() override = default;

    void loadNurseData(NurseProfileDTO* nurse);

private slots:
    void handleSave();

private:
    void setupUi();

    std::shared_ptr<StaffService> m_staffService;
    int m_editStaffId;

    QLineEdit* m_txtFullName;
    QLineEdit* m_txtCitizenId;
    QComboBox* m_cbGender;
    QDateEdit* m_dtDateOfBirth;

    QLineEdit* m_txtPhone;
    QLineEdit* m_txtEmail;
    QLineEdit* m_txtAddress;

    QComboBox* m_cbDepartment;
    QDateEdit* m_dtHireDate;
    QComboBox* m_cbShift;
    QComboBox* m_cbNurseLevel;
    QLineEdit* m_txtCertification;

    QPushButton* m_btnCancel;
    QPushButton* m_btnSave;
};
