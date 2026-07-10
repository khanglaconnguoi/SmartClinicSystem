// Patient.cpp
#include "Patient.h"

PatientView::PatientView(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void PatientView::setupUi() {
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

void PatientView::createListView() {
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

    connect(m_patientTable, &QTableWidget::cellClicked, this, &PatientView::handlePatientSelected);
}

void PatientView::createDetailView() {
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
    inputGridLayout->setHorizontalSpacing(30);   
    inputGridLayout->setVerticalSpacing(25);
    
    for (int i = 0; i < 5; ++i) {
        inputGridLayout->setRowMinimumHeight(i, 42);
    }
    
    QString labelStyle = "QLabel { font-size: 13px; font-weight: bold; color: #334155; border: none; font-family: 'Arial'; }";
    QString inputStyle = 
        "QLineEdit, QComboBox { font-size: 15px; color: #000000; border: 1px solid transparent; background-color: transparent; padding: 0px 4px; min-height: 34px; }"
        "QLineEdit:!read-only, QComboBox:enabled { border: 1px solid #0284C7; border-radius: 6px; background-color: #FFFFFF; padding: 10px 12px; }"
        "QComboBox::drop-down:disabled { border: none; width: 0px; }"
        "QComboBox::drop-down:enabled { border: none; width: 20px; }";

    QString checkboxStyle = 
        "QCheckBox { font-size: 15px; color: #000000; font-family: 'Arial'; spacing: 8px; padding: 0px; } "
        "QCheckBox::indicator { width: 18px; height: 18px; border: 2px solid #0284C7; border-radius: 4px; background: transparent; }"
        "QCheckBox::indicator:checked { background-color: #0284C7; border-color: #0284C7; image: url('data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"12\" height=\"12\" viewBox=\"0 0 12 12\"><path fill=\"white\" d=\"M10.3 2.3 4.5 8.1 1.7 5.3 0.3 6.7 4.5 10.9 11.7 3.7z\"/></svg>'); }"
        "QCheckBox::indicator:unchecked { background: transparent; } "
        "QCheckBox:disabled { color: #64748B; } "
        "QCheckBox::indicator:disabled { border-color: #CBD5E1; }";
    
    QVBoxLayout* boxID = new QVBoxLayout();
    boxID->setSpacing(8);
    QLabel* lblID = new QLabel("Mã bệnh nhân <font color='red'>*</font>", cardAdminInfo); 
    lblID->setStyleSheet(labelStyle);
    txtPatientID = new QLineEdit("", cardAdminInfo);
    txtPatientID->setStyleSheet(inputStyle);
    txtPatientID->setReadOnly(true);
    txtPatientID->setFixedWidth(160);
    boxID->addWidget(lblID); boxID->addWidget(txtPatientID);
    inputGridLayout->addLayout(boxID, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);

    QVBoxLayout* boxName = new QVBoxLayout();
    boxName->setSpacing(8);
    QLabel* lblName = new QLabel("Họ và tên <font color='red'>*</font>", cardAdminInfo); lblName->setStyleSheet(labelStyle);
    txtFullName = new QLineEdit("", cardAdminInfo); 
    txtFullName->setStyleSheet(inputStyle);
    txtFullName->setReadOnly(true);
    boxName->addWidget(lblName); boxName->addWidget(txtFullName);
    inputGridLayout->addLayout(boxName, 0, 1, Qt::AlignVCenter);

    QVBoxLayout* boxGender = new QVBoxLayout();
    boxGender->setSpacing(8);
    QLabel* lblGender = new QLabel("Giới tính <font color='red'>*</font>", cardAdminInfo); lblGender->setStyleSheet(labelStyle);
    QWidget* radioContainer = new QWidget(cardAdminInfo);
    radioContainer->setObjectName("genderContainer");
    radioContainer->setMinimumHeight(34);
    radioContainer->setStyleSheet("QWidget#genderContainer { background-color: transparent; border: 1px solid transparent; }");
    QHBoxLayout* radioLayout = new QHBoxLayout(radioContainer);
    radioLayout->setContentsMargins(4, 0, 4, 0);
    radioLayout->setSpacing(24);
    radMale = new QCheckBox("Nam", radioContainer);
    radFemale = new QCheckBox("Nữ", radioContainer);
    radMale->setStyleSheet(checkboxStyle);
    radFemale->setStyleSheet(checkboxStyle);
    radMale->setEnabled(false);
    radFemale->setEnabled(false);
    radMale->setAutoExclusive(true);
    radFemale->setAutoExclusive(true);
    m_genderGroup = new QButtonGroup(this);
    m_genderGroup->setExclusive(true);
    m_genderGroup->addButton(radMale, 0);
    m_genderGroup->addButton(radFemale, 1);
    radioLayout->addWidget(radMale);
    radioLayout->addWidget(radFemale);
    radioLayout->addStretch();
    boxGender->addWidget(lblGender); boxGender->addWidget(radioContainer);
    inputGridLayout->addLayout(boxGender, 0, 2, Qt::AlignVCenter);

    QVBoxLayout* boxDob = new QVBoxLayout();
    boxDob->setSpacing(8);
    QLabel* lblDob = new QLabel("Ngày sinh <font color='red'>*</font>", cardAdminInfo); lblDob->setStyleSheet(labelStyle);
    txtDob = new QLineEdit("", cardAdminInfo); 
    txtDob->setStyleSheet(inputStyle);
    txtDob->setReadOnly(true);
    boxDob->addWidget(lblDob); boxDob->addWidget(txtDob);
    inputGridLayout->addLayout(boxDob, 1, 0, Qt::AlignVCenter);

    QVBoxLayout* boxPhone = new QVBoxLayout();
    boxPhone->setSpacing(8);
    QLabel* lblPhone = new QLabel("Số điện thoại <font color='red'>*</font>", cardAdminInfo); lblPhone->setStyleSheet(labelStyle);
    txtPhone = new QLineEdit("", cardAdminInfo); 
    txtPhone->setStyleSheet(inputStyle);
    txtPhone->setReadOnly(true);
    boxPhone->addWidget(lblPhone); boxPhone->addWidget(txtPhone);
    inputGridLayout->addLayout(boxPhone, 1, 1, Qt::AlignVCenter);

    QVBoxLayout* boxAddress = new QVBoxLayout();
    boxAddress->setSpacing(8);
    QLabel* lblAddress = new QLabel("Địa chỉ <font color='red'>*</font>", cardAdminInfo); lblAddress->setStyleSheet(labelStyle);
    txtAddress = new QLineEdit("", cardAdminInfo); 
    txtAddress->setStyleSheet(inputStyle);
    txtAddress->setReadOnly(true);
    boxAddress->addWidget(lblAddress); boxAddress->addWidget(txtAddress);
    inputGridLayout->addLayout(boxAddress, 1, 2, Qt::AlignVCenter);

    QVBoxLayout* boxAllergies = new QVBoxLayout();
    boxAllergies->setSpacing(8);
    QLabel* lblAllergies = new QLabel("Dị ứng <font color='red'>*</font>", cardAdminInfo); lblAllergies->setStyleSheet(labelStyle);
    txtAllergies = new QLineEdit("", cardAdminInfo); 
    txtAllergies->setStyleSheet(inputStyle);
    txtAllergies->setReadOnly(true);
    boxAllergies->addWidget(lblAllergies); boxAllergies->addWidget(txtAllergies);
    inputGridLayout->addLayout(boxAllergies, 2, 0, Qt::AlignVCenter);

    QVBoxLayout* boxEmail = new QVBoxLayout();
    boxEmail->setSpacing(8);
    QLabel* lblEmail = new QLabel("Email <font color='red'>*</font>", cardAdminInfo); lblEmail->setStyleSheet(labelStyle);
    txtEmail = new QLineEdit("", cardAdminInfo); 
    txtEmail->setStyleSheet(inputStyle);
    txtEmail->setReadOnly(true);
    boxEmail->addWidget(lblEmail); boxEmail->addWidget(txtEmail);
    inputGridLayout->addLayout(boxEmail, 2, 1, Qt::AlignVCenter);

    QVBoxLayout* boxCitizenID = new QVBoxLayout();
    boxCitizenID->setSpacing(8);
    QLabel* lblCitizenID = new QLabel("Số CCCD <font color='red'>*</font>", cardAdminInfo); lblCitizenID->setStyleSheet(labelStyle);
    txtCitizenID = new QLineEdit("", cardAdminInfo); 
    txtCitizenID->setStyleSheet(inputStyle);
    txtCitizenID->setReadOnly(true);
    boxCitizenID->addWidget(lblCitizenID); boxCitizenID->addWidget(txtCitizenID);
    inputGridLayout->addLayout(boxCitizenID, 2, 2, Qt::AlignVCenter);

    QVBoxLayout* boxInsurance = new QVBoxLayout();
    boxInsurance->setSpacing(8);
    QLabel* lblInsurance = new QLabel("Mã bảo hiểm <font color='red'>*</font>", cardAdminInfo); lblInsurance->setStyleSheet(labelStyle);
    txtInsurance = new QLineEdit("", cardAdminInfo); 
    txtInsurance->setStyleSheet(inputStyle);
    txtInsurance->setReadOnly(true);
    boxInsurance->addWidget(lblInsurance); boxInsurance->addWidget(txtInsurance);
    inputGridLayout->addLayout(boxInsurance, 3, 0, Qt::AlignVCenter);

    QVBoxLayout* boxBloodType = new QVBoxLayout();
    boxBloodType->setSpacing(8);
    QLabel* lblBloodType = new QLabel("Nhóm máu <font color='red'>*</font>", cardAdminInfo); lblBloodType->setStyleSheet(labelStyle);
    txtBloodType = new QLineEdit("", cardAdminInfo); 
    txtBloodType->setStyleSheet(inputStyle);
    txtBloodType->setReadOnly(true);
    boxBloodType->addWidget(lblBloodType); boxBloodType->addWidget(txtBloodType);
    inputGridLayout->addLayout(boxBloodType, 3, 1, Qt::AlignVCenter);

    QVBoxLayout* boxEmergencyName = new QVBoxLayout();
    boxEmergencyName->setSpacing(8);
    QLabel* lblEmergencyName = new QLabel("Người liên hệ khẩn cấp <font color='red'>*</font>", cardAdminInfo); lblEmergencyName->setStyleSheet(labelStyle);
    txtEmergencyName = new QLineEdit("", cardAdminInfo); 
    txtEmergencyName->setStyleSheet(inputStyle);
    txtEmergencyName->setReadOnly(true);
    boxEmergencyName->addWidget(lblEmergencyName); boxEmergencyName->addWidget(txtEmergencyName);
    inputGridLayout->addLayout(boxEmergencyName, 4, 0, Qt::AlignVCenter);

    QVBoxLayout* boxEmergencyPhone = new QVBoxLayout();
    boxEmergencyPhone->setSpacing(8);
    QLabel* lblEmergencyPhone = new QLabel("SĐT liên hệ khẩn cấp <font color='red'>*</font>", cardAdminInfo); lblEmergencyPhone->setStyleSheet(labelStyle);
    txtEmergencyPhone = new QLineEdit("", cardAdminInfo); 
    txtEmergencyPhone->setStyleSheet(inputStyle);
    txtEmergencyPhone->setReadOnly(true);
    boxEmergencyPhone->addWidget(lblEmergencyPhone); boxEmergencyPhone->addWidget(txtEmergencyPhone);
    inputGridLayout->addLayout(boxEmergencyPhone, 4, 1, Qt::AlignVCenter);

    QList<QLineEdit*> patientInputs = { txtPatientID, txtFullName, txtDob, txtPhone, txtAddress, txtAllergies, txtEmail, txtCitizenID, txtInsurance, txtBloodType, txtEmergencyName, txtEmergencyPhone };
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

    m_recordSubPage = new QWidget(this);
    QVBoxLayout* recordPageLayout = new QVBoxLayout(m_recordSubPage);
    recordPageLayout->setContentsMargins(0, 0, 0, 0);
    recordPageLayout->setSpacing(0);

    QFrame* recordTopBar = new QFrame(m_recordSubPage);
    recordTopBar->setStyleSheet("QFrame { background-color: #FFFFFF; border-bottom: 1px solid #E2E8F0; }");
    QHBoxLayout* recordTopBarLayout = new QHBoxLayout(recordTopBar);
    recordTopBarLayout->setContentsMargins(24, 14, 24, 14);
    
    QLabel* recordMainTitle = new QLabel("HỒ SƠ BỆNH ÁN ĐIỆN TỬ", recordTopBar);
    recordMainTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #0F172A; font-family: 'Arial';");
    recordTopBarLayout->addWidget(recordMainTitle);
    recordTopBarLayout->addStretch();

    QString btnHeaderStyle = "QPushButton { font-size: 13px; font-weight: 600; font-family: 'Arial'; border-radius: 6px; padding: 8px 16px; }";
    QPushButton* btnDraft = new QPushButton("Lưu nháp", recordTopBar);
    btnDraft->setStyleSheet(btnHeaderStyle + "QPushButton { background-color: #F1F5F9; color: #334155; border: 1px solid #CBD5E1; } QPushButton:hover { background-color: #E2E8F0; }");
    QPushButton* btnSaveRecord = new QPushButton("Lưu bệnh án", recordTopBar);
    btnSaveRecord->setStyleSheet(btnHeaderStyle + "QPushButton { background-color: #0284C7; color: white; border: none; } QPushButton:hover { background-color: #0369A1; }");
    QPushButton* btnPrint = new QPushButton("In bệnh án", recordTopBar);
    btnPrint->setStyleSheet(btnHeaderStyle + "QPushButton { background-color: #FFFFFF; color: #334155; border: 1px solid #CBD5E1; } QPushButton:hover { background-color: #F1F5F9; }");
    QPushButton* btnExport = new QPushButton("Xuất PDF", recordTopBar);
    btnExport->setStyleSheet(btnHeaderStyle + "QPushButton { background-color: #FFFFFF; color: #334155; border: 1px solid #CBD5E1; } QPushButton:hover { background-color: #F1F5F9; }");

    recordTopBarLayout->addWidget(btnDraft);
    recordTopBarLayout->addWidget(btnSaveRecord);
    recordTopBarLayout->addWidget(btnPrint);
    recordTopBarLayout->addWidget(btnExport);
    recordPageLayout->addWidget(recordTopBar);

    QScrollArea* recordScroll = new QScrollArea(m_recordSubPage);
    recordScroll->setFrameShape(QFrame::NoFrame);
    recordScroll->setWidgetResizable(true);
    recordScroll->setStyleSheet("QScrollArea { background-color: #F8FAFC; }");

    QWidget* recordScrollContent = new QWidget();
    recordScrollContent->setStyleSheet("background-color: #F8FAFC;");
    QVBoxLayout* recordScrollLayout = new QVBoxLayout(recordScrollContent);
    recordScrollLayout->setContentsMargins(24, 20, 24, 24);
    recordScrollLayout->setSpacing(20);

    QString fCardStyle = "QFrame#FormCard { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 12px; }";
    QString secTitleStyle = "QLabel { font-size: 13px; font-weight: bold; color: #0284C7; font-family: 'Arial'; text-transform: uppercase; border: none; }";
    QString fInputStyle = "QTextEdit, QLineEdit, QComboBox, QDateEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px; font-size: 13px; color: #0F172A; background-color: #FFFFFF; }"
                          "QTextEdit:focus, QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border: 1px solid #0284C7; background-color: #F8FAFC; }"
                          "QTextEdit:read-only, QLineEdit:read-only, QComboBox:disabled, QDateEdit:disabled { border: 1px solid transparent; background-color: transparent; padding: 4px 0px; color: #000000; }";

    QString tableStyle = "QTableWidget { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 6px; font-size: 13px; color: #334155; }"
                         "QHeaderView::section { background-color: #F8FAFC; color: #334155; font-weight: bold; font-size: 12px; border: none; border-bottom: 2px solid #E2E8F0; padding: 6px; }";

    QFrame* pBanner = new QFrame(recordScrollContent);
    pBanner->setObjectName("FormCard"); pBanner->setStyleSheet(fCardStyle);
    QGridLayout* pBannerLayout = new QGridLayout(pBanner);
    pBannerLayout->setContentsMargins(24, 20, 24, 20);
    pBannerLayout->setHorizontalSpacing(30); pBannerLayout->setVerticalSpacing(12);

    QLabel* lblRecName = new QLabel("-", pBanner);
    lblRecName->setStyleSheet("font-size: 16px; font-weight: bold; color: #0F172A; border: none;");
    QLabel* lblRecMeta = new QLabel("-  |  - tuổi  |  --/--/----", pBanner);
    lblRecMeta->setStyleSheet("font-size: 13px; color: #475569; border: none;");
    QLabel* lblRecSub = new QLabel("Mã BN: -\nĐiện thoại: -\nĐịa chỉ: -", pBanner);
    lblRecSub->setStyleSheet("font-size: 12px; color: #64748B; border: none; line-height: 1.4;");
    pBannerLayout->addWidget(lblRecName, 0, 0, 1, 1);
    pBannerLayout->addWidget(lblRecMeta, 1, 0, 1, 1);
    pBannerLayout->addWidget(lblRecSub, 2, 0, 1, 1);

    QFrame* pDivider = new QFrame(pBanner);
    pDivider->setFrameShape(QFrame::VLine); pDivider->setStyleSheet("color: #E2E8F0; border: none; background-color: #E2E8F0; width: 1px;");
    pBannerLayout->addWidget(pDivider, 0, 1, 3, 1);

    QGridLayout* mGridL = new QGridLayout();
    auto addMRow = [&](QString l, QString v, int r, QString c) {
        QLabel* lbl = new QLabel(l, pBanner); lbl->setStyleSheet("font-size: 13px; color: #64748B; border: none;");
        QLabel* val = new QLabel(v, pBanner); val->setStyleSheet(QString("font-size: 13px; font-weight: 600; color: %1; border: none;").arg(c));
        mGridL->addWidget(lbl, r, 0); mGridL->addWidget(val, r, 1);
    };
    addMRow("Mã bệnh án:", "-", 0, "#0284C7");
    addMRow("Ngày khám:", "--/--/---- --:--", 1, "#0F172A");
    addMRow("Bác sĩ:", "-", 2, "#0F172A");
    addMRow("Khoa:", "-", 3, "#0F172A");
    pBannerLayout->addLayout(mGridL, 0, 2, 3, 1);

    QGridLayout* mGridR = new QGridLayout();
    QLabel* lblLt = new QLabel("Loại khám:", pBanner); lblLt->setStyleSheet("font-size: 13px; color: #64748B; border: none;");
    QLabel* valLt = new QLabel("-", pBanner); valLt->setStyleSheet("font-size: 13px; font-weight: 600; color: #0F172A; border: none;");
    mGridR->addWidget(lblLt, 0, 0); mGridR->addWidget(valLt, 0, 1);
    QLabel* lblSt = new QLabel("Trạng thái:", pBanner); lblSt->setStyleSheet("font-size: 13px; color: #64748B; border: none;");
    QLabel* valSt = new QLabel("-", pBanner); valSt->setStyleSheet("background-color: #FEF3C7; color: #D97706; font-size: 12px; font-weight: 600; border-radius: 4px; padding: 2px 8px; border: none;");
    mGridR->addWidget(lblSt, 1, 0); mGridR->addWidget(valSt, 1, 1, Qt::AlignLeft);
    QLabel* lblCc = new QLabel("Lần khám:", pBanner); lblCc->setStyleSheet("font-size: 13px; color: #64748B; border: none;");
    QLabel* valCc = new QLabel("-", pBanner); valCc->setStyleSheet("font-size: 13px; font-weight: 600; color: #0F172A; border: none;");
    mGridR->addWidget(lblCc, 2, 0); mGridR->addWidget(valCc, 2, 1);
    pBannerLayout->addLayout(mGridR, 0, 3, 3, 1);

    pBannerLayout->setColumnStretch(0, 2); pBannerLayout->setColumnStretch(2, 2); pBannerLayout->setColumnStretch(3, 2);
    recordScrollLayout->addWidget(pBanner);

    QHBoxLayout* bodyColumns = new QHBoxLayout(); bodyColumns->setSpacing(20);
    QVBoxLayout* leftCol = new QVBoxLayout(); leftCol->setSpacing(20);
    QVBoxLayout* rightCol = new QVBoxLayout(); rightCol->setSpacing(20);

    QList<QWidget*> recordFields;

    auto buildTextCard = [&](QString title, int height) {
        QFrame* card = new QFrame(recordScrollContent); card->setObjectName("FormCard"); card->setStyleSheet(fCardStyle);
        QVBoxLayout* lay = new QVBoxLayout(card); lay->setContentsMargins(20, 16, 20, 16); lay->setSpacing(10);
        QLabel* lblT = new QLabel(title, card); lblT->setStyleSheet(secTitleStyle);
        QTextEdit* text = new QTextEdit(card); text->setStyleSheet(fInputStyle); text->setMaximumHeight(height);
        text->setReadOnly(true);
        recordFields.append(text);
        lay->addWidget(lblT); lay->addWidget(text);
        return card;
    };
    leftCol->addWidget(buildTextCard("Lý do khám", 64));
    leftCol->addWidget(buildTextCard("Bệnh sử", 80));

    QHBoxLayout* preHistoryRow = new QHBoxLayout(); preHistoryRow->setSpacing(16);
    preHistoryRow->addWidget(buildTextCard("Tiền sử bệnh", 64), 1);
    preHistoryRow->addWidget(buildTextCard("Dị ứng", 64), 1);
    leftCol->addLayout(preHistoryRow);

    QFrame* cardDiag = new QFrame(recordScrollContent); cardDiag->setObjectName("FormCard"); cardDiag->setStyleSheet(fCardStyle);
    QVBoxLayout* layDiag = new QVBoxLayout(cardDiag); layDiag->setContentsMargins(20, 16, 20, 16); layDiag->setSpacing(12);
    QLabel* lblDiagT = new QLabel("Chẩn đoán", cardDiag); lblDiagT->setStyleSheet(secTitleStyle);
    QTableWidget* tblDiag = new QTableWidget(0, 4, cardDiag);
    tblDiag->setHorizontalHeaderLabels({"ICD-10", "Tên chẩn đoán", "Loại chẩn đoán", "Thao tác"});
    tblDiag->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblDiag->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tblDiag->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tblDiag->setStyleSheet(tableStyle);
    tblDiag->setMaximumHeight(130);
    tblDiag->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recordFields.append(tblDiag);

    QPushButton* btnAddDiag = new QPushButton("+ Thêm chẩn đoán", cardDiag);
    btnAddDiag->setStyleSheet("QPushButton { background-color: #F1F5F9; color: #0369A1; border: 1px solid #CBD5E1; font-weight: 600; font-size: 12px; border-radius: 4px; padding: 6px 12px; } QPushButton:hover { background-color: #E0F2FE; }");
    btnAddDiag->setEnabled(false);
    recordFields.append(btnAddDiag);
    layDiag->addWidget(lblDiagT); layDiag->addWidget(tblDiag); layDiag->addWidget(btnAddDiag, 0, Qt::AlignLeft);
    leftCol->addWidget(cardDiag);

    leftCol->addWidget(buildTextCard("Điều trị / Hướng điều trị", 70));

    QFrame* cardPresc = new QFrame(recordScrollContent); cardPresc->setObjectName("FormCard"); cardPresc->setStyleSheet(fCardStyle);
    QVBoxLayout* layPresc = new QVBoxLayout(cardPresc); layPresc->setContentsMargins(20, 16, 20, 16); layPresc->setSpacing(12);
    QLabel* lblPrescT = new QLabel("Toa thuốc", cardPresc); lblPrescT->setStyleSheet(secTitleStyle);
    QTableWidget* tblPresc = new QTableWidget(0, 5, cardPresc);
    tblPresc->setHorizontalHeaderLabels({"STT", "ID Thuốc", "Đơn vị", "Liều dùng", "Số lượng"});
    tblPresc->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblPresc->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tblPresc->setStyleSheet(tableStyle);
    tblPresc->setMaximumHeight(150);
    tblPresc->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recordFields.append(tblPresc);

    QPushButton* btnAddPresc = new QPushButton("+ Thêm thuốc", cardPresc);
    btnAddPresc->setStyleSheet("QPushButton { background-color: #F1F5F9; color: #0369A1; border: 1px solid #CBD5E1; font-weight: 600; font-size: 12px; border-radius: 4px; padding: 6px 12px; } QPushButton:hover { background-color: #E0F2FE; }");
    btnAddPresc->setEnabled(false);
    recordFields.append(btnAddPresc);
    layPresc->addWidget(lblPrescT); layPresc->addWidget(tblPresc); layPresc->addWidget(btnAddPresc, 0, Qt::AlignLeft);
    leftCol->addWidget(cardPresc);

    QFrame* cardVitals = new QFrame(recordScrollContent); cardVitals->setObjectName("FormCard"); cardVitals->setStyleSheet(fCardStyle);
    QVBoxLayout* layVitals = new QVBoxLayout(cardVitals); layVitals->setContentsMargins(20, 16, 20, 16); layVitals->setSpacing(14);
    QLabel* lblVitT = new QLabel("Sinh hiệu", cardVitals); lblVitT->setStyleSheet(secTitleStyle);
    layVitals->addWidget(lblVitT);
    QGridLayout* gridVit = new QGridLayout(); gridVit->setSpacing(12);
    auto addVitRow = [&](QString name, QString unit, int r) {
        QLabel* lN = new QLabel(name, cardVitals); lN->setStyleSheet("font-size: 13px; font-weight: 500; color: #334155; border: none;");
        QLineEdit* le = new QLineEdit(cardVitals); le->setStyleSheet(fInputStyle); le->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        le->setReadOnly(true);
        recordFields.append(le);
        QLabel* lU = new QLabel(unit, cardVitals); lU->setStyleSheet("font-size: 13px; color: #64748B; border: none;"); lU->setFixedWidth(50);
        gridVit->addWidget(lN, r, 0); gridVit->addWidget(le, r, 1); gridVit->addWidget(lU, r, 2);
    };
    addVitRow("Nhiệt độ", "°C", 0);
    addVitRow("Mạch", "lần/phút", 1);
    addVitRow("Huyết áp", "mmHg", 2);
    addVitRow("Nhịp thở", "lần/phút", 3);
    addVitRow("Cân nặng", "kg", 4);
    addVitRow("Chiều cao", "cm", 5);
    addVitRow("BMI", "Chỉ số", 6);
    gridVit->setColumnStretch(1, 1); layVitals->addLayout(gridVit);
    rightCol->addWidget(cardVitals);

    rightCol->addWidget(buildTextCard("Kết quả cận lâm sàng", 100));
    rightCol->addWidget(buildTextCard("Dặn dò", 100));

    QFrame* cardRec = new QFrame(recordScrollContent); cardRec->setObjectName("FormCard"); cardRec->setStyleSheet(fCardStyle);
    QVBoxLayout* layRec = new QVBoxLayout(cardRec); layRec->setContentsMargins(20, 16, 20, 16); layRec->setSpacing(10);
    QLabel* lblRecT = new QLabel("Ngày tái khám", cardRec); lblRecT->setStyleSheet(secTitleStyle);
    QDateEdit* deRec = new QDateEdit(QDate::currentDate(), cardRec); deRec->setStyleSheet(fInputStyle); deRec->setCalendarPopup(true);
    deRec->setEnabled(false);
    recordFields.append(deRec);
    layRec->addWidget(lblRecT); layRec->addWidget(deRec);
    rightCol->addWidget(cardRec);

    QFrame* cardSign = new QFrame(recordScrollContent); cardSign->setObjectName("FormCard"); cardSign->setStyleSheet(fCardStyle);
    QVBoxLayout* laySign = new QVBoxLayout(cardSign); laySign->setContentsMargins(20, 16, 20, 16); laySign->setSpacing(10);
    QLabel* lblSignT = new QLabel("Chữ ký bác sĩ", cardSign); lblSignT->setStyleSheet(secTitleStyle);
    QLabel* lblSignImg = new QLabel("", cardSign); lblSignImg->setAlignment(Qt::AlignCenter); lblSignImg->setMinimumHeight(60);
    lblSignImg->setStyleSheet("border: none;");
    QLabel* lblDocName = new QLabel("-\n--/--/---- --:--", cardSign); lblDocName->setAlignment(Qt::AlignCenter); lblDocName->setStyleSheet("font-size: 13px; font-weight: bold; color: #334155; line-height: 1.4; border: none;");
    laySign->addWidget(lblSignT); laySign->addWidget(lblSignImg); laySign->addWidget(lblDocName);
    rightCol->addWidget(cardSign);

    bodyColumns->addLayout(leftCol, 65);
    bodyColumns->addLayout(rightCol, 35);
    recordScrollLayout->addLayout(bodyColumns);

    QLabel* fText = new QLabel("🔒 Dữ liệu được bảo mật theo quy định của Bộ Y Tế", recordScrollContent);
    fText->setAlignment(Qt::AlignCenter); fText->setStyleSheet("font-size: 12px; color: #94A3B8; font-family: 'Arial'; border: none; margin-top: 10px;");
    recordScrollLayout->addWidget(fText);

    recordScroll->setWidget(recordScrollContent);
    recordPageLayout->addWidget(recordScroll, 1);

    m_detailStackedWidget->addWidget(m_infoSubPage);
    m_detailStackedWidget->addWidget(m_recordSubPage);
    mainContentLayout->addWidget(m_detailStackedWidget, 1);

    QWidget* rightMenuContainer = new QWidget(m_detailViewWidget);
    rightMenuContainer->setFixedWidth(220);
    QVBoxLayout* menuLayout = new QVBoxLayout(rightMenuContainer);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(12);

    QString btnStyle = 
        "QPushButton { background-color: #0284C7; color: #FFFFFF; font-family: 'Arial'; font-size: 14px; font-weight: bold; border: none; border-radius: 6px; padding: 14px 15px 14px 22px; text-align: left; }"
        "QPushButton:hover { background-color: #0369A1; }"
        "QPushButton:pressed { background-color: #075985; }"
        "QPushButton#activeMenuBtn { background-color: #075985; }";

    m_btnInfo = new QPushButton(" Informace o pacientovi", rightMenuContainer);
    m_btnInfo->setText(" Thông tin cá nhân");
    m_btnInfo->setCursor(Qt::PointingHandCursor);
    m_btnInfo->setStyleSheet(btnStyle);
    m_btnInfo->setObjectName("activeMenuBtn");
    
    m_btnEditInfo = new QPushButton("Chỉnh sửa thông tin", rightMenuContainer);
    m_btnEditInfo->setCursor(Qt::PointingHandCursor);
    m_btnEditInfo->setStyleSheet(btnStyle);

    m_btnDeleteInfo = new QPushButton("Xóa bệnh nhân", rightMenuContainer);
    m_btnDeleteInfo->setCursor(Qt::PointingHandCursor);
    m_btnDeleteInfo->setStyleSheet(btnStyle + "QPushButton { background-color: #DC2626; } QPushButton:hover { background-color: #B91C1C; } QPushButton:pressed { background-color: #991B1B; }");
    
    m_btnMedicalRecord = new QPushButton("Bệnh án", rightMenuContainer);
    m_btnMedicalRecord->setCursor(Qt::PointingHandCursor);
    m_btnMedicalRecord->setStyleSheet(btnStyle);

    m_btnBack = new QPushButton("🔙  Quay lại", rightMenuContainer);
    m_btnBack->setCursor(Qt::PointingHandCursor);
    m_btnBack->setStyleSheet(
        "QPushButton { background-color: #FFFFFF; color: #475569; font-family: 'Arial'; font-size: 14px; font-weight: bold; border: 1px solid #CBD5E1; border-radius: 6px; padding: 12px 15px 12px 22px; text-align: left; }"
        "QPushButton:hover { background-color: #F8FAFC; color: #0F172A; }"
    );

    menuLayout->addWidget(m_btnInfo);
    menuLayout->addWidget(m_btnEditInfo);     
    menuLayout->addWidget(m_btnDeleteInfo);   
    menuLayout->addWidget(m_btnMedicalRecord);
    menuLayout->addStretch(); 
    menuLayout->addWidget(m_btnBack);

    mainContentLayout->addWidget(rightMenuContainer);
    detailLayout->addLayout(mainContentLayout);

    QPushButton* btnEditRecord = new QPushButton("Chỉnh sửa", recordTopBar);
    btnEditRecord->setStyleSheet(btnHeaderStyle + "QPushButton { background-color: #0284C7; color: white; border: none; } QPushButton:hover { background-color: #0369A1; }");
    recordTopBarLayout->insertWidget(2, btnEditRecord);

    connect(m_btnInfo, &QPushButton::clicked, this, [this]() {
        m_btnInfo->setObjectName("activeMenuBtn");
        m_btnMedicalRecord->setObjectName("");
        m_btnInfo->setStyle(m_btnInfo->style());
        m_btnMedicalRecord->setStyle(m_btnMedicalRecord->style());
        m_btnEditInfo->setVisible(true); 
        m_detailStackedWidget->setCurrentWidget(m_infoSubPage);
    });

    connect(m_btnMedicalRecord, &QPushButton::clicked, this, [this]() {
        m_btnMedicalRecord->setObjectName("activeMenuBtn");
        m_btnInfo->setObjectName("");
        m_btnInfo->setStyle(m_btnInfo->style());
        m_btnMedicalRecord->setStyle(m_btnMedicalRecord->style());
        m_btnEditInfo->setVisible(false); 
        m_detailStackedWidget->setCurrentWidget(m_recordSubPage);
    });

    connect(m_btnBack, &QPushButton::clicked, this, &PatientView::showPatientList);
    
    connect(m_btnEditInfo, &QPushButton::clicked, this, [this, radioContainer]() {
        if (m_btnEditInfo->text() == "Chỉnh sửa thông tin") {
            txtPatientID->setReadOnly(false);
            txtFullName->setReadOnly(false);
            txtDob->setReadOnly(false);
            txtPhone->setReadOnly(false);
            txtAddress->setReadOnly(false);
            txtCitizenID->setReadOnly(false);
            txtEmail->setReadOnly(false);
            txtBloodType->setReadOnly(false);
            txtAllergies->setReadOnly(false);
            txtInsurance->setReadOnly(false);
            txtEmergencyName->setReadOnly(false);
            txtEmergencyPhone->setReadOnly(false);
            txtNotes->setReadOnly(false);
            radMale->setEnabled(true);
            radFemale->setEnabled(true);
            radioContainer->setStyleSheet(
                "QWidget#genderContainer { border: 1px solid #0284C7; border-radius: 6px; background-color: #FFFFFF; padding: 0px 8px; }"
            );
            m_btnEditInfo->setText("Lưu thông tin");
        } else {
            txtPatientID->setReadOnly(true);
            txtFullName->setReadOnly(true);
            txtDob->setReadOnly(true);
            txtPhone->setReadOnly(true);
            txtAddress->setReadOnly(true);
            txtCitizenID->setReadOnly(true);
            txtEmail->setReadOnly(true);
            txtBloodType->setReadOnly(true);
            txtAllergies->setReadOnly(true);
            txtInsurance->setReadOnly(true);
            txtEmergencyName->setReadOnly(true);
            txtEmergencyPhone->setReadOnly(true);
            txtNotes->setReadOnly(true);
            radMale->setEnabled(false);
            radFemale->setEnabled(false);
            radioContainer->setStyleSheet(
                "QWidget#genderContainer { background-color: transparent; border: 1px solid transparent; }"
            );
            m_btnEditInfo->setText("Chỉnh sửa thông tin");
        }
    });

    connect(btnEditRecord, &QPushButton::clicked, this, [=]() {
        for (QWidget* widget : recordFields) {
            if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                textEdit->setReadOnly(false);
            } else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                lineEdit->setReadOnly(false);
            } else if (QTableWidget* tableWidget = qobject_cast<QTableWidget*>(widget)) {
                tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
            } else {
                widget->setEnabled(true);
            }
        }
    });

    connect(btnSaveRecord, &QPushButton::clicked, this, [=]() {
        for (QWidget* widget : recordFields) {
            if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                textEdit->setReadOnly(true);
            } else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                lineEdit->setReadOnly(true);
            } else if (QTableWidget* tableWidget = qobject_cast<QTableWidget*>(widget)) {
                tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
            } else {
                widget->setEnabled(false);
            }
        }
    });
}

void PatientView::handlePatientSelected(int row, int column) {
    Q_UNUSED(row);
    Q_UNUSED(column);
    
    m_btnInfo->setObjectName("activeMenuBtn");
    m_btnMedicalRecord->setObjectName("");
    m_btnInfo->setStyle(m_btnInfo->style());
    m_btnMedicalRecord->setStyle(m_btnMedicalRecord->style());
    
    m_detailStackedWidget->setCurrentWidget(m_infoSubPage);
    m_mainStackedWidget->setCurrentWidget(m_detailViewWidget);
}

void PatientView::showPatientList() {
    m_mainStackedWidget->setCurrentWidget(m_listViewWidget);
}

PatientView::~PatientView() {}