#include "ClinicalExamWidget.h"
#include "CreatePrescriptionDialog.h"
#include "PatientRecordHistoryDialog.h"
#include "service/PharmacyService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"
#include "service/UserSession.h"
#include "service/Validation.h"
#include <QDoubleValidator>
#include <QDebug>
#include <QMessageBox>
#include "../../model/CommonEnums.h"

ClinicalExamWidget::ClinicalExamWidget(std::shared_ptr<MedicalRecordService> medicalRecordService, QWidget* parent)
    : QWidget(parent), m_medicalRecordService(medicalRecordService)
{
    setupUi();

    // Kết nối các tín hiệu chuyển hướng về Dashboard
    connect(m_tabAppointmentsList, &QPushButton::clicked, this, &ClinicalExamWidget::viewAppointmentsListRequested);
    connect(m_btnCancel, &QPushButton::clicked, this, &ClinicalExamWidget::backToDashboardRequested);
    connect(m_btnFinish, &QPushButton::clicked, this, &ClinicalExamWidget::finishExamRequested);
    connect(m_btnCallPatient, &QPushButton::clicked, this, [this]() {
            emit callPatientRequested();
        });

    if (m_btnSave) {
        connect(m_btnSave, &QPushButton::clicked, this, &ClinicalExamWidget::onSaveClicked);
    }

    if (m_subPrescription) {
        connect(m_subPrescription, &QPushButton::clicked, this, [this]() {
            CreatePrescriptionDialog dialog(m_pharmacyService, this);
            dialog.setRecordId(m_lblPatientCodeVal ? m_lblPatientCodeVal->text() : "");
            dialog.exec();
        });
    }

    auto openHistoryAction = [this]() {
        PatientRecordHistoryDialog dialog(m_pharmacyService, this);
        QString patCode = m_lblPatientCodeVal ? m_lblPatientCodeVal->text() : "";
        QString patName = m_lblPatientNameVal ? m_lblPatientNameVal->text() : "";
        int patId = patCode.startsWith("BN") ? patCode.mid(2).toInt() : 0;
        dialog.loadPatientHistory(patId, patName, patCode);
        dialog.exec();
    };

    if (m_btnHistory) {
        connect(m_btnHistory, &QPushButton::clicked, this, openHistoryAction);
    }
    if (m_subSummaryResults) {
        connect(m_subSummaryResults, &QPushButton::clicked, this, openHistoryAction);
    }


    // Tính toán BMI tự động
    auto onBmiInputChanged = [this]() {
        updateBmi();
    };
    connect(m_txtWeight, &QLineEdit::textChanged, this, onBmiInputChanged);
    connect(m_txtHeight, &QLineEdit::textChanged, this, onBmiInputChanged);

    // Kết nối tín hiệu validate thời gian thực
    connect(m_txtTemp, &QLineEdit::textChanged, this, &ClinicalExamWidget::validateTemperatureInput);
    connect(m_txtPulse, &QLineEdit::textChanged, this, &ClinicalExamWidget::validateHeartRateInput);
    connect(m_txtWeight, &QLineEdit::textChanged, this, &ClinicalExamWidget::validateWeightInput);
    connect(m_txtHeight, &QLineEdit::textChanged, this, &ClinicalExamWidget::validateHeightInput);
    connect(m_txtReason, &QTextEdit::textChanged, this, &ClinicalExamWidget::validateChiefComplaintInput);
    connect(m_cbDiagnosis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ClinicalExamWidget::validateDiagnosisInput);
    connect(m_txtMainDisease, &QLineEdit::textChanged, this, &ClinicalExamWidget::validateDiagnosisInput);
}

void ClinicalExamWidget::setServices(std::shared_ptr<PharmacyService> pharmacyService, std::shared_ptr<PatientService> patientService, std::shared_ptr<AppointmentService> appointmentService) {
    m_pharmacyService = pharmacyService;
    m_patientService = patientService;
    m_appointmentService = appointmentService;
}

/**
 * @brief Hàm điều phối chính: Khởi tạo các vùng giao diện của màn hình Khám lâm sàng.
 */
void ClinicalExamWidget::setupUi() {
    this->setStyleSheet("background-color: #F3F4F6; font-family: 'Segoe UI';");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 16, 16);
    mainLayout->setSpacing(12);

    // 1. Thêm thanh Tab điều hướng phía trên
    mainLayout->addLayout(setupTopTabBar());

    // 2. Thêm thẻ thông tin bệnh nhân & các nút thao tác nhanh
    mainLayout->addWidget(setupPatientInfoCard());

    // 3. Khởi tạo Không gian làm việc chính (Workspace) gồm 3 cột
    QHBoxLayout* workspaceLayout = new QHBoxLayout();
    workspaceLayout->setSpacing(12);

    workspaceLayout->addWidget(setupSubSidebar(), 2);          // Cột trái: Menu khám bệnh con
    workspaceLayout->addWidget(setupMainExamForm(), 5);        // Cột giữa: Form nhập liệu khám chính
    workspaceLayout->addWidget(setupMedicalHistoryPanel(), 3); // Cột phải: Panel ghi chú chuyên môn & tiền sử

    mainLayout->addLayout(workspaceLayout, 1);
}

