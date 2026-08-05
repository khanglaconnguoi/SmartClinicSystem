#pragma once

#include "service/StaffService.h"
#include "dto/StaffDTOs.h"
#include "../view/AvatarPickerWidget.h"
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QSpinBox>
#include <memory>

class PharmacistRegistrationDialog : public QDialog {
    Q_OBJECT
public:
    explicit PharmacistRegistrationDialog(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~PharmacistRegistrationDialog() override = default;

    void loadPharmacistData(PharmacistProfileDTO* pharmacist);
    void setReadOnlyMode(bool readOnly);

private slots:
    void handleSave();

private:
    void setupUi();

    std::shared_ptr<StaffService> m_staffService;
    int m_editStaffId = -1;

    AvatarPickerWidget *m_avatarPicker;
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
    
    QLineEdit* m_txtLicenseNumber;
    QLineEdit* m_txtPharmacySection;
    QSpinBox* m_sbExperienceYears;

    QPushButton* m_btnCancel;
    QPushButton* m_btnSave;
    bool m_isReadOnly = false;
    QLabel* m_lblPageTitle = nullptr;
};
