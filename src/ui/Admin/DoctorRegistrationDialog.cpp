// #include "DoctorRegistrationDialog.h"
// #include "../../dto/StaffDTOs.h"
// #include "../../model/Doctor.h"
// #include <QComboBox>
// #include <QDate>
// #include <QDateEdit>
// #include <QDebug>
// #include <QFormLayout>
// #include <QGraphicsDropShadowEffect>
// #include <QGroupBox>
// #include <QHBoxLayout>
// #include <QLabel>
// #include <QLineEdit>
// #include <QMessageBox>
// #include <QPushButton>
// #include <QScrollArea>
// #include <QSpinBox>
// #include <QTextEdit>
// #include <QVBoxLayout>

// DoctorRegistrationDialog::DoctorRegistrationDialog(
//     std::shared_ptr<StaffService> staffService, QWidget *parent)
//     : QDialog(parent), m_staffService(staffService), m_editStaffId(-1) {
//   setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
//   setWindowTitle("Thêm Bác sĩ");
//   setMinimumWidth(650);
//   resize(650, 850);
//   setupUi();
// }

// void DoctorRegistrationDialog::setupUi() {
//   this->setAttribute(Qt::WA_TranslucentBackground);
//   this->setStyleSheet("QDialog { background-color: transparent; } "
//                       "QLabel { color: #333333; }");

//   QVBoxLayout *mainLayout = new QVBoxLayout(this);
//   mainLayout->setContentsMargins(20, 20, 20, 20);

//   QFrame *container = new QFrame(this);
//   container->setObjectName("mainContainer");
//   container->setStyleSheet(
//       "#mainContainer { background-color: #FFFFFF; border-radius: 12px; }");

//   QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
//   shadow->setBlurRadius(20);
//   shadow->setColor(QColor(0, 0, 0, 60));
//   shadow->setOffset(0, 5);
//   container->setGraphicsEffect(shadow);

//   QVBoxLayout *containerLayout = new QVBoxLayout(container);
//   containerLayout->setContentsMargins(0, 0, 0, 0);
//   containerLayout->setSpacing(0);

//   QFrame *headerFrame = new QFrame(container);
//   headerFrame->setStyleSheet(
//       "background-color: transparent; border-bottom: 1px solid #EAEAEA;");
//   QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
//   headerLayout->setContentsMargins(24, 20, 24, 20);

//   QLabel *lblPageTitle = new QLabel("THÔNG TIN BÁC SĨ", headerFrame);
//   lblPageTitle->setStyleSheet(
//       "font-size: 20px; font-weight: bold; color: #111827;");
//   lblPageTitle->setAlignment(Qt::AlignCenter);
//   headerLayout->addWidget(lblPageTitle);

//   containerLayout->addWidget(headerFrame);

//   QScrollArea *scrollArea = new QScrollArea(container);
//   scrollArea->setWidgetResizable(true);
//   scrollArea->setFrameShape(QFrame::NoFrame);
//   scrollArea->setStyleSheet(
//       "QScrollArea { background-color: transparent; border: none; }");

//   QFrame *formCard = new QFrame(scrollArea);
//   formCard->setStyleSheet("QFrame { background-color: #FFFFFF; border: none; } "
//                           "QLabel { color: #333333; font-weight: bold; }");

//   QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
//   cardLayout->setContentsMargins(24, 24, 24, 24);
//   cardLayout->setSpacing(20);

//   QString groupBoxStyle =
//       "QGroupBox { "
//       "border: 1px solid #D0D0D0; border-radius: 6px; "
//       "margin-top: 15px; font-weight: bold; color: #333333; background-color: "
//       "transparent; } "
//       "QGroupBox::title { "
//       "subcontrol-origin: margin; subcontrol-position: top left; "
//       "padding: 0 5px; color: #1A73E8; }";

//   QString extraInputStyle =
//       "QLineEdit, QComboBox, QDateEdit, QSpinBox, QTextEdit { "
//       "border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; "
//       "font-size: 13px; color: #111827; background: #FFFFFF; min-height: 30px; "
//       "} "
//       "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, "
//       "QTextEdit:focus { "
//       "border: 1px solid #4B94F2; } "
//       "QComboBox QAbstractItemView { "
//       "background-color: #FFFFFF; color: #111827; selection-background-color: "
//       "#4B94F2; selection-color: white; }";

