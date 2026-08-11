#include "CreateLabRequestDialog.h"
#include <QGraphicsDropShadowEffect>

CreateLabRequestDialog::CreateLabRequestDialog(
    std::shared_ptr<ServiceRequestService> serviceRequestService,
    std::shared_ptr<AppointmentService> appointmentService,
    int recordId,
    int doctorId,
    const QString& doctorName,
    const QString& patientName,
    const QString& patientCode,
    QWidget* parent)
    : QDialog(parent),
      m_serviceRequestService(serviceRequestService),
      m_appointmentService(appointmentService),
      m_recordId(recordId),
      m_doctorId(doctorId),
      m_doctorName(doctorName),
      m_patientName(patientName),
      m_patientCode(patientCode) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowTitle("Chỉ Định Xét Nghiệm");
    resize(550, 420);
    setupUI();
    loadLabRooms();
}

void CreateLabRequestDialog::setupUI() {
    this->setObjectName("CreateLabRequestDialog");
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet("#CreateLabRequestDialog { background-color: transparent; } QLabel { color: #1E293B; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 28);

    QFrame *container = new QFrame(this);
    container->setObjectName("mainContainer");
    container->setStyleSheet("#mainContainer { background-color: #FFFFFF; border-radius: 14px; border: 1px solid #E2E8F0; }");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 40));
    shadow->setOffset(0, 4);
    container->setGraphicsEffect(shadow);

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(24, 20, 24, 20);
    containerLayout->setSpacing(16);

    // Title
    QLabel *lblTitle = new QLabel("YÊU CẦU XÉT NGHIỆM / CẬN LÂM SÀNG", container);
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2563EB;");
    lblTitle->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(lblTitle);

    // Patient & Doctor Info Header Box
    QFrame *infoBox = new QFrame(container);
    infoBox->setStyleSheet("QFrame { background-color: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 8px; }");
    QFormLayout *infoLayout = new QFormLayout(infoBox);
    infoLayout->setContentsMargins(12, 10, 12, 10);
    infoLayout->setSpacing(8);

    m_lblPatientInfo = new QLabel(QString("<b>%1</b> (Mã BN: %2)").arg(m_patientName, m_patientCode), infoBox);
    m_lblDoctorInfo = new QLabel(QString("<b>%1</b>").arg(m_doctorName.isEmpty() ? "Bác sĩ chỉ định" : m_doctorName), infoBox);

    infoLayout->addRow("Bệnh nhân:", m_lblPatientInfo);
    infoLayout->addRow("Bác sĩ chỉ định:", m_lblDoctorInfo);
    containerLayout->addWidget(infoBox);

    // Form inputs
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    QString inputStyle = 
        "QComboBox, QLineEdit { border: 1px solid #CBD5E1; border-radius: 8px; padding: 8px 12px; font-size: 13px; color: #0F172A; background-color: #FFFFFF; min-height: 36px; }"
        "QComboBox:focus, QLineEdit:focus { border: 1px solid #2563EB; background-color: #EFF6FF; }";

    m_cbLabRoom = new QComboBox(container);
    m_cbLabRoom->setStyleSheet(inputStyle);

    m_cbServiceName = new QComboBox(container);
    m_cbServiceName->setStyleSheet(inputStyle);
    m_cbServiceName->setEditable(true);
    m_cbServiceName->addItems({
        "Xét nghiệm công thức máu toàn phần (CBC)",
        "Xét nghiệm sinh hóa máu (Đường huyết, Ure, Creatinin)",
        "Xét nghiệm chức năng gan (ALT, AST, Bilirubin)",
        "Xét nghiệm mỡ máu (Cholesterol, Triglyceride)",
        "Xét nghiệm nước tiểu 10 thông số",
        "X-Quang ngực thẳng",
        "Siêu âm bụng tổng quát",
        "Điện tâm đồ (ECG)"
    });

    m_txtCustomService = new QLineEdit(container);
    m_txtCustomService->setStyleSheet(inputStyle);
    m_txtCustomService->setPlaceholderText("Nhập chi tiết/ghi chú dịch vụ xét nghiệm bổ sung (nếu có)...");

    formLayout->addRow("Phòng xét nghiệm (*):", m_cbLabRoom);
    formLayout->addRow("Dịch vụ xét nghiệm (*):", m_cbServiceName);
    formLayout->addRow("Ghi chú/Chi tiết:", m_txtCustomService);

    containerLayout->addLayout(formLayout);
    containerLayout->addStretch();

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_btnCancel = new QPushButton("Hủy bỏ", container);
    m_btnCancel->setFixedSize(130, 38);
    m_btnCancel->setCursor(Qt::PointingHandCursor);
    m_btnCancel->setStyleSheet("QPushButton { background-color: #F1F5F9; color: #475569; border: 1px solid #CBD5E1; border-radius: 6px; font-weight: bold; } QPushButton:hover { background-color: #E2E8F0; }");

    m_btnSubmit = new QPushButton("Gửi Yêu Cầu", container);
    m_btnSubmit->setFixedSize(130, 38);
    m_btnSubmit->setCursor(Qt::PointingHandCursor);
    m_btnSubmit->setStyleSheet("QPushButton { background-color: #2563EB; color: white; border: none; border-radius: 6px; font-weight: bold; } QPushButton:hover { background-color: #1D4ED8; }");

    btnLayout->addWidget(m_btnCancel);
    btnLayout->addWidget(m_btnSubmit);

    containerLayout->addLayout(btnLayout);
    mainLayout->addWidget(container);

    connect(m_btnSubmit, &QPushButton::clicked, this, &CreateLabRequestDialog::handleSubmit);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void CreateLabRequestDialog::loadLabRooms() {
    m_cbLabRoom->clear();
    if (!m_appointmentService) {
        m_cbLabRoom->addItem("Phòng xét nghiệm 1", 1);
        return;
    }

    auto rooms = m_appointmentService->getRoomsByType("LAB");
    if (rooms.isEmpty()) {
        m_cbLabRoom->addItem("Chưa có phòng xét nghiệm", -1);
    } else {
        for (const auto& room : rooms) {
            m_cbLabRoom->addItem(room.second, room.first);
        }
    }
}

