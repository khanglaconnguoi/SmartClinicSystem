#include "PatientRegistrationDialog.h"
#include "../../dto/PatientDTOs.h"
#include "../utils/UIValidationUtils.h"
#include "../../service/Validation.h"
#include "model/CommonEnums.h"
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

PatientRegistrationDialog::PatientRegistrationDialog(
    std::shared_ptr<PatientService> patientService, QWidget *parent)
    : QDialog(parent), m_patientService(patientService)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowTitle("Đăng Ký Bệnh Nhân Mới");
    setMinimumWidth(700);
    const QRect avail = QApplication::primaryScreen()->availableGeometry();
    const int dlgW = qMin(720, avail.width() - 40);
    const int dlgH = qMin(900, avail.height() - 40);
    setMinimumHeight(qMin(600, dlgH));
    resize(dlgW, dlgH);
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
        "#mainContainer { background-color: #FFFFFF; border-radius: 14px; }");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 5);
    container->setGraphicsEffect(shadow);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // --- Header ---
    QFrame *headerFrame = new QFrame(container);
    headerFrame->setStyleSheet(
        "background-color: transparent; border-bottom: 1px solid #EAEAEA;");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(24, 20, 24, 20);

    QLabel *lblPageTitle = new QLabel("ĐĂNG KÝ BỆNH NHÂN", headerFrame);
    lblPageTitle->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #1F2937;");
    lblPageTitle->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(lblPageTitle);
    containerLayout->addWidget(headerFrame);

    // --- Scroll Area ---
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
        "QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox { "
        "border: 1px solid #D1D5DB; border-radius: 8px; padding: 8px 12px; "
        "font-size: 13px; color: #111827; background: #FFFFFF; min-height: 36px; "
        "} "
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDoubleSpinBox:focus { "
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

    m_txtFullName = new QLineEdit(gbPersonalInfo);
    m_txtFullName->setStyleSheet(extraInputStyle);
    m_txtFullName->setPlaceholderText("VD: Nguyễn Văn A");
    form1->addRow("Họ và tên (*):", m_txtFullName);

    m_dtDateOfBirth = new QDateEdit(QDate(1990, 1, 1), gbPersonalInfo);
    m_dtDateOfBirth->setCalendarPopup(true);
    m_dtDateOfBirth->setDisplayFormat("dd/MM/yyyy");
    m_dtDateOfBirth->setStyleSheet(extraInputStyle);
    form1->addRow("Ngày sinh (*):", m_dtDateOfBirth);

    m_cbGender = new QComboBox(gbPersonalInfo);
    m_cbGender->setStyleSheet(extraInputStyle);
    for (const auto &pair : GenderText::getList())
        m_cbGender->addItem(pair.second, pair.first);
    form1->addRow("Giới tính (*):", m_cbGender);

    m_txtCitizenId = new QLineEdit(gbPersonalInfo);
    m_txtCitizenId->setStyleSheet(extraInputStyle);
    m_txtCitizenId->setPlaceholderText("Nhập 12 chữ số hợp lệ");
    form1->addRow("Số CCCD (*):", m_txtCitizenId);

    cardLayout->addWidget(gbPersonalInfo);

    // --- Nhóm 2: Thông tin liên hệ ---
    QGroupBox *gbContactInfo = new QGroupBox("Thông tin liên hệ", formCard);
    gbContactInfo->setStyleSheet(groupBoxStyle);
    QFormLayout *form2 = new QFormLayout(gbContactInfo);
    form2->setContentsMargins(16, 18, 16, 16);
    form2->setSpacing(12);
    form2->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_txtPhone = new QLineEdit(gbContactInfo);
    m_txtPhone->setStyleSheet(extraInputStyle);
    m_txtPhone->setPlaceholderText("VD: 0901234567");
    form2->addRow("Số điện thoại (*):", m_txtPhone);

    m_txtEmail = new QLineEdit(gbContactInfo);
    m_txtEmail->setStyleSheet(extraInputStyle);
    m_txtEmail->setPlaceholderText("VD: patient@example.com");
    form2->addRow("Email:", m_txtEmail);

    m_txtAddress = new QLineEdit(gbContactInfo);
    m_txtAddress->setStyleSheet(extraInputStyle);
    m_txtAddress->setPlaceholderText("VD: TP. Hồ Chí Minh");
    form2->addRow("Địa chỉ (*):", m_txtAddress);

    m_txtEmergencyContactName = new QLineEdit(gbContactInfo);
    m_txtEmergencyContactName->setStyleSheet(extraInputStyle);
    m_txtEmergencyContactName->setPlaceholderText("Họ tên người thân");
    form2->addRow("Tên người thân (*):", m_txtEmergencyContactName);

    m_txtEmergencyContactPhone = new QLineEdit(gbContactInfo);
    m_txtEmergencyContactPhone->setStyleSheet(extraInputStyle);
    m_txtEmergencyContactPhone->setPlaceholderText("SĐT người thân (không trùng SĐT bệnh nhân)");
    form2->addRow("SĐT người thân (*):", m_txtEmergencyContactPhone);

    cardLayout->addWidget(gbContactInfo);

    // --- Nhóm 3: Bảo hiểm y tế ---
    QGroupBox *gbInsuranceInfo = new QGroupBox("Thông tin Bảo hiểm Y tế", formCard);
    gbInsuranceInfo->setStyleSheet(groupBoxStyle);
    QFormLayout *form3 = new QFormLayout(gbInsuranceInfo);
    form3->setContentsMargins(16, 18, 16, 16);
    form3->setSpacing(12);
    form3->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_cbInsuranceType = new QComboBox(gbInsuranceInfo);
    m_cbInsuranceType->setStyleSheet(extraInputStyle);
    m_cbInsuranceType->addItem("Không có", "NONE");
    for (const auto &pair : InsuranceTypeText::getList())
        m_cbInsuranceType->addItem(pair.second, pair.first);
    form3->addRow("Loại bảo hiểm:", m_cbInsuranceType);

    m_txtInsuranceProvider = new QLineEdit(gbInsuranceInfo);
    m_txtInsuranceProvider->setStyleSheet(extraInputStyle);
    m_txtInsuranceProvider->setPlaceholderText("Tên công ty / Nơi cấp");
    form3->addRow("Nơi cấp:", m_txtInsuranceProvider);

    m_txtInsurancePolicy = new QLineEdit(gbInsuranceInfo);
    m_txtInsurancePolicy->setStyleSheet(extraInputStyle);
    m_txtInsurancePolicy->setPlaceholderText("Mã thẻ / Số hợp đồng");
    form3->addRow("Mã thẻ:", m_txtInsurancePolicy);

    m_spinInsuranceCoverage = new QDoubleSpinBox(gbInsuranceInfo);
    m_spinInsuranceCoverage->setStyleSheet(extraInputStyle);
    m_spinInsuranceCoverage->setRange(0.0, 100.0);
    m_spinInsuranceCoverage->setValue(80.0);
    m_spinInsuranceCoverage->setSuffix(" %");
    form3->addRow("Mức chi trả:", m_spinInsuranceCoverage);

    m_dateInsuranceFrom = new QDateEdit(QDate::currentDate(), gbInsuranceInfo);
    m_dateInsuranceFrom->setCalendarPopup(true);
    m_dateInsuranceFrom->setDisplayFormat("dd/MM/yyyy");
    m_dateInsuranceFrom->setStyleSheet(extraInputStyle);
    form3->addRow("Hiệu lực từ:", m_dateInsuranceFrom);

    m_dateInsuranceTo = new QDateEdit(QDate::currentDate().addYears(1), gbInsuranceInfo);
    m_dateInsuranceTo->setCalendarPopup(true);
    m_dateInsuranceTo->setDisplayFormat("dd/MM/yyyy");
    m_dateInsuranceTo->setStyleSheet(extraInputStyle);
    form3->addRow("Hiệu lực đến:", m_dateInsuranceTo);

    cardLayout->addWidget(gbInsuranceInfo);

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

    m_btnSave = new QPushButton("Lưu bệnh nhân", bottomFrame);
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
            &PatientRegistrationDialog::handleSave);
    connect(m_btnCancel, &QPushButton::clicked, this, [this]() {
        emit cancelled();
        reject();
    });

    // --- UI Validation ---
    UIValidationUtils::attachPrimitiveValidators(m_txtCitizenId, m_txtPhone);
    if (m_txtEmergencyContactPhone) {
        m_txtEmergencyContactPhone->setValidator(
            new QRegularExpressionValidator(
                QRegularExpression("^0\\d{0,10}$"), m_txtEmergencyContactPhone));
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

    // 1. Kiểm tra trùng lặp Số CCCD cá nhân
    if (!citizenId.isEmpty() && m_patientService) {
        auto existCitizen = m_patientService->getPatientByPhoneOrCitizenId("", citizenId);
        if (existCitizen.has_value()) {
            const auto &p = existCitizen.value();
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Trùng Lặp Thông Tin CCCD");
            msgBox.setText(
                QString("Không thể đăng ký bệnh nhân mới!\n\n"
                        "Số CCCD [%1] ĐÃ TỒN TẠI trên hệ thống.\n\n"
                        "Thông tin bệnh nhân trùng khớp:\n"
                        "• Họ tên: %2\n"
                        "• Mã bệnh nhân: %3\n"
                        "• SĐT cá nhân: %4\n\n"
                        "Vui lòng kiểm tra lại thông tin hoặc chọn bệnh nhân đã có để tiếp tục.")
                    .arg(citizenId, p.fullName, p.patientCode, p.phone));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setStyleSheet(
                "QMessageBox { background-color: #FFFFFF; border-radius: 8px; }"
                "QLabel { color: #111827; font-size: 14px; font-weight: bold; }"
                "QPushButton { background-color: #EF4444; color: white; "
                "font-weight: bold; min-width: 100px; min-height: 35px; border-radius: "
                "6px; border: none; font-size: 14px; }"
                "QPushButton:hover { background-color: #DC2828; }");
            msgBox.exec();
            return;
        }
    }

    // 2. Kiểm tra trùng lặp Số điện thoại cá nhân (KHÔNG kiểm tra SĐT người thân)
    if (!phone.isEmpty() && m_patientService) {
        auto existPhone = m_patientService->getPatientByPhoneOrCitizenId(phone, "");
        if (existPhone.has_value()) {
            const auto &p = existPhone.value();
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Trùng Lặp Số Điện Thoại Cá Nhân");
            msgBox.setText(
                QString("Không thể đăng ký bệnh nhân mới!\n\n"
                        "Số điện thoại cá nhân [%1] ĐÃ TỒN TẠI trên hệ thống.\n\n"
                        "Thông tin bệnh nhân trùng khớp:\n"
                        "• Họ tên: %2\n"
                        "• Mã bệnh nhân: %3\n"
                        "• SĐT cá nhân: %4\n\n"
                        "Vui lòng kiểm tra lại thông tin hoặc chọn bệnh nhân đã có để tiếp tục.")
                    .arg(phone, p.fullName, p.patientCode, p.phone));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setStyleSheet(
                "QMessageBox { background-color: #FFFFFF; border-radius: 8px; }"
                "QLabel { color: #111827; font-size: 14px; font-weight: bold; }"
                "QPushButton { background-color: #EF4444; color: white; "
                "font-weight: bold; min-width: 100px; min-height: 35px; border-radius: "
                "6px; border: none; font-size: 14px; }"
                "QPushButton:hover { background-color: #DC2828; }");
            msgBox.exec();
            return;
        }
    }

    QString gender = GenderText::toEn(m_cbGender->currentText());
    QDate dob = m_dtDateOfBirth->date();
    QString email = m_txtEmail->text().trimmed();
    QString address = m_txtAddress->text().trimmed();

    PatientType type = PatientType::Outpatient;
    QString bloodType = "UNKNOWN";
    QString allergies = "";

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

    auto fillInsurance = [&](InsuranceInputDTO &insDto) {
        insDto.policyNumber = insurancePolicy;
        insDto.providerName = insuranceProvider;
        insDto.insuranceType = insuranceTypeEn;
        insDto.coveragePercent = insuranceCoverage;
        insDto.validFrom = insuranceFrom;
        insDto.validTo = insuranceTo;
    };

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
        dto.type = type;
        dto.emergencyContactName = emerName;
        dto.emergencyContactPhone = emerPhone;
        if (hasInsurance) { fillInsurance(dto.insurance); }
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
        dto.type = type;
        dto.emergencyContactName = emerName;
        dto.emergencyContactPhone = emerPhone;
        if (hasInsurance) { fillInsurance(dto.insurance); }
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
        dto.type = type;
        dto.emergencyContactName = emerName;
        dto.emergencyContactPhone = emerPhone;
        if (hasInsurance) { fillInsurance(dto.insurance); }
        dto.injuryCause = "Chưa xác định";
        dto.injuryDescription = "Chưa xác định";
        dto.admissionDate = QDate::currentDate();
        dto.dischargeDate = std::nullopt;
        errorMsg = m_patientService->addEmergencyPatient(dto);
    }

    if (!errorMsg.isEmpty())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Lỗi");
        msgBox.setText(errorMsg);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #FFFFFF; border-radius: 8px; }"
            "QLabel { color: #111827; font-size: 14px; font-weight: bold; }"
            "QPushButton { background-color: #EF4444; color: white; "
            "font-weight: bold; min-width: 100px; min-height: 35px; border-radius: "
            "6px; border: none; font-size: 14px; }"
            "QPushButton:hover { background-color: #DC2828; }");
        msgBox.exec();
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Thành công");
    msgBox.setText("Đăng ký bệnh nhân thành công!");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #FFFFFF; border-radius: 8px; }"
        "QLabel { color: #111827; font-size: 15px; font-weight: bold; }"
        "QPushButton { background-color: #34A853; color: white; "
        "font-weight: bold; min-width: 100px; min-height: 35px; border-radius: "
        "6px; border: none; font-size: 14px; }"
        "QPushButton:hover { background-color: #2C8E46; }");
    msgBox.exec();

    emit saved(phone, citizenId, fullName);
    accept();
}
