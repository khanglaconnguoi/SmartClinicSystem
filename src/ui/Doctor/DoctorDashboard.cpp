#include "DoctorDashboard.h"
#include "../../model/IAuthenticatable.h"
#include <QHeaderView>
#include <QCalendarWidget> 
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

DoctorDashboardWidget::DoctorDashboardWidget(std::shared_ptr<IAuthenticatable> user, QWidget *parent)
    : BaseDashboardWidget(parent), m_currentUser(user)
{
    initializeDashboard();
}

void DoctorDashboardWidget::fillDashboardData() {
    if (m_currentUser && m_docNameLabel) {
        m_docNameLabel->setText(m_currentUser->getFullName());
    }

    if (m_docAvatarBtn && m_currentUser) {
        QPixmap rawPixmap = m_currentUser->getAvatar();

        if (rawPixmap.isNull()) {
            rawPixmap = QPixmap(36, 36);
            rawPixmap.fill(QColor("#00969A")); 
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
            "QPushButton { background-color: transparent; border: none; width: 36px; height: 36px; padding: 0px; margin: 0px; }"
        );

        m_docAvatarBtn->update();
    }

    createDoctorCards();
    createDoctorCharts();
    createDoctorTable();
}

void DoctorDashboardWidget::createDoctorCards() {
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    QStringList titles = {"Tổng số ca khám hôm nay", "Số ca phẫu thuật", "Doanh thu phòng khám"};
    QStringList values = {"18", "3", "3,500,000đ"}; 
    QStringList rates = {"▲ 12%", "▲ 8%", "▼ 1%"};

    for(int i = 0; i < 3; ++i) {
        QFrame* card = new QFrame(this);
        card->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px;");
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20, 15, 20, 15);

        QLabel* lblTitle = new QLabel(titles[i], card);
        lblTitle->setStyleSheet("color: #70757A; font-size: 13px; font-weight: 500;");
        
        QLabel* lblValue = new QLabel(values[i], card);
        lblValue->setStyleSheet("color: #202124; font-size: 26px; font-weight: bold; margin: 5px 0;");

        QLabel* lblRate = new QLabel(rates[i] + " so với tuần trước", card);
        lblRate->setStyleSheet("font-size: 12px; color: " + QString(i == 2 ? "#D93025;" : "#00969A;"));

        cardLayout->addWidget(lblTitle);
        cardLayout->addWidget(lblValue);
        cardLayout->addWidget(lblRate);

        cardsLayout->addWidget(card);
    }
    m_mainContentLayout->addLayout(cardsLayout);
}