/**
 * @brief Khởi tạo Thanh Tab chuyển đổi các phân phân hệ (Danh sách, Khám lâm sàng, Đăng ký, Thu tiền).
 */
QHBoxLayout* ClinicalExamWidget::setupTopTabBar() {
    QHBoxLayout* topTabsLayout = new QHBoxLayout();
    topTabsLayout->setSpacing(4);

    m_tabAppointmentsList = new QPushButton("DANH SÁCH", this);
    m_tabClinicalExam = new QPushButton("KHÁM LÂM SÀNG", this);
    m_tabRegistration = new QPushButton("ĐĂNG KÝ KHÁM", this);
    m_tabBilling = new QPushButton("THU TIỀN", this);

    QPushButton* tabs[] = { m_tabAppointmentsList, m_tabClinicalExam, m_tabRegistration, m_tabBilling };
    for (auto* tab : tabs) {
        tab->setCursor(Qt::PointingHandCursor);
        tab->setFixedHeight(36);
        if (tab == m_tabClinicalExam) {
            tab->setStyleSheet(
                "QPushButton { background-color: #007A7E; color: white; border: none; "
                "font-size: 13px; font-weight: bold; border-top-left-radius: 6px; border-top-right-radius: 6px; padding: 0 20px; }"
            );
        } else {
            tab->setStyleSheet(
                "QPushButton { background-color: #E5E7EB; color: #4B5563; border: 1px solid #D1D5DB; border-bottom: none; "
                "font-size: 13px; font-weight: 600; border-top-left-radius: 6px; border-top-right-radius: 6px; padding: 0 20px; }"
                "QPushButton:hover { background-color: #DBEAFE; color: #1E40AF; }"
            );
        }
        topTabsLayout->addWidget(tab);
    }
    topTabsLayout->addStretch();
    return topTabsLayout;
}

/**
 * @brief Khởi tạo Thẻ Thông tin Bệnh nhân (Tên, Mã, Tuổi, Giới tính) và Thanh Nút thao tác nhanh của Bác sĩ.
 */
