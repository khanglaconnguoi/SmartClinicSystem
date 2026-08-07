#include "PatientEditDialog.h"
#include <QMessageBox>

PatientEditDialog::PatientEditDialog(int patientId, std::shared_ptr<PatientService> patientService, QWidget *parent)
    : QDialog(parent), m_patientId(patientId), m_patientService(patientService) {
    setWindowTitle("Thông tin liên lạc bệnh nhân");
    setFixedSize(500, 480);
    setStyleSheet("QDialog { background-color: #FFFFFF; font-family: 'Segoe UI', Arial, sans-serif; }"
              "QLabel { font-size: 13px; color: #374151; font-weight: 500; background-color: transparent; border: none; }"
              "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 8px 12px; font-size: 13px; background-color: #F9FAFB; color: #111827; }"
              "QLineEdit:focus { border: 1px solid #2563EB; background-color: #FFFFFF; }"
              "QPushButton { font-weight: 600; font-size: 13px; padding: 8px 16px; border-radius: 6px; }");
    setupUi();
    loadPatientData();
    setEditMode(false);
}

void PatientEditDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    QLabel *lblTitle = new QLabel("Thông tin liên lạc của bệnh nhân", this);
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #111827; margin-bottom: 8px; background-color: transparent; border: none;");
    mainLayout->addWidget(lblTitle);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignLeft);

    QString permanentReadOnlyStyle = 
        "QLineEdit { border: 1px solid #E2E8F0; border-radius: 6px; padding: 8px 12px; font-size: 13px; background-color: #F1F5F9; color: #334155; font-weight: bold; }";

    m_txtPatientCode = new QLineEdit(this);
    m_txtPatientCode->setReadOnly(true);
    m_txtPatientCode->setStyleSheet(permanentReadOnlyStyle);

    m_txtFullName = new QLineEdit(this);
    m_txtFullName->setReadOnly(true);
    m_txtFullName->setStyleSheet(permanentReadOnlyStyle);

    m_txtPhone = new QLineEdit(this);
    m_txtEmail = new QLineEdit(this);
    m_txtAddress = new QLineEdit(this);
    m_txtEmergencyName = new QLineEdit(this);
    m_txtEmergencyPhone = new QLineEdit(this);

    formLayout->addRow("Mã bệnh nhân:", m_txtPatientCode);
    formLayout->addRow("Họ và tên:", m_txtFullName);
    formLayout->addRow("Số điện thoại *:", m_txtPhone);
    formLayout->addRow("Email:", m_txtEmail);
    formLayout->addRow("Địa chỉ *:", m_txtAddress);
    formLayout->addRow("Người liên hệ khẩn cấp:", m_txtEmergencyName);
    formLayout->addRow("SĐT người liên hệ:", m_txtEmergencyPhone);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_btnEditSave = new QPushButton("Sửa thông tin liên lạc", this);
    m_btnEditSave->setCursor(Qt::PointingHandCursor);

    m_btnCloseCancel = new QPushButton("Đóng", this);
    m_btnCloseCancel->setCursor(Qt::PointingHandCursor);

    buttonLayout->addWidget(m_btnEditSave);
    buttonLayout->addWidget(m_btnCloseCancel);

    mainLayout->addLayout(buttonLayout);

    connect(m_btnEditSave, &QPushButton::clicked, this, &PatientEditDialog::handleEditOrSave);
    connect(m_btnCloseCancel, &QPushButton::clicked, this, &PatientEditDialog::handleCloseOrCancel);
}

void PatientEditDialog::setEditMode(bool editable) {
    m_isEditMode = editable;

    // Contact fields toggle editability
    m_txtPhone->setReadOnly(!editable);
    m_txtEmail->setReadOnly(!editable);
    m_txtAddress->setReadOnly(!editable);
    m_txtEmergencyName->setReadOnly(!editable);
    m_txtEmergencyPhone->setReadOnly(!editable);

    QString readOnlyStyle =
        "QLineEdit { border: 1px solid #E2E8F0; border-radius: 6px; padding: 8px 12px; font-size: 13px; background-color: #F8FAFC; color: #334155; }";
    QString editStyle =
        "QLineEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 12px; font-size: 13px; background-color: #FFFFFF; color: #0F172A; }"
        "QLineEdit:focus { border: 1px solid #2563EB; background-color: #EFF6FF; }";

    QString currentStyle = editable ? editStyle : readOnlyStyle;
    m_txtPhone->setStyleSheet(currentStyle);
    m_txtEmail->setStyleSheet(currentStyle);
    m_txtAddress->setStyleSheet(currentStyle);
    m_txtEmergencyName->setStyleSheet(currentStyle);
    m_txtEmergencyPhone->setStyleSheet(currentStyle);

    if (editable) {
        m_btnEditSave->setText("Lưu thay đổi");
        m_btnEditSave->setStyleSheet(
            "QPushButton { background-color: #2563EB; color: white; border: none; font-weight: bold; padding: 8px 16px; border-radius: 6px; }"
            "QPushButton:hover { background-color: #1D4ED8; }"
        );
        m_btnCloseCancel->setText("Hủy");
        m_btnCloseCancel->setStyleSheet(
            "QPushButton { background-color: white; color: #4B5563; border: 1px solid #D1D5DB; font-weight: bold; padding: 8px 16px; border-radius: 6px; }"
            "QPushButton:hover { background-color: #F3F4F6; }"
        );
    } else {
        m_btnEditSave->setText("Sửa thông tin liên lạc");
        m_btnEditSave->setStyleSheet(
            "QPushButton { background-color: #D97706; color: white; border: none; font-weight: bold; padding: 8px 16px; border-radius: 6px; }"
            "QPushButton:hover { background-color: #B45309; }"
        );
        m_btnCloseCancel->setText("Đóng");
        m_btnCloseCancel->setStyleSheet(
            "QPushButton { background-color: white; color: #4B5563; border: 1px solid #D1D5DB; font-weight: bold; padding: 8px 16px; border-radius: 6px; }"
            "QPushButton:hover { background-color: #F3F4F6; }"
        );
    }
}

