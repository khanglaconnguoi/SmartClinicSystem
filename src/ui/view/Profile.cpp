#include "Profile.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QDate>
#include <QFrame>
#include <QScrollArea>
#include <QPainter>
#include <QPainterPath>
#include "../utils/UIValidationUtils.h"
#include "../../service/Validation.h"

ProfileWidget::ProfileWidget(std::shared_ptr<StaffService> staffService, QWidget *parent) 
    : QDialog(parent), m_staffService(staffService) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setMinimumSize(1100, 700);

    setStyleSheet(
        "ProfileWidget { background-color: #F8FAFC; }"
        "QWidget { font-family: 'Segoe UI', Arial, sans-serif; color: #0F172A; }"
        "QMessageBox { background-color: #FFFFFF; }"
        "QMessageBox QLabel { color: #0F172A; font-size: 15px; }"
        "QMessageBox QPushButton { background-color: #2563EB; color: white; font-weight: bold; min-width: 90px; min-height: 34px; border-radius: 6px; border: none; font-size: 14px; }"
        "QMessageBox QPushButton:hover { background-color: #1D4ED8; }"
    );
    showMaximized();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Top Bar with Back Button
    QFrame *topBar = new QFrame(this);
    topBar->setStyleSheet("background-color: #FFFFFF; border-bottom: 1px solid #E2E8F0;");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(24, 12, 24, 12);

    QPushButton *btnBack = new QPushButton("Quay về", topBar);
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setStyleSheet(
        "QPushButton { background-color: #EFF6FF; color: #2563EB; font-weight: bold; border: 1px solid #2563EB; border-radius: 6px; padding: 8px 18px; font-size: 13px; }"
        "QPushButton:hover { background-color: #DBEAFE; }"
    );
    connect(btnBack, &QPushButton::clicked, this, &QDialog::reject);

    QLabel *lblTopTitle = new QLabel("THÔNG TIN HỒ SƠ CÁ NHÂN", topBar);
    lblTopTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #1E293B;");

    topBarLayout->addWidget(btnBack);
    topBarLayout->addSpacing(16);
    topBarLayout->addWidget(lblTopTitle);
    topBarLayout->addStretch();

    mainLayout->addWidget(topBar);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(36, 20, 36, 24);
    contentLayout->setSpacing(28);

    QWidget *leftPanel = createLeftPanel();
    leftPanel->setFixedWidth(380);
    
    QWidget *rightPanel = createRightPanel();
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    contentLayout->addWidget(leftPanel, 0, Qt::AlignTop);
    contentLayout->addWidget(rightPanel, 1);

    mainLayout->addLayout(contentLayout, 1);
}

static const char* kProfileFieldStyle = 
    "QLineEdit { "
    "   font-size: 15px; "
    "   color: #1E293B; "
    "   font-weight: 500; "
    "   border: 1px solid transparent; "
    "   background: transparent; "
    "   padding: 5px 10px; "
    "   min-height: 36px; "
    "   border-radius: 6px; "
    "} "
    "QLineEdit:!read-only { "
    "   border: 1.5px solid #3B82F6; "
    "   border-radius: 6px; "
    "   background-color: #FFFFFF; "
    "   color: #0F172A; "
    "   padding: 5px 10px; "
    "   min-height: 36px; "
    "} "
    "QLineEdit:!read-only:focus { "
    "   border: 1.5px solid #1D4ED8; "
    "   background-color: #F8FAFC; "
    "}";

static const char* kProfileBioStyle = 
    "QTextEdit { "
    "   font-size: 15px; "
    "   color: #1E293B; "
    "   font-weight: 500; "
    "   border: 1px solid transparent; "
    "   background: transparent; "
    "   padding: 8px 10px; "
    "   line-height: 1.4; "
    "   border-radius: 6px; "
    "} "
    "QTextEdit:!read-only { "
    "   border: 1.5px solid #3B82F6; "
    "   border-radius: 6px; "
    "   background-color: #FFFFFF; "
    "   color: #0F172A; "
    "   padding: 10px 12px; "
    "} "
    "QTextEdit:!read-only:focus { "
    "   border: 1.5px solid #1D4ED8; "
    "   background-color: #F8FAFC; "
    "}";