QFrame* ClinicalExamWidget::setupPatientInfoCard() {
    QFrame* infoCard = new QFrame(this);
    infoCard->setObjectName("InfoCard");
    infoCard->setStyleSheet("QFrame#InfoCard { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 8px; }");
    QVBoxLayout* infoCardLayout = new QVBoxLayout(infoCard);
    infoCardLayout->setContentsMargins(16, 12, 16, 12);
    infoCardLayout->setSpacing(8);

    QHBoxLayout* topInfoRow = new QHBoxLayout();

    // -- Khối thông tin định danh bệnh nhân --
    QHBoxLayout* patDetailsLayout = new QHBoxLayout();
    patDetailsLayout->setSpacing(16);

    QLabel* avatarIcon = new QLabel("👤", infoCard);
    avatarIcon->setStyleSheet("font-size: 32px; color: #007A7E; background: transparent;");
    patDetailsLayout->addWidget(avatarIcon);

    QVBoxLayout* detailsText = new QVBoxLayout();
    detailsText->setSpacing(4);
    QHBoxLayout* detLine1 = new QHBoxLayout();
    QLabel* lblCodeTitle = new QLabel("Mã BN:", infoCard);
    lblCodeTitle->setStyleSheet("color: #6B7280; font-size: 12px; font-weight: 600; background: transparent;");
    m_lblPatientCodeVal = new QLabel("BN0000000059", infoCard);
    m_lblPatientCodeVal->setStyleSheet("color: #111827; font-size: 12px; font-weight: bold; background: transparent;");
    
    QLabel* lblNameTitle = new QLabel("Họ Tên:", infoCard);
    lblNameTitle->setStyleSheet("color: #6B7280; font-size: 12px; font-weight: 600; background: transparent;");
    m_lblPatientNameVal = new QLabel("HOÀNG AN BÌNH", infoCard);
    m_lblPatientNameVal->setStyleSheet("color: #DC2626; font-size: 14px; font-weight: bold; background: transparent;");

    detLine1->addWidget(lblCodeTitle);
    detLine1->addWidget(m_lblPatientCodeVal);
    detLine1->addSpacing(16);
    detLine1->addWidget(lblNameTitle);
    detLine1->addWidget(m_lblPatientNameVal);
    detLine1->addStretch();

    QHBoxLayout* detLine2 = new QHBoxLayout();
    QLabel* lblDobTitle = new QLabel("Ngày sinh:", infoCard);
    lblDobTitle->setStyleSheet("color: #6B7280; font-size: 12px; background: transparent;");
    m_lblPatientDobVal = new QLabel("01/01/1990", infoCard);
    m_lblPatientDobVal->setStyleSheet("color: #111827; font-size: 12px; font-weight: 600; background: transparent;");

    QLabel* lblGenderTitle = new QLabel("GT:", infoCard);
    lblGenderTitle->setStyleSheet("color: #6B7280; font-size: 12px; background: transparent;");
    m_lblPatientGenderVal = new QLabel("Nữ", infoCard);
    m_lblPatientGenderVal->setStyleSheet("color: #111827; font-size: 12px; font-weight: 600; background: transparent;");

    QLabel* lblAgeTitle = new QLabel("Tuổi:", infoCard);
    lblAgeTitle->setStyleSheet("color: #6B7280; font-size: 12px; background: transparent;");
    m_lblPatientAgeVal = new QLabel("36 tuổi", infoCard);
    m_lblPatientAgeVal->setStyleSheet("color: #111827; font-size: 12px; font-weight: 600; background: transparent;");

    detLine2->addWidget(lblDobTitle);
    detLine2->addWidget(m_lblPatientDobVal);
    detLine2->addSpacing(16);
    detLine2->addWidget(lblGenderTitle);
    detLine2->addWidget(m_lblPatientGenderVal);
    detLine2->addSpacing(16);
    detLine2->addWidget(lblAgeTitle);
    detLine2->addWidget(m_lblPatientAgeVal);
    detLine2->addStretch();

    detailsText->addLayout(detLine1);
    detailsText->addLayout(detLine2);
    patDetailsLayout->addLayout(detailsText);

    // -- Khối nút thao tác nhanh bên phải --
    QHBoxLayout* actionButtonsLayout = new QHBoxLayout();
    actionButtonsLayout->setSpacing(6);

    m_btnNew = new QPushButton("➕ THÊM MỚI", infoCard);
    m_btnSave = new QPushButton("💾 LƯU (F4)", infoCard);
    m_btnFinish = new QPushButton("🟢 KẾT THÚC KHÁM", infoCard);
    m_btnCancel = new QPushButton("🔴 HỦY KHÁM", infoCard);
    m_btnHistory = new QPushButton("⏳ LỊCH SỬ", infoCard);
    m_btnCallPatient = new QPushButton("📢 GỌI KHÁM", infoCard);
    m_btnPrint = new QPushButton("🖨️ IN PHIẾU", infoCard);

    QPushButton* actionBtns[] = { m_btnNew, m_btnSave, m_btnFinish, m_btnCancel, m_btnHistory, m_btnCallPatient, m_btnPrint };
    QString colors[] = { "#4B94F2", "#2563EB", "#059669", "#DC2626", "#4B5563", "#7C3AED", "#0891B2" };

    for (int i = 0; i < 7; ++i) {
        actionBtns[i]->setCursor(Qt::PointingHandCursor);
        actionBtns[i]->setFixedHeight(32);
        actionBtns[i]->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border: none; font-size: 11px; font-weight: bold; border-radius: 6px; padding: 0 10px; }"
            "QPushButton:hover { background-color: black; }"
        ).arg(colors[i]));
        actionButtonsLayout->addWidget(actionBtns[i]);
    }

    topInfoRow->addLayout(patDetailsLayout, 4);
    topInfoRow->addLayout(actionButtonsLayout, 6);
    infoCardLayout->addLayout(topInfoRow);

    // -- Thống kê Metadata phụ phía dưới --
    infoCardLayout->addWidget(createSeparator());
    QHBoxLayout* metaRow = new QHBoxLayout();
    metaRow->setSpacing(24);
    QLabel* m1 = new QLabel("🏥 Phòng khám: Phòng khám nội nhi", infoCard);
    QLabel* m2 = new QLabel("🕓 Bắt đầu: 13:00", infoCard);
    QLabel* m3 = new QLabel("📋 Bệnh chính: --", infoCard);
    QLabel* m4 = new QLabel("📋 Bệnh phụ: --", infoCard);

    QLabel* metas[] = { m1, m2, m3, m4 };
    for (auto* m : metas) {
        m->setStyleSheet("color: #4B5563; font-size: 11px; background: transparent; font-weight: 500;");
        metaRow->addWidget(m);
    }
    metaRow->addStretch();
    infoCardLayout->addLayout(metaRow);

    return infoCard;
}

/**
 * @brief Khởi tạo Thanh Menu con bên trái (Chuyển đổi giữa Khám lâm sàng, Đơn thuốc, Cận lâm sàng...).
 */
