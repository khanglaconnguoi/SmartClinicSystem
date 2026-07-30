#pragma once

#include "service/StaffService.h"
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
