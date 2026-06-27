#include "Profile.h"
#include "model/SystemUser.h"
#include "model/Doctor.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <memory>
#include <QDate>

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
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *lblTitle = new QLabel("THÔNG TIN CÁ NHÂN", topBar);
    lblTitle->setStyleSheet("font-size: 22px; font-weight: bold; letter-spacing: 1px; color: white;");
    lblTitle->setAlignment(Qt::AlignCenter);

    layout->addWidget(lblTitle);

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

    lblStatus = new QLabel("", idCard);
    lblStatus->setAlignment(Qt::AlignCenter);

    lblName = new QLabel("", idCard);
    lblName->setStyleSheet("font-size: 20px; font-weight: bold; color: #172B4D;");
    lblName->setAlignment(Qt::AlignCenter);

    lblStaffCode = new QLabel("", idCard);
    lblStaffCode->setStyleSheet("font-size: 14px; color: #5E6C84;");
    lblStaffCode->setAlignment(Qt::AlignCenter);

    lblRole = new QLabel("", idCard);
    lblRole->setStyleSheet("font-size: 14px; color: #5E6C84;");
    lblRole->setAlignment(Qt::AlignCenter);

    idLayout->addWidget(lblAvatar, 0, Qt::AlignCenter);
    idLayout->addWidget(lblStatus);
    idLayout->addSpacing(5);
    idLayout->addWidget(lblName);
    idLayout->addWidget(lblStaffCode);
    idLayout->addWidget(lblRole);
    idLayout->addStretch();

    QWidget *shiftCard = new QWidget(panel);
    shiftCard->setStyleSheet("background-color: #FFFFFF; border-radius: 12px;");
    QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect(shiftCard);
    shadow2->setBlurRadius(15); shadow2->setColor(QColor(0, 0, 0, 15)); shadow2->setOffset(0, 4);
    shiftCard->setGraphicsEffect(shadow2);

    QVBoxLayout *shiftLayout = new QVBoxLayout(shiftCard);
    shiftLayout->setContentsMargins(25, 25, 25, 25);
    shiftLayout->setSpacing(15);

    QLabel *lblShiftTitle = new QLabel(" [ CA TRỰC HIỆN TẠI ]", shiftCard);
    lblShiftTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #0052CC;");

    cmbShift = new QComboBox(shiftCard);
    cmbShift->addItem("Ca Sáng", "MORNING");
    cmbShift->addItem("Ca Chiều", "AFTERNOON");
    cmbShift->addItem("Ca Tối", "NIGHT");
    cmbShift->addItem("Cả Ngày", "FULL_DAY");
    cmbShift->setEnabled(false);
    cmbShift->setFixedHeight(45);
    cmbShift->setStyleSheet(
        "QComboBox { font-size: 15px; font-weight: bold; color: #172B4D; border: 1px solid #DFE1E6; border-radius: 8px; background: #F4F5F7; padding: 5px 15px; } "
        "QComboBox::drop-down { border: none; width: 30px; } "
        "QComboBox:!disabled { border: 2px solid #0052CC; background: #FFFFFF; color: #0052CC; } "
        "QComboBox:!disabled::drop-down { border-left: 1px solid #0052CC; }"
    );

    shiftLayout->addWidget(lblShiftTitle);
    shiftLayout->addWidget(cmbShift);
    shiftLayout->addStretch();

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);

    btnEdit = new QPushButton("Chỉnh Sửa", panel);
    btnEdit->setFixedHeight(45);
    btnEdit->setFixedWidth(180);
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

    QWidget *card1 = new QWidget(this);
    card1->setStyleSheet("background-color: #FFFFFF; border-radius: 12px;");
    QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect(card1);
    shadow1->setBlurRadius(15); shadow1->setColor(QColor(0, 0, 0, 15)); shadow1->setOffset(0, 4);
    card1->setGraphicsEffect(shadow1);

    QVBoxLayout *layout1 = new QVBoxLayout(card1);
    layout1->setContentsMargins(25, 25, 25, 25);
    layout1->setSpacing(15);

    QLabel *lblTitle1 = new QLabel(" [ THÔNG TIN CƠ BẢN ]", card1);
    lblTitle1->setStyleSheet("font-size: 15px; font-weight: bold; color: #0052CC;");
    layout1->addWidget(lblTitle1);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setHorizontalSpacing(40);
    gridLayout->setVerticalSpacing(12);

    txtFullName = new QLineEdit(card1);
    txtGender = new QLineEdit(card1);
    txtDob = new QLineEdit(card1);
    txtCitizenId = new QLineEdit(card1);
    txtPhone = new QLineEdit(card1);
    txtEmail = new QLineEdit(card1);
    txtAddress = new QLineEdit(card1);

    QList<QLineEdit*> editFields = {txtFullName, txtGender, txtDob, txtCitizenId, txtPhone, txtEmail, txtAddress};
    for (QLineEdit* field : editFields) {
        field->setReadOnly(true);
        field->setStyleSheet("QLineEdit { font-size: 14px; color: #172B4D; border: none; background: transparent; padding: 2px; }");
    }

    auto addFormRow = [](QGridLayout *grid, const QString &text, QLineEdit *edit, int r, int c, int rSpan = 1, int cSpan = 1) {
        QWidget *w = new QWidget();
        QHBoxLayout *h = new QHBoxLayout(w);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(10);
        QLabel *l = new QLabel(text);
        l->setStyleSheet("font-size: 14px; font-weight: 600; color: #42526E;");
        h->addWidget(l);
        h->addWidget(edit, 1);
        grid->addWidget(w, r, c, rSpan, cSpan);
    };

    addFormRow(gridLayout, "• Họ và tên:", txtFullName, 0, 0);
    addFormRow(gridLayout, "• Giới tính:", txtGender, 0, 1);
    addFormRow(gridLayout, "• Ngày sinh:", txtDob, 1, 0);
    addFormRow(gridLayout, "• Số CCCD:", txtCitizenId, 1, 1);
    addFormRow(gridLayout, "• Số điện thoại:", txtPhone, 2, 0);
    addFormRow(gridLayout, "• Email:", txtEmail, 2, 1);
    addFormRow(gridLayout, "• Địa chỉ:", txtAddress, 3, 0, 1, 2);

    layout1->addLayout(gridLayout);
    layout->addWidget(card1);

    QWidget *card2 = new QWidget(this);
    card2->setStyleSheet("background-color: #FFFFFF; border-radius: 12px;");
    QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect(card2);
    shadow2->setBlurRadius(15); shadow2->setColor(QColor(0, 0, 0, 15)); shadow2->setOffset(0, 4);
    card2->setGraphicsEffect(shadow2);

    QVBoxLayout *layout2 = new QVBoxLayout(card2);
    layout2->setContentsMargins(25, 25, 25, 25);
    layout2->setSpacing(15);

    QLabel *lblTitle2 = new QLabel(" [ CHI TIẾT CÔNG VIỆC ]", card2);
    lblTitle2->setStyleSheet("font-size: 15px; font-weight: bold; color: #0052CC;");
    layout2->addWidget(lblTitle2);

    QGridLayout *gridLayout2 = new QGridLayout();
    gridLayout2->setHorizontalSpacing(40);
    gridLayout2->setVerticalSpacing(12);

    lblDepartment = new QLabel(card2);
    lblDepartment->setStyleSheet("font-size: 14px; color: #172B4D;");
    lblHireDate = new QLabel(card2);
    lblHireDate->setStyleSheet("font-size: 14px; color: #172B4D;");

    auto addLabelRow = [](QGridLayout *grid, const QString &text, QLabel *label, int r, int c) {
        QWidget *w = new QWidget();
        QHBoxLayout *h = new QHBoxLayout(w);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(10);
        QLabel *l = new QLabel(text);
        l->setStyleSheet("font-size: 14px; font-weight: 600; color: #42526E;");
        h->addWidget(l);
        h->addWidget(label, 1);
        grid->addWidget(w, r, c);
    };

    addLabelRow(gridLayout2, "• Phòng ban:", lblDepartment, 0, 0);
    addLabelRow(gridLayout2, "• Ngày vào làm:", lblHireDate, 0, 1);

    layout2->addLayout(gridLayout2);
    layout->addWidget(card2);
    layout->addStretch();

    return panel;
}

