#pragma once

#include "service/StaffService.h"
#include "../view/AvatarPickerWidget.h"
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <memory>

class ReceptionRegistrationDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReceptionRegistrationDialog(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~ReceptionRegistrationDialog() override = default;

    void loadReceptionistData(StaffProfileDTO* receptionist);
    void setReadOnlyMode(bool readOnly);

private slots:
    void handleSave();

private:
    void setupUi();

    std::shared_ptr<StaffService> m_staffService;
    int m_editStaffId = -1;

    // Các trường form tạo Lễ tân
    AvatarPickerWidget *m_avatarPicker;
    QLineEdit *m_txtFullName;
    QLineEdit *m_txtCitizenId;
    QLineEdit *m_txtPhone;

    QComboBox *m_cbGender;
    QDateEdit *m_dtDateOfBirth;
    QLineEdit *m_txtEmail;
    QLineEdit *m_txtAddress;
    QComboBox *m_cbDepartment;
    QComboBox *m_cbShift;
    QDateEdit *m_dtHireDate;

    QPushButton* m_btnSave;
    QPushButton* m_btnCancel;
    bool m_isReadOnly = false;
    QLabel* m_lblPageTitle = nullptr;
};