QFrame* ClinicalExamWidget::setupSubSidebar() {
    QFrame* subSidebar = new QFrame(this);
    subSidebar->setObjectName("SubSidebar");
    subSidebar->setFixedWidth(180);
    subSidebar->setStyleSheet(
        "QFrame#SubSidebar { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 8px; }"
        "QPushButton { text-align: left; padding: 10px 14px; font-size: 12px; font-weight: 600; border: none; border-radius: 6px; color: #4B5563; background: transparent; }"
        "QPushButton:hover { background-color: #F3F4F6; color: #007A7E; }"
        "QPushButton#activeSub { background-color: #E0F2FE; color: #0369A1; font-weight: bold; }"
    );
    QVBoxLayout* subSidebarLayout = new QVBoxLayout(subSidebar);
    subSidebarLayout->setContentsMargins(8, 12, 8, 12);
    subSidebarLayout->setSpacing(4);

    QLabel* subHdr = new QLabel("KHÁM BỆNH", subSidebar);
    subHdr->setStyleSheet("font-size: 11px; font-weight: bold; color: #9CA3AF; margin-bottom: 8px; background: transparent; padding-left: 10px;");
    subSidebarLayout->addWidget(subHdr);

    m_subClinicalExam = new QPushButton("Khám lâm sàng", subSidebar);
    m_subClinicalExam->setObjectName("activeSub");
    m_subServiceOrder = new QPushButton("Chỉ định dịch vụ", subSidebar);
    m_subPrescription = new QPushButton("Đơn thuốc", subSidebar);
    m_subSummaryResults = new QPushButton("Kết quả khám tổng hợp", subSidebar);
    m_subSocialInsurance = new QPushButton("Thông tin nghỉ BHXH", subSidebar);
    m_subHospitalTransfer = new QPushButton("Thông tin chuyển viện", subSidebar);

    QPushButton* subs[] = { m_subClinicalExam, m_subServiceOrder, m_subPrescription, m_subSummaryResults, m_subSocialInsurance, m_subHospitalTransfer };
    for (auto* subBtn : subs) {
        subSidebarLayout->addWidget(subBtn);
    }
    subSidebarLayout->addStretch();
    return subSidebar;
}

/**
 * @brief Khởi tạo Form nhập liệu thông tin khám bệnh trung tâm (Sinh hiệu, Chỉ số BMI, Chẩn đoán, Hướng xử lý, Lời dặn).
 */
