#include "NurseRegistrationDialog.h"
#include "../../dto/StaffDTOs.h"
#include "../utils/UIValidationUtils.h"
#include "../../service/Validation.h"
#include "../../model/SystemUser.h"
#include "../../model/CommonEnums.h"
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDebug>
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

NurseRegistrationDialog::NurseRegistrationDialog(
    std::shared_ptr<StaffService> staffService, QWidget *parent)
    : QDialog(parent), m_staffService(staffService), m_editStaffId(-1) {
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  setWindowTitle("Thêm Y tá");
  setMinimumWidth(650);
  resize(650, 750);
  setupUi();
}

void NurseRegistrationDialog::setupUi() {
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

  QLabel *lblPageTitle = new QLabel("THÔNG TIN Y TÁ", headerFrame);
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

  //--- Group 1: Thông tin Cá nhân ---
  QGroupBox *gbPersonalInfo = new QGroupBox("Thông tin Cá nhân", formCard);
  gbPersonalInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form1 = new QFormLayout(gbPersonalInfo);
  form1->setContentsMargins(15, 25, 15, 15);
  form1->setSpacing(12);

  m_avatarPicker = new AvatarPickerWidget(gbPersonalInfo);
  form1->addRow("Ảnh đại diện:", m_avatarPicker);

  m_txtFullName = new QLineEdit(gbPersonalInfo);
  m_txtFullName->setStyleSheet(extraInputStyle);
  form1->addRow("Họ và Tên (*):", m_txtFullName);

  m_txtCitizenId = new QLineEdit(gbPersonalInfo);
  m_txtCitizenId->setStyleSheet(extraInputStyle);
  form1->addRow("CCCD (*):", m_txtCitizenId);

  m_cbGender = new QComboBox(gbPersonalInfo);
  for (const auto& pair : GenderText::getList()) m_cbGender->addItem(pair.second, pair.first);
  m_cbGender->setStyleSheet(extraInputStyle);
  form1->addRow("Giới tính:", m_cbGender);

  m_dtDateOfBirth = new QDateEdit(QDate::currentDate().addYears(-25), gbPersonalInfo);
  m_dtDateOfBirth->setCalendarPopup(true);
  m_dtDateOfBirth->setDisplayFormat("dd/MM/yyyy");
  m_dtDateOfBirth->setStyleSheet(extraInputStyle);
  form1->addRow("Ngày sinh:", m_dtDateOfBirth);

  cardLayout->addWidget(gbPersonalInfo);

  //--- Group 2: Liên hệ ---
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

  cardLayout->addWidget(gbContactInfo);

  //--- Group 3: Thông tin Công việc ---
  QGroupBox *gbJobInfo = new QGroupBox("Thông tin Công việc", formCard);
  gbJobInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form3 = new QFormLayout(gbJobInfo);
  form3->setContentsMargins(15, 25, 15, 15);
  form3->setSpacing(12);

  m_cbDepartment = new QComboBox(gbJobInfo);
  for (const auto& pair : DepartmentText::getList()) m_cbDepartment->addItem(pair.second, pair.first);
  m_cbDepartment->setStyleSheet(extraInputStyle);
  form3->addRow("Phòng ban:", m_cbDepartment);

  m_dtHireDate = new QDateEdit(QDate::currentDate(), gbJobInfo);
  m_dtHireDate->setCalendarPopup(true);
  m_dtHireDate->setDisplayFormat("dd/MM/yyyy");
  m_dtHireDate->setStyleSheet(extraInputStyle);
  form3->addRow("Ngày vào làm:", m_dtHireDate);

  m_cbShift = new QComboBox(gbJobInfo);
  for (const auto& pair : ShiftText::getList()) m_cbShift->addItem(pair.second, pair.first);
  m_cbShift->setStyleSheet(extraInputStyle);
  form3->addRow("Ca làm việc:", m_cbShift);

  m_cbNurseLevel = new QComboBox(gbJobInfo);
  m_cbNurseLevel->addItems({"JUNIOR", "SENIOR", "HEAD"});
  m_cbNurseLevel->setStyleSheet(extraInputStyle);
  form3->addRow("Cấp bậc Y tá:", m_cbNurseLevel);

  m_txtCertification = new QLineEdit(gbJobInfo);
  m_txtCertification->setStyleSheet(extraInputStyle);
  form3->addRow("Chứng chỉ:", m_txtCertification);

  cardLayout->addWidget(gbJobInfo);

  scrollArea->setWidget(formCard);
  containerLayout->addWidget(scrollArea);

  //--- Nút Lưu và Trở lại ---
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

  m_btnSave = new QPushButton("Lưu thông tin", bottomFrame);
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
          &NurseRegistrationDialog::handleSave);
  connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);

  // --- UI Validation (Step 1 & 2) ---
  UIValidationUtils::attachPrimitiveValidators(m_txtCitizenId, m_txtPhone);

  connect(m_txtPhone, &QLineEdit::editingFinished, this, [this]() {
      QString err = Validation::validatePhoneNumber(m_txtPhone->text());
      UIValidationUtils::applyFieldValidationStyle(m_txtPhone, err);
  });
  connect(m_txtCitizenId, &QLineEdit::editingFinished, this, [this]() {
      QString err = Validation::validateCitizenId(m_txtCitizenId->text());
      UIValidationUtils::applyFieldValidationStyle(m_txtCitizenId, err);
  });
  connect(m_txtEmail, &QLineEdit::editingFinished, this, [this]() {
      QString err = Validation::validateEmail(m_txtEmail->text());
      UIValidationUtils::applyFieldValidationStyle(m_txtEmail, err);
  });
  connect(m_txtFullName, &QLineEdit::editingFinished, this, [this]() {
      QString err = Validation::validateFullName(m_txtFullName->text());
      UIValidationUtils::applyFieldValidationStyle(m_txtFullName, err);
  });
}

