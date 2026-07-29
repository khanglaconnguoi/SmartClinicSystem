// #include "Profile.h"
// #include <QApplication>
// #include <QVBoxLayout>
// #include <QHBoxLayout>
// #include <QGridLayout>
// #include <QFormLayout>
// #include <QGraphicsDropShadowEffect>
// #include <QMessageBox>
// #include <QDate>
// #include <QFrame>
// #include <QScrollArea>

// ProfileWidget::ProfileWidget(std::shared_ptr<StaffService> staffService, QWidget *parent) 
//     : QDialog(parent), m_staffService(staffService) {
//     setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
//     setMinimumSize(1100, 700);

//     setStyleSheet(
//         "ProfileWidget { background-color: #F4F6FA; }"
//         "QWidget { font-family: 'Segoe UI', Arial, sans-serif; color: #172B4D; }"
//         "QMessageBox { background-color: #FFFFFF; }"
//         "QMessageBox QLabel { color: #172B4D; font-size: 16px; }"
//         "QMessageBox QPushButton { background-color: #0052CC; color: white; font-weight: bold; min-width: 90px; min-height: 32px; border-radius: 4px; border: none; }"
//         "QMessageBox QPushButton:hover { background-color: #0043A4; }"
//         "QLineEdit { border: none; background: transparent; padding: 0px; }" 
//         "QLineEdit:!read-only { border: 1px solid #0052CC; border-radius: 4px; background-color: #FFFFFF; padding: 2px 6px; min-height: 26px; }" 
//     );
//     showMaximized();

//     QVBoxLayout *mainLayout = new QVBoxLayout(this);
//     mainLayout->setContentsMargins(0, 0, 0, 0);
//     mainLayout->setSpacing(0);

//     QHBoxLayout *contentLayout = new QHBoxLayout();
//     contentLayout->setContentsMargins(40, 30, 40, 30);
//     contentLayout->setSpacing(35);

//     QWidget *leftPanel = createLeftPanel();
//     leftPanel->setFixedWidth(380);
    
//     QWidget *rightPanel = createRightPanel();
//     rightPanel->setMaximumWidth(800);

//     contentLayout->addStretch();
//     contentLayout->addWidget(leftPanel);
//     contentLayout->addWidget(rightPanel);
//     contentLayout->addStretch();

//     mainLayout->addLayout(contentLayout, 1);
// }

// QWidget* ProfileWidget::createLeftPanel() {
//     QWidget *panel = new QWidget(this);
//     QVBoxLayout *layout = new QVBoxLayout(panel);
//     layout->setContentsMargins(0, 0, 0, 0);
//     layout->setSpacing(25);

//     QWidget *idCard = new QWidget(panel);
//     idCard->setStyleSheet("background-color: #FFFFFF; border-radius: 10px; border: 1px solid #E1E4E8;");
//     QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect(idCard);
//     shadow1->setBlurRadius(12); shadow1->setColor(QColor(0, 0, 0, 12)); shadow1->setOffset(0, 3);
//     idCard->setGraphicsEffect(shadow1);

//     QVBoxLayout *idLayout = new QVBoxLayout(idCard);
//     idLayout->setContentsMargins(30, 35, 30, 30);
//     idLayout->setSpacing(15);

//     lblAvatar = new QLabel(idCard);
//     lblAvatar->setFixedSize(150, 150);
//     lblAvatar->setStyleSheet("background-color: #DEEBFF; border-radius: 75px; color: #0052CC; font-size: 65px; border: none;");
//     lblAvatar->setAlignment(Qt::AlignCenter);

//     lblName = new QLabel("", idCard);
//     lblName->setStyleSheet("font-size: 22px; font-weight: bold; color: #172B4D; border: none;");
//     lblName->setAlignment(Qt::AlignCenter);
//     lblName->setWordWrap(true);

//     lblRole = new QLabel("", idCard);
//     lblRole->setStyleSheet("font-size: 16px; color: #5E6C84; border: none;");
//     lblRole->setAlignment(Qt::AlignCenter);

//     lblStaffCode = new QLabel("", idCard);
//     lblStaffCode->setStyleSheet("font-size: 14px; color: #5E6C84; border: none;");
//     lblStaffCode->setAlignment(Qt::AlignCenter);

