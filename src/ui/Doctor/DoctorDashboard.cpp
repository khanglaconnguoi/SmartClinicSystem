#include "DoctorDashboard.h"
#include "../../model/IAuthenticatable.h"
#include "../../repository/DatabaseManager.h"
#include "ClinicalExamWidget.h"
#include <QDate>
#include <QHeaderView>
#include <QStackedWidget>
#include <QCalendarWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <QDebug>

// Thêm các thư viện Chart bắt buộc để vẽ cột
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

// =============================================================================
// CONSTRUCTOR
// =============================================================================
DoctorDashboardWidget::DoctorDashboardWidget(std::shared_ptr<IAuthenticatable> user, QWidget *parent)
    : BaseDashboardWidget(parent), m_currentUser(user)
{
    // Gọi hàm dựng sẵn khuôn của lớp cha
    initializeDashboard();
}

// =============================================================================
// fillDashboardData() - Hàm Template Method của lớp cha gọi vào
// =============================================================================
void DoctorDashboardWidget::fillDashboardData() {
    // 0. 🪪 Dựng sidebar của bác sĩ
    buildSidebar();

    // 1. 👨‍⚕️ Nạp thông tin bác sĩ trên Topbar
    if (m_currentUser && m_docNameLabel) {
        m_docNameLabel->setText(m_currentUser->getFullName());
    }

    if (m_docAvatarBtn && m_currentUser) {
        QPixmap rawPixmap = m_currentUser->getAvatar();
        if (rawPixmap.isNull()) {
            rawPixmap = QPixmap(36, 36);
            rawPixmap.fill(QColor("#00966C")); 
        }

        int size = 36;
        m_docAvatarBtn->setFixedSize(size, size);
        QPixmap scaledPixmap = rawPixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        QPixmap targetPixmap(size, size);
        targetPixmap.fill(Qt::transparent);

        QPainter painter(&targetPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        QPainterPath path;
        path.addEllipse(0, 0, size, size);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, scaledPixmap);
        painter.end();

        m_docAvatarBtn->setIconSize(QSize(size, size)); 
        m_docAvatarBtn->setIcon(QIcon(targetPixmap));
        m_docAvatarBtn->setStyleSheet(
            "QPushButton { "
            "   background-color: transparent; "
            "   border: none; "
            "   width: 36px; "
            "   height: 36px; "
            "   padding: 0px; "
            "   margin: 0px; "
            "}"
        );
        m_docAvatarBtn->update();

        disconnect(m_docAvatarBtn, &QPushButton::clicked, nullptr, nullptr);
        connect(m_docAvatarBtn, &QPushButton::clicked, this, [this]() {
            qDebug() << "Đang mở trang Profile của bác sĩ...";
        });
    }

    // 2. Khởi tạo QStackedWidget cho vùng nội dung chính
    m_stackedWidget = new QStackedWidget(m_mainContentWidget);
    m_mainContentLayout->addWidget(m_stackedWidget, 1);

    // 3. Xây dựng cấu trúc các trang con
    buildOverviewPage();
    buildPatientsPage();
    buildAppointmentsPage();
    buildSettingsPage();
    buildClinicalExamPage();

    // 3.5 Tải dữ liệu lịch hẹn lên các bảng
    refreshAppointmentsTables();

    // 4. Chọn trang mặc định (Dashboard Overview)
    switchPage(0, m_btnDash);
}