//   // --- Nhóm 1: Thông tin cá nhân ---
//   QGroupBox *gbPersonalInfo = new QGroupBox("Thông tin cá nhân", formCard);
//   gbPersonalInfo->setStyleSheet(groupBoxStyle);
//   QFormLayout *form1 = new QFormLayout(gbPersonalInfo);
//   form1->setContentsMargins(15, 20, 15, 15);
//   form1->setSpacing(10);
//   form1->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

//   m_txtFullName = new QLineEdit(gbPersonalInfo);
//   m_txtFullName->setStyleSheet(extraInputStyle);
//   m_txtFullName->setPlaceholderText("VD: Nguyễn Văn A");
//   form1->addRow("Họ và Tên (*):", m_txtFullName);

//   m_dtDateOfBirth = new QDateEdit(QDate(1990, 1, 1), gbPersonalInfo);
//   m_dtDateOfBirth->setCalendarPopup(true);
//   m_dtDateOfBirth->setStyleSheet(extraInputStyle);
//   form1->addRow("Ngày sinh (*):", m_dtDateOfBirth);

//   m_cbGender = new QComboBox(gbPersonalInfo);
//   m_cbGender->setStyleSheet(extraInputStyle);
//   m_cbGender->addItems({"Nam", "Nữ", "Khác"});
//   form1->addRow("Giới tính (*):", m_cbGender);

//   m_txtCitizenId = new QLineEdit(gbPersonalInfo);
//   m_txtCitizenId->setStyleSheet(extraInputStyle);
//   m_txtCitizenId->setPlaceholderText("Nhập 12 chữ số hợp lệ");
//   form1->addRow("Số CMND / CCCD (*):", m_txtCitizenId);

//   m_txtPhone = new QLineEdit(gbPersonalInfo);
//   m_txtPhone->setStyleSheet(extraInputStyle);
//   m_txtPhone->setPlaceholderText("VD: 0901234567");
//   form1->addRow("Số điện thoại (*):", m_txtPhone);

//   m_txtEmail = new QLineEdit(gbPersonalInfo);
//   m_txtEmail->setStyleSheet(extraInputStyle);
//   m_txtEmail->setPlaceholderText("VD: doctor@example.com");
//   form1->addRow("Email:", m_txtEmail);

//   m_txtAddress = new QLineEdit(gbPersonalInfo);
//   m_txtAddress->setStyleSheet(extraInputStyle);
//   m_txtAddress->setPlaceholderText("VD: TP Hồ Chí Minh");
//   form1->addRow("Địa chỉ:", m_txtAddress);

//   cardLayout->addWidget(gbPersonalInfo);

//   // --- Nhóm 2: Thông tin công việc & Tài khoản ---
//   QGroupBox *gbWorkInfo =
//       new QGroupBox("Thông tin công việc & Tài khoản", formCard);
//   gbWorkInfo->setStyleSheet(groupBoxStyle);
//   QFormLayout *form2 = new QFormLayout(gbWorkInfo);
//   form2->setContentsMargins(15, 20, 15, 15);
//   form2->setSpacing(10);
//   form2->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

//   m_cbDepartment = new QComboBox(gbWorkInfo);
//   m_cbDepartment->setStyleSheet(extraInputStyle);
//   m_cbDepartment->addItems({"1 - Khoa Khám Bệnh", "2 - Khoa Nội",
//                             "3 - Khoa Ngoại", "4 - Khoa Sản", "5 - Khoa Nhi"});
//   form2->addRow("Phòng ban:", m_cbDepartment);

//   m_cbShift = new QComboBox(gbWorkInfo);
//   m_cbShift->setStyleSheet(extraInputStyle);
//   m_cbShift->addItems({"Sáng", "Chiều", "Tối"});
//   form2->addRow("Ca làm việc:", m_cbShift);

//   m_dtHireDate = new QDateEdit(QDate::currentDate(), gbWorkInfo);
//   m_dtHireDate->setCalendarPopup(true);
//   m_dtHireDate->setStyleSheet(extraInputStyle);
//   form2->addRow("Ngày vào làm:", m_dtHireDate);

//   cardLayout->addWidget(gbWorkInfo);

//   // --- Nhóm 3: Thông tin chuyên môn ---
//   QGroupBox *gbMedicalInfo = new QGroupBox("Thông tin chuyên môn", formCard);
//   gbMedicalInfo->setStyleSheet(groupBoxStyle);
//   QFormLayout *form3 = new QFormLayout(gbMedicalInfo);
//   form3->setContentsMargins(15, 20, 15, 15);
//   form3->setSpacing(10);
//   form3->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

