// profile.cpp
#include "profile.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>

ProfileWidget::ProfileWidget(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    setStyleSheet(
        "ProfileWidget { background-color: #F4F6FA; }"
        "QWidget { font-family: 'Segoe UI', Arial, sans-serif; color: #172B4D; }"
        "QMessageBox { background-color: #FFFFFF; }"
        "QMessageBox QLabel { color: #172B4D; font-size: 14px; }"
        "QMessageBox QPushButton { background-color: #0052CC; color: white; font-weight: bold; min-width: 80px; min-height: 28px; border-radius: 4px; border: none; }"
        "QMessageBox QPushButton:hover { background-color: #0043A4; }"
        );
    showMaximized();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createTopBar());

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(30, 25, 30, 25);
    contentLayout->setSpacing(25);

    contentLayout->addWidget(createLeftPanel(), 1);
    contentLayout->addWidget(createRightPanel(), 3);

    mainLayout->addLayout(contentLayout, 1);
}

QWidget* ProfileWidget::createTopBar() {
    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(70);
    topBar->setStyleSheet("background-color: #0052CC;");

    QHBoxLayout *layout = new QHBoxLayout(topBar);
    layout->setContentsMargins(30, 0, 30, 0);

    QLabel *lblTitle = new QLabel("🏥 HỆ THỐNG QUẢN LÝ NHÂN SỰ BỆNH VIỆN", topBar);
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; letter-spacing: 0.5px; color: white;");

    QLineEdit *txtSearch = new QLineEdit(topBar);
    txtSearch->setPlaceholderText("🔍 Search Bác sĩ, Phòng ban...");
    txtSearch->setFixedWidth(350);
    txtSearch->setStyleSheet(
        "QLineEdit { "
        "   background-color: rgba(255, 255, 255, 0.15); "
        "   border: 1px solid rgba(255, 255, 255, 0.3); "
        "   border-radius: 6px; "
        "   padding-left: 12px; "
        "   color: white; "
        "   font-size: 14px; "
        "} "
        "QLineEdit:focus { "
        "   background-color: white; "
        "   color: #172B4D; "
        "}"
        );

    QLabel *lblNoti = new QLabel("🔔", topBar);
    lblNoti->setStyleSheet("font-size: 20px; cursor: pointer; color: white;");

    layout->addWidget(lblTitle);
    layout->addStretch();
    layout->addWidget(txtSearch);
    layout->addSpacing(20);
    layout->addWidget(lblNoti);

    return topBar;
}