//     lblStatus = new QLabel("", idCard);
//     lblStatus->setStyleSheet("font-size: 14px; color: #5E6C84; border: none;");
//     lblStatus->setAlignment(Qt::AlignCenter);

//     QFrame *line1 = new QFrame(idCard);
//     line1->setFrameShape(QFrame::HLine);
//     line1->setStyleSheet("background-color: #E1E4E8; border: none; max-height: 1px;");

//     idLayout->addWidget(lblAvatar, 0, Qt::AlignCenter);
//     idLayout->addSpacing(5);
//     idLayout->addWidget(lblName);
//     idLayout->addWidget(lblStatus);
//     idLayout->addSpacing(10);
//     idLayout->addWidget(line1);
//     idLayout->addSpacing(10);

//     QFormLayout *leftForm = new QFormLayout();
//     leftForm->setHorizontalSpacing(20);
//     leftForm->setVerticalSpacing(12);

//     auto addLeftFormRow = [](QFormLayout *form, const QString &text, QWidget *edit) {
//         QLabel *l = new QLabel( text);
//         l->setStyleSheet("font-size: 15px; color: #172B4D; border: none; font-weight: 600;");
//         l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//         l->setFixedWidth(125);
        
//         if (QLabel *lbl = qobject_cast<QLabel*>(edit)) {
//             lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
//             edit->setStyleSheet("font-size: 15px; color: #172B4D; border: none; background: transparent; padding: 0px;");
//         } else if (QLineEdit *le = qobject_cast<QLineEdit*>(edit)) {
//             le->setAlignment(Qt::AlignLeft);
//             edit->setStyleSheet("font-size: 15px; color: #172B4D; border: none; background: transparent; padding: 0px; margin-left: -1px;");
//         }
        
//         edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
//         form->addRow(l, edit);
//     };

//     txtGender = new QLineEdit(idCard);
//     txtPhone = new QLineEdit(idCard);
//     txtEmail = new QLineEdit(idCard);

//     txtGender->setReadOnly(true);
//     txtPhone->setReadOnly(true);
//     txtEmail->setReadOnly(true);

//     addLeftFormRow(leftForm, "Mã nhân viên", lblStaffCode);
//     addLeftFormRow(leftForm, "Chức vụ", lblRole);      
//     addLeftFormRow(leftForm, "Giới tính", txtGender);
//     addLeftFormRow(leftForm, "Số điện thoại", txtPhone);
//     addLeftFormRow(leftForm, "Email", txtEmail);

//     idLayout->addLayout(leftForm);
//     idLayout->addSpacing(15);

//     QWidget *shiftCard = new QWidget(panel);
//     shiftCard->setStyleSheet("background-color: #FFFFFF; border-radius: 10px; border: 1px solid #E1E4E8;");
//     QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect(shiftCard);
//     shadow2->setBlurRadius(12); shadow2->setColor(QColor(0, 0, 0, 12)); shadow2->setOffset(0, 3);
//     shiftCard->setGraphicsEffect(shadow2);

//     QVBoxLayout *shiftLayout = new QVBoxLayout(shiftCard);
//     shiftLayout->setContentsMargins(25, 25, 25, 25);
//     shiftLayout->setSpacing(15);

//     QLabel *lblShiftTitle = new QLabel("Ca trực hiện tại", shiftCard);
//     lblShiftTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #172B4D; border: none;");

//     cmbShift = new QComboBox(shiftCard);
//     cmbShift->addItem("Ca Sáng", "MORNING");
//     cmbShift->addItem("Ca Chiều", "AFTERNOON");
//     cmbShift->addItem("Ca Tối", "NIGHT");
//     cmbShift->addItem("Cả Ngày", "FULL_DAY");
//     cmbShift->setEnabled(false);
//     cmbShift->setFixedHeight(40);
//     cmbShift->setStyleSheet(
//         "QComboBox { font-size: 16px; color: #172B4D; border: 1px solid #DFE1E6; border-radius: 6px; background: #F4F5F7; padding: 5px 12px;  } "
//         "QComboBox::drop-down { border: none; width: 25px; } "
        
//     );