void CreateLabRequestDialog::showNotification(const QString& title, const QString& text, QMessageBox::Icon icon) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIcon(icon);

    QString btnBg = (icon == QMessageBox::Critical) ? "#EF4444" : 
                    (icon == QMessageBox::Warning) ? "#F59E0B" : "#2563EB";
    QString btnHover = (icon == QMessageBox::Critical) ? "#DC2626" : 
                       (icon == QMessageBox::Warning) ? "#D97706" : "#1D4ED8";

    msgBox.setStyleSheet(QString(
        "QMessageBox { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 10px; }"
        "QLabel { color: #0F172A; font-size: 13px; font-weight: 500; qproperty-alignment: AlignCenter; }"
        "QPushButton { background-color: %1; color: #FFFFFF; font-weight: bold; min-width: 80px; min-height: 32px; border-radius: 6px; border: none; font-size: 13px; padding: 4px 14px; }"
        "QPushButton:hover { background-color: %2; }"
    ).arg(btnBg, btnHover));

    msgBox.exec();
}

void CreateLabRequestDialog::handleSubmit() {
    if (!m_serviceRequestService) {
        showNotification("Lỗi", "Service không khả dụng.", QMessageBox::Critical);
        return;
    }

    int roomId = m_cbLabRoom->currentData().toInt();
    if (roomId <= 0) {
        showNotification("Thiếu thông tin", "Vui lòng chọn phòng xét nghiệm hợp lệ.", QMessageBox::Warning);
        return;
    }

    QString serviceName = m_cbServiceName->currentText().trimmed();
    QString customNote = m_txtCustomService->text().trimmed();
    if (!customNote.isEmpty()) {
        serviceName += " (" + customNote + ")";
    }

    if (serviceName.isEmpty()) {
        showNotification("Thiếu thông tin", "Vui lòng chọn hoặc nhập tên dịch vụ xét nghiệm.", QMessageBox::Warning);
        return;
    }

    if (m_recordId <= 0) {
        showNotification("Thiếu thông tin", "Vui lòng lưu thông tin hồ sơ bệnh án trước khi gửi yêu cầu xét nghiệm.", QMessageBox::Warning);
        return;
    }

    ServiceRequestInputDTO input;
    input.recordId = m_recordId;
    input.roomId = roomId;
    input.doctorId = m_doctorId;
    input.serviceName = serviceName;

    int reqId = m_serviceRequestService->createRequest(input);
    if (reqId > 0) {
        showNotification("Thành công", "Đã gửi yêu cầu xét nghiệm thành công!", QMessageBox::Information);
        accept();
    } else {
        showNotification("Lỗi", "Không thể tạo yêu cầu xét nghiệm. Vui lòng kiểm tra lại dữ liệu.", QMessageBox::Critical);
    }
}
