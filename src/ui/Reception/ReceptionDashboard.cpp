#include "ReceptionDashboard.h"
#include "../../model/Doctor.h"
#include "../../model/IAuthenticatable.h"
#include "../../repository/DatabaseManager.h"
#include "../../service/StaffService.h"
#include "../../service/PatientService.h"
#include "../../service/AppointmentService.h"
#include "PatientRegistrationDialog.h"
#include <QMessageBox>
#include <QStringList>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QFrame>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QGraphicsDropShadowEffect>
#include <QFormLayout>
#include <QLineEdit>

ReceptionDashboardWidget::ReceptionDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<AppointmentService> appointmentService,
    QWidget *parent)
    : BaseDashboardWidget(user, staffService, patientService, appointmentService, parent),
      m_currentUser(user), m_staffService(staffService) {
    
    initializeDashboard();

    buildSidebar();

    m_stackedWidget = new QStackedWidget(m_mainContentWidget);
    m_mainContentLayout->addWidget(m_stackedWidget, 1);

    buildOverviewPage();
    buildRegisterPage();
    buildPatientsPage();

    m_stackedWidget->addWidget(m_overviewPage);
    m_stackedWidget->addWidget(m_registerPage);
    m_stackedWidget->addWidget(m_patientsPage);

    // Default to Overview
    switchPage(0, m_btnOverview);

    fillDashboardData();
}

void ReceptionDashboardWidget::fillDashboardData() {
    // Fake data for UI preview
    if (m_lblRevenue) {
        m_lblRevenue->setText("125,500,000 VND");
    }
    if (m_lblPatientNum) {
        m_lblPatientNum->setText("428 bệnh nhân");
    }
}

QFrame* ReceptionDashboardWidget::makeCard(QWidget* parent) {
    QFrame* card = new QFrame(parent);
    card->setStyleSheet("QFrame { background-color: #FFFFFF; border-radius: 12px; }");
    
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 5);
    card->setGraphicsEffect(shadow);
    
    return card;
}

void ReceptionDashboardWidget::buildSidebar() {
    QWidget* sidebar = new QWidget(this);
    sidebar->setFixedWidth(240);
    sidebar->setStyleSheet("QWidget { background-color: #FFFFFF; border-right: 1px solid #EAEAEA; }");

    QVBoxLayout* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(0, 20, 0, 20);
    layout->setSpacing(5);

    // Title / User info (Can be inherited from Base, but we add our buttons here)
    QLabel* lblTitle = new QLabel("LỄ TÂN", sidebar);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #4B94F2; margin-bottom: 20px; border: none;");
    layout->addWidget(lblTitle);

    m_btnOverview = new QPushButton("Tổng quan", sidebar);
    m_btnOverview->setCheckable(true);
    m_btnOverview->setCursor(Qt::PointingHandCursor);

    m_btnRegister = new QPushButton("Đăng ký khám", sidebar);
    m_btnRegister->setCheckable(true);
    m_btnRegister->setCursor(Qt::PointingHandCursor);

    m_btnPatients = new QPushButton("DS Bệnh nhân", sidebar);
    m_btnPatients->setCheckable(true);
    m_btnPatients->setCursor(Qt::PointingHandCursor);

    layout->addWidget(m_btnOverview);
    layout->addWidget(m_btnRegister);
    layout->addWidget(m_btnPatients);
    layout->addStretch();
    
    m_btnLogout = new QPushButton("Đăng xuất", sidebar);
    m_btnLogout->setStyleSheet("QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; color: #D32F2F; border: none; border-radius: 0px; background-color: transparent; font-weight: bold; }"
                               "QPushButton:hover { background-color: #FFEBEE; }");
    m_btnLogout->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_btnLogout);

    // Add sidebar to main layout (BaseDashboardWidget uses QHBoxLayout for main content)
    // Actually, BaseDashboardWidget provides m_sidebarLayout, we should just use it
    m_sidebarLayout->addWidget(sidebar);

    // Connect signals
    connect(m_btnOverview, &QPushButton::clicked, this, [this](){ switchPage(0, m_btnOverview); });
    connect(m_btnRegister, &QPushButton::clicked, this, [this](){ switchPage(1, m_btnRegister); });
    connect(m_btnPatients, &QPushButton::clicked, this, [this](){ switchPage(2, m_btnPatients); });
    connect(m_btnLogout, &QPushButton::clicked, this, &BaseDashboardWidget::logoutRequested);
}