//     shiftLayout->addWidget(lblShiftTitle);
//     shiftLayout->addWidget(cmbShift);
//     btnEdit = new QPushButton("Chỉnh Sửa", panel);
//     btnEdit->setFixedHeight(45);
//     btnEdit->setFixedWidth(120);
//     btnEdit->setStyleSheet(
//         "QPushButton { background-color: #1D54C4; color: white; font-weight: bold; border-radius: 6px; border: none; font-size: 16px; }"
//         "QPushButton:hover { background-color: #154096; }"
//     );

//     layout->addWidget(idCard);
//     layout->addWidget(shiftCard);
//     layout->addWidget(btnEdit, 0, Qt::AlignCenter);
//     layout->addStretch();

//     QString leftStyle = 
//         "QLineEdit { "
//         "   font-size: 15px; "
//         "   color: #172B4D; "
//         "   border: none; "
//         "   background: transparent; "
//         "   padding: 0px; "
//         "   margin-left: -1px; "
//         "} "
//         "QLineEdit:!read-only { "
//         "   border: 1px solid #0052CC; "
//         "   border-radius: 4px; "
//         "   background-color: #FFFFFF; "
//         "   padding: 2px 6px; "
//         "   min-height: 26px; "
//         "   margin-left: 0px; "
//         "}";
//     txtPhone->setStyleSheet(leftStyle);
//     txtEmail->setStyleSheet(leftStyle);

//     connect(btnEdit, &QPushButton::clicked, this, &ProfileWidget::onEditClicked);
//     connect(txtPhone, &QLineEdit::editingFinished, this, &ProfileWidget::validatePhoneNumber);
//     connect(txtEmail, &QLineEdit::editingFinished, this, &ProfileWidget::validateEmail);
//     return panel;
// }

// QWidget* ProfileWidget::createRightPanel() {
//     QScrollArea *scrollArea = new QScrollArea(this);
//     scrollArea->setWidgetResizable(true);
//     scrollArea->setFrameShape(QFrame::NoFrame);
//     scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");

//     QWidget *scrollContent = new QWidget(scrollArea);
//     scrollContent->setStyleSheet("background-color: transparent;");
//     QVBoxLayout *layout = new QVBoxLayout(scrollContent);
//     layout->setContentsMargins(0, 0, 15, 0);
//     layout->setSpacing(25);

//     auto createCard = [this](const QString& titleText, QVBoxLayout*& contentLayoutOut) -> QWidget* {
//         QWidget *card = new QWidget(this);
//         card->setStyleSheet("background-color: #FFFFFF; border-radius: 10px; border: 1px solid #E1E4E8;");
//         QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
//         shadow->setBlurRadius(12); shadow->setColor(QColor(0, 0, 0, 12)); shadow->setOffset(0, 3);
//         card->setGraphicsEffect(shadow);

//         QVBoxLayout *cardLayout = new QVBoxLayout(card);
//         cardLayout->setContentsMargins(0, 0, 0, 0);
//         cardLayout->setSpacing(0);

//         QWidget *headerWidget = new QWidget(card);
//         headerWidget->setStyleSheet("border: none; border-bottom: 1px solid #E1E4E8; border-bottom-left-radius: 0px; border-bottom-right-radius: 0px;");
//         QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
//         headerLayout->setContentsMargins(30, 18, 30, 18);
//         QLabel *lblTitle = new QLabel(titleText, headerWidget);
//         lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #172B4D; border: none;");
//         headerLayout->addWidget(lblTitle);
        
//         QWidget *contentWidget = new QWidget(card);
//         contentWidget->setStyleSheet("border: none;");
//         QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
//         contentLayout->setContentsMargins(30, 25, 30, 25);
        
//         cardLayout->addWidget(headerWidget);
//         cardLayout->addWidget(contentWidget);
        
//         contentLayoutOut = contentLayout;
//         return card;
//     };

//     auto addFormRow = [](QFormLayout *form, const QString &text, QWidget *edit) {
//         QLabel *l = new QLabel( text);
//         l->setStyleSheet("font-size: 16px; color: #172B4D; border: none; font-weight: bold;");
//         edit->setStyleSheet("font-size: 16px; color: #172B4D; border: none; background: transparent; padding: 0px;");
//         form->addRow(l, edit);
//     };

