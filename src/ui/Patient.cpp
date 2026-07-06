#include "Patient.h"

Patient::Patient(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void Patient::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Khởi tạo Stack chính để tráo giữa màn hình Danh Sách và màn hình Chi Tiết
    m_mainStackedWidget = new QStackedWidget(this);
    m_mainLayout->addWidget(m_mainStackedWidget);

    // Tạo 2 màn hình chính
    createListView();
    createDetailView();

    // Thêm vào Stack tổng
    m_mainStackedWidget->addWidget(m_listViewWidget);
    m_mainStackedWidget->addWidget(m_detailViewWidget);

    // Mặc định hiển thị màn danh sách đầu tiên
    m_mainStackedWidget->setCurrentWidget(m_listViewWidget);
}

void Patient::createListView() {
    m_listViewWidget = new QWidget(this);
    QVBoxLayout* listLayout = new QVBoxLayout(m_listViewWidget);
    listLayout->setContentsMargins(15, 15, 15, 15);
    listLayout->setSpacing(15);

    // TabBar điều hướng 3 loại bệnh nhân
    m_patientTabBar = new QTabBar(m_listViewWidget);
    m_patientTabBar->addTab("Bệnh nhân nội trú");
    m_patientTabBar->addTab("Bệnh nhân ngoại trú");
    m_patientTabBar->addTab("Bệnh nhân cấp cứu");
    m_patientTabBar->setCursor(Qt::PointingHandCursor);
    m_patientTabBar->setStyleSheet(
        "QTabBar::tab {"
        "   background: #F8F9FA;"
        "   color: #5F6368;"
        "   border: 1px solid #DADCE0;"
        "   padding: 8px 16px;"
        "   font-family: 'Arial';"
        "   font-size: 13px;"
        "}"
        "QTabBar::tab:selected {"
        "   background: #FFFFFF;"
        "   color: #00966C;"
        "   border-bottom: 2px solid #00966C;"
        "   font-weight: bold;"
        "}"
    );
    listLayout->addWidget(m_patientTabBar);

    // Layout ngang chứa Bảng bên trái và Cột nút bên phải
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);

    // Bảng danh sách bệnh nhân mẫu
    m_patientTable = new QTableWidget(m_listViewWidget);
    m_patientTable->setColumnCount(3);
    m_patientTable->setHorizontalHeaderLabels({"STT", "Mã bệnh nhân", "Tên bệnh nhân"});
    m_patientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_patientTable->setAlternatingRowColors(true);
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows); // Chọn nguyên hàng
    m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);   // Không cho sửa trực tiếp
    m_patientTable->setCursor(Qt::PointingHandCursor);
   m_patientTable->setStyleSheet(
    "QTableWidget {"
    "   background-color: #FFFFFF;"
    "   border: 1px solid #EAEAEA;"
    "   gridline-color: #EAEAEA;"
    "   font-family: 'Arial';"
    "   font-size: 13px;"
    "}"
    "QTableWidget::item:selected {"
    "   background-color: transparent;" 
    "   color: #000000;"      
    "}"
    "QHeaderView::section {"
    "   background-color: #F8F9FA;"
    "   color: #202124;"
    "   padding: 8px;"
    "   font-weight: bold;"
    "   border: 1px solid #EAEAEA;"
    "}"
);

    // Thêm dữ liệu giả lập để test bấm click chuyển trang
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

    contentLayout->addWidget(m_patientTable);

    // Cột nút bấm bên phải (Bọc cố định 220px để tránh mất nút)
    QWidget* rightContainer = new QWidget(m_listViewWidget);
    rightContainer->setFixedWidth(220);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);

    m_addPatientButton = new QPushButton("+  Thêm bệnh nhân", rightContainer);
    m_addPatientButton->setCursor(Qt::PointingHandCursor);
    m_addPatientButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #1A73E8;"
        "   color: #FFFFFF;"
        "   font-family: 'Arial';"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 14px 15px 14px 22px;"
        "   text-align: left;"
        "}"
        "QPushButton:hover { background-color: #155CB4; }"
        "QPushButton:pressed { background-color: #0A428C; }"
    );
    rightLayout->addWidget(m_addPatientButton);
    rightLayout->addStretch();

    contentLayout->addWidget(rightContainer);
    listLayout->addLayout(contentLayout);

    // Kết nối sự kiện click dòng trên bảng để nhảy sang trang chi tiết
    connect(m_patientTable, &QTableWidget::cellClicked, this, &Patient::handlePatientSelected);
}

