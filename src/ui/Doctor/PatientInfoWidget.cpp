#include "PatientInfoWidget.h"
#include <QStringList>
#include <QDate>

PatientInfoWidget::PatientInfoWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void PatientInfoWidget::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    QFrame* cardAdminInfo = new QFrame(this);
    cardAdminInfo->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; }");
    QVBoxLayout* cardAdminLayout = new QVBoxLayout(cardAdminInfo);
    cardAdminLayout->setContentsMargins(25, 25, 25, 25);
    cardAdminLayout->setSpacing(20);

    QGridLayout* inputGridLayout = new QGridLayout();
    inputGridLayout->setHorizontalSpacing(25);   
    inputGridLayout->setVerticalSpacing(18);
    
    QString labelStyle = "QLabel { font-size: 13px; font-weight: bold; color: #334155; border: none; font-family: 'Segoe UI'; }";
    QString inputStyle = 
        "QLineEdit, QComboBox { font-size: 14px; color: #000000; border: 1px solid transparent; background-color: transparent; padding: 0px 4px; min-height: 34px; }"
        "QLineEdit:read-only, QComboBox:disabled { color: #1E293B; }";

    // --- Row 1 ---
    QVBoxLayout* boxID = new QVBoxLayout();
    boxID->setSpacing(7);
    QLabel* lblID = new QLabel("Mã bệnh nhân", cardAdminInfo); 
    lblID->setStyleSheet(labelStyle);
    txtPatientID = new QLineEdit(cardAdminInfo);
    txtPatientID->setStyleSheet(inputStyle);
    txtPatientID->setReadOnly(true);
    boxID->addWidget(lblID);
    boxID->addWidget(txtPatientID);
    inputGridLayout->addLayout(boxID, 0, 0);

    QVBoxLayout* boxName = new QVBoxLayout();
    boxName->setSpacing(7);
    QLabel* lblName = new QLabel("Họ và tên", cardAdminInfo);
    lblName->setStyleSheet(labelStyle);
    txtFullName = new QLineEdit(cardAdminInfo);
    txtFullName->setStyleSheet(inputStyle);
    txtFullName->setReadOnly(true);
    boxName->addWidget(lblName);
    boxName->addWidget(txtFullName);
    inputGridLayout->addLayout(boxName, 0, 1);

    // --- Row 0 ---
    QVBoxLayout* boxAge = new QVBoxLayout();
    boxAge->setSpacing(7);
    QLabel* lblAge = new QLabel("Tuổi", cardAdminInfo);
    lblAge->setStyleSheet(labelStyle);
    txtAge = new QLineEdit(cardAdminInfo);
    txtAge->setStyleSheet(inputStyle);
    txtAge->setReadOnly(true);
    boxAge->addWidget(lblAge);
    boxAge->addWidget(txtAge);
    inputGridLayout->addLayout(boxAge, 0, 2);

    // --- Row 1 ---

    QVBoxLayout* boxGender = new QVBoxLayout();
    boxGender->setSpacing(7);
    QLabel* lblGender = new QLabel("Giới tính", cardAdminInfo);
    lblGender->setStyleSheet(labelStyle);
    cbGender = new QComboBox(cardAdminInfo);
    cbGender->setStyleSheet(inputStyle + "QComboBox::drop-down { border: 0px; } QComboBox::down-arrow { image: url(:/icons/down_arrow.png); width: 10px; height: 10px; }");
    for (const auto& pair : GenderText::getList()) {
        cbGender->addItem(pair.second, pair.first);
    }
    cbGender->setEnabled(false);
    boxGender->addWidget(lblGender);
    boxGender->addWidget(cbGender);
    inputGridLayout->addLayout(boxGender, 1, 0);


    QVBoxLayout* boxBloodType = new QVBoxLayout();
    boxBloodType->setSpacing(7);
    QLabel* lblBloodType = new QLabel("Nhóm máu", cardAdminInfo);
    lblBloodType->setStyleSheet(labelStyle);
    cbBloodType = new QComboBox(cardAdminInfo);
    cbBloodType->setStyleSheet(inputStyle + "QComboBox::drop-down { border: 0px; } QComboBox::down-arrow { image: url(:/icons/down_arrow.png); width: 10px; height: 10px; }");
    for (const auto& pair : BloodTypeText::getList()) {
        cbBloodType->addItem(pair.second, pair.first);
    }
    cbBloodType->setEnabled(false);
    boxBloodType->addWidget(lblBloodType);
    boxBloodType->addWidget(cbBloodType);
    inputGridLayout->addLayout(boxBloodType, 1, 1);

    QVBoxLayout* boxAllergies = new QVBoxLayout();
    boxAllergies->setSpacing(7);
    QLabel* lblAllergies = new QLabel("Dị ứng", cardAdminInfo);
    lblAllergies->setStyleSheet(labelStyle);
    txtAllergies = new QLineEdit(cardAdminInfo);
    txtAllergies->setStyleSheet(inputStyle);
    txtAllergies->setReadOnly(true);
    boxAllergies->addWidget(lblAllergies);
    boxAllergies->addWidget(txtAllergies);
    inputGridLayout->addLayout(boxAllergies, 1, 2);

    cardAdminLayout->addLayout(inputGridLayout);
    mainLayout->addWidget(cardAdminInfo);
    mainLayout->addStretch();
}

void PatientInfoWidget::loadPatientData(const PatientDetailDTO& detail) {
    if (txtPatientID) txtPatientID->setText(detail.patientCode);
    if (txtFullName) txtFullName->setText(detail.fullName);
    
    if (txtAge) {
        int age = QDate::currentDate().year() - detail.dateOfBirth.year();
        if (QDate::currentDate().dayOfYear() < detail.dateOfBirth.dayOfYear()) {
            age--;
        }
        txtAge->setText(QString::number(std::max(0, age)));
    }
    
    if (cbGender) cbGender->setCurrentText(GenderText::toVi(detail.gender));
    if (cbBloodType) {
        QString bt = detail.bloodType.trimmed().toUpper();
        if (bt.isEmpty() || bt == "UNKNOWN") {
            cbBloodType->setCurrentText(BloodTypeText::toVi("UNKNOWN"));
            cbBloodType->setEnabled(true);
        } else {
            cbBloodType->setCurrentText(BloodTypeText::toVi(bt));
            cbBloodType->setEnabled(false);
        }
    }
    
    if (txtAllergies) {
        QStringList allergyList;
        for (const auto& a : detail.allergies) {
            QString typeLabel = a.ingredientId.has_value() ? "[Hoạt chất]" : "[Khác]";
            allergyList.append(QString("%1 %2").arg(typeLabel, a.allergenName));
        }
        txtAllergies->setText(allergyList.join(", "));
    }
}
