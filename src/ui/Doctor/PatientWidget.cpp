// PatientWidget.cpp
#include "PatientWidget.h"
#include "ui/Pharmacy/AddMedicationDialog.h"
#include "ui/Doctor/CreatePrescriptionDialog.h"

PatientWidget::PatientWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void PatientWidget::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_mainStackedWidget = new QStackedWidget(this);
    m_mainLayout->addWidget(m_mainStackedWidget);

    createListView();
    createDetailView();

    m_mainStackedWidget->addWidget(m_listViewWidget);
    m_mainStackedWidget->addWidget(m_detailViewWidget);

    m_mainStackedWidget->setCurrentWidget(m_listViewWidget);
}

void PatientWidget::createListView() {
    m_listViewWidget = new QWidget(this);
    QVBoxLayout* listLayout = new QVBoxLayout(m_listViewWidget);
    listLayout->setContentsMargins(16, 12, 16, 16);
    listLayout->setSpacing(16);

    QHBoxLayout* topTabsLayout = new QHBoxLayout();
    topTabsLayout->setSpacing(4);

    m_patientTabBar = new QTabBar(m_listViewWidget);
    m_patientTabBar->addTab("Bệnh nhân nội trú");
    m_patientTabBar->addTab("Bệnh nhân ngoại trú");
    m_patientTabBar->addTab("Bệnh nhân cấp cứu");
    m_patientTabBar->setCursor(Qt::PointingHandCursor);
    m_patientTabBar->setStyleSheet(
        "QTabBar::tab { background-color: #F1F5F9; color: #475569; border: 1px solid #CBD5E1; border-bottom: none; font-size: 13px; font-weight: 600; border-top-left-radius: 6px; border-top-right-radius: 6px; padding: 8px 20px; }"
        "QTabBar::tab:selected { background-color: #0284C7; color: white; border: none; font-weight: bold; }"
        "QTabBar::tab:hover:!selected { background-color: #E0F2FE; color: #0369A1; }"
    );
    topTabsLayout->addWidget(m_patientTabBar);
    topTabsLayout->addStretch();
    listLayout->addLayout(topTabsLayout);

    QScrollArea* scrollArea = new QScrollArea(m_listViewWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: #F8FAFC; width: 6px; border-radius: 3px; }"
        "QScrollBar::handle:vertical { background: #CBD5E1; border-radius: 3px; min-height: 30px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 12, 10);
    scrollLayout->setSpacing(20);
    scrollArea->setWidget(scrollContent);

    QHBoxLayout* cardRow = new QHBoxLayout();
    cardRow->setSpacing(16);

    struct CardInfo { QString icon, title, value, sub, color, bgLight; };
    QList<CardInfo> cards = {
        { "👥", "Tổng bệnh nhân",      "1,248",       "Tất cả hồ sơ hệ thống",  "#0284C7", "#E0F2FE" },
        { "⏳", "Đang chờ khám",       "18",          "Bệnh nhân tại sảnh",    "#D97706", "#FFFBEB" },
        { "🟢", "Đã hoàn thành",      "45",          "Trong ngày hôm nay",     "#059669", "#ECFDF5" },
        { "🚨", "Cấp cứu / Khẩn cấp",  "2",           "Cần xử lý ngay",         "#DC2626", "#FEF2F2" }
    };

    for (const auto& info : cards) {
        QFrame* card = new QFrame(scrollContent);
        card->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 14px; }");
        auto* shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(16); shadow->setOffset(0, 2); shadow->setColor(QColor(0, 0, 0, 10));
        card->setGraphicsEffect(shadow);
        card->setMinimumHeight(110);

        QVBoxLayout* cv = new QVBoxLayout(card);
        cv->setContentsMargins(20, 18, 20, 18); cv->setSpacing(8);

        QLabel* iconBadge = new QLabel(info.icon, card);
        iconBadge->setFixedSize(40, 40); iconBadge->setAlignment(Qt::AlignCenter);
        iconBadge->setStyleSheet(QString("background-color: %1; border-radius: 10px; font-size: 18px; border:none;").arg(info.bgLight));

        QLabel* titleLbl = new QLabel(info.title, card);
        titleLbl->setStyleSheet("color: #64748B; font-size: 12px; font-weight: 500; border:none;");

        QLabel* valueLbl = new QLabel(info.value, card);
        valueLbl->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: bold; border:none;").arg(info.color));

        QLabel* subLbl = new QLabel(info.sub, card);
        subLbl->setStyleSheet("color: #94A3B8; font-size: 11px; border:none;");

        cv->addWidget(iconBadge); cv->addWidget(titleLbl); cv->addWidget(valueLbl); cv->addWidget(subLbl);
        cardRow->addWidget(card);
    }
    scrollLayout->addLayout(cardRow);

    QFrame* container = new QFrame(scrollContent);
    container->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 14px; }");
    auto* containerShadow = new QGraphicsDropShadowEffect(container);
    containerShadow->setBlurRadius(16); containerShadow->setOffset(0, 2); containerShadow->setColor(QColor(0, 0, 0, 10));
    container->setGraphicsEffect(containerShadow);

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(24, 20, 24, 24);
    containerLayout->setSpacing(16);

    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(12);

    QLineEdit* searchBar = new QLineEdit(container);
    searchBar->setPlaceholderText("🔍 Tìm kiếm tên, mã bệnh nhân, số điện thoại...");
    searchBar->setFixedWidth(340);
    searchBar->setStyleSheet(
        "QLineEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 12px; font-size: 13px; color: #0F172A; background-color: #FFFFFF; }"
        "QLineEdit:focus { border: 1px solid #0284C7; background-color: #F0F9FF; }"
    );

    toolbarLayout->addWidget(searchBar);
    toolbarLayout->addStretch();
    containerLayout->addLayout(toolbarLayout);

    m_patientTable = new QTableWidget(container);
    m_patientTable->setColumnCount(3);
    m_patientTable->setHorizontalHeaderLabels({"STT", "Mã bệnh nhân", "Tên bệnh nhân"});
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_patientTable->setAlternatingRowColors(true);
    m_patientTable->setShowGrid(false);
    m_patientTable->setFocusPolicy(Qt::NoFocus);
    m_patientTable->setCursor(Qt::PointingHandCursor);
    m_patientTable->verticalHeader()->setVisible(false);
    
    m_patientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_patientTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_patientTable->horizontalHeader()->setFixedHeight(40);
    m_patientTable->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: transparent; font-size: 13px; color: #334155; }"
        "QTableWidget::item { padding: 12px 8px; border-bottom: 1px solid #F1F5F9; }"
        "QTableWidget::item:selected { background-color: #E0F2FE; color: #0369A1; font-weight: 500; }"
        "QHeaderView::section { background-color: #F8FAFC; color: #475569; font-weight: bold; font-size: 12px; border: none; border-bottom: 2px solid #E2E8F0; padding: 4px; }"
    );

    m_patientTable->setRowCount(3);
    m_patientTable->setItem(0, 0, new QTableWidgetItem("1"));
    m_patientTable->setItem(0, 1, new QTableWidgetItem("BN001"));
    m_patientTable->setItem(0, 2, new QTableWidgetItem("Nguyễn Văn A"));
    m_patientTable->setItem(1, 0, new QTableWidgetItem("2"));
    m_patientTable->setItem(1, 1, new QTableWidgetItem("BN002"));
    m_patientTable->setItem(1, 2, new QTableWidgetItem("Lê Thị B"));
    m_patientTable->setItem(2, 0, new QTableWidgetItem("3"));
    m_patientTable->setItem(2, 1, new QTableWidgetItem("BN003"));
    m_patientTable->setItem(2, 2, new QTableWidgetItem("Trần Văn C"));
    for (int i = 0; i < 3; ++i) m_patientTable->setRowHeight(i, 44);

    containerLayout->addWidget(m_patientTable);
    scrollLayout->addWidget(container);
    listLayout->addWidget(scrollArea, 1);

    connect(m_patientTable, &QTableWidget::cellClicked, this, &PatientWidget::handlePatientSelected);
}

