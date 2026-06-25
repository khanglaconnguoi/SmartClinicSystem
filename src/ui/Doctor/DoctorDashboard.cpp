#include "DoctorDashboard.h"
#include "../../model/IAuthenticatable.h"
#include <QHeaderView>

// Thêm các thư viện Chart bắt buộc để vẽ cột
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

DoctorDashboardWidget::DoctorDashboardWidget(std::shared_ptr<IAuthenticatable> user, QWidget *parent)
    : BaseDashboardWidget(parent), m_currentUser(user)
{
    // Gọi hàm dựng sẵn khuôn của lớp cha
    initializeDashboard();
}

void DoctorDashboardWidget::fillDashboardData() {
    // 1. 👨‍⚕️ VẼ CHỮ: Cập nhật thông tin tên Bác sĩ thật lên Topbar của lớp cha
    if (m_currentUser && m_docInfo) {
        m_docInfo->setText("👨‍⚕️ " + m_currentUser->getFullName() + " (Doctor)");
    }

    // 2. 📊 VẼ CARD: Tạo các thẻ thông số thống kê thả vào m_mainContentLayout
    createDoctorCards();

    // 3. 📈 VẼ BIỂU ĐỒ: Tạo vùng biểu đồ cột đôi và lịch trực hôm nay
    createDoctorCharts();

    // 4. 📋 VẼ BẢNG: Đổ dữ liệu danh sách bệnh nhân vào bảng
    createDoctorTable();
}

void DoctorDashboardWidget::createDoctorCards() {
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    QStringList titles = {"Tổng số ca khám hôm nay", "Số ca phẫu thuật", "Doanh thu phòng khám"};
    QStringList values = {"18", "3", "3,500,000đ"}; // Điền số liệu thực tế thay vì số 0
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
        lblRate->setStyleSheet("font-size: 12px; color: " + QString(i == 2 ? "#D93025;" : "#00966C;"));

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

    // Khung trắng bọc biểu đồ
    QFrame* chartCard = new QFrame(this);
    chartCard->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px;");
    QVBoxLayout* cardLayout = new QVBoxLayout(chartCard);
    cardLayout->setContentsMargins(15, 15, 15, 15);

    // 🌟 VẼ BIỂU ĐỒ CỘT ĐÔI XANH NGỌC ĐẶC TRƯNG CỦA BẠN
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
    row2Layout->addWidget(chartCard, 2); // Tỷ lệ chiếm 2 phần

    // Khung Lịch trực bên phải
    QFrame* scheduleCard = new QFrame(this);
    scheduleCard->setStyleSheet("background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 12px;");
    QVBoxLayout* schedLayout = new QVBoxLayout(scheduleCard);
    schedLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel* schedTitle = new QLabel("📅 Lịch trực hôm nay", scheduleCard);
    schedTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #202124;");
    schedLayout->addWidget(schedTitle);
    schedLayout->addWidget(new QLabel("• 08:00 - 10:00: Hội chẩn ca bệnh nặng"));
    schedLayout->addWidget(new QLabel("• 10:30 - 12:00: Khám bệnh tại phòng số 4"));
    schedLayout->addWidget(new QLabel("• 14:00 - 17:30: Đi buồng & Kiểm tra nội trú"));
    schedLayout->addStretch();

    row2Layout->addWidget(scheduleCard, 1); // Tỷ lệ chiếm 1 phần

    m_mainContentLayout->addLayout(row2Layout);
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
    m_mainContentLayout->addWidget(m_patientTable);
}