QWidget* ProfileWidget::createLeftPanel() {
    QWidget *panel = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    QWidget *idCard = new QWidget(panel);
    idCard->setStyleSheet("background-color: #FFFFFF; border-radius: 12px; border: 1px solid #E2E8F0;");
    QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect(idCard);
    shadow1->setBlurRadius(14); shadow1->setColor(QColor(0, 0, 0, 10)); shadow1->setOffset(0, 3);
    idCard->setGraphicsEffect(shadow1);

    QVBoxLayout *idLayout = new QVBoxLayout(idCard);
    idLayout->setContentsMargins(24, 28, 24, 24);
    idLayout->setSpacing(12);

    lblAvatar = new AvatarPickerWidget(idCard);
    lblAvatar->setFixedSize(140, 140);
    lblAvatar->setEnabled(false);
    lblAvatar->setCursor(Qt::ArrowCursor);

    lblName = new QLabel("", idCard);
    lblName->setStyleSheet("font-size: 20px; font-weight: bold; color: #0F172A; border: none; background: transparent;");
    lblName->setAlignment(Qt::AlignCenter);
    lblName->setWordWrap(true);

    lblRole = new QLabel("", idCard);
    lblRole->setStyleSheet("font-size: 14px; color: #64748B; border: none; background: transparent; font-weight: 500;");
    lblRole->setAlignment(Qt::AlignCenter);

    lblStaffCode = new QLabel("", idCard);
    lblStaffCode->setStyleSheet("font-size: 14px; color: #64748B; border: none; background: transparent;");
    lblStaffCode->setAlignment(Qt::AlignCenter);

    lblStatus = new QLabel("", idCard);
    lblStatus->setStyleSheet("font-size: 13px; color: #059669; font-weight: 600; border: none; background: transparent;");
    lblStatus->setAlignment(Qt::AlignCenter);

    QFrame *line1 = new QFrame(idCard);
    line1->setFrameShape(QFrame::HLine);
    line1->setStyleSheet("background-color: #E2E8F0; border: none; max-height: 1px;");

    idLayout->addWidget(lblAvatar, 0, Qt::AlignCenter);
    idLayout->addSpacing(4);
    idLayout->addWidget(lblName);
    idLayout->addWidget(lblStatus);
    idLayout->addSpacing(6);
    idLayout->addWidget(line1);
    idLayout->addSpacing(8);

    QFormLayout *leftForm = new QFormLayout();
    leftForm->setHorizontalSpacing(16);
    leftForm->setVerticalSpacing(10);

    auto addLeftFormRow = [](QFormLayout *form, const QString &text, QWidget *edit) {
        QLabel *l = new QLabel(text);
        l->setStyleSheet("font-size: 14px; color: #475569; border: none; font-weight: 600; background: transparent;");
        l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        l->setFixedWidth(110);
        
        if (QLabel *lbl = qobject_cast<QLabel*>(edit)) {
            lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            lbl->setStyleSheet("font-size: 14px; color: #1E293B; border: none; background: transparent; font-weight: 500; padding: 5px 10px; min-height: 36px;");
        } else if (QLineEdit *le = qobject_cast<QLineEdit*>(edit)) {
            le->setAlignment(Qt::AlignLeft);
            le->setStyleSheet(kProfileFieldStyle);
        }
        
        edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        form->addRow(l, edit);
    };

    txtGender = new QLineEdit(idCard);
    txtPhone = new QLineEdit(idCard);
    txtEmail = new QLineEdit(idCard);

    txtGender->setReadOnly(true);
    txtPhone->setReadOnly(true);
    txtEmail->setReadOnly(true);

    addLeftFormRow(leftForm, "Mã nhân viên", lblStaffCode);
    addLeftFormRow(leftForm, "Chức vụ", lblRole);      
    addLeftFormRow(leftForm, "Giới tính", txtGender);
    addLeftFormRow(leftForm, "Số điện thoại", txtPhone);
    addLeftFormRow(leftForm, "Email", txtEmail);

    idLayout->addLayout(leftForm);

    QWidget *shiftCard = new QWidget(panel);
    shiftCard->setStyleSheet("background-color: #FFFFFF; border-radius: 12px; border: 1px solid #E2E8F0;");
    QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect(shiftCard);
    shadow2->setBlurRadius(14); shadow2->setColor(QColor(0, 0, 0, 10)); shadow2->setOffset(0, 3);
    shiftCard->setGraphicsEffect(shadow2);

    QVBoxLayout *shiftLayout = new QVBoxLayout(shiftCard);
    shiftLayout->setContentsMargins(24, 20, 24, 20);
    shiftLayout->setSpacing(10);

    QLabel *lblShiftTitle = new QLabel("Ca trực hiện tại", shiftCard);
    lblShiftTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #0F172A; border: none; background: transparent;");

    cmbShift = new QComboBox(shiftCard);
    for (const auto& pair : ShiftText::getList()) cmbShift->addItem(pair.second, pair.first);
    cmbShift->setEnabled(false);
    cmbShift->setFixedHeight(38);
    cmbShift->setStyleSheet(
        "QComboBox { font-size: 14px; color: #1E293B; border: 1px solid #CBD5E1; border-radius: 6px; background: #F8FAFC; padding: 4px 12px; font-weight: 500; } "
        "QComboBox::drop-down { border: none; width: 25px; } "
    );

    shiftLayout->addWidget(lblShiftTitle);
    shiftLayout->addWidget(cmbShift);

    btnEdit = new QPushButton("Chỉnh Sửa", panel);
    btnEdit->setFixedHeight(42);
    btnEdit->setFixedWidth(140);
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setStyleSheet(
        "QPushButton { background-color: #2563EB; color: white; font-weight: bold; border-radius: 8px; border: none; font-size: 14px; }"
        "QPushButton:hover { background-color: #1D4ED8; }"
    );

    layout->addWidget(idCard);
    layout->addWidget(shiftCard);
    layout->addWidget(btnEdit, 0, Qt::AlignCenter);
    layout->addStretch();

    connect(btnEdit, &QPushButton::clicked, this, &ProfileWidget::onEditClicked);
    connect(txtPhone, &QLineEdit::editingFinished, this, &ProfileWidget::validatePhoneNumber);
    connect(txtEmail, &QLineEdit::editingFinished, this, &ProfileWidget::validateEmail);
    return panel;
}

