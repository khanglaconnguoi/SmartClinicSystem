#include "AdminPatientRegistrationDialog.h"
#include "../../dto/PatientDTOs.h"
#include "../../model/CommonEnums.h"
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

AdminPatientRegistrationDialog::AdminPatientRegistrationDialog(
    std::shared_ptr<PatientService> patientService, QWidget *parent)
    : QDialog(parent), m_patientService(patientService) {
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  setWindowTitle("Thêm Bệnh nhân");
  setMinimumWidth(650);
  resize(650, 800);
  setupUi();
}

void AdminPatientRegistrationDialog::setupUi() {
  this->setAttribute(Qt::WA_TranslucentBackground);
  this->setStyleSheet("QDialog { background-color: transparent; } "
                      "QLabel { color: #333333; }");

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(20, 20, 20, 20);

  QFrame *container = new QFrame(this);
  container->setObjectName("mainContainer");
  container->setStyleSheet(
      "#mainContainer { background-color: #FFFFFF; border-radius: 12px; }");

  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 60));
  shadow->setOffset(0, 5);
  container->setGraphicsEffect(shadow);

  QVBoxLayout *containerLayout = new QVBoxLayout(container);
  containerLayout->setContentsMargins(0, 0, 0, 0);
  containerLayout->setSpacing(0);

  QFrame *headerFrame = new QFrame(container);
  headerFrame->setStyleSheet(
      "background-color: transparent; border-bottom: 1px solid #EAEAEA;");
  QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
  headerLayout->setContentsMargins(24, 20, 24, 20);

  QLabel *lblPageTitle = new QLabel("ĐĂNG KÝ BỆNH NHÂN", headerFrame);
  lblPageTitle->setStyleSheet(
      "font-size: 20px; font-weight: bold; color: #111827;");
  lblPageTitle->setAlignment(Qt::AlignCenter);
  headerLayout->addWidget(lblPageTitle);

  containerLayout->addWidget(headerFrame);

  QScrollArea *scrollArea = new QScrollArea(container);
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet(
      "QScrollArea { background-color: transparent; border: none; }");

  QFrame *formCard = new QFrame(scrollArea);
  formCard->setStyleSheet("QFrame { background-color: #FFFFFF; border: none; } "
                          "QLabel { color: #333333; font-weight: bold; }");

  QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
  cardLayout->setContentsMargins(24, 24, 24, 24);
  cardLayout->setSpacing(20);

  QString groupBoxStyle =
      "QGroupBox { "
      "border: 1px solid #D0D0D0; border-radius: 6px; "
      "margin-top: 15px; font-weight: bold; color: #333333; background-color: "
      "transparent; } "
      "QGroupBox::title { "
      "subcontrol-origin: margin; subcontrol-position: top left; "
      "padding: 0 5px; color: #1A73E8; }";

  QString extraInputStyle =
      "QLineEdit, QComboBox, QDateEdit { "
      "border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; "
      "font-size: 13px; color: #111827; background: #FFFFFF; min-height: 30px; "
      "} "
      "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { "
      "border: 1px solid #4B94F2; } "
      "QComboBox QAbstractItemView { "
      "background-color: #FFFFFF; color: #111827; selection-background-color: #4B94F2; selection-color: white; }";

  // --- Loại Bệnh nhân ---
  m_cbPatientType = new QComboBox(formCard);
  m_cbPatientType->addItems({"Ngoại trú", "Nội trú", "Cấp cứu"});
  m_cbPatientType->setStyleSheet(extraInputStyle);
  
  QHBoxLayout* typeLayout = new QHBoxLayout();
  QLabel* lblType = new QLabel("Loại Bệnh nhân:");
  typeLayout->addWidget(lblType);
  typeLayout->addWidget(m_cbPatientType, 1);
  cardLayout->addLayout(typeLayout);

  // --- Group 1: Thông tin Cá nhân ---
  QGroupBox *gbPersonalInfo = new QGroupBox("Thông tin Cơ bản", formCard);
  gbPersonalInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form1 = new QFormLayout(gbPersonalInfo);
  form1->setContentsMargins(15, 25, 15, 15);
  form1->setSpacing(12);

  m_txtFullName = new QLineEdit(gbPersonalInfo);
  m_txtFullName->setStyleSheet(extraInputStyle);
  form1->addRow("Họ và Tên (*):", m_txtFullName);

  m_txtCitizenId = new QLineEdit(gbPersonalInfo);
  m_txtCitizenId->setStyleSheet(extraInputStyle);
  form1->addRow("CCCD (*):", m_txtCitizenId);

  m_cbGender = new QComboBox(gbPersonalInfo);
  m_cbGender->addItems({"Nam", "Nữ", "Khác"});
  m_cbGender->setStyleSheet(extraInputStyle);
  form1->addRow("Giới tính:", m_cbGender);

  m_dtDateOfBirth = new QDateEdit(QDate::currentDate().addYears(-30), gbPersonalInfo);
  m_dtDateOfBirth->setCalendarPopup(true);
  m_dtDateOfBirth->setDisplayFormat("dd/MM/yyyy");
  m_dtDateOfBirth->setStyleSheet(extraInputStyle);
  form1->addRow("Ngày sinh:", m_dtDateOfBirth);

  cardLayout->addWidget(gbPersonalInfo);

  // --- Group 2: Liên hệ ---
  QGroupBox *gbContactInfo = new QGroupBox("Thông tin Liên hệ", formCard);
  gbContactInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form2 = new QFormLayout(gbContactInfo);
  form2->setContentsMargins(15, 25, 15, 15);
  form2->setSpacing(12);

  m_txtPhone = new QLineEdit(gbContactInfo);
  m_txtPhone->setStyleSheet(extraInputStyle);
  form2->addRow("Số điện thoại (*):", m_txtPhone);

  m_txtEmail = new QLineEdit(gbContactInfo);
  m_txtEmail->setStyleSheet(extraInputStyle);
  form2->addRow("Email:", m_txtEmail);

  m_txtAddress = new QLineEdit(gbContactInfo);
  m_txtAddress->setStyleSheet(extraInputStyle);
  form2->addRow("Địa chỉ:", m_txtAddress);

  m_txtEmergencyContactName = new QLineEdit(gbContactInfo);
  m_txtEmergencyContactName->setStyleSheet(extraInputStyle);
  form2->addRow("Tên người nhà:", m_txtEmergencyContactName);

  m_txtEmergencyContactPhone = new QLineEdit(gbContactInfo);
  m_txtEmergencyContactPhone->setStyleSheet(extraInputStyle);
  form2->addRow("SĐT người nhà:", m_txtEmergencyContactPhone);

  cardLayout->addWidget(gbContactInfo);

  // --- Group 3: Thông tin Y tế cơ bản ---
  QGroupBox *gbMedicalInfo = new QGroupBox("Thông tin Y tế", formCard);
  gbMedicalInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form3 = new QFormLayout(gbMedicalInfo);
  form3->setContentsMargins(15, 25, 15, 15);
  form3->setSpacing(12);

  m_cbBloodType = new QComboBox(gbMedicalInfo);
  m_cbBloodType->addItems({"Chưa rõ", "A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"});
  m_cbBloodType->setStyleSheet(extraInputStyle);
  form3->addRow("Nhóm máu:", m_cbBloodType);

  m_txtAllergies = new QLineEdit(gbMedicalInfo);
  m_txtAllergies->setPlaceholderText("Cách nhau bởi dấu phẩy (vd: Penicillin, Hải sản)");
  m_txtAllergies->setStyleSheet(extraInputStyle);
  form3->addRow("Dị ứng:", m_txtAllergies);

  m_txtInsurance = new QLineEdit(gbMedicalInfo);
  m_txtInsurance->setPlaceholderText("Mã thẻ BHYT");
  m_txtInsurance->setStyleSheet(extraInputStyle);
  form3->addRow("Bảo hiểm:", m_txtInsurance);

  cardLayout->addWidget(gbMedicalInfo);

  scrollArea->setWidget(formCard);
  containerLayout->addWidget(scrollArea);

  // --- Nút Lưu và Trở lại ---
  QFrame *bottomFrame = new QFrame(container);
  bottomFrame->setStyleSheet(
      "background-color: transparent; border-top: 1px solid #EAEAEA;");
  QHBoxLayout *btnLayout = new QHBoxLayout(bottomFrame);
  btnLayout->setContentsMargins(20, 15, 20, 15);

  btnLayout->addStretch();
  m_btnCancel = new QPushButton("Hủy", bottomFrame);
  m_btnCancel->setCursor(Qt::PointingHandCursor);
  m_btnCancel->setFixedSize(100, 40);
  m_btnCancel->setStyleSheet(
      "QPushButton { background-color: #EAEAEA; color: "
      "#333; font-size: 14px; font-weight: bold; "
      "border-radius: 4px; border: none; }"
      "QPushButton:hover { background-color: #D6D6D6; }");
  btnLayout->addWidget(m_btnCancel);

  m_btnSave = new QPushButton("Lưu bệnh nhân", bottomFrame);
  m_btnSave->setCursor(Qt::PointingHandCursor);
  m_btnSave->setFixedSize(140, 40);
  m_btnSave->setStyleSheet(
      "QPushButton { background-color: #34A853; color: white; font-size: 14px; "
      "font-weight: bold; border-radius: 4px; border: none; }"
      "QPushButton:hover { background-color: #2C8E46; }");
  btnLayout->addWidget(m_btnSave);

  containerLayout->addWidget(bottomFrame);
  mainLayout->addWidget(container);

  connect(m_btnSave, &QPushButton::clicked, this,
          &AdminPatientRegistrationDialog::handleSave);
  connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void AdminPatientRegistrationDialog::handleSave() {
  if (!m_patientService) {
    QMessageBox::critical(this, "Lỗi", "Service không khả dụng.");
    return;
  }

  QString fullName = m_txtFullName->text().trimmed();
  QString citizenId = m_txtCitizenId->text().trimmed();
  QString phone = m_txtPhone->text().trimmed();

  if (fullName.isEmpty() || citizenId.isEmpty() || phone.isEmpty()) {
    QMessageBox::warning(this, "Thiếu thông tin",
                         "Vui lòng nhập đầy đủ các trường bắt buộc (*)");
    return;
  }

  QString genderText = m_cbGender->currentText();
  QString gender = (genderText == "Nam") ? "MALE" : (genderText == "Nữ") ? "FEMALE" : "OTHER";

  QDate dob = m_dtDateOfBirth->date();
  QString email = m_txtEmail->text().trimmed();
  QString address = m_txtAddress->text().trimmed();
  
  QString typeText = m_cbPatientType->currentText();
  PatientType type = (typeText == "Nội trú") ? PatientType::INPATIENT : 
                     (typeText == "Cấp cứu") ? PatientType::EMERGENCY : PatientType::OUTPATIENT;

  QString bloodType = m_cbBloodType->currentText();
  if(bloodType == "Chưa rõ") bloodType = "UNKNOWN";

  QString allergies = m_txtAllergies->text().trimmed();
  QString insurance = m_txtInsurance->text().trimmed();
  QString emerName = m_txtEmergencyContactName->text().trimmed();
  QString emerPhone = m_txtEmergencyContactPhone->text().trimmed();

  bool success = false;
  if (type == PatientType::OUTPATIENT) {
      success = m_patientService->AddOutPatient(
          0, -1, fullName, dob, gender, citizenId, phone, email, address,
          bloodType, allergies, insurance, type, emerName, emerPhone
      );
  } else if (type == PatientType::INPATIENT) {
      success = m_patientService->AddInPatient(
          0, fullName, dob, gender, citizenId, phone, email, address,
          bloodType, allergies, insurance, type, emerName, emerPhone,
          "TBD", "-1", QDate::currentDate(), QDate::currentDate().addDays(1), "Chưa xác định"
      );
  } else if (type == PatientType::EMERGENCY) {
      success = m_patientService->AddEmergencyPatient(
          0, fullName, dob, gender, citizenId, phone, email, address,
          bloodType, allergies, insurance, type, emerName, emerPhone,
          "TBD", "-1", "Chưa xác định", "Chưa xác định", QDate::currentDate(), QDate::currentDate().addDays(1)
      );
  }

  if (!success) {
      // QMessageBox::critical(this, "Lỗi", "Không thể thêm bệnh nhân. Vui lòng kiểm tra lại thông tin.");
      // Validation error is already shown in PatientService
      return;
  }

  QMessageBox::information(this, "Thành công", "Đăng ký bệnh nhân thành công!");
  accept();
}
