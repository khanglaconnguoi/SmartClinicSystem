#include "ReceptionRegistrationDialog.h"
#include "../../dto/StaffDTOs.h"
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

ReceptionRegistrationDialog::ReceptionRegistrationDialog(
    std::shared_ptr<StaffService> staffService, QWidget *parent)
    : QDialog(parent), m_staffService(staffService) {
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  setWindowTitle("Thêm Lễ tân");
  setMinimumWidth(650);
  resize(650, 700);
  setupUi();
}

void ReceptionRegistrationDialog::setupUi() {
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

  QLabel *lblPageTitle = new QLabel("THÔNG TIN LỄ TÂN", headerFrame);
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

  // --- Group 1: Thông tin Cá nhân ---
  QGroupBox *gbPersonalInfo = new QGroupBox("Thông tin Cá nhân", formCard);
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

  m_dtDateOfBirth = new QDateEdit(QDate::currentDate().addYears(-25), gbPersonalInfo);
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

  cardLayout->addWidget(gbContactInfo);

  // --- Group 3: Thông tin Công việc ---
  QGroupBox *gbJobInfo = new QGroupBox("Thông tin Công việc", formCard);
  gbJobInfo->setStyleSheet(groupBoxStyle);
  QFormLayout *form3 = new QFormLayout(gbJobInfo);
  form3->setContentsMargins(15, 25, 15, 15);
  form3->setSpacing(12);

  m_cbShift = new QComboBox(gbJobInfo);
  m_cbShift->addItems({"Sáng", "Chiều", "Tối", "Cả ngày"});
  m_cbShift->setStyleSheet(extraInputStyle);
  form3->addRow("Ca làm việc:", m_cbShift);

  cardLayout->addWidget(gbJobInfo);

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
          &ReceptionRegistrationDialog::handleSave);
  connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void ReceptionRegistrationDialog::handleSave() {
  if (!m_staffService) {
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

  Gender gender = (m_cbGender->currentText() == "Nam")  ? Gender::Male
                  : (m_cbGender->currentText() == "Nữ") ? Gender::Female
                                                        : Gender::Other;

  QDate dob = m_dtDateOfBirth->date();
  QString email = m_txtEmail->text().trimmed();
  QString address = m_txtAddress->text().trimmed();
  
  QString shiftText = m_cbShift->currentText();
  QString shift = (shiftText == "Sáng") ? "MORNING" :
                  (shiftText == "Chiều") ? "AFTERNOON" :
                  (shiftText == "Tối") ? "NIGHT" : "FULL_DAY";

  ReceptionistInputDTO dto;
  dto.fullName = fullName;
  dto.gender = gender;
  dto.dateOfBirth = dob;
  dto.citizenId = citizenId;
  dto.phoneNumber = phone;
  dto.email = email;
  dto.address = address;
  dto.shift = shift;

  QString errorMsg = m_staffService->hireNewReceptionist(dto);
  if (!errorMsg.isEmpty()) {
      QMessageBox::critical(this, "Lỗi", errorMsg);
      return;
  }
  
  QMessageBox::information(this, "Thành công", "Tạo tài khoản Lễ tân thành công!");
  accept();
}