void ProfileWidget::setReadOnlyMode() {
    if (btnEdit) {
        btnEdit->hide();
    }
}

QWidget* ProfileWidget::createRightPanel() {
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: transparent; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 0px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #CBD5E1; min-height: 40px; border-radius: 4px; }"
        "QScrollBar::handle:vertical:hover { background: #94A3B8; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
    );

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("background-color: transparent;");
    QVBoxLayout *layout = new QVBoxLayout(scrollContent);
    layout->setContentsMargins(0, 0, 12, 0);
    layout->setSpacing(20);

    auto createCard = [this](const QString& titleText, QVBoxLayout*& contentLayoutOut) -> QWidget* {
        QWidget *card = new QWidget(this);
        card->setStyleSheet("background-color: #FFFFFF; border-radius: 12px; border: 1px solid #E2E8F0;");
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(14); shadow->setColor(QColor(0, 0, 0, 10)); shadow->setOffset(0, 3);
        card->setGraphicsEffect(shadow);

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(0, 0, 0, 0);
        cardLayout->setSpacing(0);

        QWidget *headerWidget = new QWidget(card);
        headerWidget->setStyleSheet("border: none; border-bottom: 1px solid #E2E8F0; border-top-left-radius: 12px; border-top-right-radius: 12px;");
        QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
        headerLayout->setContentsMargins(26, 16, 26, 16);
        QLabel *lblTitle = new QLabel(titleText, headerWidget);
        lblTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #0F172A; border: none; background: transparent;");
        headerLayout->addWidget(lblTitle);
        
        QWidget *contentWidget = new QWidget(card);
        contentWidget->setStyleSheet("border: none; background: transparent;");
        QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(26, 22, 26, 22);
        
        cardLayout->addWidget(headerWidget);
        cardLayout->addWidget(contentWidget);
        
        contentLayoutOut = contentLayout;
        return card;
    };

    auto addFormRow = [](QFormLayout *form, const QString &text, QWidget *edit) {
        QLabel *l = new QLabel(text);
        l->setStyleSheet("font-size: 14px; color: #475569; border: none; font-weight: 600; background: transparent;");
        l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        l->setFixedWidth(135);

        if (QLabel *lbl = qobject_cast<QLabel*>(edit)) {
            lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            lbl->setStyleSheet("font-size: 14px; color: #1E293B; border: none; background: transparent; font-weight: 500; padding: 5px 10px; min-height: 36px;");
        } else if (QLineEdit *le = qobject_cast<QLineEdit*>(edit)) {
            le->setAlignment(Qt::AlignLeft);
            le->setStyleSheet(kProfileFieldStyle);
        }
        edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        form->addRow(l, edit);
    };

    QVBoxLayout *layout1;
    QWidget *card1 = createCard("Thông tin cá nhân", layout1);
    QFormLayout *form1 = new QFormLayout();
    form1->setHorizontalSpacing(24);
    form1->setVerticalSpacing(12);
    form1->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    txtFullName = new QLineEdit(card1);
    txtDob = new QLineEdit(card1);
    txtCitizenId = new QLineEdit(card1);
    txtAddress = new QLineEdit(card1);

    addFormRow(form1, "Họ và tên", txtFullName);
    addFormRow(form1, "Ngày sinh", txtDob);
    addFormRow(form1, "Số CCCD", txtCitizenId);
    addFormRow(form1, "Địa chỉ", txtAddress);
    layout1->addLayout(form1);
    layout->addWidget(card1);

    QVBoxLayout *layout2;
    QWidget *card2 = createCard("Chi tiết công việc", layout2);
    QFormLayout *form2 = new QFormLayout();
    form2->setHorizontalSpacing(24);
    form2->setVerticalSpacing(12);

    lblDepartment = new QLabel(card2);
    lblHireDate = new QLabel(card2);

    addFormRow(form2, "Phòng ban", lblDepartment);
    addFormRow(form2, "Ngày vào làm", lblHireDate);
    layout2->addLayout(form2);
    layout->addWidget(card2);

    QVBoxLayout *layout3;
    cardRoleSpecific = createCard("Chi tiết chuyên môn", layout3);

    widgetDoctorFields = new QWidget(cardRoleSpecific);
    widgetDoctorFields->setStyleSheet("border: none; background: transparent;");
    QVBoxLayout *vboxDoc = new QVBoxLayout(widgetDoctorFields);
    vboxDoc->setContentsMargins(0, 0, 0, 0);
    QFormLayout *formDoc = new QFormLayout();
    formDoc->setHorizontalSpacing(24);
    formDoc->setVerticalSpacing(12);

    txtSpecialty = new QLineEdit(widgetDoctorFields);
    txtLicenseNumber = new QLineEdit(widgetDoctorFields);
    txtConsultationFee = new QLineEdit(widgetDoctorFields);
    txtDocRoom = new QLineEdit(widgetDoctorFields);

    addFormRow(formDoc, "Chuyên khoa", txtSpecialty);
    addFormRow(formDoc, "Số CCHN", txtLicenseNumber);
    addFormRow(formDoc, "Phí khám", txtConsultationFee);
    addFormRow(formDoc, "Phòng khám", txtDocRoom);
    formDoc->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    vboxDoc->addLayout(formDoc);
    layout3->addWidget(widgetDoctorFields);

    widgetNurseFields = new QWidget(cardRoleSpecific);
    widgetNurseFields->setStyleSheet("border: none; background: transparent;");
    QVBoxLayout *vboxNurse = new QVBoxLayout(widgetNurseFields);
    vboxNurse->setContentsMargins(0, 0, 0, 0);
    QFormLayout *formNurse = new QFormLayout();
    formNurse->setHorizontalSpacing(24);
    formNurse->setVerticalSpacing(12);

    txtNurseLevel = new QLineEdit(widgetNurseFields);
    txtCertification = new QLineEdit(widgetNurseFields);
    txtNurseRoom = new QLineEdit(widgetNurseFields);

    addFormRow(formNurse, "Cấp độ y tá", txtNurseLevel);
    addFormRow(formNurse, "Chứng chỉ", txtCertification);
    addFormRow(formNurse, "Phòng làm việc", txtNurseRoom);
    vboxNurse->addLayout(formNurse);
    layout3->addWidget(widgetNurseFields);
    layout->addWidget(cardRoleSpecific);

    QVBoxLayout *layoutBio;
    cardBio = createCard("Tiểu sử", layoutBio);
    cardBio->setMinimumHeight(150); 

    txtBio = new QTextEdit(cardBio); 
    txtBio->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    txtBio->setAcceptRichText(false); 
    txtBio->setStyleSheet(kProfileBioStyle);
    layoutBio->addWidget(txtBio);
    layout->addWidget(cardBio);

    QList<QLineEdit*> allFields = {
        txtFullName, txtDob, txtCitizenId, txtAddress,
        txtSpecialty, txtLicenseNumber, txtConsultationFee, txtDocRoom,
        txtNurseLevel, txtCertification, txtNurseRoom
    };
    txtBio->setReadOnly(true);
    for (QLineEdit* field : allFields) {
        field->setReadOnly(true);
        field->setStyleSheet(kProfileFieldStyle);
    }

    cardRoleSpecific->hide();
    cardBio->hide();

    // --- UI Validation Hooks ---
    UIValidationUtils::attachPrimitiveValidators(txtCitizenId, txtPhone);
    UIValidationUtils::attachPrimitiveValidators(nullptr, txtConsultationFee); 

    connect(txtPhone, &QLineEdit::editingFinished, this, [this]() {
        if (!txtPhone->isReadOnly()) {
            QString err = Validation::validatePhoneNumber(txtPhone->text());
            UIValidationUtils::applyFieldValidationStyle(txtPhone, err);
        }
    });
    connect(txtEmail, &QLineEdit::editingFinished, this, [this]() {
        if (!txtEmail->isReadOnly()) {
            QString err = Validation::validateEmail(txtEmail->text());
            UIValidationUtils::applyFieldValidationStyle(txtEmail, err);
        }
    });
    connect(txtAddress, &QLineEdit::editingFinished, this, [this]() {
        if (!txtAddress->isReadOnly()) {
            QString err = Validation::validateTrimmedNotEmpty(txtAddress->text(), "Vui lòng nhập địa chỉ");
            UIValidationUtils::applyFieldValidationStyle(txtAddress, err);
        }
    });
    connect(txtNurseLevel, &QLineEdit::editingFinished, this, [this]() {
        if (!txtNurseLevel->isReadOnly()) {
            QString err = Validation::validateTrimmedNotEmpty(txtNurseLevel->text(), "Vui lòng nhập cấp độ y tá");
            UIValidationUtils::applyFieldValidationStyle(txtNurseLevel, err);
        }
    });
    connect(txtCertification, &QLineEdit::editingFinished, this, [this]() {
        if (!txtCertification->isReadOnly()) {
            QString err = Validation::validateTrimmedNotEmpty(txtCertification->text(), "Vui lòng nhập chứng chỉ");
            UIValidationUtils::applyFieldValidationStyle(txtCertification, err);
        }
    });

    scrollArea->setWidget(scrollContent);
    return scrollArea;
}