void PatientWidget::createDetailView() {
    m_detailViewWidget = new QWidget(this);
    QVBoxLayout* detailLayout = new QVBoxLayout(m_detailViewWidget);
    detailLayout->setContentsMargins(15, 15, 15, 15);
    detailLayout->setSpacing(15);

    QHBoxLayout* mainContentLayout = new QHBoxLayout();
    mainContentLayout->setSpacing(20);

    m_detailStackedWidget = new QStackedWidget(m_detailViewWidget);
    m_detailStackedWidget->setStyleSheet("background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px;");

    m_infoSubPage = new QWidget(this);
    QHBoxLayout* infoMainLayout = new QHBoxLayout(m_infoSubPage);
    infoMainLayout->setContentsMargins(20, 20, 20, 20);  
    infoMainLayout->setSpacing(20);
    QVBoxLayout* leftPartLayout = new QVBoxLayout();
    leftPartLayout->setSpacing(20);

    QFrame* cardAdminInfo = new QFrame(m_infoSubPage);
    cardAdminInfo->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; }");
    QVBoxLayout* cardAdminLayout = new QVBoxLayout(cardAdminInfo);
    cardAdminLayout->setContentsMargins(25, 25, 25, 25);
    cardAdminLayout->setSpacing(20); 
    QHBoxLayout* headerTitleLayout = new QHBoxLayout();
    headerTitleLayout->setSpacing(15);
    QVBoxLayout* textTitleLayout = new QVBoxLayout();
    QLabel* lblMainTitle = new QLabel("Thông tin cá nhân bệnh nhân", cardAdminInfo);
    lblMainTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #0F172A; border: none; font-family: 'Arial';");
    QLabel* lblSubTitle = new QLabel("Quản lý và cập nhật thông tin cá nhân của bệnh nhân", cardAdminInfo);
    lblSubTitle->setStyleSheet("font-size: 12px; color: #64748B; border: none; font-family: 'Arial';");
    textTitleLayout->addWidget(lblMainTitle);
    textTitleLayout->addWidget(lblSubTitle);
    headerTitleLayout->addLayout(textTitleLayout);
    headerTitleLayout->addStretch();
    cardAdminLayout->addLayout(headerTitleLayout);

    QGridLayout* inputGridLayout = new QGridLayout();
    inputGridLayout->setHorizontalSpacing(25);   
    inputGridLayout->setVerticalSpacing(18);
    
    QString labelStyle = "QLabel { font-size: 13px; font-weight: bold; color: #334155; border: none; font-family: 'Arial'; }";
    QString inputStyle = 
        "QLineEdit, QComboBox { font-size: 14px; color: #000000; border: 1px solid transparent; background-color: transparent; padding: 0px 4px; min-height: 34px; }"
        "QLineEdit:!read-only, QComboBox:enabled { border: 1px solid #0284C7; border-radius: 6px; background-color: #FFFFFF; padding: 8px 12px; }"
        "QComboBox::drop-down:disabled { border: none; width: 0px; }"
        "QComboBox::drop-down:enabled { border: none; width: 20px; }";

    QString checkboxStyle = 
        "QCheckBox { font-size: 14px; color: #000000; font-family: 'Arial'; spacing: 8px; padding: 0px; } "
        "QCheckBox::indicator { width: 18px; height: 18px; border: 2px solid #0284C7; border-radius: 4px; background: transparent; }"
        "QCheckBox::indicator:checked { background-color: #0284C7; border-color: #0284C7; image: url('data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"12\" height=\"12\" viewBox=\"0 0 12 12\"><path fill=\"white\" d=\"M10.3 2.3 4.5 8.1 1.7 5.3 0.3 6.7 4.5 10.9 11.7 3.7z\"/></svg>'); }"
        "QCheckBox::indicator:unchecked { background: transparent; } "
        "QCheckBox:disabled { color: #64748B; } "
        "QCheckBox::indicator:disabled { border-color: #CBD5E1; }";

    QVBoxLayout* boxID = new QVBoxLayout();
    boxID->setSpacing(7);
    QLabel* lblID = new QLabel("Mã bệnh nhân <font color='red'>*</font>", cardAdminInfo); 
    lblID->setStyleSheet(labelStyle);
    txtPatientID = new QLineEdit("", cardAdminInfo);
    txtPatientID->setStyleSheet(inputStyle);
    txtPatientID->setReadOnly(true);
    txtPatientID->setFixedWidth(160);
    boxID->addWidget(lblID); boxID->addWidget(txtPatientID);
    inputGridLayout->addLayout(boxID, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QVBoxLayout* boxName = new QVBoxLayout();
    boxName->setSpacing(7);
    QLabel* lblName = new QLabel("Họ và tên <font color='red'>*</font>", cardAdminInfo); lblName->setStyleSheet(labelStyle);
    txtFullName = new QLineEdit("", cardAdminInfo); 
    txtFullName->setStyleSheet(inputStyle);
    txtFullName->setReadOnly(true);
    boxName->addWidget(lblName); boxName->addWidget(txtFullName);
    inputGridLayout->addLayout(boxName, 0, 1, Qt::AlignVCenter);

    QVBoxLayout* boxGender = new QVBoxLayout();
    boxGender->setSpacing(7);
    QLabel* lblGender = new QLabel("Giới tính <font color='red'>*</font>", cardAdminInfo); lblGender->setStyleSheet(labelStyle);
    cbGender = new QComboBox(cardAdminInfo);
    for (const auto& pair : GenderText::getList()) {
        cbGender->addItem(pair.second, pair.first);
    }
    cbGender->setStyleSheet(inputStyle);
    cbGender->setEnabled(false);
    boxGender->addWidget(lblGender); boxGender->addWidget(cbGender);
    inputGridLayout->addLayout(boxGender, 0, 2, Qt::AlignVCenter);

    QVBoxLayout* boxDob = new QVBoxLayout();
    boxDob->setSpacing(6);
    QLabel* lblDob = new QLabel("Ngày sinh <font color='red'>*</font>", cardAdminInfo); lblDob->setStyleSheet(labelStyle);
    txtDob = new QLineEdit("", cardAdminInfo); 
    txtDob->setStyleSheet(inputStyle);
    txtDob->setReadOnly(true);
    boxDob->addWidget(lblDob); boxDob->addWidget(txtDob);
    inputGridLayout->addLayout(boxDob, 1, 0, Qt::AlignVCenter);

    QVBoxLayout* boxPhone = new QVBoxLayout();
    boxPhone->setSpacing(6);
    QLabel* lblPhone = new QLabel("Số điện thoại <font color='red'>*</font>", cardAdminInfo); lblPhone->setStyleSheet(labelStyle);
    txtPhone = new QLineEdit("", cardAdminInfo); 
    txtPhone->setStyleSheet(inputStyle);
    txtPhone->setReadOnly(true);
    boxPhone->addWidget(lblPhone); boxPhone->addWidget(txtPhone);
    inputGridLayout->addLayout(boxPhone, 1, 1, Qt::AlignVCenter);

    QVBoxLayout* boxAddress = new QVBoxLayout();
    boxAddress->setSpacing(6);
    QLabel* lblAddress = new QLabel("Địa chỉ <font color='red'>*</font>", cardAdminInfo); lblAddress->setStyleSheet(labelStyle);
    txtAddress = new QLineEdit("", cardAdminInfo); 
    txtAddress->setStyleSheet(inputStyle);
    txtAddress->setReadOnly(true);
    boxAddress->addWidget(lblAddress); boxAddress->addWidget(txtAddress);
    inputGridLayout->addLayout(boxAddress, 1, 2, Qt::AlignVCenter);

    QVBoxLayout* boxAllergies = new QVBoxLayout();
    boxAllergies->setSpacing(6);
    QLabel* lblAllergies = new QLabel("Dị ứng <font color='red'>*</font>", cardAdminInfo); lblAllergies->setStyleSheet(labelStyle);
    txtAllergies = new QLineEdit("", cardAdminInfo); 
    txtAllergies->setStyleSheet(inputStyle);
    txtAllergies->setReadOnly(true);
    boxAllergies->addWidget(lblAllergies); boxAllergies->addWidget(txtAllergies);
    inputGridLayout->addLayout(boxAllergies, 2, 0, Qt::AlignVCenter);

    QVBoxLayout* boxEmail = new QVBoxLayout();
    boxEmail->setSpacing(6);
    QLabel* lblEmail = new QLabel("Email <font color='red'>*</font>", cardAdminInfo); lblEmail->setStyleSheet(labelStyle);
    txtEmail = new QLineEdit("", cardAdminInfo); 
    txtEmail->setStyleSheet(inputStyle);
    txtEmail->setReadOnly(true);
    boxEmail->addWidget(lblEmail); boxEmail->addWidget(txtEmail);
    inputGridLayout->addLayout(boxEmail, 2, 1, Qt::AlignVCenter);

    QVBoxLayout* boxCitizenID = new QVBoxLayout();
    boxCitizenID->setSpacing(6);
    QLabel* lblCitizenID = new QLabel("Số CCCD <font color='red'>*</font>", cardAdminInfo); lblCitizenID->setStyleSheet(labelStyle);
    txtCitizenID = new QLineEdit("", cardAdminInfo); 
    txtCitizenID->setStyleSheet(inputStyle);
    txtCitizenID->setReadOnly(true);
    boxCitizenID->addWidget(lblCitizenID); boxCitizenID->addWidget(txtCitizenID);
    inputGridLayout->addLayout(boxCitizenID, 2, 2, Qt::AlignVCenter);

    QVBoxLayout* boxInsurance = new QVBoxLayout();
    boxInsurance->setSpacing(6);
    QLabel* lblInsurance = new QLabel("Mã bảo hiểm <font color='red'>*</font>", cardAdminInfo); lblInsurance->setStyleSheet(labelStyle);
    txtInsurance = new QLineEdit("", cardAdminInfo); 
    txtInsurance->setStyleSheet(inputStyle);
    txtInsurance->setReadOnly(true);
    boxInsurance->addWidget(lblInsurance); boxInsurance->addWidget(txtInsurance);
    inputGridLayout->addLayout(boxInsurance, 3, 0, Qt::AlignVCenter);

    QVBoxLayout* boxBloodType = new QVBoxLayout();
    boxBloodType->setSpacing(6);
    QLabel* lblBloodType = new QLabel("Nhóm máu <font color='red'>*</font>", cardAdminInfo); lblBloodType->setStyleSheet(labelStyle);
    cbBloodType = new QComboBox(cardAdminInfo); 
    for (const auto& pair : BloodTypeText::getList()) {
        cbBloodType->addItem(pair.second, pair.first);
    }
    cbBloodType->setStyleSheet(inputStyle);
    cbBloodType->setEnabled(false);
    boxBloodType->addWidget(lblBloodType); boxBloodType->addWidget(cbBloodType);
    inputGridLayout->addLayout(boxBloodType, 3, 1, Qt::AlignVCenter);

    QVBoxLayout* boxEmergencyName = new QVBoxLayout();
    boxEmergencyName->setSpacing(6);
    QLabel* lblEmergencyName = new QLabel("Người liên hệ khẩn cấp <font color='red'>*</font>", cardAdminInfo); lblEmergencyName->setStyleSheet(labelStyle);
    txtEmergencyName = new QLineEdit("", cardAdminInfo); 
    txtEmergencyName->setStyleSheet(inputStyle);
    txtEmergencyName->setReadOnly(true);
    boxEmergencyName->addWidget(lblEmergencyName); boxEmergencyName->addWidget(txtEmergencyName);
    inputGridLayout->addLayout(boxEmergencyName, 3, 2, Qt::AlignVCenter);

    QVBoxLayout* boxEmergencyPhone = new QVBoxLayout();
    boxEmergencyPhone->setSpacing(6);
    QLabel* lblEmergencyPhone = new QLabel("SĐT liên hệ khẩn cấp <font color='red'>*</font>", cardAdminInfo); lblEmergencyPhone->setStyleSheet(labelStyle);
    txtEmergencyPhone = new QLineEdit("", cardAdminInfo); 
    txtEmergencyPhone->setStyleSheet(inputStyle);
    txtEmergencyPhone->setReadOnly(true);
    boxEmergencyPhone->addWidget(lblEmergencyPhone); boxEmergencyPhone->addWidget(txtEmergencyPhone);
    inputGridLayout->addLayout(boxEmergencyPhone, 4, 0, Qt::AlignVCenter);

    QList<QLineEdit*> patientInputs = { txtPatientID, txtFullName, txtDob, txtPhone, txtAddress, txtAllergies, txtEmail, txtCitizenID, txtInsurance, txtEmergencyName, txtEmergencyPhone };
    for (QLineEdit* input : patientInputs) {
        input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        input->setMinimumHeight(34);
    }
    cardAdminLayout->addLayout(inputGridLayout);
    leftPartLayout->addWidget(cardAdminInfo);
    QFrame* cardNotes = new QFrame(m_infoSubPage);
    cardNotes->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; }");
    QVBoxLayout* cardNotesLayout = new QVBoxLayout(cardNotes);
    cardNotesLayout->setContentsMargins(25, 25, 25, 25);
    cardNotesLayout->setSpacing(15);
    QLabel* lblNotesTitle = new QLabel("Ghi chú", cardNotes); lblNotesTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #0F172A; border: none;");
    cardNotesLayout->addWidget(lblNotesTitle);

    txtNotes = new QTextEdit(cardNotes);
    txtNotes->setPlaceholderText("Nhập ghi chú về bệnh nhân...");
    txtNotes->setReadOnly(true);
    txtNotes->setStyleSheet(
        "QTextEdit { border: 1px solid transparent; background-color: transparent; padding: 10px 0px; font-size: 14px; color: #000000; min-height: 100px; }"
        "QTextEdit:!read-only { border: 1px solid #0284C7; border-radius: 6px; background-color: #FFFFFF; padding: 12px; }"
    );
    cardNotesLayout->addWidget(txtNotes);
    leftPartLayout->addWidget(cardNotes);
    infoMainLayout->addLayout(leftPartLayout, 1);

    m_detailStackedWidget->addWidget(m_infoSubPage);
    mainContentLayout->addWidget(m_detailStackedWidget, 1);

    QFrame* rightMenuContainer = new QFrame(m_detailViewWidget);
    rightMenuContainer->setFixedWidth(240);
    rightMenuContainer->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; }");
    QVBoxLayout* menuLayout = new QVBoxLayout(rightMenuContainer);
    menuLayout->setContentsMargins(15, 20, 15, 20);
    menuLayout->setSpacing(10);

    QString menuBtnStyle = 
        "QPushButton { font-size: 14px; font-weight: 500; font-family: 'Arial'; color: #475569; border: none; border-radius: 6px; padding: 12px 15px; text-align: left; background: transparent; }"
        "QPushButton:hover { background-color: #F1F5F9; color: #0F172A; }"
        "QPushButton#activeMenuBtn { background-color: #E0F2FE; color: #0284C7; font-weight: bold; }";

    m_btnInfo = new QPushButton("📄 Thông tin cá nhân", rightMenuContainer);
    m_btnInfo->setObjectName("activeMenuBtn");
    m_btnInfo->setStyleSheet(menuBtnStyle);

    m_btnEditInfo = new QPushButton("✏️ Chỉnh sửa thông tin", rightMenuContainer);
    m_btnEditInfo->setStyleSheet(
        "QPushButton { background-color: #0284C7; color: white; font-size: 13px; font-weight: bold; font-family: 'Arial'; border: none; border-radius: 6px; padding: 12px 15px; text-align: center; }"
        "QPushButton:hover { background-color: #0369A1; }"
    );

    m_btnDeleteInfo = new QPushButton("🗑️ Xóa hồ sơ", rightMenuContainer);
    m_btnDeleteInfo->setStyleSheet(
        "QPushButton { background-color: #EF4444; color: white; font-size: 13px; font-weight: bold; font-family: 'Arial'; border: none; border-radius: 6px; padding: 12px 15px; text-align: center; }"
        "QPushButton:hover { background-color: #DC2626; }"
    );

    m_btnBack = new QPushButton("⬅️ Quay lại", rightMenuContainer);
    m_btnBack->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; color: #475569; font-family: 'Arial'; font-size: 14px; font-weight: bold; border: 1px solid #CBD5E1; border-radius: 6px; padding: 12px 15px 12px 22px; text-align: left; }"
        "QPushButton:hover { background-color: #F8FAFC; color: #0F172A; }"
    );

    menuLayout->addWidget(m_btnInfo);
    menuLayout->addWidget(m_btnEditInfo);     
    menuLayout->addWidget(m_btnDeleteInfo);   
    menuLayout->addStretch(); 
    menuLayout->addWidget(m_btnBack);

    mainContentLayout->addWidget(rightMenuContainer);
    detailLayout->addLayout(mainContentLayout);

    connect(m_btnInfo, &QPushButton::clicked, this, [this]() {
        m_btnInfo->setObjectName("activeMenuBtn");
        m_btnInfo->setStyle(m_btnInfo->style());
        m_btnEditInfo->setVisible(true); 
        m_detailStackedWidget->setCurrentWidget(m_infoSubPage);
    });

    connect(m_btnBack, &QPushButton::clicked, this, &PatientWidget::showPatientList);
    
    connect(m_btnEditInfo, &QPushButton::clicked, this, [this]() {
        if (m_btnEditInfo->text() == "Chỉnh sửa thông tin") {
            txtPatientID->setReadOnly(false);
            txtFullName->setReadOnly(false);
            txtDob->setReadOnly(false);
            txtPhone->setReadOnly(false);
            txtAddress->setReadOnly(false);
            txtCitizenID->setReadOnly(false);
            txtEmail->setReadOnly(false);
            txtAllergies->setReadOnly(false);
            txtInsurance->setReadOnly(false);
            txtEmergencyName->setReadOnly(false);
            txtEmergencyPhone->setReadOnly(false);
            txtNotes->setReadOnly(false);
            cbGender->setEnabled(true);
            cbBloodType->setEnabled(true);
            m_btnEditInfo->setText("Lưu thông tin");
        } else {
            txtPatientID->setReadOnly(true);
            txtFullName->setReadOnly(true);
            txtDob->setReadOnly(true);
            txtPhone->setReadOnly(true);
            txtAddress->setReadOnly(true);
            txtCitizenID->setReadOnly(true);
            txtEmail->setReadOnly(true);
            txtAllergies->setReadOnly(true);
            txtInsurance->setReadOnly(true);
            txtEmergencyName->setReadOnly(true);
            txtEmergencyPhone->setReadOnly(true);
            txtNotes->setReadOnly(true);
            cbGender->setEnabled(false);
            cbBloodType->setEnabled(false);
            m_btnEditInfo->setText("Chỉnh sửa thông tin");
        }
    });
}

