#include "ClinicalExamWidget.h"
#include <QDoubleValidator>
#include <QDebug>

ClinicalExamWidget::ClinicalExamWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();

    // Kết nối các tín hiệu chuyển hướng về Dashboard
    connect(m_tabDanhSach, &QPushButton::clicked, this, &ClinicalExamWidget::viewAppointmentsListRequested);
    connect(m_btnCancel, &QPushButton::clicked, this, &ClinicalExamWidget::backToDashboardRequested);
    connect(m_btnFinish, &QPushButton::clicked, this, &ClinicalExamWidget::finishExamRequested);

    // Tính toán BMI tự động
    auto onBmiInputChanged = [this]() {
        updateBmi();
    };
    connect(m_txtWeight, &QLineEdit::textChanged, this, onBmiInputChanged);
    connect(m_txtHeight, &QLineEdit::textChanged, this, onBmiInputChanged);
}

void ClinicalExamWidget::setupUi() {
    this->setStyleSheet("background-color: #F3F4F6; font-family: 'Segoe UI';");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 16, 16);
    mainLayout->setSpacing(12);

    // =========================================================================
    // 1. TOP TABS BAR (DANH SÁCH, KHÁM LÂM SÀNG, v.v.)
    // =========================================================================
    QHBoxLayout* topTabsLayout = new QHBoxLayout();
    topTabsLayout->setSpacing(4);

    m_tabDanhSach = new QPushButton("DANH SÁCH", this);
    m_tabKhamLamSang = new QPushButton("KHÁM LÂM SÀNG", this);
    m_tabDangKyKham = new QPushButton("ĐĂNG KÝ KHÁM", this);
    m_tabThuTien = new QPushButton("THU TIỀN", this);

    QPushButton* tabs[] = { m_tabDanhSach, m_tabKhamLamSang, m_tabDangKyKham, m_tabThuTien };
    for (auto* tab : tabs) {
        tab->setCursor(Qt::PointingHandCursor);
        tab->setFixedHeight(36);
        if (tab == m_tabKhamLamSang) {
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
    mainLayout->addLayout(topTabsLayout);

    // =========================================================================
    // 2. PATIENT INFO & QUICK ACTIONS PANEL
    // =========================================================================
    QFrame* infoCard = new QFrame(this);
    infoCard->setObjectName("InfoCard");
    infoCard->setStyleSheet("QFrame#InfoCard { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 8px; }");
    QVBoxLayout* infoCardLayout = new QVBoxLayout(infoCard);
    infoCardLayout->setContentsMargins(16, 12, 16, 12);
    infoCardLayout->setSpacing(8);

    QHBoxLayout* topInfoRow = new QHBoxLayout();

    // Khối thông tin bệnh nhân bên trái
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

    // Khối nút thao tác nhanh bên phải
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

    // Hàng Metadata nhỏ ở dưới
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

    mainLayout->addWidget(infoCard);

    // =========================================================================
    // 3. MAIN WORKSPACE (LEFT SUB-SIDEBAR + FORM + RIGHT TEXT FIELDS)
    // =========================================================================
    QHBoxLayout* workspaceLayout = new QHBoxLayout();
    workspaceLayout->setSpacing(12);

    // 3A. SUB-SIDEBAR TRÁI
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

    m_subKhamLamSang = new QPushButton("Khám lâm sàng", subSidebar);
    m_subKhamLamSang->setObjectName("activeSub");
    m_subChiDinhDichVu = new QPushButton("Chỉ định dịch vụ", subSidebar);
    m_subDonThuoc = new QPushButton("Đơn thuốc", subSidebar);
    m_subKetQuaTongHop = new QPushButton("Kết quả khám tổng hợp", subSidebar);
    m_subBhxh = new QPushButton("Thông tin nghỉ BHXH", subSidebar);
    m_subChuyenVien = new QPushButton("Thông tin chuyển viện", subSidebar);

    QPushButton* subs[] = { m_subKhamLamSang, m_subChiDinhDichVu, m_subDonThuoc, m_subKetQuaTongHop, m_subBhxh, m_subChuyenVien };
    for (auto* subBtn : subs) {
        subSidebarLayout->addWidget(subBtn);
    }
    subSidebarLayout->addStretch();
    workspaceLayout->addWidget(subSidebar);

    // 3B. FORM NHẬP LIỆU CHÍNH (GIỮA)
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

    // Chọn mẫu
    QHBoxLayout* templateRow = new QHBoxLayout();
    QLabel* lblTemplate = new QLabel("Chọn mẫu:", mainForm);
    m_cbTemplate = new QComboBox(mainForm);
    m_cbTemplate->addItem("--- Chọn mẫu khám ---");
    m_cbTemplate->addItem("Khám Nội nhi chuẩn");
    m_cbTemplate->addItem("Khám Tai Mũi Họng nhanh");
    m_cbTemplate->addItem("Kiểm tra sức khỏe định kỳ");
    m_cbTemplate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    templateRow->addWidget(lblTemplate);
    templateRow->addWidget(m_cbTemplate);
    mainFormLayout->addLayout(templateRow);

    // Khối các chỉ số sinh tồn (Chỉ số)
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

    // Lý do khám & chẩn đoán
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
    m_cbDiagnosis = new QComboBox(mainForm);
    m_cbDiagnosis->addItem("--- Chọn chẩn đoán ---");
    m_cbDiagnosis->addItem("Viêm dạ dày cấp tính");
    m_cbDiagnosis->addItem("Viêm họng hạt");
    m_cbDiagnosis->addItem("Suy nhược cơ thể nhẹ");
    formGrid->addWidget(m_cbDiagnosis, 1, 0);

    formGrid->addWidget(new QLabel("Bệnh chính (ICD10):", mainForm), 0, 1);
    m_txtMainDisease = new QLineEdit(mainForm);
    m_txtMainDisease->setPlaceholderText("Ví dụ: K29 (Viêm dạ dày)");
    formGrid->addWidget(m_txtMainDisease, 1, 1);

    formGrid->addWidget(new QLabel("Hướng xử lý:", mainForm), 2, 0);
    m_cbDirection = new QComboBox(mainForm);
    m_cbDirection->addItem("Cho về nhà, cấp toa thuốc");
    m_cbDirection->addItem("Nhập viện điều trị");
    m_cbDirection->addItem("Chuyển tuyến điều trị");
    formGrid->addWidget(m_cbDirection, 3, 0);

    formGrid->addWidget(new QLabel("Xử trí cụ thể:", mainForm), 2, 1);
    m_cbAction = new QComboBox(mainForm);
    m_cbAction->addItem("Nghỉ ngơi tại chỗ, uống thuốc theo đơn");
    m_cbAction->addItem("Thực hiện xét nghiệm bổ sung");
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
    workspaceLayout->addWidget(mainForm, 6);

    // 3C. PANEL HỒ SƠ CHUYÊN MÔN (PHẢI)
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

    workspaceLayout->addWidget(rightPanel, 4);

    mainLayout->addLayout(workspaceLayout, 1);
}

void ClinicalExamWidget::loadPatientInfo(const QString& name, const QString& id, const QString& time, const QString& specialty) {
    if (m_lblPatientNameVal) m_lblPatientNameVal->setText(name.toUpper());
    if (m_lblPatientCodeVal) m_lblPatientCodeVal->setText(id);
    
    // Tự động phân bổ tuổi/giới tính ngẫu nhiên để tăng tính sinh động của mockup
    int age = 30 + (id.length() % 25);
    if (m_lblPatientAgeVal) m_lblPatientAgeVal->setText(QString("%1 tuổi").arg(age));
    if (m_lblPatientGenderVal) m_lblPatientGenderVal->setText(age % 2 == 0 ? "Nam" : "Nữ");
    
    qDebug() << "Loaded patient info to clinical workspace:" << name << id << time << specialty;
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