void ProfileWidget::loadProfile(int staffId) {
    auto profile = m_staffService->getOwnProfile(staffId);
    if (!profile) {
        return;
    }
    currentAvatar = profile->avatar;
    
    lblAvatar->setAvatarPixmap(profile->avatar);
    if (profile->avatar.isNull()) {
        lblAvatar->setText(profile->fullName.left(1).toUpper());
    }
    lblAvatar->setAlignment(Qt::AlignCenter);
    lblName->setText(profile->fullName);
    lblStatus->setText(profile->isActive ? "Đang làm việc" : "Đã nghỉ việc");
    txtFullName->setText(profile->fullName);
    txtFullName->setCursorPosition(0);
    txtPhone->setText(profile->phoneNumber);
    txtPhone->setCursorPosition(0);
    txtEmail->setText(profile->email);
    txtEmail->setCursorPosition(0);
    txtAddress->setText(profile->address);
    txtAddress->setCursorPosition(0);
    txtDob->setText(profile->dateOfBirth.toString("dd/MM/yyyy"));
    txtDob->setCursorPosition(0);
    txtCitizenId->setText(profile->citizenId);
    txtCitizenId->setCursorPosition(0);
    lblDepartment->setText(profile->departmentName.isEmpty() ? "Chưa phân khoa" : profile->departmentName);
    currentDepartmentId = profile->departmentId;
    lblStaffCode->setText(profile->staffCode);
    lblHireDate->setText(profile->hireDate.toString("dd/MM/yyyy"));
    
    txtGender->setText(GenderText::toVi(profile->gender));
    txtGender->setCursorPosition(0);

    if (auto docProfile = dynamic_cast<DoctorProfileDTO*>(profile.get())) {
        lblRole->setText("Bác sĩ");
        widgetDoctorFields->show();
        widgetNurseFields->hide();
        cardRoleSpecific->show();
        cardBio->show(); 
        
        lblRole->setText(userRoleToVi(profile->role));
        txtSpecialty->setText(docProfile->specialty);
        txtSpecialty->setCursorPosition(0);
        txtLicenseNumber->setText(docProfile->licenseNumber);
        txtLicenseNumber->setCursorPosition(0);
        txtConsultationFee->setText(QString::number(docProfile->consultationFee));
        txtConsultationFee->setCursorPosition(0);
        txtDocRoom->setText(docProfile->roomNumber.isEmpty() ? "Chưa gán phòng" : docProfile->roomNumber);
        txtDocRoom->setCursorPosition(0);
        txtBio->setPlainText(docProfile->bio);
        currentExperienceYears = docProfile->experienceYears;
    } 
    else if (auto nurseProfile = dynamic_cast<NurseProfileDTO*>(profile.get())) {
        lblRole->setText("Y tá");
        widgetNurseFields->show();
        widgetDoctorFields->hide();
        cardRoleSpecific->show();
        cardBio->hide(); 
        
        lblRole->setText(userRoleToVi(profile->role));
        txtNurseLevel->setText(NurseLevelText::toVi(nurseProfile->nurseLevel));
        txtNurseLevel->setCursorPosition(0);
        txtCertification->setText(nurseProfile->certification);
        txtCertification->setCursorPosition(0);
        txtNurseRoom->setText(nurseProfile->roomName.isEmpty() ? "Chưa gán phòng" : nurseProfile->roomName);
        txtNurseRoom->setCursorPosition(0);
    } 
    else {
        lblRole->setText("Nhân viên");
        cardRoleSpecific->hide();
        cardBio->hide(); 
    }
    this->currentStaffId = staffId;
}