//     QVBoxLayout *layout1;
//     QWidget *card1 = createCard("Thông tin cá nhân", layout1);
//     QFormLayout *form1 = new QFormLayout();
//     form1->setHorizontalSpacing(80);
//     form1->setVerticalSpacing(18);
//     form1->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

//     txtFullName = new QLineEdit(card1);
//     txtFullName->setMinimumWidth(520);
//     txtFullName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//     txtDob = new QLineEdit(card1);
//     txtDob->setMinimumWidth(520);
//     txtDob->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//     txtCitizenId = new QLineEdit(card1);
//     txtCitizenId->setMinimumWidth(520);
//     txtCitizenId->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//     txtAddress = new QLineEdit(card1);
//     txtAddress->setMinimumWidth(520);
//     txtAddress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//     addFormRow(form1, "Họ và tên", txtFullName);
//     addFormRow(form1, "Ngày sinh", txtDob);
//     addFormRow(form1, "Số CCCD", txtCitizenId);
//     addFormRow(form1, "Địa chỉ", txtAddress);
//     layout1->addLayout(form1);
//     layout->addWidget(card1);

//     QVBoxLayout *layout2;
//     QWidget *card2 = createCard("Chi tiết công việc", layout2);
//     QFormLayout *form2 = new QFormLayout();
//     form2->setHorizontalSpacing(80);
//     form2->setVerticalSpacing(18);

//     lblDepartment = new QLabel(card2);
//     lblHireDate = new QLabel(card2);

//     addFormRow(form2, "Phòng ban", lblDepartment);
//     addFormRow(form2, "Ngày vào làm", lblHireDate);
//     layout2->addLayout(form2);
//     layout->addWidget(card2);

//     QVBoxLayout *layout3;
//     cardRoleSpecific = createCard("Chi tiết chuyên môn", layout3);

//     widgetDoctorFields = new QWidget(cardRoleSpecific);
//     widgetDoctorFields->setStyleSheet("border: none;");
//     QVBoxLayout *vboxDoc = new QVBoxLayout(widgetDoctorFields);
//     vboxDoc->setContentsMargins(0, 0, 0, 0);
//     QFormLayout *formDoc = new QFormLayout();
//     formDoc->setHorizontalSpacing(80);
//     formDoc->setVerticalSpacing(18);

//     txtSpecialty = new QLineEdit(widgetDoctorFields);
//     txtSpecialty->setMinimumWidth(520);
//     txtSpecialty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//     txtLicenseNumber = new QLineEdit(widgetDoctorFields);
//     txtLicenseNumber->setMinimumWidth(520);
//     txtLicenseNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//     txtConsultationFee = new QLineEdit(widgetDoctorFields);
//     txtConsultationFee->setMinimumWidth(520);
//     txtConsultationFee->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//     addFormRow(formDoc, "Chuyên khoa", txtSpecialty);
//     addFormRow(formDoc, "Số CCHN", txtLicenseNumber);
//     addFormRow(formDoc, "Phí khám", txtConsultationFee);
//     formDoc->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
//     vboxDoc->addLayout(formDoc);
//     layout3->addWidget(widgetDoctorFields);

//     widgetNurseFields = new QWidget(cardRoleSpecific);
//     widgetNurseFields->setStyleSheet("border: none;");
//     QVBoxLayout *vboxNurse = new QVBoxLayout(widgetNurseFields);
//     vboxNurse->setContentsMargins(0, 0, 0, 0);
//     QFormLayout *formNurse = new QFormLayout();
//     formNurse->setHorizontalSpacing(50);
//     formNurse->setVerticalSpacing(18);

//     txtNurseLevel = new QLineEdit(widgetNurseFields);
//     txtCertification = new QLineEdit(widgetNurseFields);

//     addFormRow(formNurse, "Cấp độ y tá", txtNurseLevel);
//     addFormRow(formNurse, "Chứng chỉ", txtCertification);
//     vboxNurse->addLayout(formNurse);
//     layout3->addWidget(widgetNurseFields);
//     layout->addWidget(cardRoleSpecific);

//     QVBoxLayout *layoutBio;
//     cardBio = createCard("Tiểu sử", layoutBio);
//     cardBio->setMinimumHeight(150); 