QWidget* ClinicalExamWidget::setupMainExamForm() {
    QFrame* mainForm = new QFrame(this);
    mainForm->setObjectName("MainForm");
    mainForm->setStyleSheet(
        "QFrame#MainForm { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 8px; }"
        "QLabel { font-size: 12px; font-weight: bold; color: #374151; border: none; background: transparent; }"
        "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; }"
        "QLineEdit:focus { border: 1px solid #007A7E; background-color: #F0FDFA; }"
        "QComboBox { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: white; }"
        "QComboBox:focus { border: 1px solid #007A7E; }"
    );
    QVBoxLayout* mainFormLayout = new QVBoxLayout(mainForm);
    mainFormLayout->setContentsMargins(16, 16, 16, 16);
    mainFormLayout->setSpacing(14);

    // -- Chọn mẫu khám --
    QHBoxLayout* templateRow = new QHBoxLayout();
    QLabel* lblTemplate = new QLabel("Chọn mẫu:", mainForm);
    m_cbTemplate = new QComboBox(mainForm);
    for (const auto& pair : ClinicalTemplateText::getList()) m_cbTemplate->addItem(pair.second, pair.first);
    m_cbTemplate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    templateRow->addWidget(lblTemplate);
    templateRow->addWidget(m_cbTemplate);
    mainFormLayout->addLayout(templateRow);

    // -- Khối các chỉ số sinh tồn (Vital Signs) --
    QFrame* vitalBox = new QFrame(mainForm);
    vitalBox->setObjectName("VitalBox");
    vitalBox->setStyleSheet("QFrame#VitalBox { background-color: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 8px; }"
                            "QLabel { border: none; background: transparent; }");
    QGridLayout* vitalGrid = new QGridLayout(vitalBox);
    vitalGrid->setContentsMargins(12, 12, 12, 12);
    vitalGrid->setSpacing(10);

    vitalGrid->addWidget(new QLabel("Cân nặng (kg)", vitalBox), 0, 0);
    m_txtWeight = new QLineEdit(vitalBox);
    m_txtWeight->setValidator(new QDoubleValidator(1, 200, 1, this));
    m_txtWeight->setPlaceholderText("50");
    vitalGrid->addWidget(m_txtWeight, 1, 0);

    vitalGrid->addWidget(new QLabel("Chiều cao (cm)", vitalBox), 0, 1);
    m_txtHeight = new QLineEdit(vitalBox);
    m_txtHeight->setValidator(new QDoubleValidator(50, 250, 1, this));
    m_txtHeight->setPlaceholderText("160");
    vitalGrid->addWidget(m_txtHeight, 1, 1);

    vitalGrid->addWidget(new QLabel("Nhiệt độ (°C)", vitalBox), 0, 2);
    m_txtTemp = new QLineEdit(vitalBox);
    m_txtTemp->setPlaceholderText("36.5");
    vitalGrid->addWidget(m_txtTemp, 1, 2);

    vitalGrid->addWidget(new QLabel("Mạch (lần/phút)", vitalBox), 0, 3);
    m_txtPulse = new QLineEdit(vitalBox);
    m_txtPulse->setPlaceholderText("80");
    vitalGrid->addWidget(m_txtPulse, 1, 3);

    vitalGrid->addWidget(new QLabel("Huyết áp (mmHg)", vitalBox), 0, 4);
    m_txtBp = new QLineEdit(vitalBox);
    m_txtBp->setPlaceholderText("120/80");
    vitalGrid->addWidget(m_txtBp, 1, 4);

    vitalGrid->addWidget(new QLabel("BMI", vitalBox), 0, 5);
    m_lblBmiVal = new QLabel("-- (Nhập số đo)", vitalBox);
    m_lblBmiVal->setStyleSheet("font-weight: bold; color: #1E3A8A; font-size: 12px;");
    vitalGrid->addWidget(m_lblBmiVal, 1, 5);

    mainFormLayout->addWidget(vitalBox);

    // -- Lý do khám & Chẩn đoán bệnh --
    QVBoxLayout* fieldsLayout = new QVBoxLayout();
    fieldsLayout->setSpacing(10);

    QHBoxLayout* fRow1 = new QHBoxLayout();
    QVBoxLayout* col1 = new QVBoxLayout();
    col1->addWidget(new QLabel("Lý do khám:", mainForm));
    m_txtReason = new QTextEdit(mainForm);
    m_txtReason->setStyleSheet("QTextEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; }");
    m_txtReason->setFixedHeight(48);
    col1->addWidget(m_txtReason);
    fRow1->addLayout(col1);
    fieldsLayout->addLayout(fRow1);

    QGridLayout* formGrid = new QGridLayout();
    formGrid->setSpacing(10);

    formGrid->addWidget(new QLabel("Chẩn đoán ban đầu:", mainForm), 0, 0);
    QHBoxLayout* diagRowLayout = new QHBoxLayout();
    diagRowLayout->setSpacing(6);
    m_cbDiagnosis = new QComboBox(mainForm);
    for (const auto& pair : DiagnosisText::getList()) m_cbDiagnosis->addItem(pair.second, pair.first);
    m_cbSeverity = new QComboBox(mainForm);
    for (const auto& pair : DiagnosisSeverityText::getList()) m_cbSeverity->addItem(pair.second, pair.first);
    diagRowLayout->addWidget(m_cbDiagnosis, 3);
    diagRowLayout->addWidget(m_cbSeverity, 1);
    formGrid->addLayout(diagRowLayout, 1, 0);

    formGrid->addWidget(new QLabel("Bệnh chính (ICD10):", mainForm), 0, 1);
    m_txtMainDisease = new QLineEdit(mainForm);
    m_txtMainDisease->setPlaceholderText("Ví dụ: K29 (Viêm dạ dày)");
    formGrid->addWidget(m_txtMainDisease, 1, 1);

    formGrid->addWidget(new QLabel("Hướng xử lý:", mainForm), 2, 0);
    m_cbDirection = new QComboBox(mainForm);
    for (const auto& pair : DirectionText::getList()) m_cbDirection->addItem(pair.second, pair.first);
    formGrid->addWidget(m_cbDirection, 3, 0);

    formGrid->addWidget(new QLabel("Xử trí cụ thể:", mainForm), 2, 1);
    m_cbAction = new QComboBox(mainForm);
    for (const auto& pair : ActionText::getList()) m_cbAction->addItem(pair.second, pair.first);
    formGrid->addWidget(m_cbAction, 3, 1);

    fieldsLayout->addLayout(formGrid);

    QVBoxLayout* advCol = new QVBoxLayout();
    advCol->addWidget(new QLabel("Lời dặn bác sĩ:", mainForm));
    m_txtAdvice = new QLineEdit(mainForm);
    m_txtAdvice->setPlaceholderText("Ví dụ: Uống thuốc sau ăn, tái khám đúng hẹn...");
    advCol->addWidget(m_txtAdvice);
    fieldsLayout->addLayout(advCol);

    mainFormLayout->addLayout(fieldsLayout);
    mainFormLayout->addStretch();
    return mainForm;
}

/**
 * @brief Khởi tạo Panel Hồ sơ Chuyên môn bên phải (Tiền sử bệnh, Bệnh sử, Khám toàn thân, Tóm tắt CLS).
 */