void ProfileWidget::onEditClicked() {
    if (btnEdit->text() == "Chỉnh Sửa" || btnEdit->text() == "Chỉnh sửa") {
        lblAvatar->setEnabled(true);
        lblAvatar->setCursor(Qt::PointingHandCursor);
        lblAvatar->setToolTip("Nhấn hoặc kéo thả ảnh vào đây để đổi Avatar");

        txtPhone->setReadOnly(false);
        txtEmail->setReadOnly(false);
        txtAddress->setReadOnly(false);
        cmbShift->setEnabled(false);
        
        if (widgetDoctorFields->isVisible()) {
            txtBio->setReadOnly(false); 
        }
        
        if (widgetNurseFields->isVisible()) {
            txtNurseLevel->setReadOnly(false);
            txtCertification->setReadOnly(false);
        }

        btnEdit->setText("Lưu");
        btnEdit->setStyleSheet(
            "QPushButton { background-color: #059669; color: white; font-weight: bold; border-radius: 8px; border: none; font-size: 14px; }"
            "QPushButton:hover { background-color: #047857; }"
        );
        return;
    }

    std::unique_ptr<StaffInputDTO> inputDTO;
    
    if (widgetDoctorFields->isVisible()) {
        auto docDTO = std::make_unique<DoctorInputDTO>();
        docDTO->specialty = txtSpecialty->text();
        docDTO->licenseNumber = txtLicenseNumber->text();
        docDTO->consultationFee = txtConsultationFee->text().toDouble();
        docDTO->experienceYears = currentExperienceYears; 
        docDTO->bio = txtBio->toPlainText(); 
        inputDTO = std::move(docDTO);
    } else if (widgetNurseFields->isVisible()) {
        auto nurseDTO = std::make_unique<NurseInputDTO>();
        nurseDTO->nurseLevel = txtNurseLevel->text();
        nurseDTO->certification = txtCertification->text();
        inputDTO = std::move(nurseDTO);
    } else {
        inputDTO = std::make_unique<StaffInputDTO>();
    }

    QPixmap pickedAvatar = lblAvatar->getAvatarPixmap();
    if (!pickedAvatar.isNull()) {
        currentAvatar = pickedAvatar;
    }

    inputDTO->fullName = txtFullName->text();
    inputDTO->avatar = currentAvatar;
    inputDTO->phoneNumber = txtPhone->text();
    inputDTO->email = txtEmail->text();
    inputDTO->address = txtAddress->text();
    inputDTO->citizenId = txtCitizenId->text();
    inputDTO->dateOfBirth = QDate::fromString(txtDob->text(), "dd/MM/yyyy");
    inputDTO->gender = GenderText::toEn(txtGender->text());
    inputDTO->departmentId = currentDepartmentId; 
    inputDTO->shift = cmbShift->currentData().toString();

    QString errorMsg;
    if (auto docInput = dynamic_cast<DoctorInputDTO*>(inputDTO.get())) {
        errorMsg = m_staffService->editDoctorInformation(*docInput, currentStaffId);
    } else if (auto nurseInput = dynamic_cast<NurseInputDTO*>(inputDTO.get())) {
        errorMsg = m_staffService->editNurseInformation(*nurseInput, currentStaffId);
    } else {
        errorMsg = m_staffService->editStaffBaseInformation(*inputDTO, currentStaffId);
    }

    bool success = errorMsg.isEmpty();

    if (success) {
        QMessageBox::information(this, "Thành công", "Đã cập nhật hồ sơ!");
        
        lblAvatar->setEnabled(false);
        lblAvatar->setCursor(Qt::ArrowCursor);
        lblAvatar->setToolTip("");

        txtFullName->setReadOnly(true);
        txtPhone->setReadOnly(true);
        txtEmail->setReadOnly(true);
        txtAddress->setReadOnly(true);
        txtDob->setReadOnly(true);
        txtCitizenId->setReadOnly(true);
        txtGender->setReadOnly(true);
        txtSpecialty->setReadOnly(true);
        txtLicenseNumber->setReadOnly(true);
        txtConsultationFee->setReadOnly(true);
        txtBio->setReadOnly(true);
        txtNurseLevel->setReadOnly(true);
        txtCertification->setReadOnly(true);
        cmbShift->setEnabled(false);

        btnEdit->setText("Chỉnh Sửa");
        btnEdit->setStyleSheet(
            "QPushButton { background-color: #2563EB; color: white; font-weight: bold; border-radius: 8px; border: none; font-size: 14px; }"
            "QPushButton:hover { background-color: #1D4ED8; }"
        );

        loadProfile(currentStaffId);
    } else {
        QMessageBox::critical(this, "Lỗi", errorMsg);
    }
}

void ProfileWidget::validatePhoneNumber() {
    if (txtPhone->isReadOnly()) return;
    
    QString erro = Validation::validatePhoneNumber(txtPhone->text());
    if (!erro.isEmpty()) { 
        txtPhone->setStyleSheet("QLineEdit { border: 1.5px solid #EF4444; border-radius: 6px; background-color: #FEF2F2; color: #991B1B; padding: 4px 8px; min-height: 32px; font-size: 14px; }");
        QMessageBox::warning(this, "Định dạng sai", erro);
    } else {
        txtPhone->setStyleSheet(kProfileFieldStyle);
    }
}

void ProfileWidget::validateEmail() {
    if (txtEmail->isReadOnly()) return;
    
    QString erro = Validation::validateEmail(txtEmail->text());
    if (!erro.isEmpty()) { 
        txtEmail->setStyleSheet("QLineEdit { border: 1.5px solid #EF4444; border-radius: 6px; background-color: #FEF2F2; color: #991B1B; padding: 4px 8px; min-height: 32px; font-size: 14px; }");
        QMessageBox::warning(this, "Định dạng sai", erro);
    } else {
        txtEmail->setStyleSheet(kProfileFieldStyle);
    }
}