void NurseRegistrationDialog::loadNurseData(NurseProfileDTO* nurse) {
    if (!nurse) return;
    m_editStaffId = nurse->staffId;

    m_avatarPicker->setAvatarPixmap(nurse->avatar);
    m_txtFullName->setText(nurse->fullName);
    m_txtCitizenId->setText(nurse->citizenId);
    m_txtPhone->setText(nurse->phoneNumber);
    m_txtEmail->setText(nurse->email);
    m_txtAddress->setText(nurse->address);
    
    QString genderText = GenderText::toVi(nurse->gender);
    m_cbGender->setCurrentText(genderText);
    
    m_dtDateOfBirth->setDate(nurse->dateOfBirth);
    
    m_dtHireDate->setDate(nurse->hireDate);
    m_dtHireDate->setReadOnly(true); 
    
    for (int i = 0; i < m_cbDepartment->count(); ++i) {
        if (m_cbDepartment->itemText(i).startsWith(QString::number(nurse->departmentId) + " -")) {
            m_cbDepartment->setCurrentIndex(i);
            break;
        }
    }
    
    m_cbShift->setCurrentText(ShiftText::toVi(nurse->shift));
    
    m_cbNurseLevel->setCurrentText(nurse->nurseLevel);
    m_txtCertification->setText(nurse->certification);
}

void NurseRegistrationDialog::handleSave() {
  if (!m_staffService) {
    QMessageBox::critical(this, "Lỗi", "Service không khả dụng.");
    return;
  }

  QString fullName = m_txtFullName->text().trimmed();
  QString citizenId = m_txtCitizenId->text().trimmed();
  QString phone = m_txtPhone->text().trimmed();
  QString email = m_txtEmail->text().trimmed();
  QString address = m_txtAddress->text().trimmed();

  if (fullName.isEmpty() || citizenId.isEmpty() || phone.isEmpty()) {
    QMessageBox::warning(this, "Thiếu thông tin",
                         "Vui lòng nhập đầy đủ các trường bắt buộc (*)");
    return;
  }

  QString gender = GenderText::toEn(m_cbGender->currentText());

  QDate dob = m_dtDateOfBirth->date();
  
  QString shift = ShiftText::toEn(m_cbShift->currentText());
  int departmentId = m_cbDepartment->currentText().split(" - ").first().toInt();

  QString nurseLevel = m_cbNurseLevel->currentText();
  QString certification = m_txtCertification->text().trimmed();

  QPixmap avatar = m_avatarPicker->getAvatarPixmap();

  NurseInputDTO dto;
  dto.fullName = fullName;
  dto.avatar = avatar;
  dto.gender = gender;
  dto.dateOfBirth = dob;
  dto.citizenId = citizenId;
  dto.phoneNumber = phone;
  dto.email = email;
  dto.address = address;
  dto.departmentId = departmentId;
  dto.shift = shift;
  dto.nurseLevel = nurseLevel;
  dto.certification = certification;

  if (m_editStaffId == -1) {
      StaffHireResult result = m_staffService->hireNewNurse(dto);
      if (!result.errorMessage.isEmpty()) {
          QMessageBox::critical(this, "Lỗi", result.errorMessage);
          return;
      }
      QMessageBox::information(this, "Thành công", 
                               QString("Tạo tài khoản Y tá thành công!\nMã nhân viên: %1\nMật khẩu: %2")
                               .arg(result.staffCode)
                               .arg(result.plainPassword));
  } else {
      QString errorMsg = m_staffService->editNurseInformation(dto, m_editStaffId);
      if (!errorMsg.isEmpty()) {
          QMessageBox::critical(this, "Lỗi", errorMsg);
          return;
      }
      QMessageBox::information(this, "Thành công", "Cập nhật thông tin Y tá thành công!");
  }

  accept();
}
