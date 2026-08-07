#include "ClinicalExamWidget.h"
#include "CreatePrescriptionDialog.h"
#include "CreateLabRequestDialog.h"
#include "PatientRecordHistoryDialog.h"
#include "service/PharmacyService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"
#include "service/UserSession.h"
#include "service/Validation.h"
#include <QDoubleValidator>
#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression>
#include "../../model/CommonEnums.h"

ClinicalExamWidget::ClinicalExamWidget(std::shared_ptr<MedicalRecordService> medicalRecordService, QWidget* parent)
    : QWidget(parent), m_medicalRecordService(medicalRecordService)
{
    setupUi();

    // Kết nối các tín hiệu chuyển hướng về Dashboard
    connect(m_btnCancel, &QPushButton::clicked, this, &ClinicalExamWidget::onCancelExamClicked);
    connect(m_btnFinish, &QPushButton::clicked, this, &ClinicalExamWidget::onFinishExamClicked);
    if (m_btnCallPatient) {
        connect(m_btnCallPatient, &QPushButton::clicked, this, [this]() {
            emit callPatientRequested(m_currentAppointmentId);
        });
    }

    if (m_btnSave) {
        connect(m_btnSave, &QPushButton::clicked, this, &ClinicalExamWidget::onSaveClicked);
    }

    auto openPrescriptionAction = [this]() {
        if (m_currentMedicalRecordId <= 0) {
            QMessageBox::warning(this, "Chưa lưu bệnh án", "Vui lòng 'Viết Hồ Sơ Bệnh Án' và lưu thành công trước khi kê đơn thuốc!");
            return;
        }
        CreatePrescriptionDialog dialog(m_pharmacyService, m_patientService, m_currentPatientId, this);
        dialog.setRecordId(QString::number(m_currentMedicalRecordId));
        int doctorId = 1;
        QString doctorName = "Bác sĩ";
        if (UserSession::getInstance().isLoggedIn() && UserSession::getInstance().getCurrentAccount()) {
            doctorId = UserSession::getInstance().getCurrentAccount()->getAccountId();
            doctorName = UserSession::getInstance().getCurrentAccount()->getFullName();
        }
        dialog.setDoctorId(QString::number(doctorId));
        dialog.setDoctorName(doctorName);
        dialog.exec();
    };

    if (m_btnPrescription) connect(m_btnPrescription, &QPushButton::clicked, this, openPrescriptionAction);

    if (m_btnServiceOrder) connect(m_btnServiceOrder, &QPushButton::clicked, this, &ClinicalExamWidget::openLabRequestDialog);

    auto openHistoryAction = [this]() {
        PatientRecordHistoryDialog dialog(m_pharmacyService, m_medicalRecordService, this);
        QString patCode = m_lblPatientCodeVal ? m_lblPatientCodeVal->text() : "";
        QString patName = m_lblPatientNameVal ? m_lblPatientNameVal->text() : "";
        int patId = patCode.startsWith("BN") ? patCode.mid(2).toInt() : 0;
        dialog.loadPatientHistory(patId, patName, patCode);
        dialog.exec();
    };

    if (m_btnHistory) {
        connect(m_btnHistory, &QPushButton::clicked, this, openHistoryAction);
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

void ClinicalExamWidget::setServices(
    std::shared_ptr<PharmacyService> pharmacyService, 
    std::shared_ptr<PatientService> patientService, 
    std::shared_ptr<AppointmentService> appointmentService,
    std::shared_ptr<ServiceRequestService> serviceRequestService) {
    m_pharmacyService = pharmacyService;
    m_patientService = patientService;
    m_appointmentService = appointmentService;
    m_serviceRequestService = serviceRequestService;
}

void ClinicalExamWidget::openLabRequestDialog() {
    if (m_currentMedicalRecordId <= 0) {
        QMessageBox::warning(this, "Chưa lưu bệnh án", "Vui lòng 'Viết Hồ Sơ Bệnh Án' và lưu thành công trước khi gửi yêu cầu xét nghiệm!");
        return;
    }

    int doctorId = 1;
    QString doctorName = "Bác sĩ";
    if (UserSession::getInstance().isLoggedIn() && UserSession::getInstance().getCurrentAccount()) {
        doctorId = UserSession::getInstance().getCurrentAccount()->getAccountId();
        doctorName = UserSession::getInstance().getCurrentAccount()->getFullName();
    }

    QString patName = m_lblPatientNameVal ? m_lblPatientNameVal->text() : "Bệnh nhân";
    QString patCode = m_lblPatientCodeVal ? m_lblPatientCodeVal->text() : "";

    CreateLabRequestDialog dialog(
        m_serviceRequestService,
        m_appointmentService,
        m_currentMedicalRecordId,
        doctorId,
        doctorName,
        patName,
        patCode,
        this
    );

    if (dialog.exec() == QDialog::Accepted) {
        loadLabResults();
    }
}

void ClinicalExamWidget::loadLabResults() {
    if (!m_serviceResultsLayout) return;

    // Clear old result boxes and labels
    QLayoutItem* item;
    while ((item = m_serviceResultsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }
    m_serviceResultBoxes.clear();

    if (!m_serviceRequestService || m_currentMedicalRecordId <= 0) return;

    auto requests = m_serviceRequestService->getRequestsByRoom(-1, "");
    QString summaryText;

    for (const auto &req : requests) {
        if (req.recordId == m_currentMedicalRecordId) {
            // Add title label
            QString title = QString("[%1] %2 — Trạng thái: %3")
                .arg(req.serviceName, req.roomName, serviceRequestStatusToVi(req.status));
            QLabel* lbl = new QLabel(title, this);
            lbl->setStyleSheet("font-size: 12px; font-weight: bold; color: #374151; background: transparent; margin-top: 4px;");
            m_serviceResultsLayout->addWidget(lbl);

            // Add result text box
            QTextEdit* box = new QTextEdit(this);
            box->setReadOnly(true);
            box->setFixedHeight(68);
            box->setPlainText(req.resultNote.isEmpty() ? "Chưa có kết quả." : req.resultNote);
            box->setStyleSheet(
                "QTextEdit { background: transparent; border: 1px solid #D1D5DB; "
                "border-radius: 6px; padding: 6px 10px; font-size: 12px; color: #111827; }"
            );
            m_serviceResultsLayout->addWidget(box);
            m_serviceResultBoxes.append(box);

            summaryText += QString("[%1] %2 - Trạng thái: %3\n")
                .arg(req.roomName, req.serviceName, serviceRequestStatusToVi(req.status));
            if (!req.resultNote.isEmpty()) {
                summaryText += QString("   -> Kết quả: %1\n").arg(req.resultNote);
            }
            summaryText += "\n";
        }
    }

    if (m_txtClsSummary && !summaryText.isEmpty()) {
        m_txtClsSummary->setText(summaryText);
    }
}

void ClinicalExamWidget::setupUi() {
    this->setStyleSheet("background-color: #F3F4F6; font-family: 'Segoe UI';");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 16, 16);
    mainLayout->setSpacing(12);

    mainLayout->addWidget(setupPatientInfoCard());

    QHBoxLayout* workspaceLayout = new QHBoxLayout();
    workspaceLayout->setSpacing(12);

    workspaceLayout->addWidget(setupMainExamForm(), 6);        
    workspaceLayout->addWidget(setupMedicalHistoryPanel(), 4); 

    mainLayout->addLayout(workspaceLayout, 1);
}



QFrame* ClinicalExamWidget::setupPatientInfoCard() {
    QFrame* infoCard = new QFrame(this);
    infoCard->setObjectName("InfoCard");
    infoCard->setStyleSheet("QFrame#InfoCard { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 8px; }");
    QVBoxLayout* infoCardLayout = new QVBoxLayout(infoCard);
    infoCardLayout->setContentsMargins(16, 12, 16, 12);
    infoCardLayout->setSpacing(8);

    QHBoxLayout* topInfoRow = new QHBoxLayout();

    QHBoxLayout* patDetailsLayout = new QHBoxLayout();
    patDetailsLayout->setSpacing(16);

    QLabel* avatarIcon = new QLabel("BN", infoCard);
    avatarIcon->setStyleSheet("font-size: 16px; font-weight: bold; color: #2563EB; background: #EFF6FF; border-radius: 18px; padding: 8px;");
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

    QHBoxLayout* actionButtonsLayout = new QHBoxLayout();
    actionButtonsLayout->setSpacing(6);

    m_btnSave = new QPushButton("Lưu Hồ Sơ Bệnh Án", infoCard);
    m_btnPrescription = new QPushButton("Kê Đơn Thuốc", infoCard);
    m_btnServiceOrder = new QPushButton("Yêu Cầu Xét Nghiệm", infoCard);
    m_btnFinish = new QPushButton("Hoàn Tất Khám", infoCard);
    m_btnCancel = new QPushButton("Hủy Khám", infoCard);
    m_btnHistory = new QPushButton("Lịch Sử Khám", infoCard);

    QPushButton* actionBtns[] = { m_btnSave, m_btnPrescription, m_btnServiceOrder, m_btnFinish, m_btnCancel, m_btnHistory };
    QString colors[] = { "#2563EB", "#7C3AED", "#0891B2", "#059669", "#DC2626", "#4B5563" };

    for (int i = 0; i < 6; ++i) {
        actionBtns[i]->setCursor(Qt::PointingHandCursor);
        actionBtns[i]->setFixedHeight(34);
        actionBtns[i]->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border: none; font-size: 11px; font-weight: bold; border-radius: 6px; padding: 0 12px; }"
            "QPushButton:hover { opacity: 0.9; background-color: #1E293B; }"
        ).arg(colors[i]));
        actionButtonsLayout->addWidget(actionBtns[i]);
    }

    topInfoRow->addLayout(patDetailsLayout, 4);
    topInfoRow->addLayout(actionButtonsLayout, 6);
    infoCardLayout->addLayout(topInfoRow);

    infoCardLayout->addWidget(createSeparator());
    QHBoxLayout* metaRow = new QHBoxLayout();
    metaRow->setSpacing(24);
    QLabel* m1 = new QLabel("Phòng khám: Phòng khám nội nhi", infoCard);
    QLabel* m2 = new QLabel("Bắt đầu: 13:00", infoCard);
    QLabel* m3 = new QLabel("Bệnh chính: --", infoCard);
    QLabel* m4 = new QLabel("Bệnh phụ: --", infoCard);

    QLabel* metas[] = { m1, m2, m3, m4 };
    for (auto* m : metas) {
        m->setStyleSheet("color: #4B5563; font-size: 11px; background: transparent; font-weight: 500;");
        metaRow->addWidget(m);
    }
    metaRow->addStretch();
    infoCardLayout->addLayout(metaRow);

    return infoCard;
}



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

    QHBoxLayout* templateRow = new QHBoxLayout();
    QLabel* lblTemplate = new QLabel("Chọn mẫu:", mainForm);
    m_cbTemplate = new QComboBox(mainForm);
    for (const auto& pair : ClinicalTemplateText::getList()) m_cbTemplate->addItem(pair.second, pair.first);
    m_cbTemplate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    templateRow->addWidget(lblTemplate);
    templateRow->addWidget(m_cbTemplate);
    mainFormLayout->addLayout(templateRow);

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

    // --- SERVICE REQUEST RESULTS SECTION ---
    QLabel* lblResults = new QLabel("Kết quả xét nghiệm lâm sàng:", mainForm);
    lblResults->setStyleSheet("font-size: 12px; font-weight: bold; color: #111827; margin-top: 6px;");
    fieldsLayout->addWidget(lblResults);

    QWidget* resultsContainer = new QWidget(mainForm);
    m_serviceResultsLayout = new QVBoxLayout(resultsContainer);
    m_serviceResultsLayout->setContentsMargins(0, 0, 0, 0);
    m_serviceResultsLayout->setSpacing(6);
    fieldsLayout->addWidget(resultsContainer);

    mainFormLayout->addLayout(fieldsLayout);
    mainFormLayout->addStretch();
    return mainForm;
}

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
    m_currentMedicalRecordId = 0;

    if (m_lblPatientNameVal) m_lblPatientNameVal->setText(name.toUpper());
    if (m_lblPatientCodeVal) m_lblPatientCodeVal->setText(id);
    
    int age = 30 + (id.length() % 25);
    if (m_lblPatientAgeVal) m_lblPatientAgeVal->setText(QString("%1 tuổi").arg(age));
    if (m_lblPatientGenderVal) m_lblPatientGenderVal->setText(age % 2 == 0 ? "Nam" : "Nữ");
    
    if (m_patientService && patientId > 0) {
        auto patientOpt = m_patientService->getPatientById(patientId);
        if (patientOpt.has_value()) {
            const auto& pat = patientOpt.value();
            if (!pat.gender.isEmpty() && m_lblPatientGenderVal) {
                m_lblPatientGenderVal->setText(GenderText::toVi(pat.gender));
            }
            if (pat.dateOfBirth.isValid() && m_lblPatientDobVal) {
                m_lblPatientDobVal->setText(pat.dateOfBirth.toString("dd/MM/yyyy"));
                int realAge = pat.dateOfBirth.daysTo(QDate::currentDate()) / 365;
                if (realAge >= 0 && m_lblPatientAgeVal) {
                    m_lblPatientAgeVal->setText(QString("%1 tuổi").arg(realAge));
                }
            }
        }
    }
    
    if (m_medicalRecordService && appointmentId > 0) {
        auto recordOpt = m_medicalRecordService->getLatestRecordByAppointmentId(appointmentId);
        if (recordOpt.has_value()) {
            const auto& rec = recordOpt.value();
            m_currentMedicalRecordId = rec.recordId;
            
            if (m_txtTemp) m_txtTemp->setText(QString::number(rec.vitals.temperature));
            if (m_txtBp) m_txtBp->setText(rec.vitals.bloodPressure);
            if (m_txtPulse) m_txtPulse->setText(QString::number(rec.vitals.heartRate));
            if (m_txtWeight) m_txtWeight->setText(QString::number(rec.vitals.weight));
            if (m_txtHeight) m_txtHeight->setText(QString::number(rec.vitals.height));
            
            if (m_txtReason) m_txtReason->setText(rec.chiefComplaint);
            
            QRegularExpression rxNotes("Bệnh sử: (.*?)\\nTiền sử: (.*?)\\nKhám lâm sàng chung: (.*?)\\nCận lâm sàng tóm tắt: (.*)", QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch matchNotes = rxNotes.match(rec.clinicalNotes);
            if (matchNotes.hasMatch()) {
                if (m_txtHistoryIllness) m_txtHistoryIllness->setText(matchNotes.captured(1).trimmed());
                if (m_txtHistoryPersonal) m_txtHistoryPersonal->setText(matchNotes.captured(2).trimmed());
                if (m_txtExamGeneral) m_txtExamGeneral->setText(matchNotes.captured(3).trimmed());
                if (m_txtClsSummary) m_txtClsSummary->setText(matchNotes.captured(4).trimmed());
            }

            QRegularExpression rxTreatment("Hướng xử lý: (.*?)\\. Xử trí: (.*?)\\. Lời dặn: (.*)", QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch matchTreatment = rxTreatment.match(rec.treatment);
            if (matchTreatment.hasMatch()) {
                if (m_cbDirection) m_cbDirection->setCurrentText(matchTreatment.captured(1).trimmed());
                if (m_cbAction) m_cbAction->setCurrentText(matchTreatment.captured(2).trimmed());
                if (m_txtAdvice) m_txtAdvice->setText(matchTreatment.captured(3).trimmed());
            }

            if (!rec.diagnoses.isEmpty()) {
                const auto& d = rec.diagnoses.first();
                if (m_txtMainDisease) m_txtMainDisease->setText(d.icdCode);
                if (m_cbDiagnosis) m_cbDiagnosis->setCurrentText(d.description);
                if (m_cbSeverity) m_cbSeverity->setCurrentText(DiagnosisSeverityText::toVi(d.severity));
            }
        }
    }
    
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

    MedicalRecordInsertDTO dto;
    dto.patientId = m_currentPatientId;
    
    int doctorId = 1; 
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

    QString err;
    if (m_currentMedicalRecordId > 0) {
        MedicalRecordUpdateDTO updateDto;
        updateDto.recordId = m_currentMedicalRecordId;
        updateDto.doctorId = dto.doctorId;
        updateDto.appointmentId = dto.appointmentId;
        updateDto.visitDateTime = dto.visitDateTime;
        updateDto.vitals = dto.vitals;
        updateDto.chiefComplaint = dto.chiefComplaint;
        updateDto.clinicalNotes = dto.clinicalNotes;
        updateDto.treatment = dto.treatment;
        updateDto.nextVisitDate = dto.nextVisitDate;
        updateDto.diagnoses = dto.diagnoses;
        
        err = m_medicalRecordService->updateMedicalRecord(updateDto);
    } else {
        err = m_medicalRecordService->createMedicalRecord(dto, &m_currentMedicalRecordId);
    }
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Lỗi kiểm tra dữ liệu", err);
        
        validateTemperatureInput();
        validateHeartRateInput();
        validateWeightInput();
        validateHeightInput();
        validateChiefComplaintInput();
        validateDiagnosisInput();
        return;
    }

    QMessageBox::information(this, "Thành công", "Đã lưu hồ sơ bệnh án thành công!");
    loadLabResults();
    
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

void ClinicalExamWidget::onFinishExamClicked() {
    if (m_currentMedicalRecordId <= 0) {
        QMessageBox::warning(this, "Chưa lưu bệnh án", "Hồ sơ bệnh án chưa được khởi tạo hoặc lưu. Vui lòng nhấn 'Viết Hồ Sơ Bệnh Án' để lưu trước khi hoàn tất khám!");
        return;
    }
    emit finishExamRequested();
}

void ClinicalExamWidget::onCancelExamClicked() {
    if (m_currentAppointmentId <= 0) {
        emit backToDashboardRequested();
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Xác nhận hủy",
                                  "Bạn có chắc chắn muốn hủy cuộc hẹn này không?\n\nTrạng thái cuộc hẹn sẽ được cập nhật thành 'Đã hủy' (CANCELLED).",
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (m_appointmentService) {
            m_appointmentService->updateAppointmentStatus(m_currentAppointmentId, AppointmentStatusText::CANCELLED);
            QMessageBox::information(this, "Thành công", "Đã hủy cuộc hẹn thành công.");
        } else {
            QMessageBox::warning(this, "Lỗi", "Không thể kết nối với dịch vụ cuộc hẹn.");
        }
        emit backToDashboardRequested();
    }
}