// =============================================================================
// SIDEBAR BUILDER
// =============================================================================
void DoctorDashboardWidget::buildSidebar() {
    if (!m_sidebarLayout) return;

    m_btnDash     = new QPushButton("📊 Dashboard",    m_sidebarFrame);
    m_btnPatients = new QPushButton("👥 Patients",     m_sidebarFrame);
    m_btnAppoint  = new QPushButton("📅 Appointments", m_sidebarFrame);
    m_btnSetting  = new QPushButton("⚙️ Settings",     m_sidebarFrame);

    m_sidebarLayout->addWidget(m_btnDash);
    m_sidebarLayout->addWidget(m_btnPatients);
    m_sidebarLayout->addWidget(m_btnAppoint);
    m_sidebarLayout->addWidget(m_btnSetting);
    m_sidebarLayout->addStretch();

    m_btnLogout = new QPushButton("🚪 Log Out", m_sidebarFrame);
    m_btnLogout->setStyleSheet("color: #D93025;");
    m_sidebarLayout->addWidget(m_btnLogout);

    connect(m_btnLogout, &QPushButton::clicked, this, &BaseDashboardWidget::logoutRequested);

    connect(m_btnDash,     &QPushButton::clicked, this, [this]() { switchPage(0, m_btnDash); });
    connect(m_btnPatients, &QPushButton::clicked, this, [this]() { switchPage(1, m_btnPatients); });
    connect(m_btnAppoint,  &QPushButton::clicked, this, [this]() { switchPage(2, m_btnAppoint); });
    connect(m_btnSetting,  &QPushButton::clicked, this, [this]() { switchPage(3, m_btnSetting); });
}