void ProfileWidget::loadDoctorProfile(const QString &staffCode) {
    QSqlQuery query;
    query.prepare("SELECT staff_id, staff_code, full_name, role, gender, date_of_birth, "
                  "citizen_id, phone_number, email, address, hire_date, shift, "
                  "password_hash, is_active, department_id, avatar "
                  "FROM staff "
                  "WHERE staff_code LIKE :staff_code OR staff_id = :staff_id_fallback");
    query.bindValue(":staff_code", "%" + staffCode + "%");
    query.bindValue(":staff_id_fallback", staffCode.toInt());

    if (query.exec() && query.next()) {
        currentStaffId = query.value("staff_id").toInt();
        currentStaffCode = query.value("staff_code").toString().trimmed();

        QPixmap avatarPixmap;
        QVariant avatarData = query.value("avatar");
        if (avatarData.isValid() && !avatarData.isNull()) {
            avatarPixmap.loadFromData(avatarData.toByteArray());
        }

        QString specialty = "Chưa cập nhật";
        QString licenseNumber = "Chưa có";
        int experienceYears = 0;
        int consultationFee = 0;
        QString bio = "";

        QSqlQuery dpQuery;
        dpQuery.prepare("SELECT specialty, license_number, experience_years, consultation_fee, bio "
                        "FROM doctor_profiles WHERE staff_id = :staff_id");
        dpQuery.bindValue(":staff_id", currentStaffId);
        if (dpQuery.exec() && dpQuery.next()) {
            specialty = dpQuery.value("specialty").toString();
            licenseNumber = dpQuery.value("license_number").toString();
            experienceYears = dpQuery.value("experience_years").toInt();
            consultationFee = dpQuery.value("consultation_fee").toInt();
            bio = dpQuery.value("bio").toString();
        }

        QString departmentName = "Chưa phân phòng";
        int deptId = query.value("department_id").toInt();
        if (deptId > 0) {
            QSqlQuery dQuery;
            dQuery.prepare("SELECT department_name FROM departments WHERE department_id = :dept_id");
            dQuery.bindValue(":dept_id", deptId);
            if (dQuery.exec() && dQuery.next()) {
                departmentName = dQuery.value("department_name").toString();
            }
        }

        UserRole roleEnum = roleFromString(query.value("role").toString());
        bool isActive = query.value("is_active").toInt() == 1;

        std::unique_ptr<SystemUser> user = std::make_unique<Doctor>(
            currentStaffId,
            currentStaffCode,
            query.value("password_hash").toString(),
            query.value("full_name").toString(),
            avatarPixmap,
            roleEnum,
            isActive,
            specialty,
            licenseNumber,
            experienceYears,
            consultationFee,
            bio
        );

        lblName->setText(user->getFullName());
        
        QString displayRole = user->getDisplayRole();
        displayRole.replace("Doctor", "Bác sĩ");
        lblRole->setText("Chức vụ: " + displayRole);
        
        lblStaffCode->setText("Mã số: " + currentStaffCode);

        if (isActive) {
            lblStatus->setText("● ĐANG HOẠT ĐỘNG");
            lblStatus->setStyleSheet("color: #36B37E; font-weight: bold; font-size: 13px;");
        } else {
            lblStatus->setText("● NGỪNG HOẠT ĐỘNG");
            lblStatus->setStyleSheet("color: #FF5630; font-weight: bold; font-size: 13px;");
        }

        QString shiftVal = query.value("shift").toString();
        int shiftIdx = cmbShift->findData(shiftVal);
        if (shiftIdx >= 0) {
            cmbShift->setCurrentIndex(shiftIdx);
        }

        QPixmap userAvatar = user->getAvatar();
        if (!userAvatar.isNull()) {
            lblAvatar->setPixmap(userAvatar.scaled(130, 130, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            lblAvatar->setText("👨‍⚕️");
        }

        txtFullName->setText(user->getFullName());
        
        QString genderStr = query.value("gender").toString();
        if (genderStr == "MALE") txtGender->setText("Nam");
        else if (genderStr == "FEMALE") txtGender->setText("Nữ");
        else txtGender->setText("Khác");

        QString dobStr = query.value("date_of_birth").toString();
        QDate dobDate = QDate::fromString(dobStr, "yyyy-MM-dd");
        txtDob->setText(dobDate.isValid() ? dobDate.toString("dd-MM-yyyy") : dobStr);

        txtCitizenId->setText(query.value("citizen_id").toString());
        txtPhone->setText(query.value("phone_number").toString());
        txtEmail->setText(query.value("email").toString());
        txtAddress->setText(query.value("address").toString());

        lblDepartment->setText(departmentName);

        QString hireStr = query.value("hire_date").toString();
        QDate hireDate = QDate::fromString(hireStr, "yyyy-MM-dd");
        lblHireDate->setText(hireDate.isValid() ? hireDate.toString("dd-MM-yyyy") : hireStr);

    } else {
        QString errorMsg = "Không tìm thấy hồ sơ bác sĩ.";
        QMessageBox::warning(this, "Lỗi", errorMsg);
    }
}

void ProfileWidget::onEditClicked() {
    QList<QLineEdit*> editFields = {txtFullName, txtGender, txtDob, txtCitizenId, txtPhone, txtEmail, txtAddress};

    if (btnEdit->text() == "Chỉnh Sửa") {
        for (QLineEdit* field : editFields) {
            field->setReadOnly(false);
            field->setStyleSheet("QLineEdit { font-size: 14px; color: #172B4D; border: 1px solid #0052CC; border-radius: 4px; padding: 2px; background: #FFFFFF; }");
        }
        
        cmbShift->setEnabled(true);

        btnEdit->setText("Lưu Thay Đổi");
        btnEdit->setStyleSheet("QPushButton { background-color: #36B37E; color: white; font-weight: bold; border-radius: 6px; border: none; } QPushButton:hover { background-color: #2a8f64; }");
    } else {
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE staff SET full_name = :full_name, gender = :gender, date_of_birth = :dob, "
                            "citizen_id = :citizen_id, phone_number = :phone, email = :email, address = :address, "
                            "shift = :shift, "
                            "updated_at = datetime('now') WHERE staff_id = :staff_id");

        updateQuery.bindValue(":full_name", txtFullName->text());
        
        QString genderSave = "OTHER";
        QString currentGender = txtGender->text().trimmed().toLower();
        if (currentGender == "nam") genderSave = "MALE";
        else if (currentGender == "nữ" || currentGender == "nu") genderSave = "FEMALE";
        updateQuery.bindValue(":gender", genderSave);

        QString dobInput = txtDob->text().trimmed();
        QDate dobParsed = QDate::fromString(dobInput, "dd-MM-yyyy");
        updateQuery.bindValue(":dob", dobParsed.isValid() ? dobParsed.toString("yyyy-MM-dd") : dobInput);

        updateQuery.bindValue(":citizen_id", txtCitizenId->text());
        updateQuery.bindValue(":phone", txtPhone->text());
        updateQuery.bindValue(":email", txtEmail->text());
        updateQuery.bindValue(":address", txtAddress->text());
        updateQuery.bindValue(":shift", cmbShift->currentData().toString()); 
        updateQuery.bindValue(":staff_id", currentStaffId);

        if (updateQuery.exec()) {
            QMessageBox::information(this, "Thành công", "Đã cập nhật hồ sơ!");
            
            for (QLineEdit* field : editFields) {
                field->setReadOnly(true);
                field->setStyleSheet("QLineEdit { font-size: 14px; color: #172B4D; border: none; background: transparent; padding: 2px; }");
            }
            
            cmbShift->setEnabled(false);

            btnEdit->setText("Chỉnh Sửa");
            btnEdit->setStyleSheet("QPushButton { background-color: #0052CC; color: white; font-weight: bold; border-radius: 6px; border: none; } QPushButton:hover { background-color: #0043A4; }");
            
            loadDoctorProfile(currentStaffCode);
        } else {
            QMessageBox::critical(this, "Lỗi", "Không thể cập nhật: " + updateQuery.lastError().text());
        }
    }
}