QFrame* ClinicalExamWidget::setupMedicalHistoryPanel() {
    QFrame* rightPanel = new QFrame(this);
    rightPanel->setObjectName("RightPanel");
    rightPanel->setStyleSheet(
        "QFrame#RightPanel { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 8px; }"
        "QLabel { font-size: 12px; font-weight: bold; color: #111827; border: none; background: transparent; }"
        "QTextEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #374151; }"
        "QTextEdit:focus { border: 1px solid #007A7E; }"
    );
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(14, 14, 14, 14);
    rightLayout->setSpacing(8);

    rightLayout->addWidget(new QLabel("Tiền sử bệnh án:", rightPanel));
    m_txtHistoryPersonal = new QTextEdit(rightPanel);
    m_txtHistoryPersonal->setPlaceholderText("Mô tả tiền sử bệnh lý của bệnh nhân và gia đình...");
    rightLayout->addWidget(m_txtHistoryPersonal);

    rightLayout->addWidget(new QLabel("Bệnh sử lâm sàng:", rightPanel));
    m_txtHistoryIllness = new QTextEdit(rightPanel);
    m_txtHistoryIllness->setPlaceholderText("Diễn biến bệnh lý gần đây...");
    rightLayout->addWidget(m_txtHistoryIllness);

    rightLayout->addWidget(new QLabel("Khám toàn thân & Cơ quan:", rightPanel));
    m_txtExamGeneral = new QTextEdit(rightPanel);
    m_txtExamGeneral->setPlaceholderText("Tình trạng hô hấp, tuần hoàn, tiêu hóa...");
    rightLayout->addWidget(m_txtExamGeneral);

    rightLayout->addWidget(new QLabel("Tóm tắt kết quả cận lâm sàng:", rightPanel));
    m_txtClsSummary = new QTextEdit(rightPanel);
    m_txtClsSummary->setPlaceholderText("Tóm tắt kết quả xét nghiệm máu, X-Quang, siêu âm...");
    rightLayout->addWidget(m_txtClsSummary);

    return rightPanel;
}

void ClinicalExamWidget::clearExamForm() {
    if (m_txtTemp) m_txtTemp->clear();
    if (m_txtBp) m_txtBp->clear();
    if (m_txtPulse) m_txtPulse->clear();
    if (m_txtWeight) m_txtWeight->clear();
    if (m_txtHeight) m_txtHeight->clear();
    if (m_txtReason) m_txtReason->clear();
    if (m_txtMainDisease) m_txtMainDisease->clear();
    if (m_txtSubDisease) m_txtSubDisease->clear();
    if (m_txtAdvice) m_txtAdvice->clear();
    if (m_txtHistoryIllness) m_txtHistoryIllness->clear();
    if (m_txtHistoryPersonal) m_txtHistoryPersonal->clear();
    if (m_txtExamGeneral) m_txtExamGeneral->clear();
    if (m_txtClsSummary) m_txtClsSummary->clear();
    if (m_lblBmiVal) m_lblBmiVal->setText("0.0");
    if (m_cbDiagnosis) m_cbDiagnosis->setCurrentIndex(0);
    if (m_cbSeverity) m_cbSeverity->setCurrentIndex(0);
}

void ClinicalExamWidget::loadPatientInfo(int patientId, int appointmentId, const QString& name, const QString& id, const QString& time, const QString& specialty) {
    clearExamForm();
    m_currentPatientId = patientId;
    m_currentAppointmentId = appointmentId;

    if (m_lblPatientNameVal) m_lblPatientNameVal->setText(name.toUpper());
    if (m_lblPatientCodeVal) m_lblPatientCodeVal->setText(id);
    
    // LƯU Ý / CẢNH BÁO: Tính năng tuổi (age) hiện tại đang sử dụng công thức giả lập (mockup calculation) dựa theo độ dài ID.
    // Giữ nguyên công thức tính theo yêu cầu, chỉ đặt comment lưu ý tại đây. Khi đưa vào sản xuất cần lấy chính xác từ dateOfBirth.
    int age = 30 + (id.length() % 25);
    if (m_lblPatientAgeVal) m_lblPatientAgeVal->setText(QString("%1 tuổi").arg(age));
    if (m_lblPatientGenderVal) m_lblPatientGenderVal->setText(age % 2 == 0 ? "Nam" : "Nữ");
    
    qDebug() << "Loaded patient info to clinical workspace:" << name << id << time << specialty << "patientId:" << patientId << "appointmentId:" << appointmentId;
}

void ClinicalExamWidget::loadPatientInfo(const PatientDetailDTO& patient, int appointmentId, const QString& time, const QString& specialty) {
    loadPatientInfo(patient.patientId, appointmentId, patient.fullName, patient.patientCode, time, specialty);
    if (!patient.gender.isEmpty() && m_lblPatientGenderVal) {
        m_lblPatientGenderVal->setText(GenderText::toVi(patient.gender));
    }
}

void ClinicalExamWidget::updateBmi() {
    double weight = m_txtWeight->text().toDouble();
    double heightCm = m_txtHeight->text().toDouble();

    if (weight > 0 && heightCm > 0) {
        double heightM = heightCm / 100.0;
        double bmi = weight / (heightM * heightM);
        
        QString assessment;
        if (bmi < 18.5) assessment = "Cân nặng thấp (gầy)";
        else if (bmi < 25.0) assessment = "Bình thường";
        else if (bmi < 30.0) assessment = "Tiền béo phì";
        else assessment = "Béo phì";

        m_lblBmiVal->setText(QString("%1 (%2)").arg(QString::number(bmi, 'f', 2), assessment));
        m_lblBmiVal->setStyleSheet(QString("font-weight: bold; color: %1;").arg(bmi < 25.0 ? "#059669" : "#DC2626"));
    } else {
        m_lblBmiVal->setText("-- (Nhập số đo)");
        m_lblBmiVal->setStyleSheet("font-weight: bold; color: #1E3A8A;");
    }
}