// =============================================================================
// PAGE BUILDERS
// =============================================================================
void DoctorDashboardWidget::buildOverviewPage() {
    m_overviewPage = new QWidget(this);
    QVBoxLayout* pageLayout = new QVBoxLayout(m_overviewPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(20);

    createDoctorCards(m_overviewPage, pageLayout);
    createDoctorCharts(m_overviewPage, pageLayout);
    createDoctorTable(m_overviewPage, pageLayout);

    m_stackedWidget->addWidget(m_overviewPage);
}

void DoctorDashboardWidget::buildPatientsPage() {
    m_patientsPage = new QWidget(this);
    QVBoxLayout* pageLayout = new QVBoxLayout(m_patientsPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(20);

    QLabel* title = new QLabel("👥 Quản Lý Bệnh Nhân", m_patientsPage);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #111827;");
    pageLayout->addWidget(title);

    QTableWidget* fullPatientTable = new QTableWidget(m_patientsPage);
    fullPatientTable->setColumnCount(6);
    fullPatientTable->setHorizontalHeaderLabels({"Mã BN", "Họ Tên", "Ngày Sinh", "Giới Tính", "Số Điện Thoại", "Bệnh Án"});
    fullPatientTable->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 8px; gridline-color: #F1F3F4; }"
        "QHeaderView::section { background-color: #F8F9FA; padding: 10px; font-weight: bold; border: none; border-bottom: 2px solid #EAEAEA; color: #5F6368; }"
        "QTableWidget::item { padding: 12px; color: #3C4043; }"
    );
    fullPatientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    fullPatientTable->verticalHeader()->setVisible(false);
    
    fullPatientTable->setRowCount(4);
    auto addRow = [&](int r, QString code, QString name, QString dob, QString gender, QString phone, QString record) {
        fullPatientTable->setItem(r, 0, new QTableWidgetItem(code));
        fullPatientTable->setItem(r, 1, new QTableWidgetItem(name));
        fullPatientTable->setItem(r, 2, new QTableWidgetItem(dob));
        fullPatientTable->setItem(r, 3, new QTableWidgetItem(gender));
        fullPatientTable->setItem(r, 4, new QTableWidgetItem(phone));
        fullPatientTable->setItem(r, 5, new QTableWidgetItem(record));
    };
    addRow(0, "BN-0029", "Trần Văn Nam", "12/04/1988", "Nam", "0912345678", "Viêm dạ dày");
    addRow(1, "BN-1082", "Lê Thị Hồng", "24/09/1995", "Nữ", "0987654321", "Suy nhược cơ thể");
    addRow(2, "BN-4820", "Phan Hoàng Bách", "05/01/2002", "Nam", "0909998888", "Chấn thương phần mềm");
    addRow(3, "BN-3921", "Vũ Minh Ánh", "18/07/1990", "Nữ", "0911223344", "Viêm đường hô hấp");

    pageLayout->addWidget(fullPatientTable, 1);
    m_stackedWidget->addWidget(m_patientsPage);
}

void DoctorDashboardWidget::buildAppointmentsPage() {
    m_appointmentsPage = new QWidget(this);
    QVBoxLayout* pageLayout = new QVBoxLayout(m_appointmentsPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(20);

    QLabel* title = new QLabel("📅 Lịch Hẹn Khám & Điều Trị", m_appointmentsPage);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #111827;");
    pageLayout->addWidget(title);

    m_appointmentsTable = new QTableWidget(m_appointmentsPage);
    m_appointmentsTable->setColumnCount(6);
    m_appointmentsTable->setHorizontalHeaderLabels({"Thời Gian", "Mã BN", "Tên Bệnh Nhân", "Dịch Vụ Khám", "Phòng Khám", "Trạng Thái"});
    m_appointmentsTable->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 8px; gridline-color: #F1F3F4; }"
        "QHeaderView::section { background-color: #F8F9FA; padding: 10px; font-weight: bold; border: none; border-bottom: 2px solid #EAEAEA; color: #5F6368; }"
        "QTableWidget::item { padding: 12px; color: #3C4043; }"
    );
    m_appointmentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_appointmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_appointmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_appointmentsTable->verticalHeader()->setVisible(false);

    connect(m_appointmentsTable, &QTableWidget::cellClicked, this, [this](int row, int /*col*/) {
        if (row >= 0 && row < m_apptPageMeta.size()) {
            const auto& meta = m_apptPageMeta[row];
            openClinicalExam(meta.name, meta.code, meta.time, meta.reason, row, false);
        }
    });

    pageLayout->addWidget(m_appointmentsTable, 1);
    m_stackedWidget->addWidget(m_appointmentsPage);
}


void DoctorDashboardWidget::buildSettingsPage() {
    m_settingsPage = new QWidget(this);
    QVBoxLayout* pageLayout = new QVBoxLayout(m_settingsPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(20);

    QLabel* title = new QLabel("⚙️ Cấu Hình Hệ Thống", m_settingsPage);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #111827;");
    pageLayout->addWidget(title);

    QLabel* desc = new QLabel("Trang cấu hình và cài đặt thông tin cá nhân dành cho bác sĩ.", m_settingsPage);
    desc->setStyleSheet("color: #5F6368; font-size: 14px;");
    pageLayout->addWidget(desc);
    pageLayout->addStretch();

    m_stackedWidget->addWidget(m_settingsPage);
}

// =============================================================================
// SUB-LAYOUTS FOR OVERVIEW PAGE
// =============================================================================
void DoctorDashboardWidget::createDoctorCards(QWidget* parentPage, QVBoxLayout* pageLayout) {
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    QStringList titles = {"Tổng số ca khám hôm nay", "Số ca phẫu thuật", "Doanh thu phòng khám"};
    QStringList values = {"18", "3", "3,500,000đ"};
    QStringList rates = {"▲ 12%", "▲ 8%", "▼ 1%"};

    for(int i = 0; i < 3; ++i) {
        QFrame* card = new QFrame(parentPage);
        card->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px;");
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20, 15, 20, 15);

        QLabel* lblTitle = new QLabel(titles[i], card);
        lblTitle->setStyleSheet("color: #70757A; font-size: 13px; font-weight: 500;");
        
        QLabel* lblValue = new QLabel(values[i], card);
        lblValue->setStyleSheet("color: #202124; font-size: 26px; font-weight: bold; margin: 5px 0;");

        QLabel* lblRate = new QLabel(rates[i] + " so với tuần trước", card);
        lblRate->setStyleSheet("font-size: 12px; color: " + QString(i == 2 ? "#D93025;" : "#00966C;"));

        cardLayout->addWidget(lblTitle);
        cardLayout->addWidget(lblValue);
        cardLayout->addWidget(lblRate);

        cardsLayout->addWidget(card);
    }
    pageLayout->addLayout(cardsLayout);
}

void DoctorDashboardWidget::createDoctorCharts(QWidget* parentPage, QVBoxLayout* pageLayout) {
    QHBoxLayout* row2Layout = new QHBoxLayout();
    row2Layout->setSpacing(20);

    // Khung trắng bọc biểu đồ
    QFrame* chartCard = new QFrame(parentPage);
    chartCard->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px;");
    QVBoxLayout* cardLayout = new QVBoxLayout(chartCard);
    cardLayout->setContentsMargins(15, 15, 15, 15);

    QBarSet *setMedical = new QBarSet("Bệnh nhân nội trú");
    QBarSet *setAppointed = new QBarSet("Bệnh nhân vãng lai");
    setMedical->setColor(QColor("#00966C"));   
    setAppointed->setColor(QColor("#C2E3D8")); 

    *setMedical << 75 << 40 << 55 << 90 << 45 << 80 << 50;
    *setAppointed << 50 << 25 << 35 << 40 << 20 << 60 << 65;

    QBarSeries *series = new QBarSeries();
    series->append(setMedical);
    series->append(setAppointed);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Thống kê bệnh nhân hàng tuần");
    chart->setAnimationOptions(QChart::SeriesAnimations); 
    chart->setBackgroundVisible(false);

    QStringList categories = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setGridLineVisible(false); 
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing); 
    chartView->setMinimumHeight(230);

    cardLayout->addWidget(chartView);
    row2Layout->addWidget(chartCard, 2);

    // Khung Lịch trực bên phải
    QFrame* scheduleCard = new QFrame(parentPage);
    scheduleCard->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px;");
    QVBoxLayout* schedLayout = new QVBoxLayout(scheduleCard);
    schedLayout->setContentsMargins(15, 15, 15, 15);
    schedLayout->setSpacing(8);
    
    // Header có liên kết chuyển hướng
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* schedTitle = new QLabel("📅 Lịch trực hôm nay", scheduleCard);
    schedTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #202124;");
    
    QPushButton* btnLink = new QPushButton("Xem lịch hẹn →", scheduleCard);
    btnLink->setCursor(Qt::PointingHandCursor);
    btnLink->setStyleSheet(
        "QPushButton { background: transparent; color: #00966C; font-size: 11px; font-weight: bold; border: none; padding: 0; }"
        "QPushButton:hover { color: #007D5A; text-decoration: underline; }"
    );
    headerLayout->addWidget(schedTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(btnLink);
    schedLayout->addLayout(headerLayout);

    // 🌟 Sử dụng QCalendarWidget đã được tinh chỉnh UI cực kỳ hiện đại
    QCalendarWidget* calendar = new QCalendarWidget(scheduleCard);
    calendar->setGridVisible(false);
    calendar->setNavigationBarVisible(true);
    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendar->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    calendar->setStyleSheet(
        "QCalendarWidget {"
        "   background-color: #FFFFFF;"
        "   border: none;"
        "}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar {"
        "   background-color: #FFFFFF;"
        "   border-bottom: 1px solid #F1F3F4;"
        "}"
        "QCalendarWidget QToolButton {"
        "   color: #3C4043;"
        "   font-family: 'Segoe UI';"
        "   font-size: 11px;"
        "   font-weight: bold;"
        "   background-color: transparent;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 3px;"
        "}"
        "QCalendarWidget QToolButton:hover {"
        "   background-color: #F1F3F4;"
        "}"
        "QCalendarWidget QAbstractItemView:enabled {"
        "   color: #3C4043;"
        "   background-color: #FFFFFF;"
        "   selection-background-color: #00966C;"
        "   selection-color: #FFFFFF;"
        "   font-family: 'Segoe UI';"
        "   font-size: 11px;"
        "   border: none;"
        "}"
        "QCalendarWidget QAbstractItemView:disabled {"
        "   color: #C5CAD4;"
        "}"
    );
    schedLayout->addWidget(calendar, 1);

    // Kết nối click từ Lịch và nút liên kết để chuyển trang sang Appointments
    auto handleNavigation = [this]() {
        switchPage(2, m_btnAppoint);
    };
    connect(btnLink, &QPushButton::clicked, this, handleNavigation);
    connect(calendar, &QCalendarWidget::clicked, this, handleNavigation);

    row2Layout->addWidget(scheduleCard, 1);

    pageLayout->addLayout(row2Layout);
}

void DoctorDashboardWidget::createDoctorTable(QWidget* parentPage, QVBoxLayout* pageLayout) {
    QLabel* tblTitle = new QLabel("Danh sách bệnh nhân hẹn khám hôm nay", parentPage);
    tblTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #202124; margin-top: 5px;");
    pageLayout->addWidget(tblTitle);

    m_patientTable = new QTableWidget(parentPage);
    m_patientTable->setColumnCount(5);
    m_patientTable->setHorizontalHeaderLabels({"Tên Bệnh Nhân", "Mã Định Danh", "Giờ Hẹn", "Chuyên Khoa", "Trạng Thái"});
    
    m_patientTable->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 8px; gridline-color: #F1F3F4; }"
        "QHeaderView::section { background-color: #F8F9FA; padding: 10px; font-weight: bold; border: none; border-bottom: 2px solid #EAEAEA; color: #5F6368; }"
        "QTableWidget::item { padding: 12px; color: #3C4043; }"
    );
    
    m_patientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_patientTable->verticalHeader()->setVisible(false);

    // Click đơn vào bệnh nhân bất kỳ trong danh sách sẽ mở ra màn hình khám lâm sàng ngay lập tức
    connect(m_patientTable, &QTableWidget::cellClicked, this, [this](int row, int /*col*/) {
        if (row >= 0 && row < m_rowApptMeta.size()) {
            const auto& meta = m_rowApptMeta[row];
            openClinicalExam(meta.name, meta.code, meta.time, meta.reason, row, true);
        }
    });

    m_patientTable->setFixedHeight(140);
    pageLayout->addWidget(m_patientTable);
}