QWidget* ProfileWidget::createLeftPanel() {
    QWidget *panel = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    QWidget *idCard = new QWidget(panel);
    idCard->setStyleSheet("background-color: #FFFFFF; border-radius: 12px;");
    QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect(idCard);
    shadow1->setBlurRadius(15); shadow1->setColor(QColor(0, 0, 0, 15)); shadow1->setOffset(0, 4);
    idCard->setGraphicsEffect(shadow1);

    QVBoxLayout *idLayout = new QVBoxLayout(idCard);
    idLayout->setContentsMargins(25, 30, 25, 30);
    idLayout->setSpacing(15);

    lblAvatar = new QLabel(idCard);
    lblAvatar->setFixedSize(130, 130);
    lblAvatar->setStyleSheet("background-color: #DEEBFF; border-radius: 65px; color: #0052CC; font-size: 55px;");
    lblAvatar->setText("👨‍⚕️");
    lblAvatar->setAlignment(Qt::AlignCenter);

    lblStatus = new QLabel("● TRẠNG THÁI: ACTIVE", idCard);
    lblStatus->setStyleSheet("color: #36B37E; font-weight: bold; font-size: 13px;");
    lblStatus->setAlignment(Qt::AlignCenter);

    lblName = new QLabel("BS. ANH KHOA", idCard);
    lblName->setStyleSheet("font-size: 20px; font-weight: bold; color: #172B4D;");
    lblName->setAlignment(Qt::AlignCenter);

    lblRole = new QLabel("Chức vụ: DOCTOR", idCard);
    lblRole->setStyleSheet("font-size: 14px; color: #5E6C84;");
    lblRole->setAlignment(Qt::AlignCenter);

    lblStaffCode = new QLabel("Mã số: STAFF_CODE", idCard);
    lblStaffCode->setStyleSheet("font-size: 14px; color: #5E6C84;");
    lblStaffCode->setAlignment(Qt::AlignCenter);

    lblStaffId = new QLabel("ID Hệ thống: #STAFF_ID", idCard);
    lblStaffId->setStyleSheet("font-size: 12px; color: #97A0AF; font-style: italic;");
    lblStaffId->setAlignment(Qt::AlignCenter);

    idLayout->addWidget(lblAvatar, 0, Qt::AlignCenter);
    idLayout->addWidget(lblStatus);
    idLayout->addSpacing(5);
    idLayout->addWidget(lblName);
    idLayout->addWidget(lblRole);
    idLayout->addWidget(lblStaffCode);
    idLayout->addStretch();
    idLayout->addWidget(lblStaffId);

    QWidget *shiftCard = new QWidget(panel);
    shiftCard->setStyleSheet("background-color: #FFFFFF; border-radius: 12px;");
    QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect(shiftCard);
    shadow2->setBlurRadius(15); shadow2->setColor(QColor(0, 0, 0, 15)); shadow2->setOffset(0, 4);
    shiftCard->setGraphicsEffect(shadow2);

    QVBoxLayout *shiftLayout = new QVBoxLayout(shiftCard);
    shiftLayout->setContentsMargins(20, 20, 20, 20);
    shiftLayout->setSpacing(6);

    QLabel *lblShiftTitle = new QLabel(" [ THẺ CA TRỰC HIỆN TẠI ]", shiftCard);
    lblShiftTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #0052CC;");

    lblShift = new QLabel("Ca trực: FULL_DAY", shiftCard);
    lblShift->setStyleSheet("font-size: 16px; font-weight: bold; color: #172B4D; margin-top: 5px;");

    QLabel *lblShiftNote = new QLabel("(Morning/Afternoon/Night)", shiftCard);
    lblShiftNote->setStyleSheet("font-size: 13px; color: #7A869A;");

    shiftLayout->addWidget(lblShiftTitle);
    shiftLayout->addWidget(lblShift);
    shiftLayout->addWidget(lblShiftNote);

    // SỬA: Đưa nút Chỉnh Sửa ra chính giữa bảng điều khiển bên dưới
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);

    btnEdit = new QPushButton("Chỉnh Sửa", panel);
    btnEdit->setFixedHeight(45);
    btnEdit->setFixedWidth(180); // Cố định độ rộng vừa vặn để nút không bị bè ngang quá to
    btnEdit->setStyleSheet(
        "QPushButton { background-color: #0052CC; color: white; font-weight: bold; border-radius: 6px; border: none; }"
        "QPushButton:hover { background-color: #0043A4; }"
        );

    btnLayout->addStretch();
    btnLayout->addWidget(btnEdit);
    btnLayout->addStretch();

    layout->addWidget(idCard, 1);
    layout->addWidget(shiftCard);
    layout->addLayout(btnLayout);

    connect(btnEdit, &QPushButton::clicked, this, &ProfileWidget::onEditClicked);

    return panel;
}

QWidget* ProfileWidget::createRightPanel() {
    QWidget *panel = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(25);

    QList<QPair<QString, QString>> adminInfo = {
        {"• Họ và tên:", "TRẦN NGUYỄN ANH KHOA"},
        {"• Giới tính:", "MALE"},
        {"• Ngày sinh:", "DD/MM/YYYY"},
        {"• Số CCCD:", "079xxxxxxxxx"},
        {"• Số điện thoại:", "090xxxxxxx"},
        {"• Email:", "khoa.nguyen@hospital.com"},
        {"• Địa chỉ thường trú:", "227 Nguyễn Văn Cừ, P4, Q5, TP.HCM"}
    };
    layout->addWidget(createInfoCard(" [ THẺ 1: THÔNG TIN HÀNH CHÍNH & NHÂN SỰ ]", adminInfo));

    QList<QPair<QString, QString>> workInfo = {
        {"• Phòng ban:", "[KHOA TIM MẠCH] (Department_id)"},
        {"• Ngày vào làm (Hire Date):", "DD/MM/YYYY"}
    };
    layout->addWidget(createInfoCard(" [ THẺ 2: LỊCH TRÌNH VÀ PHÂN CÔNG CÔNG TÁC ]", workInfo));

    layout->addWidget(createSecurityCard());

    return panel;
}

