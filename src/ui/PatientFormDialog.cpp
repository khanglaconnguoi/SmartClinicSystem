/**
 * @file    PatientFormDialog.cpp
 * @brief   Implementation cho PatientFormDialog.
 */
#include "PatientFormDialog.h"

#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

PatientFormDialog::PatientFormDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle(tr("Thêm Bệnh Nhân"));
}

PatientFormDialog::PatientFormDialog(const Patient& patient,
                                     QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle(tr("Sửa Bệnh Nhân"));
    populateForm(patient);
}

void PatientFormDialog::setupUi() {
    setMinimumWidth(420);

    // Tạo các widget input
    m_editFullName = new QLineEdit(this);
    m_editFullName->setPlaceholderText(tr("Nhập họ và tên..."));

    m_editBirthDate = new QDateEdit(this);
    m_editBirthDate->setCalendarPopup(true);
    m_editBirthDate->setDisplayFormat("dd/MM/yyyy");
    m_editBirthDate->setDate(QDate::currentDate());

    m_comboGender = new QComboBox(this);
    m_comboGender->addItem(tr("Nam"), static_cast<int>(Gender::Male));
    m_comboGender->addItem(tr("Nữ"), static_cast<int>(Gender::Female));
    m_comboGender->addItem(tr("Khác"), static_cast<int>(Gender::Other));

    m_editPhoneNumber = new QLineEdit(this);
    m_editPhoneNumber->setPlaceholderText(tr("Nhập số điện thoại..."));

    m_editAddress = new QLineEdit(this);
    m_editAddress->setPlaceholderText(tr("Nhập địa chỉ..."));

    m_editCitizenId = new QLineEdit(this);
    m_editCitizenId->setPlaceholderText(tr("Nhập số CCCD..."));

    m_editEmail = new QLineEdit(this);
    m_editEmail->setPlaceholderText(tr("Nhập email..."));

    m_editInsurance = new QLineEdit(this);
    m_editInsurance->setPlaceholderText(tr("Nhập số bảo hiểm..."));

    // Form layout
    auto* formLayout = new QFormLayout;
    formLayout->addRow(tr("Họ và Tên *:"), m_editFullName);
    formLayout->addRow(tr("Ngày Sinh:"), m_editBirthDate);
    formLayout->addRow(tr("Giới Tính:"), m_comboGender);
    formLayout->addRow(tr("Số ĐT *:"), m_editPhoneNumber);
    formLayout->addRow(tr("CCCD:"), m_editCitizenId);
    formLayout->addRow(tr("Email:"), m_editEmail);
    formLayout->addRow(tr("Bảo Hiểm:"), m_editInsurance);
    formLayout->addRow(tr("Địa Chỉ:"), m_editAddress);

    // Buttons
    m_btnSave = new QPushButton(tr("Lưu"), this);
    m_btnCancel = new QPushButton(tr("Hủy"), this);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnSave);
    btnLayout->addWidget(m_btnCancel);

    // Main layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(btnLayout);

    // Kết nối signal-slot bằng cú pháp con trỏ hàm (Convention §4.1)
    connect(m_btnSave, &QPushButton::clicked,
            this, &PatientFormDialog::handleSaveClicked);
    connect(m_btnCancel, &QPushButton::clicked,
            this, &PatientFormDialog::reject);
}

void PatientFormDialog::populateForm(const Patient& patient) {
    m_patientId = patient.id();
    m_isActive = patient.isActive();

    m_editFullName->setText(patient.fullName());
    m_editBirthDate->setDate(patient.birthDate());
    m_editPhoneNumber->setText(patient.phoneNumber());
    m_editAddress->setText(patient.address());
    m_editCitizenId->setText(patient.citizenId());
    m_editEmail->setText(patient.email());
    m_editInsurance->setText(patient.insurance());

    // Chọn đúng giới tính trong combo box
    int genderIdx = m_comboGender->findData(
        static_cast<int>(patient.gender()));
    if (genderIdx >= 0) {
        m_comboGender->setCurrentIndex(genderIdx);
    }
}

void PatientFormDialog::handleSaveClicked() {
    // Validation cơ bản ngay trên form
    if (m_editFullName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Thiếu thông tin"),
                             tr("Vui lòng nhập Họ và Tên."));
        m_editFullName->setFocus();
        return;
    }

    if (m_editPhoneNumber->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Thiếu thông tin"),
                             tr("Vui lòng nhập Số điện thoại."));
        m_editPhoneNumber->setFocus();
        return;
    }

    accept();
}

Patient PatientFormDialog::getPatient() const {
    Patient p;
    p.setId(m_patientId);
    p.setFullName(m_editFullName->text().trimmed());
    p.setBirthDate(m_editBirthDate->date());
    p.setGender(static_cast<Gender>(
        m_comboGender->currentData().toInt()));
    p.setPhoneNumber(m_editPhoneNumber->text().trimmed());
    p.setAddress(m_editAddress->text().trimmed());
    p.setCitizenId(m_editCitizenId->text().trimmed());
    p.setEmail(m_editEmail->text().trimmed());
    p.setInsurance(m_editInsurance->text().trimmed());
    p.setIsActive(m_isActive);
    return p;
}