QFrame* ClinicalExamWidget::createSeparator() {
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #E5E7EB; border: none; max-height: 1px;");
    return line;
}

QList<Diagnosis> ClinicalExamWidget::getDiagnosesFromUi() const {
    QList<Diagnosis> list;
    QString diagnosisDesc = m_cbDiagnosis->currentIndex() > 0 ? m_cbDiagnosis->currentText() : "";
    QString mainDisease = m_txtMainDisease->text().trimmed();

    if (!diagnosisDesc.isEmpty() || !mainDisease.isEmpty()) {
        Diagnosis d;
        d.icdCode = mainDisease;
        d.description = !diagnosisDesc.isEmpty() ? diagnosisDesc : mainDisease;
        d.severity = m_cbSeverity ? DiagnosisSeverityText::toEn(m_cbSeverity->currentText()) : DiagnosisSeverityText::MODERATE;
        list.append(d);
    }
    return list;
}

void ClinicalExamWidget::validateTemperatureInput() {
    QString text = m_txtTemp->text().trimmed();
    static const QString normalLineEditStyle = "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";
    if (text.isEmpty()) {
        m_txtTemp->setStyleSheet(normalLineEditStyle);
        m_txtTemp->setToolTip("");
        return;
    }
    bool ok;
    double val = text.toDouble(&ok);
    QString err = "";
    if (!ok) {
        err = "Nhiệt độ phải là số thực hợp lệ.";
    } else {
        err = MedicalRecordService::validateTemperature(val);
    }

    if (!err.isEmpty()) {
        m_txtTemp->setStyleSheet("QLineEdit { border: 1px solid #DC2626; background-color: #FEF2F2; color: #111827; }");
        m_txtTemp->setToolTip(err);
    } else {
        m_txtTemp->setStyleSheet(normalLineEditStyle);
        m_txtTemp->setToolTip("");
    }
}

void ClinicalExamWidget::validateHeartRateInput() {
    QString text = m_txtPulse->text().trimmed();
    static const QString normalLineEditStyle = "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";
    if (text.isEmpty()) {
        m_txtPulse->setStyleSheet(normalLineEditStyle);
        m_txtPulse->setToolTip("");
        return;
    }
    bool ok;
    int val = text.toInt(&ok);
    QString err = "";
    if (!ok) {
        err = "Nhịp tim phải là số nguyên hợp lệ.";
    } else {
        err = MedicalRecordService::validateHeartRate(val);
    }

    if (!err.isEmpty()) {
        m_txtPulse->setStyleSheet("QLineEdit { border: 1px solid #DC2626; background-color: #FEF2F2; color: #111827; }");
        m_txtPulse->setToolTip(err);
    } else {
        m_txtPulse->setStyleSheet(normalLineEditStyle);
        m_txtPulse->setToolTip("");
    }
}

void ClinicalExamWidget::validateWeightInput() {
    QString text = m_txtWeight->text().trimmed();
    static const QString normalLineEditStyle = "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";
    if (text.isEmpty()) {
        m_txtWeight->setStyleSheet(normalLineEditStyle);
        m_txtWeight->setToolTip("");
        return;
    }
    bool ok;
    double val = text.toDouble(&ok);
    QString err = "";
    if (!ok) {
        err = "Cân nặng phải là số thực hợp lệ.";
    } else {
        err = MedicalRecordService::validateWeight(val);
    }

    if (!err.isEmpty()) {
        m_txtWeight->setStyleSheet("QLineEdit { border: 1px solid #DC2626; background-color: #FEF2F2; color: #111827; }");
        m_txtWeight->setToolTip(err);
    } else {
        m_txtWeight->setStyleSheet(normalLineEditStyle);
        m_txtWeight->setToolTip("");
    }
}

void ClinicalExamWidget::validateHeightInput() {
    QString text = m_txtHeight->text().trimmed();
    static const QString normalLineEditStyle = "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";
    if (text.isEmpty()) {
        m_txtHeight->setStyleSheet(normalLineEditStyle);
        m_txtHeight->setToolTip("");
        return;
    }
    bool ok;
    double val = text.toDouble(&ok);
    QString err = "";
    if (!ok) {
        err = "Chiều cao phải là số thực hợp lệ.";
    } else {
        err = MedicalRecordService::validateHeight(val);
    }

    if (!err.isEmpty()) {
        m_txtHeight->setStyleSheet("QLineEdit { border: 1px solid #DC2626; background-color: #FEF2F2; color: #111827; }");
        m_txtHeight->setToolTip(err);
    } else {
        m_txtHeight->setStyleSheet(normalLineEditStyle);
        m_txtHeight->setToolTip("");
    }
}