//   m_cbSpecialty = new QComboBox(gbMedicalInfo);
//   m_cbSpecialty->setStyleSheet(extraInputStyle);
//   m_cbSpecialty->addItems(
//       {"Nội khoa", "Ngoại khoa", "Nhi khoa", "Da liễu", "Răng Hàm Mặt"});
//   form3->addRow("Chuyên khoa:", m_cbSpecialty);

//   m_txtLicenseNumber = new QLineEdit(gbMedicalInfo);
//   m_txtLicenseNumber->setStyleSheet(extraInputStyle);
//   form3->addRow("Số CCHN:", m_txtLicenseNumber);

//   m_sbExperienceYears = new QSpinBox(gbMedicalInfo);
//   m_sbExperienceYears->setStyleSheet(extraInputStyle);
//   m_sbExperienceYears->setRange(0, 50);
//   form3->addRow("Số năm kinh nghiệm:", m_sbExperienceYears);

//   m_sbConsultationFee = new QSpinBox(gbMedicalInfo);
//   m_sbConsultationFee->setStyleSheet(extraInputStyle);
//   m_sbConsultationFee->setRange(0, 10000000);
//   m_sbConsultationFee->setSingleStep(50000);
//   m_sbConsultationFee->setValue(150000);
//   form3->addRow("Phí khám (VNĐ):", m_sbConsultationFee);

//   m_txtBio = new QTextEdit(gbMedicalInfo);
//   m_txtBio->setStyleSheet(extraInputStyle);
//   m_txtBio->setFixedHeight(60);
//   form3->addRow("Tiểu sử / Giới thiệu:", m_txtBio);

//   cardLayout->addWidget(gbMedicalInfo);

//   scrollArea->setWidget(formCard);
//   containerLayout->addWidget(scrollArea);

//   // --- Nút Lưu và Trở lại ---
//   QFrame *bottomFrame = new QFrame(container);
//   bottomFrame->setStyleSheet(
//       "background-color: transparent; border-top: 1px solid #EAEAEA;");
//   QHBoxLayout *btnLayout = new QHBoxLayout(bottomFrame);
//   btnLayout->setContentsMargins(20, 15, 20, 15);

//   btnLayout->addStretch();
//   m_btnCancel = new QPushButton("Hủy", bottomFrame);
//   m_btnCancel->setCursor(Qt::PointingHandCursor);
//   m_btnCancel->setFixedSize(100, 40);
//   m_btnCancel->setStyleSheet(
//       "QPushButton { background-color: #EAEAEA; color: "
//       "#333; font-size: 14px; font-weight: bold; "
//       "border-radius: 4px; border: none; }"
//       "QPushButton:hover { background-color: #D6D6D6; }");
//   btnLayout->addWidget(m_btnCancel);

//   m_btnSave = new QPushButton("Lưu thông tin", bottomFrame);
//   m_btnSave->setCursor(Qt::PointingHandCursor);
//   m_btnSave->setFixedSize(140, 40);
//   m_btnSave->setStyleSheet(
//       "QPushButton { background-color: #34A853; color: white; font-size: 14px; "
//       "font-weight: bold; border-radius: 4px; border: none; }"
//       "QPushButton:hover { background-color: #2C8E46; }");
//   btnLayout->addWidget(m_btnSave);

//   containerLayout->addWidget(bottomFrame);
//   mainLayout->addWidget(container);

//   connect(m_btnSave, &QPushButton::clicked, this,
//           &DoctorRegistrationDialog::handleSave);
//   connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
// }

// void DoctorRegistrationDialog::loadDoctorData(DoctorProfileDTO *doctor) {
//   if (!doctor)
//     return;
//   m_editStaffId = doctor->staffId;

//   m_txtFullName->setText(doctor->fullName);
//   m_txtCitizenId->setText(doctor->citizenId);
//   m_txtPhone->setText(doctor->phoneNumber);
//   m_txtEmail->setText(doctor->email);
//   m_txtAddress->setText(doctor->address);

//   QString genderText = GenderText::toVi(doctor->gender);
//   m_cbGender->setCurrentText(genderText);

//   m_dtDateOfBirth->setDate(doctor->dateOfBirth);

//   m_dtHireDate->setDate(doctor->hireDate);
//   m_dtHireDate->setReadOnly(true); // cannot edit hire date

//   for (int i = 0; i < m_cbDepartment->count(); ++i) {
//     if (m_cbDepartment->itemText(i).startsWith(
//             QString::number(doctor->departmentId) + " -")) {
//       m_cbDepartment->setCurrentIndex(i);
//       break;
//     }
//   }