// =============================================================================
// NAVIGATION & PAGE SWITCHING HELPERS
// =============================================================================
void DoctorDashboardWidget::switchPage(int index, QPushButton* activeBtn) {
    if (!m_stackedWidget) return;

    // Đổi trạng thái active class của các nút sidebar
    QPushButton* btns[] = { m_btnDash, m_btnPatients, m_btnAppoint, m_btnSetting };
    for (auto* btn : btns) {
        if (btn) btn->setObjectName("");
    }
    if (activeBtn) activeBtn->setObjectName("activeBtn");

    // Yêu cầu sidebar vẽ lại stylesheet để nhận diện cấu trúc CSS mới
    if (m_sidebarFrame) {
        m_sidebarFrame->setStyleSheet(m_sidebarFrame->styleSheet());
    }

    // Chuyển trang stacked widget
    m_stackedWidget->setCurrentIndex(index);
}

void DoctorDashboardWidget::buildClinicalExamPage() {
    m_clinicalExamPage = new ClinicalExamWidget(this);
    m_stackedWidget->addWidget(m_clinicalExamPage);

    // Khi người dùng bấm thoát/hủy ở giao diện Khám lâm sàng -> quay lại Dashboard chính
    connect(m_clinicalExamPage, &ClinicalExamWidget::backToDashboardRequested, this, [this]() {
        switchPage(0, m_btnDash);
    });

    // Khi người dùng bấm tab Danh Sách trên giao diện Khám lâm sàng -> chuyển tới trang Appointments (lịch hẹn khám)
    connect(m_clinicalExamPage, &ClinicalExamWidget::viewAppointmentsListRequested, this, [this]() {
        switchPage(2, m_btnAppoint);
    });

    // Khi kết thúc khám một bệnh nhân -> tự động chuyển tiếp tới ca khám tiếp theo
    connect(m_clinicalExamPage, &ClinicalExamWidget::finishExamRequested, this, &DoctorDashboardWidget::handlePatientExamFinished);
}