void DoctorDashboardWidget::createDoctorCharts() {
    QHBoxLayout* row2Layout = new QHBoxLayout();
    row2Layout->setSpacing(20);

    QFrame* chartCard = new QFrame(this);
    chartCard->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px;");
    QVBoxLayout* cardLayout = new QVBoxLayout(chartCard);
    cardLayout->setContentsMargins(15, 15, 15, 15);

    QBarSet *setMedical = new QBarSet("Bệnh nhân nội trú");
    QBarSet *setAppointed = new QBarSet("Bệnh nhân vãng lai");
    setMedical->setColor(QColor("#00969A"));   
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
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartView->setMinimumHeight(450); 

    cardLayout->addWidget(chartView);
    row2Layout->addWidget(chartCard, 2); 

    QVBoxLayout* rightColLayout = new QVBoxLayout();
    rightColLayout->setSpacing(20);

    QFrame* calendarCard = new QFrame(this);
    calendarCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    calendarCard->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px;");
    QVBoxLayout* calLayout = new QVBoxLayout(calendarCard);
    calLayout->setContentsMargins(10, 10, 10, 10);
    
    QCalendarWidget* calendarWidget = new QCalendarWidget(calendarCard);
    calendarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 🌟 FIX: Đặt chiều cao tối thiểu lớn hơn để đảm bảo hiển thị đủ 6 hàng ngày
    calendarWidget->setMinimumHeight(350); 
    
    calendarWidget->setStyleSheet(
        "QCalendarWidget { color: #3C4043; }"
        "QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: #FFFFFF; min-height: 40px; }"
        "QCalendarWidget QToolButton { color: #3C4043; font-weight: bold; font-size: 14px; }"
        "QCalendarWidget QMenu { background-color: #FFFFFF; color: #3C4043; border: 1px solid #EAEAEA; }"
        "QCalendarWidget QMenu::item { padding: 5px 25px; }"
        "QCalendarWidget QMenu::item:selected { background-color: #00969A; color: white; }"
        "QCalendarWidget QSpinBox { background-color: #FFFFFF; color: #3C4043; selection-background-color: #00969A; selection-color: white; }"
        "QCalendarWidget QSpinBox::up-button, QCalendarWidget QSpinBox::down-button { subcontrol-origin: border; width: 16px; }"
        "QCalendarWidget QAbstractItemView { color: #3C4043; selection-background-color: transparent; alternate-background-color: #FFFFFF; }"
        "QCalendarWidget QAbstractItemView::item:selected { background-color: #00969A; border-radius: 14px; color: white; }"
    );
    calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calLayout->addWidget(calendarWidget);

    rightColLayout->addWidget(calendarCard, 1);

    QFrame* scheduleCard = new QFrame(this);
    // 🌟 FIX: Ép cứng khung Lịch Trực để nó không tranh giành chiều cao với Lịch Tháng
    scheduleCard->setFixedHeight(140);
    scheduleCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    scheduleCard->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px;");
    QVBoxLayout* schedLayout = new QVBoxLayout(scheduleCard);
    schedLayout->setContentsMargins(20, 15, 20, 15);
    
    QLabel* schedTitle = new QLabel("📅 Lịch trực hôm nay", scheduleCard);
    schedTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #202124; border: none;");
    schedLayout->addWidget(schedTitle);
    
    QLabel* task1 = new QLabel("• 08:00 - 10:00: Hội chẩn ca bệnh nặng", scheduleCard);
    task1->setStyleSheet("border: none; color: #3C4043;");
    QLabel* task2 = new QLabel("• 10:30 - 12:00: Khám bệnh tại phòng số 4", scheduleCard);
    task2->setStyleSheet("border: none; color: #3C4043;");
    QLabel* task3 = new QLabel("• 14:00 - 17:30: Đi buồng & Kiểm tra nội trú", scheduleCard);
    task3->setStyleSheet("border: none; color: #3C4043;");
    
    schedLayout->addWidget(task1);
    schedLayout->addWidget(task2);
    schedLayout->addWidget(task3);

    rightColLayout->addWidget(scheduleCard, 0);

    row2Layout->addLayout(rightColLayout, 1); 

    // 🌟 FIX QUAN TRỌNG NHẤT: Thêm số '1' (Stretch factor) vào dòng này!
    // Nó sẽ bảo Layout tổng: "Khi phóng to màn hình, hãy dành toàn bộ chiều cao dư thừa cho hàng Biểu đồ & Lịch"
    m_mainContentLayout->addLayout(row2Layout, 1); 
}

void DoctorDashboardWidget::createDoctorTable() {
    QLabel* tblTitle = new QLabel("Danh sách bệnh nhân hẹn khám hôm nay", this);
    tblTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #202124; margin-top: 5px;");
    m_mainContentLayout->addWidget(tblTitle);

    m_patientTable = new QTableWidget(this);
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

    m_patientTable->setRowCount(3);
    auto addPatientRow = [this](int row, QString name, QString id, QString time, QString dept, QString status) {
        m_patientTable->setItem(row, 0, new QTableWidgetItem(name));
        m_patientTable->setItem(row, 1, new QTableWidgetItem(id));
        m_patientTable->setItem(row, 2, new QTableWidgetItem(time));
        m_patientTable->setItem(row, 3, new QTableWidgetItem(dept));
        m_patientTable->setItem(row, 4, new QTableWidgetItem(status));
    };

    addPatientRow(0, "Isa Isganderov", "PT-47229027", "13:00", "Dermatology", "Đang chờ");
    addPatientRow(1, "Murad Mamedli", "PT-15287353", "13:30", "Ophthalmology", "Đã khám xong");
    addPatientRow(2, "Diana Huseynova", "PT-89789765", "14:00", "Radiology", "Đã hủy ca");

    m_patientTable->setFixedHeight(160);
    
    // Bảng sẽ nằm im ở dưới cùng và không bị kéo giãn theo cửa sổ
    m_mainContentLayout->addWidget(m_patientTable, 0); 
}