QWidget* ProfileWidget::createInfoCard(const QString &title, const QList<QPair<QString, QString>> &items) {
    QWidget *card = new QWidget(this);
    card->setStyleSheet("background-color: #FFFFFF; border-radius: 12px;");
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(15); shadow->setColor(QColor(0, 0, 0, 15)); shadow->setOffset(0, 4);
    card->setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(25, 25, 25, 25);
    layout->setSpacing(15);

    QLabel *lblTitle = new QLabel(title, card);
    lblTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #0052CC;");
    layout->addWidget(lblTitle);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setHorizontalSpacing(40);
    gridLayout->setVerticalSpacing(12);

    int row = 0;
    int col = 0;
    for (const auto &item : items) {
        QWidget *itemWidget = new QWidget(card);
        QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        itemLayout->setSpacing(10);

        QLabel *lblKey = new QLabel(item.first, itemWidget);
        lblKey->setStyleSheet("font-size: 14px; font-weight: 600; color: #42526E;");
        QLabel *lblVal = new QLabel(item.second, itemWidget);
        lblVal->setStyleSheet("font-size: 14px; color: #172B4D;");

        itemLayout->addWidget(lblKey);
        itemLayout->addWidget(lblVal, 1);

        if (item.first.contains("Địa chỉ")) {
            if (col == 1) { row++; col = 0; }
            gridLayout->addWidget(itemWidget, row, 0, 1, 2);
            row++;
        } else {
            gridLayout->addWidget(itemWidget, row, col);
            col++;
            if (col > 1) { col = 0; row++; }
        }
    }

    layout->addLayout(gridLayout);
    return card;
}

QWidget* ProfileWidget::createSecurityCard() {
    QWidget *card = new QWidget(this);
    card->setStyleSheet("background-color: #FFFFFF; border-radius: 12px;");
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(15); shadow->setColor(QColor(0, 0, 0, 15)); shadow->setOffset(0, 4);
    card->setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(25, 25, 25, 25);
    layout->setSpacing(15);

    QLabel *lblTitle = new QLabel(" [ THẺ 3: AN NINH & NHẬT KÝ HỆ THỐNG ]", card);
    lblTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #0052CC;");
    layout->addWidget(lblTitle);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(12);

    auto addSecurityItem = [](const QString &key, const QString &val, bool isSpec, QVBoxLayout *lay, QWidget *p) {
        QWidget *w = new QWidget(p);
        QHBoxLayout *l = new QHBoxLayout(w);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(10);

        QLabel *lblKey = new QLabel(key, w);
        lblKey->setStyleSheet("font-size: 14px; font-weight: 600; color: #42526E;");
        QLabel *lblVal = new QLabel(val, w);
        if (isSpec) {
            lblVal->setStyleSheet("font-size: 14px; color: #006644; background-color: #E3FCEF; padding: 2px 8px; border-radius: 4px; font-weight: bold;");
            lblVal->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        } else {
            lblVal->setStyleSheet("font-size: 14px; color: #172B4D;");
        }

        l->addWidget(lblKey);
        l->addWidget(lblVal, 1);
        lay->addWidget(w);
    };

    addSecurityItem("• Mật khẩu:", "[••••••••] (Đã mã hóa dạng Password_hash)", false, infoLayout, card);
    addSecurityItem("• Trạng thái dữ liệu:", "Hợp lệ (Is_deleted = 0)", true, infoLayout, card);
    addSecurityItem("• Ngày tạo hồ sơ (Created_at):", "DD/MM/YYYY HH:MM", false, infoLayout, card);
    addSecurityItem("• Ngày cập nhật mới nhất (Updated_at):", "DD/MM/YYYY HH:MM", false, infoLayout, card);

    layout->addLayout(infoLayout);
    return card;
}

void ProfileWidget::onEditClicked() {
    QMessageBox::information(this, "Chức năng", "Mở cửa sổ form chỉnh sửa thông tin hành chính!");
}