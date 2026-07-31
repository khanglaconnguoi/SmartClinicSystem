#include "PatientRegistrationDialog.h"
#include "../../dto/PatientDTOs.h"
#include "../utils/UIValidationUtils.h"
#include "../../service/Validation.h"
#include "model/CommonEnums.h"
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
#include <QDoubleSpinBox>

PatientRegistrationDialog::PatientRegistrationDialog(
    std::shared_ptr<PatientService> patientService, QWidget *parent)
    : QDialog(parent), m_patientService(patientService)
{
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  setWindowTitle("Thêm Bệnh nhân");
  setMinimumWidth(650);
  resize(650, 800);
  setupUi();
}

void PatientRegistrationDialog::setupUi()
{
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
      "background-color: #FFFFFF; color: #111827; selection-background-color: "
      "#4B94F2; selection-color: white; }";

  //-- -Loại Bệnh nhân-- -
      m_cbPatientType = new QComboBox(formCard);
  for (const auto &item : patientTypeList)
    m_cbPatientType->addItem(item.viText, item.enText);
  m_cbPatientType->setStyleSheet(extraInputStyle);

  QHBoxLayout *typeLayout = new QHBoxLayout();
  QLabel *lblType = new QLabel("Loại Bệnh nhân:");
  typeLayout->addWidget(lblType);
  typeLayout->addWidget(m_cbPatientType, 1);
  cardLayout->addLayout(typeLayout);

  //-- -Group 1 : Thông tin Cá nhân-- -
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
  for (const auto &pair : GenderText::getList())
    m_cbGender->addItem(pair.second, pair.first);
  m_cbGender->setStyleSheet(extraInputStyle);
  form1->addRow("Giới tính:", m_cbGender);

  m_dtDateOfBirth =
      new QDateEdit(QDate::currentDate().addYears(-30), gbPersonalInfo);
  m_dtDateOfBirth->setCalendarPopup(true);
  m_dtDateOfBirth->setDisplayFormat("dd/MM/yyyy");
  m_dtDateOfBirth->setStyleSheet(extraInputStyle);
  form1->addRow("Ngày sinh:", m_dtDateOfBirth);

  cardLayout->addWidget(gbPersonalInfo);

  //-- -Group 2 : Liên hệ-- -
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

  //-- -Group 3 : Thông tin Y tế cơ bản-- -
      QGroupBox *gbMedicalInfo = new QGroupBox("Thông tin Y tế", formCard);
  gbMedicalInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form3 = new QFormLayout(gbMedicalInfo);
  form3->setContentsMargins(15, 25, 15, 15);
  form3->setSpacing(12);

  m_cbBloodType = new QComboBox(gbMedicalInfo);
  for (const auto &pair : BloodTypeText::getList())
    m_cbBloodType->addItem(pair.second, pair.first);
  m_cbBloodType->setStyleSheet(extraInputStyle);
  form3->addRow("Nhóm máu:", m_cbBloodType);

  m_txtAllergies = new QLineEdit(gbMedicalInfo);
  m_txtAllergies->setPlaceholderText(
      "Cách nhau bởi dấu phẩy (vd: Penicillin, Hải sản)");
  m_txtAllergies->setStyleSheet(extraInputStyle);
  form3->addRow("Dị ứng:", m_txtAllergies);

  //-- -Group 4 : Thông tin Bảo hiểm Y tế-- -
      QGroupBox *gbInsuranceInfo = new QGroupBox("Thông tin Bảo hiểm Y tế", formCard);
  gbInsuranceInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form4 = new QFormLayout(gbInsuranceInfo);
  form4->setContentsMargins(15, 25, 15, 15);
  form4->setSpacing(12);

  m_cbInsuranceType = new QComboBox(gbInsuranceInfo);
  m_cbInsuranceType->addItem("Không có", "NONE");
  for (const auto &pair : InsuranceTypeText::getList())
    m_cbInsuranceType->addItem(pair.second, pair.first);
  m_cbInsuranceType->setStyleSheet(extraInputStyle);
  form4->addRow("Loại bảo hiểm:", m_cbInsuranceType);

  m_txtInsuranceProvider = new QLineEdit(gbInsuranceInfo);
  m_txtInsuranceProvider->setPlaceholderText("Tên công ty / Nơi cấp...");
  m_txtInsuranceProvider->setStyleSheet(extraInputStyle);
  form4->addRow("Nơi cấp:", m_txtInsuranceProvider);

  m_txtInsurancePolicy = new QLineEdit(gbInsuranceInfo);
  m_txtInsurancePolicy->setPlaceholderText("Mã thẻ / Hợp đồng...");
  m_txtInsurancePolicy->setStyleSheet(extraInputStyle);
  form4->addRow("Mã thẻ:", m_txtInsurancePolicy);

  m_spinInsuranceCoverage = new QDoubleSpinBox(gbInsuranceInfo);
  m_spinInsuranceCoverage->setRange(0.0, 100.0);
  m_spinInsuranceCoverage->setValue(80.0);
  m_spinInsuranceCoverage->setSuffix(" %");
  m_spinInsuranceCoverage->setStyleSheet(extraInputStyle);
  form4->addRow("Mức chi trả:", m_spinInsuranceCoverage);

  m_dateInsuranceFrom = new QDateEdit(QDate::currentDate(), gbInsuranceInfo);
  m_dateInsuranceFrom->setCalendarPopup(true);
  m_dateInsuranceFrom->setDisplayFormat("dd/MM/yyyy");
  m_dateInsuranceFrom->setStyleSheet(extraInputStyle);
  form4->addRow("Hiệu lực từ:", m_dateInsuranceFrom);

  m_dateInsuranceTo = new QDateEdit(QDate::currentDate().addYears(1), gbInsuranceInfo);
  m_dateInsuranceTo->setCalendarPopup(true);
  m_dateInsuranceTo->setDisplayFormat("dd/MM/yyyy");
  m_dateInsuranceTo->setStyleSheet(extraInputStyle);
  form4->addRow("Hiệu lực đến:", m_dateInsuranceTo);

  cardLayout->addWidget(gbMedicalInfo);
  cardLayout->addWidget(gbInsuranceInfo);

  scrollArea->setWidget(formCard);
  containerLayout->addWidget(scrollArea);

  //-- -Nút Lưu và Trở lại-- -
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
          &PatientRegistrationDialog::handleSave);
  connect(m_btnCancel, &QPushButton::clicked, this, [this]()
          {
    emit cancelled();
    reject(); });

  // --- UI Validation (Step 1 & 2) ---
  UIValidationUtils::attachPrimitiveValidators(m_txtCitizenId, m_txtPhone);
  if (m_txtEmergencyContactPhone) {
      m_txtEmergencyContactPhone->setValidator(new QRegularExpressionValidator(QRegularExpression("^0\\d{0,10}$"), m_txtEmergencyContactPhone));
  }

  connect(m_txtPhone, &QLineEdit::editingFinished, this, [this]() {
      QString err = Validation::validatePhoneNumber(m_txtPhone->text());
      UIValidationUtils::applyFieldValidationStyle(m_txtPhone, err);
  });
  connect(m_txtEmergencyContactPhone, &QLineEdit::editingFinished, this, [this]() {
      QString text = m_txtEmergencyContactPhone->text().trimmed();
      QString err = text.isEmpty() ? "" : Validation::validatePhoneNumber(text);
      UIValidationUtils::applyFieldValidationStyle(m_txtEmergencyContactPhone, err);
  });
  connect(m_txtCitizenId, &QLineEdit::editingFinished, this, [this]() {
      QString err = Validation::validateCitizenId(m_txtCitizenId->text());
      UIValidationUtils::applyFieldValidationStyle(m_txtCitizenId, err);
  });
  connect(m_txtEmail, &QLineEdit::editingFinished, this, [this]() {
      QString text = m_txtEmail->text().trimmed();
      QString err = text.isEmpty() ? "" : Validation::validateEmail(text);
      UIValidationUtils::applyFieldValidationStyle(m_txtEmail, err);
  });
  connect(m_txtFullName, &QLineEdit::editingFinished, this, [this]() {
      QString err = Validation::validateFullName(m_txtFullName->text());
      UIValidationUtils::applyFieldValidationStyle(m_txtFullName, err);
  });
}