void ReceptionDashboardWidget::switchPage(int index, QPushButton* activeBtn) {
    m_stackedWidget->setCurrentIndex(index);
    m_btnOverview->setChecked(false);
    m_btnRegister->setChecked(false);
    m_btnPatients->setChecked(false);
    activeBtn->setChecked(true);
}

void ReceptionDashboardWidget::buildOverviewPage() {
    m_overviewPage = new QWidget();
    m_overviewPage->setStyleSheet("background-color: #F8F9FA;");
    
    QVBoxLayout* layout = new QVBoxLayout(m_overviewPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel* lblTitle = new QLabel("Tổng quan tháng này", m_overviewPage);
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #202124;");
    layout->addWidget(lblTitle);

    // Cards layout
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    // Card 1: Revenue
    QFrame* card1 = makeCard(m_overviewPage);
    QVBoxLayout* c1Layout = new QVBoxLayout(card1);
    QLabel* c1Title = new QLabel("Doanh thu", card1);
    c1Title->setStyleSheet("color: #5F6368; font-size: 14px;");
    m_lblRevenue = new QLabel("0 VND", card1);
    m_lblRevenue->setStyleSheet("color: #4B94F2; font-size: 28px; font-weight: bold;");
    c1Layout->addWidget(c1Title);
    c1Layout->addWidget(m_lblRevenue);
    cardsLayout->addWidget(card1);

    // Card 2: Patients
    QFrame* card2 = makeCard(m_overviewPage);
    QVBoxLayout* c2Layout = new QVBoxLayout(card2);
    QLabel* c2Title = new QLabel("Lượt khám", card2);
    c2Title->setStyleSheet("color: #5F6368; font-size: 14px;");
    m_lblPatientNum = new QLabel("0", card2);
    m_lblPatientNum->setStyleSheet("color: #34A853; font-size: 28px; font-weight: bold;");
    c2Layout->addWidget(c2Title);
    c2Layout->addWidget(m_lblPatientNum);
    cardsLayout->addWidget(card2);

    cardsLayout->addStretch();
    layout->addLayout(cardsLayout);

    // Recent activity table (Mock)
    QLabel* lblSubTitle = new QLabel("Hoạt động gần đây", m_overviewPage);
    lblSubTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #202124; margin-top: 20px;");
    layout->addWidget(lblSubTitle);

    QTableWidget* mockTable = new QTableWidget(3, 4, m_overviewPage);
    mockTable->setHorizontalHeaderLabels({"Thời gian", "Bệnh nhân", "Bác sĩ", "Trạng thái"});
    mockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mockTable->setStyleSheet("QTableWidget { background-color: white; border-radius: 8px; border: 1px solid #EAEAEA; }"
                             "QHeaderView::section { background-color: #F1F3F4; font-weight: bold; border: none; padding: 8px; }");
    layout->addWidget(mockTable);

    layout->addStretch();
}

void ReceptionDashboardWidget::loadDoctorsBySpecialty(const QString& specialty) {
    m_comboDoctor->clear();
    auto doctors = m_staffService->searchDoctors("", specialty, -1, "", true, false);
    for (const auto& doc : doctors) {
        auto docModel = std::dynamic_pointer_cast<Doctor>(doc);
        if (docModel) {
            m_comboDoctor->addItem(docModel->getFullName(), docModel->getStaffCode());
        }
    }
}

void ReceptionDashboardWidget::onContinueClicked() {
    QString phone = m_txtPatientPhone->text().trimmed();
    QString citizenId = m_txtPatientCitizenId->text().trimmed();
    if (phone.isEmpty() && citizenId.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập Số điện thoại hoặc CMND/CCCD bệnh nhân.");
        return;
    }
    
    auto patientOpt = m_basePatientService->getPatientByPhoneOrCitizenId(phone, citizenId);
    if (patientOpt) {
        m_currentPatientId = patientOpt->patientId;
        m_txtPatientPhone->setText(patientOpt->phone); // Fill phone if found by CitizenId
        m_txtPatientPhone->setReadOnly(true);
        m_txtPatientCitizenId->setReadOnly(true);
        
        m_apptCard->setVisible(true);
        m_btnContinue->setText("Đã xác nhận");
        m_btnContinue->setEnabled(false);
        m_btnContinue->setStyleSheet("background-color: #EAEAEA; color: #999; padding: 10px 25px; border-radius: 6px; font-size: 15px; font-weight: bold;");
    } else {
        QMessageBox::warning(this, "Lỗi", "Không tìm thấy bệnh nhân. Vui lòng tạo bệnh nhân mới (hoặc yêu cầu đồng nghiệp tạo) trước khi đăng ký khám.");
    }
}

void ReceptionDashboardWidget::onConfirmClicked() {
    if (m_currentPatientId == -1) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng xác nhận thông tin bệnh nhân trước.");
        return;
    }
    
    QString doctorCode = m_comboDoctor->currentData().toString();
    if (doctorCode.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng chọn bác sĩ.");
        return;
    }
    
    QString date = m_dateEdit->date().toString("yyyy-MM-dd");
    
    // Get selected time slot
    QList<QTableWidgetItem*> selectedItems = m_timeSlotTable->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng chọn một khung giờ trống.");
        return;
    }
    
    int row = selectedItems.first()->row();
    QString timeSlot = m_timeSlotTable->item(row, 0)->text();
    QString startTime = timeSlot.split(" - ").first();
    int createdBy = m_currentUser->getAccountId();
    
    AppointmentInputDTO input = {
        m_currentPatientId,
        doctorCode,
        createdBy,
        date,
        startTime,
        "Khám bệnh"
    };
    
    bool success = m_baseAppointmentService->createAppointment(input);
    if (success) {
        QMessageBox::information(this, "Thành công", "Đăng ký lịch khám thành công!");
        
        // Reset form
        m_currentPatientId = -1;
        m_txtPatientPhone->clear();
        m_txtPatientPhone->setReadOnly(false);
        m_txtPatientCitizenId->clear();
        m_txtPatientCitizenId->setReadOnly(false);
        
        m_apptCard->setVisible(false);
        m_btnContinue->setText("Xác nhận & Tiếp tục");
        m_btnContinue->setEnabled(true);
        m_btnContinue->setStyleSheet("background-color: #4B94F2; color: white; padding: 10px 25px; border-radius: 6px; font-size: 15px; font-weight: bold;");
        m_timeSlotTable->setRowCount(0);
    } else {
        QMessageBox::warning(this, "Lỗi", "Đã xảy ra lỗi khi tạo lịch khám.");
    }
}