void Patient::createDetailView() {
    m_detailViewWidget = new QWidget(this);
    QVBoxLayout* detailLayout = new QVBoxLayout(m_detailViewWidget);
    detailLayout->setContentsMargins(15, 15, 15, 15);
    detailLayout->setSpacing(15);

    // Bố cục ngang cho trang chi tiết: Khung nội dung (Trái) và Cột menu chức năng (Phải)
    QHBoxLayout* mainContentLayout = new QHBoxLayout();
    mainContentLayout->setSpacing(20);

    m_detailStackedWidget = new QStackedWidget(m_detailViewWidget);
    m_detailStackedWidget->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 8px;");

    // Sub-page 1: Khung thông tin cá nhân rỗng
    m_infoSubPage = new QWidget(this);
    QHBoxLayout* infoMainLayout = new QHBoxLayout(m_infoSubPage);
    infoMainLayout -> setContentsMargins(20,20,20,20);  
    infoMainLayout->setSpacing(20);
    QVBoxLayout* leftPartLayout = new QVBoxLayout();
    leftPartLayout->setSpacing(20);

    QFrame* cardAdminInfo = new QFrame(m_infoSubPage);
    cardAdminInfo->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 8px; }");
    QVBoxLayout* cardAdminLayout = new QVBoxLayout(cardAdminInfo);
    cardAdminLayout->setContentsMargins(25, 25, 25, 25);
    cardAdminLayout->setSpacing(20); 
    QHBoxLayout* headerTitleLayout = new QHBoxLayout();
    headerTitleLayout ->setSpacing(15);
    QVBoxLayout* textTitleLayout = new QVBoxLayout();
    QLabel* lblMainTitle = new QLabel("Thông tin cá nhân bệnh nhân", cardAdminInfo);
    lblMainTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1F2937; border: none; font-family: 'Arial';");
    QLabel* lblSubTitle = new QLabel("Quản lý và cập nhật thông tin cá nhân của bệnh nhân", cardAdminInfo);
    lblSubTitle->setStyleSheet("font-size: 12px; color: #6B7280; border: none; font-family: 'Arial';");
    textTitleLayout->addWidget(lblMainTitle);
    textTitleLayout->addWidget(lblSubTitle);
    headerTitleLayout ->addLayout(textTitleLayout);
    headerTitleLayout -> addStretch();
    cardAdminLayout -> addLayout(headerTitleLayout);

    QGridLayout* inputGridLayout = new QGridLayout();
    inputGridLayout -> setHorizontalSpacing(15);   
    inputGridLayout -> setVerticalSpacing(15);
    QString labelStyle = "QLabel { font-size: 13px; font-weight: bold; color: #374151; border: none; font-family: 'Arial'; }";
    QString inputStyle = "QLineEdit, QComboBox { padding: 10px 12px; border: 1px solid #D1D5DB; border-radius: 6px; background-color: #FFFFFF; font-size: 13px; color: #1F2937; } "
                         "QLineEdit:focus, QComboBox:focus { border: 1px solid #1A73E8; }";
    
    QVBoxLayout* boxID= new QVBoxLayout();
    QLabel* lblID = new QLabel("Mã bệnh nhân <font color='red'>*</font>", cardAdminInfo); 
    lblID->setStyleSheet(labelStyle);
    txtPatientID = new QLineEdit("", cardAdminInfo);
    txtPatientID->setStyleSheet(inputStyle);
    boxID->addWidget(lblID); boxID->addWidget(txtPatientID);
    inputGridLayout->addLayout(boxID, 0, 0);

    QVBoxLayout* boxName = new QVBoxLayout();
    QLabel* lblName = new QLabel("Họ và tên <font color='red'>*</font>", cardAdminInfo); lblName->setStyleSheet(labelStyle);
    txtFullName = new QLineEdit("", cardAdminInfo); 
    txtFullName->setStyleSheet(inputStyle);
    boxName->addWidget(lblName); boxName->addWidget(txtFullName);
    inputGridLayout->addLayout(boxName, 0, 1);

    QVBoxLayout* boxDob = new QVBoxLayout();
    QLabel* lblDob = new QLabel("Ngày sinh <font color='red'>*</font>", cardAdminInfo); lblDob->setStyleSheet(labelStyle);
     txtDob = new QLineEdit("", cardAdminInfo); 
     txtDob->setStyleSheet(inputStyle);
    boxDob->addWidget(lblDob); boxDob->addWidget(txtDob);
    inputGridLayout->addLayout(boxDob, 0, 2);

    QVBoxLayout* boxGender = new QVBoxLayout();
    QLabel* lblGender = new QLabel("Giới tính <font color='red'>*</font>", cardAdminInfo); lblGender->setStyleSheet(labelStyle);
    QComboBox* cmbGender = new QComboBox(cardAdminInfo); cmbGender->addItems({"Nam", "Nữ", "Khác"}); cmbGender->setStyleSheet(inputStyle);
    boxGender->addWidget(lblGender); boxGender->addWidget(cmbGender);
    inputGridLayout->addLayout(boxGender, 1, 0);

    QVBoxLayout* boxPhone = new QVBoxLayout();
    QLabel* lblPhone = new QLabel("Số điện thoại <font color='red'>*</font>", cardAdminInfo); lblPhone->setStyleSheet(labelStyle);
    QLineEdit* txtPhone = new QLineEdit("", cardAdminInfo); txtPhone->setStyleSheet(inputStyle);
    boxPhone->addWidget(lblPhone); boxPhone->addWidget(txtPhone);
    inputGridLayout->addLayout(boxPhone, 1, 1);

    QVBoxLayout* boxAddress = new QVBoxLayout();
    QLabel* lblAddress = new QLabel("Địa chỉ <font color='red'>*</font>", cardAdminInfo); lblAddress->setStyleSheet(labelStyle);
    QLineEdit* txtAddress = new QLineEdit("", cardAdminInfo); txtAddress->setStyleSheet(inputStyle);
    boxAddress->addWidget(lblAddress); boxAddress->addWidget(txtAddress);
    inputGridLayout->addLayout(boxAddress, 1, 2);

    cardAdminLayout ->addLayout(inputGridLayout);
    leftPartLayout -> addWidget(cardAdminInfo);

    QFrame* cardNotes = new QFrame(m_infoSubPage);
    cardNotes->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 8px; }");
    QVBoxLayout* cardNotesLayout = new QVBoxLayout(cardNotes);
    cardNotesLayout->setContentsMargins(25, 25, 25, 25);
    cardNotesLayout->setSpacing(15);

    QHBoxLayout* notesTitleLayout = new QHBoxLayout();
    notesTitleLayout->setSpacing(10);
    QLabel* lblNotesTitle = new QLabel("Ghi chú", cardNotes); lblNotesTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #1F2937; border: none;");
    notesTitleLayout->addWidget(lblNotesTitle);
    notesTitleLayout->addStretch();
    cardNotesLayout->addLayout(notesTitleLayout);

    QTextEdit* txtNotes = new QTextEdit(cardNotes);
    txtNotes->setPlaceholderText("Nhập ghi chú về bệnh nhân...");
    txtNotes->setStyleSheet("QTextEdit { padding: 12px; border: 1px solid #D1D5DB; border-radius: 6px; background-color: #FFFFFF; font-size: 13px; color: #1F2937; } "
                            "QTextEdit:focus { border: 1px solid #1A73E8; }");
    cardNotesLayout->addWidget(txtNotes);
    
    leftPartLayout->addWidget(cardNotes);
    infoMainLayout->addLayout(leftPartLayout, 1);
    // Sub-page 2: Khung bệnh án lịch sử rỗng
    m_recordSubPage = new QWidget(this);
    QVBoxLayout* recordLayout = new QVBoxLayout(m_recordSubPage);
    QLabel* recordPlaceholder = new QLabel("Khung cấu trúc: Bệnh án bệnh nhân", m_recordSubPage);
    recordPlaceholder->setAlignment(Qt::AlignCenter);
    recordPlaceholder->setStyleSheet("font-size: 16px; color: #777777; font-weight: 500;");
    recordLayout->addWidget(recordPlaceholder);

    m_detailStackedWidget->addWidget(m_infoSubPage);
    m_detailStackedWidget->addWidget(m_recordSubPage);
    mainContentLayout->addWidget(m_detailStackedWidget, 1);

    // --- Khung Phải: Thiết kế cột nút bấm điều hướng dọc ---
    QWidget* rightMenuContainer = new QWidget(m_detailViewWidget);
    rightMenuContainer->setFixedWidth(220);
    QVBoxLayout* menuLayout = new QVBoxLayout(rightMenuContainer);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(12);

    QString btnStyle = 
        "QPushButton {"
        "   background-color: #1A73E8;"
        "   color: #FFFFFF;"
        "   font-family: 'Arial';"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 14px 15px 14px 22px;"
        "   text-align: left;"
        "}"
        "QPushButton:hover { background-color: #155CB4; }"
        "QPushButton:pressed { background-color: #0A428C; }"
        "QPushButton#activeMenuBtn { background-color: #0A428C; }"; // Màu đậm cố định khi active

    m_btnInfo = new QPushButton(" Thông tin cá nhân", rightMenuContainer);
    m_btnInfo->setCursor(Qt::PointingHandCursor);
    m_btnInfo->setStyleSheet(btnStyle);
    m_btnInfo->setObjectName("activeMenuBtn"); // Mặc định nút này được active trước
    
    m_btnEditInfo = new QPushButton("Chỉnh sửa thông tin", rightMenuContainer);
    m_btnEditInfo->setCursor(Qt::PointingHandCursor);
    m_btnEditInfo->setStyleSheet(btnStyle + "QPushButton { background-color: #00966C; } QPushButton:hover { background-color: #007D5A; }");

     m_btnDeleteInfo = new QPushButton("Xóa bệnh nhân", rightMenuContainer);
    m_btnDeleteInfo->setCursor(Qt::PointingHandCursor);
    m_btnDeleteInfo->setStyleSheet(btnStyle + "QPushButton { background-color: #D93025; } QPushButton:hover { background-color: #C53030; }");
    
    m_btnMedicalRecord = new QPushButton("Bệnh án", rightMenuContainer);
    m_btnMedicalRecord->setCursor(Qt::PointingHandCursor);
    m_btnMedicalRecord->setStyleSheet(btnStyle);

    m_btnBack = new QPushButton("🔙  Quay lại", rightMenuContainer);
    m_btnBack->setCursor(Qt::PointingHandCursor);
    m_btnBack->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   color: #5F6368;"
        "   font-family: 'Arial';"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border: 1px solid #DADCE0;"
        "   border-radius: 6px;"
        "   padding: 12px 15px 12px 22px;"
        "   text-align: left;"
        "}"
        "QPushButton:hover { background-color: #F1F3F4; }"
    );

   menuLayout->addWidget(m_btnInfo);
    menuLayout->addWidget(m_btnEditInfo);     
    menuLayout->addWidget(m_btnDeleteInfo);   
    menuLayout->addWidget(m_btnMedicalRecord);
    menuLayout->addStretch(); 
    menuLayout->addWidget(m_btnBack);

    mainContentLayout->addWidget(rightMenuContainer);
    detailLayout->addLayout(mainContentLayout);

    // Kết nối các nút điều hướng của trang chi tiết bệnh nhân
    connect(m_btnInfo, &QPushButton::clicked, this, [this]() {
        m_btnInfo->setObjectName("activeMenuBtn");
        m_btnMedicalRecord->setObjectName("");
        m_btnInfo->setStyle(m_btnInfo->style());
        m_btnMedicalRecord->setStyle(m_btnMedicalRecord->style());
        m_detailStackedWidget->setCurrentWidget(m_infoSubPage);
    });

    connect(m_btnMedicalRecord, &QPushButton::clicked, this, [this]() {
        m_btnMedicalRecord->setObjectName("activeMenuBtn");
        m_btnInfo->setObjectName("");
        m_btnInfo->setStyle(m_btnInfo->style());
        m_btnMedicalRecord->setStyle(m_btnMedicalRecord->style());
        m_detailStackedWidget->setCurrentWidget(m_recordSubPage);
    });

    connect(m_btnBack, &QPushButton::clicked, this, &Patient::showPatientList);
}

// Xử lý khi click vào 1 ô bất kỳ trên bảng
void Patient::handlePatientSelected(int row, int column) {
    Q_UNUSED(row);
    Q_UNUSED(column);
    
    // Reset lại trạng thái menu bên phải trước khi hiển thị trang chi tiết
    m_btnInfo->setObjectName("activeMenuBtn");
    m_btnMedicalRecord->setObjectName("");
    m_btnInfo->setStyle(m_btnInfo->style());
    m_btnMedicalRecord->setStyle(m_btnMedicalRecord->style());
    
    m_detailStackedWidget->setCurrentWidget(m_infoSubPage);
    m_mainStackedWidget->setCurrentWidget(m_detailViewWidget); // Tráo sang trang chi tiết bệnh nhân
}

// Quay lại màn hình danh sách ban đầu
void Patient::showPatientList() {
    m_mainStackedWidget->setCurrentWidget(m_listViewWidget);
}

Patient::~Patient() {}