//   QString shiftText = (doctor->shift == "MORNING")     ? "Sáng"
//                       : (doctor->shift == "AFTERNOON") ? "Chiều"
//                       : (doctor->shift == "NIGHT")     ? "Tối"
//                                                        : "Cả ngày";
//   m_cbShift->setCurrentText(shiftText);

//   m_cbSpecialty->setCurrentText(doctor->specialty);
//   m_txtLicenseNumber->setText(doctor->licenseNumber);
//   m_sbExperienceYears->setValue(doctor->experienceYears);
//   m_sbConsultationFee->setValue(doctor->consultationFee);
//   m_txtBio->setText(doctor->bio);
// }

// void DoctorRegistrationDialog::handleSave() {
//   if (!m_staffService) {
//     QMessageBox::critical(this, "Lỗi", "Service không khả dụng.");
//     return;
//   }

//   QString fullName = m_txtFullName->text().trimmed();
//   QString citizenId = m_txtCitizenId->text().trimmed();
//   QString phone = m_txtPhone->text().trimmed();
//   QString email = m_txtEmail->text().trimmed();
//   QString address = m_txtAddress->text().trimmed();

//   if (fullName.isEmpty() || citizenId.isEmpty() || phone.isEmpty()) {
//     QMessageBox::warning(this, "Thiếu thông tin",
//                          "Vui lòng nhập đầy đủ các trường bắt buộc (*)");
//     return;
//   }

//   QString gender = GenderText::toEn(m_cbGender->currentText());

//   QDate dob = m_dtDateOfBirth->date();

//   QString shiftText = m_cbShift->currentText();
//   QString shift = (shiftText == "Sáng")    ? "MORNING"
//                   : (shiftText == "Chiều") ? "AFTERNOON"
//                   : (shiftText == "Tối")   ? "NIGHT"
//                                            : "FULL_DAY";
//   QString specialty = m_cbSpecialty->currentText();
//   int departmentId = m_cbDepartment->currentText().split(" - ").first().toInt();

//   QString licenseNumber = m_txtLicenseNumber->text().trimmed();
//   int experienceYears = m_sbExperienceYears->value();
//   int consultationFee = m_sbConsultationFee->value();
//   QString bio = m_txtBio->toPlainText().trimmed();

//   QPixmap avatar;

//   if (m_editStaffId == -1) {
//     DoctorInputDTO dto;
//     dto.fullName = fullName;
//     dto.avatar = avatar;
//     dto.gender = gender;
//     dto.dateOfBirth = dob;
//     dto.citizenId = citizenId;
//     dto.phoneNumber = phone;
//     dto.email = email;
//     dto.address = address;
//     dto.departmentId = departmentId;
//     dto.shift = shift;
//     dto.specialty = specialty;
//     dto.licenseNumber = licenseNumber;
//     dto.experienceYears = experienceYears;
//     dto.consultationFee = consultationFee;
//     dto.bio = bio;

//     StaffHireResult result = m_staffService->hireNewDoctor(dto);
//     if (!result.errorMessage.isEmpty()) {
//       QMessageBox::critical(this, "Lỗi", result.errorMessage);
//       return;
//     }
//     QMessageBox::information(this, "Thành công",
//                              QString("Tạo tài khoản Bác sĩ thành công!\nMã nhân viên: %1\nMật khẩu: %2")
//                              .arg(result.staffCode)
//                              .arg(result.plainPassword));
//   } else {
//     DoctorUpdateDTO updateDto;
//     updateDto.staffId = m_editStaffId;
//     updateDto.fullName = fullName;
//     updateDto.gender = gender;
//     updateDto.dateOfBirth = dob.toString("yyyy-MM-dd");
//     updateDto.citizenId = citizenId;
//     updateDto.phoneNumber = phone;
//     updateDto.email = email;
//     updateDto.address = address;
//     updateDto.departmentId = departmentId;
//     updateDto.shift = shift;
//     updateDto.specialty = specialty;
//     updateDto.licenseNumber = licenseNumber;
//     updateDto.experienceYears = experienceYears;
//     updateDto.consultationFee = consultationFee;
//     updateDto.bio = bio;

//     QString errorMsg = m_staffService->editDoctorInformation(updateDto);
//     if (!errorMsg.isEmpty()) {
//       QMessageBox::critical(this, "Lỗi", errorMsg);
//       return;
//     }
//     QMessageBox::information(this, "Thành công",
//                              "Cập nhật thông tin Bác sĩ thành công!");
//   }

//   accept();
// }