//     txtBio = new QTextEdit(cardBio); 
//     txtBio->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//     txtBio->setAcceptRichText(false); 
//     txtBio->setStyleSheet(
//         "QTextEdit { "
//         "   font-size: 16px; "
//         "   color: #172B4D; "
//         "   border: none; "
//         "   background: transparent; "
//         "   padding: 5px 0px; "
//         "   line-height: 1.4; "
//         "} "
//         "QTextEdit:!read-only { "
//         "   border: 1px solid #0052CC; "
//         "   border-radius: 6px; "
//         "   background: #FFFFFF; "
//         "   padding: 8px; "
//         "}"
//     );
//     layoutBio->addWidget(txtBio);
//     layout->addWidget(cardBio);

//     QList<QLineEdit*> allFields = {
//         txtFullName, txtDob, txtCitizenId, txtAddress,
//         txtSpecialty, txtLicenseNumber, txtConsultationFee,
//         txtNurseLevel, txtCertification
//     };
//     txtBio->setReadOnly(true);
//     for (QLineEdit* field : allFields) {
//         field->setReadOnly(true);
//     }

//     QString rightStyle = 
//         "QLineEdit { "
//         "   font-size: 16px; "
//         "   color: #172B4D; "
//         "   border: none; "
//         "   background: transparent; "
//         "   padding: 0px; "
//         "} "
//         "QLineEdit:!read-only { "
//         "   border: 1px solid #0052CC; "
//         "   border-radius: 4px; "
//         "   background-color: #FFFFFF; "
//         "   padding: 2px 6px; "
//         "   min-height: 28px; "
//         "}";
//     txtAddress->setStyleSheet(rightStyle);
//     txtNurseLevel->setStyleSheet(rightStyle);
//     txtCertification->setStyleSheet(rightStyle);

//     cardRoleSpecific->hide();
//     cardBio->hide();

//     scrollArea->setWidget(scrollContent);
//     return scrollArea;
// }

// void ProfileWidget::loadProfile(int staffId) {
//     auto profile = m_staffService->getOwnProfile(staffId);
//     if (!profile) {
//         return;
//     }
    
//     QPixmap avatarPixmap = profile->avatar.scaled(
//             lblAvatar->size() / 1.75,
//             Qt::KeepAspectRatioByExpanding,
//             Qt::SmoothTransformation
//         );
//     lblAvatar->setPixmap(avatarPixmap);
//     lblAvatar->setAlignment(Qt::AlignCenter);
//     lblName->setText(profile->fullName);
//     lblStatus->setText(profile->isActive ? "Đang làm việc" : "Đã nghỉ việc");
//     txtFullName->setText(profile->fullName);
//     txtPhone->setText(profile->phoneNumber);
//     txtEmail->setText(profile->email);
//     txtAddress->setText(profile->address);
//     txtDob->setText(profile->dateOfBirth.toString("dd/MM/yyyy"));
//     txtCitizenId->setText(profile->citizenId);
//     lblDepartment->setText(profile->departmentName);
//     currentDepartmentId = profile->departmentId;
//     lblStaffCode->setText(profile->staffCode);
//     lblHireDate -> setText(profile->hireDate.toString("dd/MM/yyyy"));
    
//     txtGender->setText(profile->gender == Gender::Male ? "Nam" : "Nữ");

//     if (auto docProfile = dynamic_cast<DoctorProfileDTO*>(profile.get())) {
//         lblRole->setText("Bác sĩ");
//         widgetDoctorFields->show();
//         widgetNurseFields->hide();
//         cardRoleSpecific->show();
//         cardBio->show(); 
        
//         lblRole->setText(roleToString(profile->role) == "DOCTOR" ? "Bác sĩ" : roleToString(profile->role));
//         txtSpecialty->setText(docProfile->specialty);
//         txtLicenseNumber->setText(docProfile->licenseNumber);
//         txtConsultationFee->setText(QString::number(docProfile->consultationFee));
//         txtBio->setPlainText(docProfile->bio);
//         currentExperienceYears = docProfile->experienceYears;
//     } 
//     else if (auto nurseProfile = dynamic_cast<NurseProfileDTO*>(profile.get())) {
//         lblRole->setText("Y tá");
//         widgetNurseFields->show();
//         widgetDoctorFields->hide();
//         cardRoleSpecific->show();
//         cardBio->hide(); 
        
