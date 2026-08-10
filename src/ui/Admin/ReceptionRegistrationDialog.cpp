#include "ReceptionRegistrationDialog.h"
#include "../../dto/StaffDTOs.h"
#include "../utils/UIValidationUtils.h"
#include "../../service/Validation.h"
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
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>

ReceptionRegistrationDialog::ReceptionRegistrationDialog(
    std::shared_ptr<StaffService> staffService, QWidget *parent)
    : QDialog(parent), m_staffService(staffService), m_editStaffId(-1)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowTitle("Thêm Lễ tân");
    setMinimumWidth(700);
    const QRect avail = QApplication::primaryScreen()->availableGeometry();
    const int dlgW = qMin(720, avail.width() - 40);
    const int dlgH = qMin(820, avail.height() - 40);
    setMinimumHeight(qMin(600, dlgH));
    resize(dlgW, dlgH);
    setupUi();
}

void ReceptionRegistrationDialog::setupUi()
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet("QDialog { background-color: transparent; } "
                        "QLabel { color: #333333; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QFrame *container = new QFrame(this);
    container->setObjectName("mainContainer");
    container->setStyleSheet(
        "#mainContainer { background-color: #FFFFFF; border-radius: 14px; }");

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

    m_lblPageTitle = new QLabel("THÔNG TIN LỄ TÂN", headerFrame);
    m_lblPageTitle->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #1F2937;");
    m_lblPageTitle->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(m_lblPageTitle);

    containerLayout->addWidget(headerFrame);

    QScrollArea *scrollArea = new QScrollArea(container);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: transparent; border: none; }");

    QFrame *formCard = new QFrame(scrollArea);
    formCard->setStyleSheet(
        "QFrame { background-color: #FFFFFF; border: none; } "
        "QLabel { color: #374151; font-size: 13px; font-weight: 600; }");

    QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(24, 20, 24, 24);
    cardLayout->setSpacing(18);

    QString groupBoxStyle =
        "QGroupBox { "
        "border: 1px solid #E5E7EB; border-radius: 10px; "
        "margin-top: 16px; padding-top: 8px; font-weight: 700; color: #374151; "
        "background-color: #FCFDFF; } "
        "QGroupBox::title { "
        "subcontrol-origin: margin; subcontrol-position: top left; "
        "left: 12px; padding: 0 8px; color: #2563EB; font-size: 14px; }";

    QString extraInputStyle =
        "QLineEdit, QComboBox, QDateEdit { "
        "border: 1px solid #D1D5DB; border-radius: 8px; padding: 8px 12px; "
        "font-size: 13px; color: #111827; background: #FFFFFF; min-height: 36px; "
        "} "
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { "
        "border: 1px solid #4B94F2; background-color: #F8FBFF; } "
        "QComboBox QAbstractItemView { "
        "background-color: #FFFFFF; color: #111827; selection-background-color: "
        "#4B94F2; selection-color: white; }";

    // --- Nhóm 1: Thông tin cá nhân ---
    QGroupBox *gbPersonalInfo = new QGroupBox("Thông tin cá nhân", formCard);
    gbPersonalInfo->setStyleSheet(groupBoxStyle);
    QFormLayout *form1 = new QFormLayout(gbPersonalInfo);
    form1->setContentsMargins(16, 18, 16, 16);
    form1->setSpacing(12);
    form1->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_avatarPicker = new AvatarPickerWidget(gbPersonalInfo);
    form1->addRow("Ảnh đại diện:", m_avatarPicker);

    m_txtFullName = new QLineEdit(gbPersonalInfo);
    m_txtFullName->setStyleSheet(extraInputStyle);
    m_txtFullName->setPlaceholderText("VD: Nguyễn Thị B");
    form1->addRow("Họ và tên (*):", m_txtFullName);

    m_dtDateOfBirth = new QDateEdit(QDate(1990, 1, 1), gbPersonalInfo);
    m_dtDateOfBirth->setCalendarPopup(true);
    m_dtDateOfBirth->setDisplayFormat("dd/MM/yyyy");
    m_dtDateOfBirth->setStyleSheet(extraInputStyle);
    form1->addRow("Ngày sinh (*):", m_dtDateOfBirth);

    m_cbGender = new QComboBox(gbPersonalInfo);
    m_cbGender->setStyleSheet(extraInputStyle);
    for (const auto& pair : GenderText::getList()) m_cbGender->addItem(pair.second, pair.first);
    form1->addRow("Giới tính (*):", m_cbGender);

    m_txtCitizenId = new QLineEdit(gbPersonalInfo);
    m_txtCitizenId->setStyleSheet(extraInputStyle);
    m_txtCitizenId->setPlaceholderText("Nhập 12 chữ số hợp lệ");
    form1->addRow("Số CMND/CCCD (*):", m_txtCitizenId);

    m_txtPhone = new QLineEdit(gbPersonalInfo);
    m_txtPhone->setStyleSheet(extraInputStyle);
    m_txtPhone->setPlaceholderText("VD: 0901234567");
    form1->addRow("Số điện thoại (*):", m_txtPhone);

    m_txtEmail = new QLineEdit(gbPersonalInfo);
    m_txtEmail->setStyleSheet(extraInputStyle);
    m_txtEmail->setPlaceholderText("VD: reception@example.com");
    form1->addRow("Email (*):", m_txtEmail);

    m_txtAddress = new QLineEdit(gbPersonalInfo);
    m_txtAddress->setStyleSheet(extraInputStyle);
    m_txtAddress->setPlaceholderText("VD: TP. Hồ Chí Minh");
    form1->addRow("Địa chỉ (*):", m_txtAddress);

    cardLayout->addWidget(gbPersonalInfo);

    // --- Nhóm 2: Thông tin công việc ---
    QGroupBox *gbWorkInfo = new QGroupBox("Thông tin công việc", formCard);
    gbWorkInfo->setStyleSheet(groupBoxStyle);
    QFormLayout *form2 = new QFormLayout(gbWorkInfo);
    form2->setContentsMargins(16, 18, 16, 16);
    form2->setSpacing(12);
    form2->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_cbDepartment = new QComboBox(gbWorkInfo);
    m_cbDepartment->setStyleSheet(extraInputStyle);
    for (const auto& pair : DepartmentText::getList()) m_cbDepartment->addItem(pair.second, pair.first);
    form2->addRow("Phòng ban (*):", m_cbDepartment);

    m_cbShift = new QComboBox(gbWorkInfo);
    m_cbShift->setStyleSheet(extraInputStyle);
    for (const auto& pair : ShiftText::getList()) m_cbShift->addItem(pair.second, pair.first);
    form2->addRow("Ca làm việc (*):", m_cbShift);

    m_dtHireDate = new QDateEdit(QDate::currentDate(), gbWorkInfo);
    m_dtHireDate->setCalendarPopup(true);
    m_dtHireDate->setDisplayFormat("dd/MM/yyyy");
    m_dtHireDate->setStyleSheet(extraInputStyle);
    form2->addRow("Ngày vào làm (*):", m_dtHireDate);

    cardLayout->addWidget(gbWorkInfo);

    scrollArea->setWidget(formCard);
    containerLayout->addWidget(scrollArea);

    // --- Nút Lưu và Trở lại ---
    QFrame *bottomFrame = new QFrame(container);
    bottomFrame->setStyleSheet(
        "background-color: transparent; border-top: 1px solid #E5E7EB;");
    QHBoxLayout *btnLayout = new QHBoxLayout(bottomFrame);
    btnLayout->setContentsMargins(20, 15, 20, 15);

    btnLayout->addStretch();
    m_btnCancel = new QPushButton("Hủy", bottomFrame);
    m_btnCancel->setCursor(Qt::PointingHandCursor);
    m_btnCancel->setFixedSize(140, 40);
    m_btnCancel->setStyleSheet(
        "QPushButton { background-color: #EAEAEA; color: "
        "#333; font-size: 13px; font-weight: 600; "
        "border-radius: 8px; border: none; padding: 0 10px; }"
        "QPushButton:hover { background-color: #D6D6D6; }");
    btnLayout->addWidget(m_btnCancel);

    m_btnSave = new QPushButton("Lưu thông tin", bottomFrame);
    m_btnSave->setCursor(Qt::PointingHandCursor);
    m_btnSave->setFixedSize(140, 40);
    m_btnSave->setStyleSheet(
        "QPushButton { background-color: #34A853; color: white; font-size: 13px; "
        "font-weight: 600; border-radius: 8px; border: none; padding: 0 10px; }"
        "QPushButton:hover { background-color: #2C8E46; }");
    btnLayout->addWidget(m_btnSave);

    containerLayout->addWidget(bottomFrame);
    mainLayout->addWidget(container);

    connect(m_btnSave, &QPushButton::clicked, this,
            &ReceptionRegistrationDialog::handleSave);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    // --- UI Validation ---
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

void ReceptionRegistrationDialog::loadReceptionistData(StaffProfileDTO* receptionist) {
    if (!receptionist) return;
    m_editStaffId = receptionist->staffId;

    m_avatarPicker->setAvatarPixmap(receptionist->avatar);
    m_txtFullName->setText(receptionist->fullName);
    m_txtCitizenId->setText(receptionist->citizenId);
    m_txtPhone->setText(receptionist->phoneNumber);
    m_txtEmail->setText(receptionist->email);
    m_txtAddress->setText(receptionist->address);

    m_cbGender->setCurrentText(GenderText::toVi(receptionist->gender));
    m_dtDateOfBirth->setDate(receptionist->dateOfBirth);

    for (int i = 0; i < m_cbDepartment->count(); ++i) {
        if (m_cbDepartment->itemText(i).startsWith(
                QString::number(receptionist->departmentId) + " -")) {
            m_cbDepartment->setCurrentIndex(i);
            break;
        }
    }
    m_cbShift->setCurrentText(ShiftText::toVi(receptionist->shift));
    m_dtHireDate->setReadOnly(true); // Không cho chỉnh sửa ngày vào làm

    setReadOnlyMode(true);
}

void ReceptionRegistrationDialog::setReadOnlyMode(bool readOnly) {
    m_isReadOnly = readOnly;

    m_txtFullName->setReadOnly(readOnly);
    m_txtCitizenId->setReadOnly(readOnly);
    m_txtPhone->setReadOnly(readOnly);
    m_txtEmail->setReadOnly(readOnly);
    m_txtAddress->setReadOnly(readOnly);

    m_dtDateOfBirth->setEnabled(!readOnly);
    m_dtHireDate->setEnabled(!readOnly);
    m_cbGender->setEnabled(!readOnly);
    m_cbDepartment->setEnabled(!readOnly);
    m_cbShift->setEnabled(!readOnly);
    if (m_avatarPicker) m_avatarPicker->setEnabled(!readOnly);

    if (m_lblPageTitle) {
        m_lblPageTitle->setText(readOnly ? "THÔNG TIN CHI TIẾT LỄ TÂN"
                                         : (m_editStaffId != -1 ? "CHỈNH SỬA THÔNG TIN LỄ TÂN" : "THÊM LỄ TÂN MỚI"));
    }

    if (m_btnSave) {
        if (readOnly) {
            m_btnSave->setText("Chỉnh sửa");
            m_btnSave->setStyleSheet(
                "QPushButton { background-color: #2563EB; color: white; font-size: 13px; "
                "font-weight: 600; border-radius: 8px; border: none; padding: 0 10px; }"
                "QPushButton:hover { background-color: #1D4ED8; }");
        } else {
            m_btnSave->setText("Lưu thông tin");
            m_btnSave->setStyleSheet(
                "QPushButton { background-color: #34A853; color: white; font-size: 13px; "
                "font-weight: 600; border-radius: 8px; border: none; padding: 0 10px; }"
                "QPushButton:hover { background-color: #2C8E46; }");
        }
    }

    if (m_btnCancel) {
        m_btnCancel->setText(readOnly ? "Đóng" : "Hủy");
    }
}

void ReceptionRegistrationDialog::handleSave()
{
    if (m_isReadOnly) {
        setReadOnlyMode(false);
        return;
    }

    if (!m_staffService)
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

    QString gender = GenderText::toEn(m_cbGender->currentText());
    QDate dob = m_dtDateOfBirth->date();
    QString email = m_txtEmail->text().trimmed();
    QString address = m_txtAddress->text().trimmed();
    QString shift = ShiftText::toEn(m_cbShift->currentText());
    int departmentId = m_cbDepartment->currentText().split(" - ").first().toInt();
    QPixmap avatar = m_avatarPicker->getAvatarPixmap();

    ReceptionistInputDTO dto;
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

    QString errorMsg;
    if (m_editStaffId == -1) {
        StaffHireResult result = m_staffService->hireNewReceptionist(dto);
        errorMsg = result.errorMessage;
        if (errorMsg.isEmpty()) {
            QDialog successDialog(this);
            successDialog.setWindowTitle("Thành công");
            successDialog.setMinimumWidth(380);
            successDialog.setStyleSheet(
                "QDialog { background-color: #FFFFFF; border-radius: 8px; }"
                "QLabel { color: #1F2937; font-size: 14px; }"
                "QLineEdit { background-color: #F9FAFB; border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 14px; font-weight: bold; color: #111827; }"
                "QPushButton { background-color: #34A853; color: white; "
                "font-weight: bold; min-width: 100px; min-height: 35px; border-radius: "
                "6px; border: none; font-size: 14px; }"
                "QPushButton:hover { background-color: #2C8E46; }");

            QVBoxLayout *dlgLayout = new QVBoxLayout(&successDialog);
            dlgLayout->setSpacing(15);
            dlgLayout->setContentsMargins(24, 24, 24, 20);

            QLabel *lblTitle = new QLabel("Tạo tài khoản Lễ tân thành công!", &successDialog);
            lblTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #111827;");
            dlgLayout->addWidget(lblTitle);

            QFormLayout *formLayout = new QFormLayout();
            formLayout->setSpacing(10);

            QLineEdit *txtStaffCode = new QLineEdit(result.staffCode, &successDialog);
            txtStaffCode->setReadOnly(true);
            QLabel *lblCode = new QLabel("Mã nhân viên:", &successDialog);
            lblCode->setStyleSheet("font-weight: 600; color: #374151;");
            formLayout->addRow(lblCode, txtStaffCode);

            QLineEdit *txtPassword = new QLineEdit(result.plainPassword, &successDialog);
            txtPassword->setReadOnly(true);
            QLabel *lblPass = new QLabel("Mật khẩu:", &successDialog);
            lblPass->setStyleSheet("font-weight: 600; color: #374151;");
            formLayout->addRow(lblPass, txtPassword);

            dlgLayout->addLayout(formLayout);

            QHBoxLayout *btnLayout = new QHBoxLayout();
            btnLayout->addStretch();
            QPushButton *btnOk = new QPushButton("OK", &successDialog);
            btnOk->setCursor(Qt::PointingHandCursor);
            connect(btnOk, &QPushButton::clicked, &successDialog, &QDialog::accept);
            btnLayout->addWidget(btnOk);
            btnLayout->addStretch();

            dlgLayout->addLayout(btnLayout);

            successDialog.exec();
            accept();
            return;
        }
    } else {
        errorMsg = m_staffService->editReceptionistInformation(dto, m_editStaffId);
        if (errorMsg.isEmpty()) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Thành công");
            msgBox.setText("Cập nhật thông tin Lễ tân thành công!");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStyleSheet(
                "QMessageBox { background-color: #FFFFFF; border-radius: 8px; }"
                "QLabel { color: #111827; font-size: 15px; font-weight: bold; }"
                "QPushButton { background-color: #34A853; color: white; "
                "font-weight: bold; min-width: 100px; min-height: 35px; border-radius: "
                "6px; border: none; font-size: 14px; }"
                "QPushButton:hover { background-color: #2C8E46; }");
            msgBox.exec();
            accept();
            return;
        }
    }

    if (!errorMsg.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Lỗi");
        msgBox.setText(errorMsg);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #FFFFFF; border-radius: 8px; }"
            "QLabel { color: #111827; font-size: 15px; font-weight: bold; }"
            "QPushButton { background-color: #EF4444; color: white; "
            "font-weight: bold; min-width: 100px; min-height: 35px; border-radius: "
            "6px; border: none; font-size: 14px; }"
            "QPushButton:hover { background-color: #DC2828; }");
        msgBox.exec();
        return;
    }

    accept();
}