void PatientRegistrationDialog::handleSave()
{
  if (!m_patientService)
  {
    QMessageBox::critical(this, "Lỗi", "Service không khả dụng.");
    return;
  }

  QString fullName = m_txtFullName->text().trimmed();
  QString citizenId = m_txtCitizenId->text().trimmed();
  QString phone = m_txtPhone->text().trimmed();

  if (fullName.isEmpty() || citizenId.isEmpty() || phone.isEmpty())
  {
    QMessageBox::warning(this, "Thiếu thông tin",
                         "Vui lòng nhập đầy đủ các trường bắt buộc (*)");
    return;
  }

  QString genderText = m_cbGender->currentText();
  QString gender = GenderText::toEn(genderText);

  QDate dob = m_dtDateOfBirth->date();
  QString email = m_txtEmail->text().trimmed();
  QString address = m_txtAddress->text().trimmed();

  PatientType type = patientTypeFromVi(m_cbPatientType->currentText());

  QString bloodType = BloodTypeText::toEn(m_cbBloodType->currentText());

  QString allergies = m_txtAllergies->text().trimmed();

  QString insuranceTypeStr = m_cbInsuranceType->currentText();
  bool hasInsurance = (insuranceTypeStr != "Không có");
  QString insuranceTypeEn = InsuranceTypeText::toEn(insuranceTypeStr);
  QString insuranceProvider = m_txtInsuranceProvider->text().trimmed();
  QString insurancePolicy = m_txtInsurancePolicy->text().trimmed();
  double insuranceCoverage = m_spinInsuranceCoverage->value();
  QDate insuranceFrom = m_dateInsuranceFrom->date();
  QDate insuranceTo = m_dateInsuranceTo->date();

  QString emerName = m_txtEmergencyContactName->text().trimmed();
  QString emerPhone = m_txtEmergencyContactPhone->text().trimmed();

  QString errorMsg;
  if (type == PatientType::Outpatient)
  {
    OutPatientInputDTO dto;
    dto.fullName = fullName;
    dto.dateOfBirth = dob;
    dto.gender = gender;
    dto.citizenId = citizenId;
    dto.phone = phone;
    dto.email = email;
    dto.address = address;
    dto.bloodType = bloodType;
    if (hasInsurance)
    {
      InsuranceInputDTO insDto;
      insDto.policyNumber = insurancePolicy;
      insDto.providerName = insuranceProvider;
      insDto.insuranceType = insuranceTypeEn;
      insDto.coveragePercent = insuranceCoverage;
      insDto.validFrom = insuranceFrom;
      insDto.validTo = insuranceTo;
      dto.insurance = insDto;
    }
    dto.type = type;
    dto.emergencyContactName = emerName;
    dto.emergencyContactPhone = emerPhone;

    errorMsg = m_patientService->addOutPatient(dto);
  }
  else if (type == PatientType::Inpatient)
  {
    InPatientInputDTO dto;
    dto.fullName = fullName;
    dto.dateOfBirth = dob;
    dto.gender = gender;
    dto.citizenId = citizenId;
    dto.phone = phone;
    dto.email = email;
    dto.address = address;
    dto.bloodType = bloodType;
    if (hasInsurance)
    {
      InsuranceInputDTO insDto;
      insDto.policyNumber = insurancePolicy;
      insDto.providerName = insuranceProvider;
      insDto.insuranceType = insuranceTypeEn;
      insDto.coveragePercent = insuranceCoverage;
      insDto.validFrom = insuranceFrom;
      insDto.validTo = insuranceTo;
      dto.insurance = insDto;
    }
    dto.type = type;
    dto.emergencyContactName = emerName;
    dto.emergencyContactPhone = emerPhone;
    dto.admissionDate = QDate::currentDate();
    dto.dischargeDate = std::nullopt;
    dto.reason = "Chưa xác định";

    errorMsg = m_patientService->addInPatient(dto);
  }
  else if (type == PatientType::Emergency)
  {
    EmergencyPatientInputDTO dto;
    dto.fullName = fullName;
    dto.dateOfBirth = dob;
    dto.gender = gender;
    dto.citizenId = citizenId;
    dto.phone = phone;
    dto.email = email;
    dto.address = address;
    dto.bloodType = bloodType;
    if (hasInsurance)
    {
      InsuranceInputDTO insDto;
      insDto.policyNumber = insurancePolicy;
      insDto.providerName = insuranceProvider;
      insDto.insuranceType = insuranceTypeEn;
      insDto.coveragePercent = insuranceCoverage;
      insDto.validFrom = insuranceFrom;
      insDto.validTo = insuranceTo;
      dto.insurance = insDto;
    }
    dto.type = type;
    dto.emergencyContactName = emerName;
    dto.emergencyContactPhone = emerPhone;
    dto.injuryCause = "Chưa xác định";
    dto.injuryDescription = "Chưa xác định";
    dto.admissionDate = QDate::currentDate();
    dto.dischargeDate = std::nullopt;

    errorMsg = m_patientService->addEmergencyPatient(dto);
  }

  if (!errorMsg.isEmpty())
  {
    QMessageBox::warning(this, "Lỗi kiểm tra dữ liệu", errorMsg);
    return;
  }

  QMessageBox::information(this, "Thành công", "Đăng ký bệnh nhân thành công!");
  emit saved(phone, fullName);
  accept();
}