void PatientWidget::setPatientService(std::shared_ptr<PatientService> patientService) {
    m_patientService = patientService;
    loadPatientsData();
}

void PatientWidget::loadPatientsData() {
    if (!m_patientService || !m_patientTable) return;
    PatientSearchCriteria criteria;
    m_patientList = m_patientService->searchPatientsPaged(criteria).items;
    
    m_patientTable->setRowCount(m_patientList.size());
    for (int i = 0; i < m_patientList.size(); ++i) {
        const auto& p = m_patientList[i];
        m_patientTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        m_patientTable->setItem(i, 1, new QTableWidgetItem(p.patientCode));
        m_patientTable->setItem(i, 2, new QTableWidgetItem(p.fullName));
        m_patientTable->setRowHeight(i, 44);
    }
}

void PatientWidget::handlePatientSelected(int row, int column) {
    Q_UNUSED(column);
    if (row >= 0 && row < m_patientList.size()) {
        const auto& summary = m_patientList[row];
        if (m_patientService) {
            auto detailOpt = m_patientService->getPatientById(summary.patientId);
            if (detailOpt.has_value()) {
                const auto& detail = detailOpt.value();
                if (txtPatientID) txtPatientID->setText(detail.patientCode);
                if (txtFullName) txtFullName->setText(detail.fullName);
                if (txtDob) txtDob->setText(detail.dateOfBirth.toString("dd/MM/yyyy"));
                if (txtPhone) txtPhone->setText(detail.phone);
                if (txtAddress) txtAddress->setText(detail.address);
                if (txtCitizenID) txtCitizenID->setText(detail.citizenId);
                if (txtEmail) txtEmail->setText(detail.email);
                if (cbBloodType) cbBloodType->setCurrentText(BloodTypeText::toVi(detail.bloodType));
                QStringList allergyList;
                for (const auto& a : detail.allergies) {
                    allergyList.append(a.allergenName);
                }
                if (txtAllergies) txtAllergies->setText(allergyList.join(", "));
                if (txtInsurance) txtInsurance->setText(detail.insurance.has_value() ? detail.insurance->policyNumber : "");
                if (txtEmergencyName) txtEmergencyName->setText(detail.emergencyContactName);
                if (txtEmergencyPhone) txtEmergencyPhone->setText(detail.emergencyContactPhone);
                if (cbGender) cbGender->setCurrentText(GenderText::toVi(detail.gender));
            }
        }
    }
    
    m_btnInfo->setObjectName("activeMenuBtn");
    m_btnInfo->setStyle(m_btnInfo->style());
    
    m_detailStackedWidget->setCurrentWidget(m_infoSubPage);
    m_mainStackedWidget->setCurrentWidget(m_detailViewWidget);
}

void PatientWidget::showPatientList() {
    m_mainStackedWidget->setCurrentWidget(m_listViewWidget);
}

PatientWidget::~PatientWidget() {}