void PatientEditDialog::handleEditOrSave() {
    if (!m_isEditMode) {
        setEditMode(true);
    } else {
        handleSave();
    }
}

void PatientEditDialog::handleCloseOrCancel() {
    if (m_isEditMode) {
        loadPatientData();
        setEditMode(false);
    } else {
        reject();
    }
}

void PatientEditDialog::loadPatientData() {
    if (!m_patientService) return;
    m_patientDetail = m_patientService->getPatientById(m_patientId);
    if (!m_patientDetail.has_value()) {
        QMessageBox::warning(this, "Lỗi", "Không tìm thấy thông tin bệnh nhân.");
        reject();
        return;
    }

    const auto &p = m_patientDetail.value();
    m_txtPatientCode->setText(p.patientCode);
    m_txtFullName->setText(p.fullName);
    m_txtPhone->setText(p.phone);
    m_txtEmail->setText(p.email);
    m_txtAddress->setText(p.address);
    m_txtEmergencyName->setText(p.emergencyContactName);
    m_txtEmergencyPhone->setText(p.emergencyContactPhone);
}

void PatientEditDialog::handleSave() {
    if (!m_patientDetail.has_value() || !m_patientService) return;

    const auto &detail = m_patientDetail.value();

    if (m_txtPhone->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Số điện thoại không được để trống.");
        return;
    }
    if (m_txtAddress->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Địa chỉ không được để trống.");
        return;
    }

    QString errStr;

    if (detail.currentType == PatientType::Inpatient) {
        InPatientInputDTO dto;
        dto.fullName = m_txtFullName->text().trimmed();
        dto.dateOfBirth = detail.dateOfBirth;
        dto.gender = detail.gender;
        dto.citizenId = detail.citizenId;
        dto.phone = m_txtPhone->text().trimmed();
        dto.email = m_txtEmail->text().trimmed();
        dto.address = m_txtAddress->text().trimmed();
        dto.bloodType = detail.bloodType;
        dto.emergencyContactName = m_txtEmergencyName->text().trimmed();
        dto.emergencyContactPhone = m_txtEmergencyPhone->text().trimmed();
        dto.type = PatientType::Inpatient;

        dto.roomId = detail.roomId.value_or(0);
        dto.doctorId = detail.doctorId.value_or(0);
        dto.admissionDate = detail.admissionDate.value_or(QDate::currentDate());
        dto.dischargeDate = detail.dischargeDate;

        errStr = m_patientService->updateInPatient(m_patientId, dto, detail.status);
    } else if (detail.currentType == PatientType::Emergency) {
        EmergencyPatientInputDTO dto;
        dto.fullName = m_txtFullName->text().trimmed();
        dto.dateOfBirth = detail.dateOfBirth;
        dto.gender = detail.gender;
        dto.citizenId = detail.citizenId;
        dto.phone = m_txtPhone->text().trimmed();
        dto.email = m_txtEmail->text().trimmed();
        dto.address = m_txtAddress->text().trimmed();
        dto.bloodType = detail.bloodType;
        dto.emergencyContactName = m_txtEmergencyName->text().trimmed();
        dto.emergencyContactPhone = m_txtEmergencyPhone->text().trimmed();
        dto.type = PatientType::Emergency;

        dto.roomId = detail.roomId.value_or(0);
        dto.doctorId = detail.doctorId.value_or(0);
        dto.admissionDate = detail.admissionDate.value_or(QDate::currentDate());
        dto.dischargeDate = detail.dischargeDate;

        errStr = m_patientService->updateEmergencyPatient(m_patientId, dto, detail.status);
    } else {
        OutPatientInputDTO dto;
        dto.fullName = m_txtFullName->text().trimmed();
        dto.dateOfBirth = detail.dateOfBirth;
        dto.gender = detail.gender;
        dto.citizenId = detail.citizenId;
        dto.phone = m_txtPhone->text().trimmed();
        dto.email = m_txtEmail->text().trimmed();
        dto.address = m_txtAddress->text().trimmed();
        dto.bloodType = detail.bloodType;
        dto.emergencyContactName = m_txtEmergencyName->text().trimmed();
        dto.emergencyContactPhone = m_txtEmergencyPhone->text().trimmed();
        dto.type = PatientType::Outpatient;
        dto.doctorId = detail.doctorId;

        errStr = m_patientService->updateOutPatient(m_patientId, dto, detail.status);
    }

    if (!errStr.isEmpty()) {
        QMessageBox::warning(this, "Cập nhật thất bại", errStr);
        return;
    }

    QMessageBox::information(this, "Thành công", "Đã cập nhật thông tin bệnh nhân thành công.");
    emit patientUpdated();
    accept();
}