void ClinicalExamWidget::validateDiagnosisInput() {
    QList<Diagnosis> list = getDiagnosesFromUi();
    QString err = MedicalRecordService::validateDiagnosisList(list);
    static const QString normalLineEditStyle = "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";
    static const QString normalComboStyle = "QComboBox { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";
    if (!err.isEmpty()) {
        m_cbDiagnosis->setStyleSheet("QComboBox { border: 1px solid #DC2626; background-color: #FEF2F2; color: #111827; }");
        m_txtMainDisease->setStyleSheet("QLineEdit { border: 1px solid #DC2626; background-color: #FEF2F2; color: #111827; }");
        m_cbDiagnosis->setToolTip(err);
        m_txtMainDisease->setToolTip(err);
    } else {
        m_cbDiagnosis->setStyleSheet(normalComboStyle);
        m_txtMainDisease->setStyleSheet(normalLineEditStyle);
        m_cbDiagnosis->setToolTip("");
        m_txtMainDisease->setToolTip("");
    }
}

void ClinicalExamWidget::validateChiefComplaintInput() {
    QString text = m_txtReason->toPlainText().trimmed();
    QString err = Validation::validateTrimmedNotEmpty(text, "Lý do khám không được để trống.");
    static const QString normalTextEditStyle = "QTextEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";
    if (!err.isEmpty()) {
        m_txtReason->setStyleSheet("QTextEdit { border: 1px solid #DC2626; background-color: #FEF2F2; color: #111827; }");
        m_txtReason->setToolTip(err);
    } else {
        m_txtReason->setStyleSheet(normalTextEditStyle);
        m_txtReason->setToolTip("");
    }
}

void ClinicalExamWidget::onSaveClicked() {
    if (!m_medicalRecordService) {
        QMessageBox::critical(this, "Lỗi", "Chưa kết nối dịch vụ hồ sơ bệnh án (MedicalRecordService).");
        return;
    }

    // 1. Thu thập dữ liệu
    MedicalRecordInsertDTO dto;
    dto.patientId = m_currentPatientId;
    
    int doctorId = 1; // mặc định test mode
    if (UserSession::getInstance().isLoggedIn() && UserSession::getInstance().getCurrentAccount()) {
        doctorId = UserSession::getInstance().getCurrentAccount()->getAccountId();
    }
    dto.doctorId = doctorId;
    dto.appointmentId = m_currentAppointmentId;
    dto.visitDateTime = QDateTime::currentDateTime();

    dto.vitals.temperature = m_txtTemp->text().trimmed().toDouble();
    dto.vitals.bloodPressure = m_txtBp->text().trimmed();
    dto.vitals.heartRate = m_txtPulse->text().trimmed().toInt();
    dto.vitals.weight = m_txtWeight->text().trimmed().toDouble();
    dto.vitals.height = m_txtHeight->text().trimmed().toDouble();

    dto.chiefComplaint = m_txtReason->toPlainText().trimmed();
    dto.clinicalNotes = QString(
        "Bệnh sử: %1\n"
        "Tiền sử: %2\n"
        "Khám lâm sàng chung: %3\n"
        "Cận lâm sàng tóm tắt: %4"
    ).arg(
        m_txtHistoryIllness->toPlainText().trimmed(),
        m_txtHistoryPersonal->toPlainText().trimmed(),
        m_txtExamGeneral->toPlainText().trimmed(),
        m_txtClsSummary->toPlainText().trimmed()
    );
    dto.treatment = QString("Hướng xử lý: %1. Xử trí: %2. Lời dặn: %3")
        .arg(m_cbDirection->currentText(), m_cbAction->currentText(), m_txtAdvice->text().trimmed());
    
    dto.nextVisitDate = std::nullopt;
    dto.diagnoses = getDiagnosesFromUi();

    // 2. Chạy validate tổng (bằng cách gọi createMedicalRecord)
    QString err = m_medicalRecordService->createMedicalRecord(dto);
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Lỗi kiểm tra dữ liệu", err);
        
        // Cập nhật lại màu viền đỏ của các trường sai
        validateTemperatureInput();
        validateHeartRateInput();
        validateWeightInput();
        validateHeightInput();
        validateChiefComplaintInput();
        validateDiagnosisInput();
        return;
    }

    QMessageBox::information(this, "Thành công", "Đã lưu hồ sơ bệnh án thành công!");
    
    // Reset stylesheets
    static const QString lineEditStyle = "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";
    static const QString comboStyle = "QComboBox { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";
    static const QString textEditStyle = "QTextEdit { border: 1px solid #D1D5DB; border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; background-color: #FFFFFF; }";

    m_txtTemp->setStyleSheet(lineEditStyle);
    m_txtPulse->setStyleSheet(lineEditStyle);
    m_txtWeight->setStyleSheet(lineEditStyle);
    m_txtHeight->setStyleSheet(lineEditStyle);
    m_txtReason->setStyleSheet(textEditStyle);
    m_cbDiagnosis->setStyleSheet(comboStyle);
    m_txtMainDisease->setStyleSheet(lineEditStyle);
}