//         lblRole->setText(roleToString(profile->role) == "NURSE" ? "Y tá" : roleToString(profile->role));
//         txtNurseLevel->setText(nurseProfile->nurseLevel);
//         txtCertification->setText(nurseProfile->certification);
//     }
//     else {
//         lblRole->setText("Nhân viên");
//         cardRoleSpecific->hide();
//         cardBio->hide(); 
//     }
//     this->currentStaffId = staffId;
// }

// void ProfileWidget::onEditClicked() {
//     if (btnEdit->text() == "Chỉnh Sửa" || btnEdit->text() == "Chỉnh sửa") {
//         txtPhone->setReadOnly(false);
//         txtEmail->setReadOnly(false);
//         txtAddress->setReadOnly(false);
//         cmbShift->setEnabled(false);
        
//         if (widgetDoctorFields->isVisible()) {
//             txtBio->setReadOnly(false); 
//         }
        
//         if (widgetNurseFields->isVisible()) {
//             txtNurseLevel->setReadOnly(false);
//             txtCertification->setReadOnly(false);
//         }

//         btnEdit->setText("Lưu");
//         return;
//     }

//     std::unique_ptr<StaffInputDTO> inputDTO;
    
//     if (widgetDoctorFields->isVisible()) {
//         auto docDTO = std::make_unique<DoctorInputDTO>();
//         docDTO->specialty = txtSpecialty->text();
//         docDTO->licenseNumber = txtLicenseNumber->text();
//         docDTO->consultationFee = txtConsultationFee->text().toDouble();
//         docDTO->experienceYears = currentExperienceYears; 
//         docDTO->bio = txtBio->toPlainText(); 
//         inputDTO = std::move(docDTO);
//     } else if (widgetNurseFields->isVisible()) {
//         auto nurseDTO = std::make_unique<NurseInputDTO>();
//         nurseDTO->nurseLevel = txtNurseLevel->text();
//         nurseDTO->certification = txtCertification->text();
//         inputDTO = std::move(nurseDTO);
//     } else {
//         inputDTO = std::make_unique<StaffInputDTO>();
//     }

//     inputDTO->fullName = txtFullName->text();
//     inputDTO->phoneNumber = txtPhone->text();
//     inputDTO->email = txtEmail->text();
//     inputDTO->address = txtAddress->text();
//     inputDTO->citizenId = txtCitizenId->text();
//     inputDTO->dateOfBirth = QDate::fromString(txtDob->text(), "dd/MM/yyyy");
//     inputDTO->gender = GenderText::toEn(txtGender->text());
//     inputDTO->departmentId = currentDepartmentId; 
//     inputDTO->shift = cmbShift->currentData().toString();

//     QString errorMsg;
//     if (auto docInput = dynamic_cast<DoctorInputDTO*>(inputDTO.get())) {
//        errorMsg = m_staffService->editDoctorInformation(DoctorUpdateDTO(*docInput, currentStaffId));
//     } else if (auto nurseInput = dynamic_cast<NurseInputDTO*>(inputDTO.get())) {
//         errorMsg = m_staffService->editNurseInformation(NurseUpdateDTO(*nurseInput, currentStaffId));
//     } else {
//         errorMsg = m_staffService->editStaffBaseInformation(StaffUpdateDTO(*inputDTO, currentStaffId));
//     }

//     bool success = errorMsg.isEmpty();

//     if (success) {
//         QMessageBox::information(this, "Thành công", "Đã cập nhật hồ sơ!");
        
//         txtFullName->setReadOnly(true);
//         txtPhone->setReadOnly(true);
//         txtEmail->setReadOnly(true);
//         txtAddress->setReadOnly(true);
//         txtDob->setReadOnly(true);
//         txtCitizenId->setReadOnly(true);
//         txtGender->setReadOnly(true);
//         txtSpecialty->setReadOnly(true);
//         txtLicenseNumber->setReadOnly(true);
//         txtConsultationFee->setReadOnly(true);
//         txtBio->setReadOnly(true);
//         txtNurseLevel->setReadOnly(true);
//         txtCertification->setReadOnly(true);
//         cmbShift->setEnabled(false);
//         btnEdit->setText("Chỉnh Sửa");

