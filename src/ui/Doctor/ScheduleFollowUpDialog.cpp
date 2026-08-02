#include "ScheduleFollowUpDialog.h"
#include <QMessageBox>

ScheduleFollowUpDialog::ScheduleFollowUpDialog(
    int patientId,
    const QString &patientName,
    int doctorId,
    std::shared_ptr<AppointmentService> appointmentService,
    QWidget *parent)
    : QDialog(parent),
      m_patientId(patientId),
      m_patientName(patientName),
      m_doctorId(doctorId),
      m_appointmentService(appointmentService) {
    setWindowTitle("Đặt lịch hẹn tái khám");
    setFixedSize(460, 360);
    setStyleSheet("QDialog { background-color: #FFFFFF; font-family: 'Segoe UI', Arial, sans-serif; }"
                  "QLabel { font-size: 13px; color: #374151; font-weight: 500; }"
                  "QLineEdit, QDateEdit, QTimeEdit, QComboBox { border: 1px solid #D1D5DB; border-radius: 6px; padding: 8px 12px; font-size: 13px; background-color: #F9FAFB; color: #111827; }"
                  "QLineEdit:focus, QDateEdit:focus, QTimeEdit:focus, QComboBox:focus { border: 1px solid #2563EB; background-color: #FFFFFF; }"
                  "QPushButton { font-weight: 600; font-size: 13px; padding: 8px 16px; border-radius: 6px; }"
                  "#btnSave { background-color: #2563EB; color: white; border: none; }"
                  "#btnSave:hover { background-color: #1D4ED8; }"
                  "#btnCancel { background-color: white; color: #4B5563; border: 1px solid #D1D5DB; }"
                  "#btnCancel:hover { background-color: #F3F4F6; }");

    setupUi();
    loadRooms();
}

void ScheduleFollowUpDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    QLabel *lblTitle = new QLabel("Hẹn tái khám cho bệnh nhân", this);
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #111827;");
    mainLayout->addWidget(lblTitle);

    QLabel *lblPatient = new QLabel(QString("Bệnh nhân: <b>%1</b>").arg(m_patientName), this);
    lblPatient->setStyleSheet("font-size: 14px; color: #2563EB;");
    mainLayout->addWidget(lblPatient);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    m_dateEdit = new QDateEdit(QDate::currentDate().addDays(7), this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd/MM/yyyy");
    m_dateEdit->setMinimumDate(QDate::currentDate());

    m_timeEdit = new QTimeEdit(QTime(9, 0), this);
    m_timeEdit->setDisplayFormat("HH:mm");

    m_cbRooms = new QComboBox(this);

    m_txtReason = new QLineEdit(this);
    m_txtReason->setText("Tái khám theo chỉ định bác sĩ");

    formLayout->addRow("Ngày tái khám *:", m_dateEdit);
    formLayout->addRow("Giờ tái khám *:", m_timeEdit);
    formLayout->addRow("Phòng khám:", m_cbRooms);
    formLayout->addRow("Lý do / Ghi chú:", m_txtReason);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_btnCancel = new QPushButton("Hủy", this);
    m_btnCancel->setObjectName("btnCancel");
    m_btnCancel->setCursor(Qt::PointingHandCursor);

    m_btnSave = new QPushButton("Đặt lịch hẹn", this);
    m_btnSave->setObjectName("btnSave");
    m_btnSave->setCursor(Qt::PointingHandCursor);

    buttonLayout->addWidget(m_btnCancel);
    buttonLayout->addWidget(m_btnSave);

    mainLayout->addLayout(buttonLayout);

    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_btnSave, &QPushButton::clicked, this, &ScheduleFollowUpDialog::handleSave);
}

void ScheduleFollowUpDialog::loadRooms() {
    m_cbRooms->clear();
    if (!m_appointmentService) {
        m_cbRooms->addItem("Phòng khám mặc định", 1);
        return;
    }
    auto rooms = m_appointmentService->getExaminationRooms();
    if (rooms.isEmpty()) {
        m_cbRooms->addItem("Phòng khám mặc định", 1);
    } else {
        for (const auto &r : rooms) {
            m_cbRooms->addItem(r.second, r.first);
        }
    }
}

void ScheduleFollowUpDialog::handleSave() {
    if (!m_appointmentService) {
        QMessageBox::warning(this, "Lỗi", "Dịch vụ cuộc hẹn chưa được khởi tạo.");
        return;
    }

    AppointmentInputDTO input;
    input.patientId = m_patientId;
    input.doctorId = m_doctorId;
    input.createdBy = m_doctorId;
    input.date = m_dateEdit->date();
    input.startTime = m_timeEdit->time();
    input.endTime = m_timeEdit->time().addSecs(1800); // 30 mins
    input.reason = m_txtReason->text().trimmed();

    QString err = m_appointmentService->createAppointment(input);
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Lỗi đặt lịch hẹn", err);
        return;
    }

    QMessageBox::information(this, "Thành công",
        QString("Đã đặt lịch hẹn tái khám thành công cho bệnh nhân %1 vào ngày %2 lúc %3.")
        .arg(m_patientName, m_dateEdit->date().toString("dd/MM/yyyy"), m_timeEdit->time().toString("HH:mm")));

    emit appointmentScheduled();
    accept();
}