void DoctorDashboardWidget::openClinicalExam(const QString& name, const QString& id, const QString& time, const QString& specialty, int rowIndex, bool isFromTodayList) {
    if (!m_clinicalExamPage || !m_stackedWidget) return;

    m_currentExaminingRow = rowIndex;
    m_isExaminingFromTodayList = isFromTodayList;
    m_clinicalExamPage->loadPatientInfo(name, id, time, specialty);

    // Tìm chỉ số của m_clinicalExamPage trong stacked widget
    int idx = m_stackedWidget->indexOf(m_clinicalExamPage);
    if (idx != -1) {
        switchPage(idx, nullptr); // ActiveBtn = nullptr vì đây là trang chi tiết, không nằm trên sidebar chính
    }
}

void DoctorDashboardWidget::handlePatientExamFinished() {
    if (m_currentExaminingRow == -1) return;

    if (m_isExaminingFromTodayList) {
        if (!m_patientTable) return;

        // 1. Cập nhật trạng thái của bệnh nhân vừa khám xong thành "COMPLETED" trong CSDL
        if (m_currentExaminingRow >= 0 && m_currentExaminingRow < m_rowApptMeta.size()) {
            int apptId = m_rowApptMeta[m_currentExaminingRow].appointmentId;
            DatabaseManager::getInstance().updateAppointmentStatus(apptId, "COMPLETED");
        }

        // Cập nhật trạng thái hiển thị trên bảng UI và đồng bộ dữ liệu
        refreshAppointmentsTables();

        // 2. Tìm bệnh nhân tiếp theo trong danh sách
        int nextRow = m_currentExaminingRow + 1; // Số hàng sau khi refresh vẫn giữ nguyên index vì thứ tự xếp hạng không đổi
        if (m_patientTable && nextRow < m_patientTable->rowCount()) {
            m_currentExaminingRow = nextRow;
            
            QString name = m_patientTable->item(nextRow, 0)->text();
            QString id = m_patientTable->item(nextRow, 1)->text();
            QString time = m_patientTable->item(nextRow, 2)->text();
            QString dept = m_patientTable->item(nextRow, 3)->text();
            
            m_clinicalExamPage->loadPatientInfo(name, id, time, dept);
            
            QMessageBox::information(this, "Nova Care Clinic",
                QString("Đã kết thúc ca khám hiện tại.\nChuyển sang bệnh nhân tiếp theo: %1").arg(name));
        } else {
            QMessageBox::information(this, "Nova Care Clinic",
                "Đã hoàn thành khám cho toàn bộ bệnh nhân trong danh sách hôm nay!");
            m_currentExaminingRow = -1;
            switchPage(0, m_btnDash); // Quay lại trang chủ dashboard
        }
    } else {
        // Khám từ trang Lịch Hẹn tổng hợp (không tự động chuyển tiếp vì có thể là các ngày khác nhau)
        if (m_currentExaminingRow >= 0 && m_currentExaminingRow < m_apptPageMeta.size()) {
            int apptId = m_apptPageMeta[m_currentExaminingRow].appointmentId;
            DatabaseManager::getInstance().updateAppointmentStatus(apptId, "COMPLETED");
        }

        // Đồng bộ dữ liệu mới nhất
        refreshAppointmentsTables();

        QMessageBox::information(this, "Nova Care Clinic", "Đã kết thúc ca khám thành công!");
        m_currentExaminingRow = -1;
        switchPage(2, m_btnAppoint); // Quay lại trang Lịch hẹn
    }
}

