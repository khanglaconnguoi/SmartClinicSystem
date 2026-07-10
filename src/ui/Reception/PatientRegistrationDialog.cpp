#include "PatientRegistrationDialog.h"
#include <QComboBox>
#include <QDateEdit>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

PatientRegistrationDialog::PatientRegistrationDialog(QWidget *parent)
    : QDialog(parent) {
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setMinimumWidth(650);
  setupUi();
}

void PatientRegistrationDialog::setupUi() {
  QVBoxLayout *wrapperLayout = new QVBoxLayout(this);
  wrapperLayout->setContentsMargins(15, 15, 15, 15); // Space for shadow

  QFrame *container = new QFrame(this);
  container->setStyleSheet("QFrame { background-color: #F8F9FA; border-radius: "
                           "12px; border: 1px solid #D0D0D0; }");

  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 80));
  shadow->setOffset(0, 4);
  container->setGraphicsEffect(shadow);

  QVBoxLayout *mainLayout = new QVBoxLayout(container);
  mainLayout->setContentsMargins(20, 20, 20, 20);
  mainLayout->setSpacing(20);

  // Force QLabel styling to fix global theme leaking
  container->setStyleSheet(
      "QFrame { background-color: #F8F9FA; border-radius: 12px; border: 1px "
      "solid #D0D0D0; } "
      "QLabel { color: #333333; font-weight: bold; background: transparent; "
      "border: none; padding: 0; }");

  QLabel *lblTitle = new QLabel("THÔNG TIN BỆNH NHÂN", container);
  lblTitle->setStyleSheet(
      "font-size: 18px; font-weight: bold; color: #202124; border: none;");
  lblTitle->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(lblTitle);

  QString inputStyle =
      "QLineEdit, QComboBox, QDateEdit { "
      "min-height: 30px; padding-left: 8px; border: 1px solid #BDBDBD; "
      "border-radius: 4px; background-color: #FFFFFF; color: #333333; "
      "font-weight: normal; } "
      "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { "
      "border: 1px solid #4B94F2; } "
      "QComboBox QAbstractItemView { "
      "background-color: #FFFFFF; color: #333333; selection-background-color: "
      "#4B94F2; selection-color: white; }";

  QString groupBoxStyle =
      "QGroupBox { "
      "border: 1px solid #D0D0D0; border-radius: 6px; "
      "margin-top: 15px; font-weight: bold; color: #333333; background-color: "
      "transparent; } "
      "QGroupBox::title { "
      "subcontrol-origin: margin; subcontrol-position: top left; "
      "padding: 0 5px; color: #1A73E8; }";

  QGroupBox *gbPersonalInfo = new QGroupBox("Thông tin cá nhân", container);
  gbPersonalInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form1 = new QFormLayout(gbPersonalInfo);
  form1->setContentsMargins(15, 20, 15, 15);
  form1->setSpacing(10);
  form1->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

  m_txtFullName = new QLineEdit(this);
  m_txtFullName->setStyleSheet(inputStyle);
  m_txtFullName->setPlaceholderText("Họ và tên...");
  form1->addRow("Họ và tên (*):", m_txtFullName);

  m_dateOfBirth = new QDateEdit(QDate::currentDate(), this);
  m_dateOfBirth->setCalendarPopup(true);
  m_dateOfBirth->setStyleSheet(inputStyle);
  form1->addRow("Ngày sinh (*):", m_dateOfBirth);

  m_cboGender = new QComboBox(this);
  m_cboGender->addItems({"Nam", "Nữ", "Khác"});
  m_cboGender->setStyleSheet(inputStyle);
  form1->addRow("Giới tính (*):", m_cboGender);

  m_txtCitizenId = new QLineEdit(this);
  m_txtCitizenId->setStyleSheet(inputStyle);
  form1->addRow("CMND/CCCD (*):", m_txtCitizenId);

  m_txtPhone = new QLineEdit(this);
  m_txtPhone->setStyleSheet(inputStyle);
  form1->addRow("Số điện thoại (*):", m_txtPhone);

  m_txtEmail = new QLineEdit(this);
  m_txtEmail->setStyleSheet(inputStyle);
  form1->addRow("Email:", m_txtEmail);

  m_txtAddress = new QLineEdit(this);
  m_txtAddress->setStyleSheet(inputStyle);
  form1->addRow("Địa chỉ:", m_txtAddress);

  mainLayout->addWidget(gbPersonalInfo);

  QGroupBox *gbMedicalInfo = new QGroupBox("Thông tin y tế", container);
  gbMedicalInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form2 = new QFormLayout(gbMedicalInfo);
  form2->setContentsMargins(15, 20, 15, 15);
  form2->setSpacing(10);
  form2->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

  m_cboBloodType = new QComboBox(this);
  m_cboBloodType->addItems(
      {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"});
  m_cboBloodType->setStyleSheet(inputStyle);
  m_cboBloodType->setCurrentText("O-");
  form2->addRow("Nhóm máu:", m_cboBloodType);

  m_cboPatientType = new QComboBox(this);
  m_cboPatientType->addItems({"Ngoại trú", "Nội trú", "Cấp cứu"});
  m_cboPatientType->setStyleSheet(inputStyle);
  form2->addRow("Loại bệnh nhân:", m_cboPatientType);

  mainLayout->addWidget(gbMedicalInfo);

  QGroupBox *gbEmergency = new QGroupBox("Người liên hệ khẩn cấp", container);
  gbEmergency->setStyleSheet(groupBoxStyle);
  QFormLayout *form3 = new QFormLayout(gbEmergency);
  form3->setContentsMargins(15, 20, 15, 15);
  form3->setSpacing(10);
  form3->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

  m_txtEmergencyName = new QLineEdit(this);
  m_txtEmergencyName->setStyleSheet(inputStyle);
  form3->addRow("Họ tên:", m_txtEmergencyName);

  m_txtEmergencyPhone = new QLineEdit(this);
  m_txtEmergencyPhone->setStyleSheet(inputStyle);
  form3->addRow("Số điện thoại:", m_txtEmergencyPhone);

  mainLayout->addWidget(gbEmergency);

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  m_btnCancel = new QPushButton("Hủy", container);
  m_btnCancel->setStyleSheet(
      "background-color: #EAEAEA; color: #333; padding: 8px 20px; "
      "border-radius: 4px; font-weight: bold; border: none;");
  m_btnCancel->setCursor(Qt::PointingHandCursor);
  connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
  btnLayout->addWidget(m_btnCancel);

  m_btnSave = new QPushButton("Lưu thông tin", container);
  m_btnSave->setStyleSheet(
      "background-color: #34A853; color: white; padding: 8px 20px; "
      "border-radius: 4px; font-weight: bold; border: none;");
  m_btnSave->setCursor(Qt::PointingHandCursor);
  connect(m_btnSave, &QPushButton::clicked, this, [this]() {
    emit saved(m_txtCitizenId->text().trimmed(), m_txtFullName->text().trimmed());
    accept();
  });
  btnLayout->addWidget(m_btnSave);

  mainLayout->addLayout(btnLayout);
  wrapperLayout->addWidget(container);
}