//        txtPhone->setStyleSheet(
//             "QLineEdit { "
//             "   font-size: 15px; "
//             "   color: #172B4D; "
//             "   border: none; "
//             "   background: transparent; "
//             "   padding: 0px; "
//             "   margin-left: -1px; "
//             "} "
//             "QLineEdit:!read-only { "
//             "   border: 1px solid #0052CC; "
//             "   border-radius: 4px; "
//             "   background-color: #FFFFFF; "
//             "   padding: 2px 6px; "
//             "   min-height: 26px; "
//             "   margin-left: 0px; "
//             "}"
//         );
//         txtPhone->setStyleSheet(
//             "QLineEdit { "
//             "   font-size: 15px; "
//             "   color: #172B4D; "
//             "   border: none; "
//             "   background: transparent; "
//             "   padding: 0px; "
//             "   margin-left: -1px; "
//             "} "
//             "QLineEdit:!read-only { "
//             "   border: 1px solid #0052CC; "
//             "   border-radius: 4px; "
//             "   background-color: #FFFFFF; "
//             "   padding: 2px 6px; "
//             "   min-height: 26px; "
//             "   margin-left: 0px; "
//             "}"
//         );
// txtEmail->setStyleSheet(
//             "QLineEdit { "
//             "   font-size: 15px; "
//             "   color: #172B4D; "
//             "   border: none; "
//             "   background: transparent; "
//             "   padding: 0px; "
//             "   margin-left: -1px; "
//             "} "
//             "QLineEdit:!read-only { "
//             "   border: 1px solid #0052CC; "
//             "   border-radius: 4px; "
//             "   background-color: #FFFFFF; "
//             "   padding: 2px 6px; "
//             "   min-height: 26px; "
//             "   margin-left: 0px; "
//             "}"
//         );

//         loadProfile(currentStaffId);
//     } else {
//         QMessageBox::critical(this, "Lỗi", errorMsg);
//     }
// }

// void ProfileWidget::validatePhoneNumber() {
//     if (txtPhone->isReadOnly()) return;
    
//     QString erro = Validation::validatePhoneNumber(txtPhone->text());
    
//     if (!erro.isEmpty()) { 
//         txtPhone->setStyleSheet(
//             "QLineEdit { border: 1px solid #FF3B30; border-radius: 4px; background-color: #FFE5E5; padding: 2px 6px; min-height: 26px; }"
//         );
//         QMessageBox::warning(this, "Định dạng sai", erro);
//     }
//     else {
//         txtPhone->setStyleSheet(
//             "QLineEdit { "
//             "   font-size: 15px; "
//             "   color: #172B4D; "
//             "   border: none; "
//             "   background: transparent; "
//             "   padding: 0px; "
//             "   margin-left: -1px; "
//             "} "
//             "QLineEdit:!read-only { "
//             "   border: 1px solid #0052CC; "
//             "   border-radius: 4px; "
//             "   background-color: #FFFFFF; "
//             "   padding: 2px 6px; "
//             "   min-height: 26px; "
//             "   margin-left: 0px; "
//             "}"
//         );
//     }
// }

// void ProfileWidget::validateEmail() {
//     if (txtEmail->isReadOnly()) return;
    
//     QString erro = Validation::validateEmail(txtEmail->text());
    
//     if (!erro.isEmpty()) { 
//         txtEmail->setStyleSheet(
//             "QLineEdit { border: 1px solid #FF3B30; border-radius: 4px; background-color: #FFE5E5; padding: 2px 6px; min-height: 26px; }"
//         );
//         QMessageBox::warning(this, "Định dạng sai", erro);
//     }
//     else {
//         txtEmail->setStyleSheet(
//             "QLineEdit { "
//             "   font-size: 15px; "
//             "   color: #172B4D; "
//             "   border: none; "
//             "   background: transparent; "
//             "   padding: 0px; "
//             "   margin-left: -1px; "
//             "} "
//             "QLineEdit:!read-only { "
//             "   border: 1px solid #0052CC; "
//             "   border-radius: 4px; "
//             "   background-color: #FFFFFF; "
//             "   padding: 2px 6px; "
//             "   min-height: 26px; "
//             "   margin-left: 0px; "
//             "}"
//         );
//     }
// }