void DoctorDashboardWidget::refreshAppointmentsTables() {
    int docId = m_currentUser ? m_currentUser->getAccountId() : 1;
    QString todayStr = QDate::currentDate().toString("yyyy-MM-dd");

    // 1. Đồng bộ dữ liệu m_patientTable (Danh sách khám hôm nay)
    if (m_patientTable) {
        auto records = DatabaseManager::getInstance().getDoctorAppointments(docId, todayStr);
        m_rowApptMeta.clear();
        m_patientTable->setRowCount(0);
        int rowIdx = 0;
        for (const auto& rec : records) {
            QString statusText = rec.status;
            QString statusColor = "#3C4043"; // Mặc định
            if (rec.status == "SCHEDULED") {
                statusText = "Đang chờ";
                statusColor = "#1A73E8"; // Xanh dương
            } else if (rec.status == "COMPLETED") {
                statusText = "Đã khám xong";
                statusColor = "#059669"; // Xanh lá
            } else if (rec.status == "CANCELLED") {
                statusText = "Đã hủy ca";
                statusColor = "#D93025"; // Đỏ
            }

            m_patientTable->insertRow(rowIdx);
            
            QTableWidgetItem* nameItem = new QTableWidgetItem(rec.patientName);
            QTableWidgetItem* codeItem = new QTableWidgetItem(rec.patientCode);
            QTableWidgetItem* timeItem = new QTableWidgetItem(rec.startTime);
            QTableWidgetItem* deptItem = new QTableWidgetItem(rec.reason);
            QTableWidgetItem* statusItem = new QTableWidgetItem(statusText);

            statusItem->setForeground(QBrush(QColor(statusColor)));

            m_patientTable->setItem(rowIdx, 0, nameItem);
            m_patientTable->setItem(rowIdx, 1, codeItem);
            m_patientTable->setItem(rowIdx, 2, timeItem);
            m_patientTable->setItem(rowIdx, 3, deptItem);
            m_patientTable->setItem(rowIdx, 4, statusItem);

            m_rowApptMeta.append({rec.appointmentId, rec.patientId, rec.patientName, rec.patientCode, rec.startTime, rec.reason});
            rowIdx++;
        }
    }

    // 2. Đồng bộ dữ liệu m_appointmentsTable (Trang Lịch hẹn tổng hợp)
    if (m_appointmentsTable) {
        auto records = DatabaseManager::getInstance().getDoctorAppointments(docId);
        m_apptPageMeta.clear();
        m_appointmentsTable->setRowCount(0);
        int rowIdx = 0;
        for (const auto& rec : records) {
            QString statusText = rec.status;
            QString statusColor = "#3C4043";
            if (rec.status == "SCHEDULED") {
                statusText = "Đang chờ";
                statusColor = "#1A73E8";
            } else if (rec.status == "COMPLETED") {
                statusText = "Đã khám";
                statusColor = "#059669";
            } else if (rec.status == "CANCELLED") {
                statusText = "Đã hủy";
                statusColor = "#D93025";
            }

            m_appointmentsTable->insertRow(rowIdx);
            
            QString timeStr = QString("%1 %2").arg(rec.appointmentDate, rec.startTime);

            QTableWidgetItem* timeItem = new QTableWidgetItem(timeStr);
            QTableWidgetItem* codeItem = new QTableWidgetItem(rec.patientCode);
            QTableWidgetItem* nameItem = new QTableWidgetItem(rec.patientName);
            QTableWidgetItem* reasonItem = new QTableWidgetItem(rec.reason);
            QTableWidgetItem* roomItem = new QTableWidgetItem(rec.roomNumber);
            QTableWidgetItem* statusItem = new QTableWidgetItem(statusText);

            statusItem->setForeground(QBrush(QColor(statusColor)));

            m_appointmentsTable->setItem(rowIdx, 0, timeItem);
            m_appointmentsTable->setItem(rowIdx, 1, codeItem);
            m_appointmentsTable->setItem(rowIdx, 2, nameItem);
            m_appointmentsTable->setItem(rowIdx, 3, reasonItem);
            m_appointmentsTable->setItem(rowIdx, 4, roomItem);
            m_appointmentsTable->setItem(rowIdx, 5, statusItem);

            m_apptPageMeta.append({rec.appointmentId, rec.patientId, rec.patientName, rec.patientCode, rec.startTime, rec.reason});
            rowIdx++;
        }
    }
}