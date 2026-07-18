#pragma once

#include <QDialog>
#include <memory>
#include "../../service/StaffService.h"

class QLineEdit;
class QComboBox;
class QDateEdit;
class QPushButton;

class ReceptionRegistrationDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReceptionRegistrationDialog(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~ReceptionRegistrationDialog() override = default;

private slots:
    void handleSave();

private:
    void setupUi();

    std::shared_ptr<StaffService> m_staffService;

    // Các trường form tạo Lễ tân
    QLineEdit *m_txtFullName;
    QLineEdit *m_txtCitizenId;
    QLineEdit *m_txtPhone;

    QComboBox *m_cbGender;
    QDateEdit *m_dtDateOfBirth;
    QLineEdit *m_txtEmail;
    QLineEdit *m_txtAddress;
    QComboBox *m_cbShift;

    QPushButton* m_btnSave;
    QPushButton* m_btnCancel;
};