void ReceptionDashboardWidget::buildRegisterPage() {
    m_registerPage = new QWidget();
    m_registerPage->setStyleSheet("background-color: #F8F9FA;");
    
    QVBoxLayout* layout = new QVBoxLayout(m_registerPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* lblTitle = new QLabel("Đăng ký khám bệnh", m_registerPage);
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #202124;");
    
    QPushButton* btnAddPatient = new QPushButton("+ Thêm bệnh nhân mới", m_registerPage);
    btnAddPatient->setStyleSheet("background-color: #34A853; color: white; padding: 8px 15px; border-radius: 6px; font-weight: bold;");
    btnAddPatient->setCursor(Qt::PointingHandCursor);
    
    titleLayout->addWidget(lblTitle);
    titleLayout->addStretch();
    titleLayout->addWidget(btnAddPatient);
    
    layout->addLayout(titleLayout);

    connect(btnAddPatient, &QPushButton::clicked, this, [this]() {
        PatientRegistrationDialog dialog(this);
        connect(&dialog, &PatientRegistrationDialog::saved, this, [this](QString citizenId, QString /*name*/) {
            // Mock patient registration success
            m_currentPatientId = 9999; // Dummy ID for UI flow
            
            // Auto-fill Step 1
            m_txtPatientCitizenId->setText(citizenId.isEmpty() ? "123456789 (Mới)" : citizenId); 
            m_txtPatientPhone->setText("09xxxxxxxx (Mới)");
            m_txtPatientPhone->setReadOnly(true);
            m_txtPatientCitizenId->setReadOnly(true);
            
            // Mark Step 1 as completed
            m_btnContinue->setText("Đã xác nhận");
            m_btnContinue->setEnabled(false);
            m_btnContinue->setStyleSheet("background-color: #EAEAEA; color: #999; padding: 10px 25px; border-radius: 6px; font-size: 15px; font-weight: bold;");
            
            // Show Step 2
            m_apptCard->setVisible(true);
        });
        dialog.exec();
    });

    // Common style for inputs
    QString inputStyle = "QLineEdit, QComboBox, QDateEdit { "
                         "min-height: 36px; padding-left: 10px; border: 1px solid #BDBDBD; "
                         "border-radius: 6px; background-color: #FFFFFF; "
                         "color: #333333; font-size: 14px; } "
                         "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { "
                         "border: 1px solid #4B94F2; } "
                         "QComboBox::drop-down, QDateEdit::drop-down { "
                         "subcontrol-origin: padding; subcontrol-position: top right; "
                         "width: 30px; border-left: 1px solid #EAEAEA; }";

    // ---------------------------------------------------------
    // STEP 1: PATIENT INFO CARD
    // ---------------------------------------------------------
    QFrame* patientCard = makeCard(m_registerPage);
    QVBoxLayout* patientLayout = new QVBoxLayout(patientCard);
    patientLayout->setContentsMargins(25, 25, 25, 25);
    patientLayout->setSpacing(15);

    QLabel* lblStep1 = new QLabel("Bước 1: Thông tin bệnh nhân", patientCard);
    lblStep1->setStyleSheet("font-size: 18px; font-weight: bold; color: #4B94F2;");
    patientLayout->addWidget(lblStep1);

    QHBoxLayout* patientFieldsLayout = new QHBoxLayout();
    
    QVBoxLayout* col1 = new QVBoxLayout();
    QLabel* lblPhone = new QLabel("Số điện thoại (*):", patientCard);
    lblPhone->setStyleSheet("font-weight: bold; color: #555;");
    m_txtPatientPhone = new QLineEdit(patientCard);
    m_txtPatientPhone->setPlaceholderText("Nhập số điện thoại để tìm kiếm...");
    m_txtPatientPhone->setStyleSheet(inputStyle);
    col1->addWidget(lblPhone);
    col1->addWidget(m_txtPatientPhone);

    QVBoxLayout* col2 = new QVBoxLayout();
    QLabel* lblCitizen = new QLabel("Căn cước công dân (*):", patientCard);
    lblCitizen->setStyleSheet("font-weight: bold; color: #555;");
    m_txtPatientCitizenId = new QLineEdit(patientCard);
    m_txtPatientCitizenId->setPlaceholderText("Nhập CMND/CCCD để tìm kiếm...");
    m_txtPatientCitizenId->setStyleSheet(inputStyle);
    col2->addWidget(lblCitizen);
    col2->addWidget(m_txtPatientCitizenId);

    patientFieldsLayout->addLayout(col1);
    patientFieldsLayout->addLayout(col2);
    patientLayout->addLayout(patientFieldsLayout);

    QHBoxLayout* btnNextLayout = new QHBoxLayout();
    btnNextLayout->addStretch();
    m_btnContinue = new QPushButton("Xác nhận & Tiếp tục", patientCard);
    m_btnContinue->setStyleSheet("background-color: #4B94F2; color: white; padding: 10px 25px; border-radius: 6px; font-size: 15px; font-weight: bold;");
    m_btnContinue->setCursor(Qt::PointingHandCursor);
    btnNextLayout->addWidget(m_btnContinue);
    
    connect(m_btnContinue, &QPushButton::clicked, this, &ReceptionDashboardWidget::onContinueClicked);
    
    patientLayout->addLayout(btnNextLayout);
    layout->addWidget(patientCard);

    // ---------------------------------------------------------
    // STEP 2: APPOINTMENT INFO CARD
    // ---------------------------------------------------------
    m_apptCard = makeCard(m_registerPage);
    QVBoxLayout* apptLayout = new QVBoxLayout(m_apptCard);
    apptLayout->setContentsMargins(25, 25, 25, 25);
    apptLayout->setSpacing(15);
    
    // Initially hidden until step 1 is completed
    m_apptCard->setVisible(false);

    QLabel* lblStep2 = new QLabel("Bước 2: Thông tin lịch khám", m_apptCard);
    lblStep2->setStyleSheet("font-size: 18px; font-weight: bold; color: #4B94F2;");
    apptLayout->addWidget(lblStep2);

    QHBoxLayout* apptFieldsLayout = new QHBoxLayout();

    QVBoxLayout* colSpecialty = new QVBoxLayout();
    QLabel* lblSpecialty = new QLabel("Chuyên khoa:", m_apptCard);
    lblSpecialty->setStyleSheet("font-weight: bold; color: #555;");
    m_comboSpecialty = new QComboBox(m_apptCard);
    m_comboSpecialty->addItems({"Tất cả", "Khoa Nội", "Khoa Ngoại", "Khoa Nhi", "Da liễu"});
    m_comboSpecialty->setStyleSheet(inputStyle);
    colSpecialty->addWidget(lblSpecialty);
    colSpecialty->addWidget(m_comboSpecialty);
    
    connect(m_comboSpecialty, &QComboBox::currentTextChanged, this, &ReceptionDashboardWidget::loadDoctorsBySpecialty);

    QVBoxLayout* colDoctor = new QVBoxLayout();
    QLabel* lblDoctor = new QLabel("Bác sĩ:", m_apptCard);
    lblDoctor->setStyleSheet("font-weight: bold; color: #555;");
    m_comboDoctor = new QComboBox(m_apptCard);
    m_comboDoctor->setStyleSheet(inputStyle);
    colDoctor->addWidget(lblDoctor);
    colDoctor->addWidget(m_comboDoctor);

    QVBoxLayout* colDate = new QVBoxLayout();
    QLabel* lblDate = new QLabel("Ngày khám:", m_apptCard);
    lblDate->setStyleSheet("font-weight: bold; color: #555;");
    m_dateEdit = new QDateEdit(QDate::currentDate(), m_apptCard);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setStyleSheet(inputStyle);
    colDate->addWidget(lblDate);
    colDate->addWidget(m_dateEdit);

    apptFieldsLayout->addLayout(colSpecialty);
    apptFieldsLayout->addLayout(colDoctor);
    apptFieldsLayout->addLayout(colDate);
    apptLayout->addLayout(apptFieldsLayout);

    QPushButton* btnSearchSlots = new QPushButton("TÌM GIỜ TRỐNG", m_apptCard);
    btnSearchSlots->setStyleSheet("background-color: #F29C1F; color: white; padding: 12px; border-radius: 6px; font-size: 14px; font-weight: bold; margin-top: 10px;");
    btnSearchSlots->setCursor(Qt::PointingHandCursor);
    apptLayout->addWidget(btnSearchSlots);

    // Table for time slots
    m_timeSlotTable = new QTableWidget(0, 2, m_apptCard);
    m_timeSlotTable->setHorizontalHeaderLabels({"Khung giờ", "Trạng thái"});
    m_timeSlotTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_timeSlotTable->setStyleSheet("QTableWidget { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 6px; }"
                                   "QHeaderView::section { background-color: #F1F3F4; font-weight: bold; padding: 8px; border: none; }");
    m_timeSlotTable->setMinimumHeight(150);
    m_timeSlotTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_timeSlotTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    apptLayout->addWidget(m_timeSlotTable);

    QHBoxLayout* btnSubmitLayout = new QHBoxLayout();
    btnSubmitLayout->addStretch();
    m_btnConfirm = new QPushButton("Hoàn tất đăng ký", m_apptCard);
    m_btnConfirm->setStyleSheet("background-color: #34A853; color: white; padding: 12px 30px; border-radius: 6px; font-size: 16px; font-weight: bold;");
    m_btnConfirm->setCursor(Qt::PointingHandCursor);
    btnSubmitLayout->addWidget(m_btnConfirm);
    
    apptLayout->addLayout(btnSubmitLayout);
    layout->addWidget(m_apptCard);
    layout->addStretch();

    // ---------------------------------------------------------
    // SIGNALS / LOGIC
    // ---------------------------------------------------------
    connect(btnSearchSlots, &QPushButton::clicked, this, &ReceptionDashboardWidget::loadAvailableTimeSlots);
    connect(m_btnConfirm, &QPushButton::clicked, this, &ReceptionDashboardWidget::onConfirmClicked);
    
    // Initial data load
    loadDoctorsBySpecialty("Tất cả");
}

void ReceptionDashboardWidget::loadAvailableTimeSlots() {
    m_timeSlotTable->setRowCount(0);
    // Add some fake time slots
    QStringList timeSlots = {"08:00 - 08:30", "09:00 - 09:30", "14:00 - 14:30", "15:30 - 16:00"};
    for (int i = 0; i < timeSlots.size(); ++i) {
        m_timeSlotTable->insertRow(i);
        m_timeSlotTable->setItem(i, 0, new QTableWidgetItem(timeSlots[i]));
        QTableWidgetItem* statusItem = new QTableWidgetItem("Trống");
        statusItem->setForeground(QBrush(QColor("#34A853")));
        m_timeSlotTable->setItem(i, 1, statusItem);
    }
}

void ReceptionDashboardWidget::buildPatientsPage() {
    m_patientsPage = new QWidget();
    m_patientsPage->setStyleSheet("background-color: #F8F9FA;");
    
    QVBoxLayout* layout = new QVBoxLayout(m_patientsPage);
    layout->setContentsMargins(30, 30, 30, 30);
    
    QLabel* lblTitle = new QLabel("Danh sách bệnh nhân (Đang phát triển)", m_patientsPage);
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #202124;");
    
    layout->addWidget(lblTitle);
    layout->addStretch();
}
