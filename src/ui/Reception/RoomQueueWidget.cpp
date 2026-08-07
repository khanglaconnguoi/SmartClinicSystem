#include "RoomQueueWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

RoomQueueWidget::RoomQueueWidget(int roomId, const QString& roomName, int doctorId, const QString& doctorName, int currentTicket, int nextTicket, int waitingCount, QWidget *parent)
    : QFrame(parent), m_roomId(roomId), m_doctorId(doctorId) 
{
    setupUi(roomName, doctorName, currentTicket, nextTicket, waitingCount);
}

void RoomQueueWidget::setupUi(const QString& roomName, const QString& doctorName, int currentTicket, int /*nextTicket*/, int waitingCount) {
    setFixedSize(210, 195);
    setStyleSheet(
        "RoomQueueWidget {"
        "    background-color: #FFFFFF;"
        "    border-radius: 14px;"
        "    border: 1px solid #E5E7EB;"
        "}"
        "RoomQueueWidget:hover {"
        "    border: 2px solid #2563EB;"
        "    background-color: #F8FAFC;"
        "}"
    );
    setCursor(Qt::PointingHandCursor);

    // Drop shadow
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(12);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(2);

    // Top: Room Name & Doctor Name
    auto *lblRoom = new QLabel(roomName, this);
    lblRoom->setStyleSheet("font-size: 16px; font-weight: 800; color: #1E3A8A; background: transparent; border: none;");
    lblRoom->setAlignment(Qt::AlignCenter);

    auto *lblDoctor = new QLabel(doctorName.isEmpty() ? "Bác sĩ chưa gán" : doctorName, this);
    lblDoctor->setStyleSheet("font-size: 13px; color: #4B5563; font-weight: 600; background: transparent; border: none;");
    lblDoctor->setAlignment(Qt::AlignCenter);
    lblDoctor->setWordWrap(true);

    layout->addWidget(lblRoom);
    layout->addWidget(lblDoctor);
    layout->addStretch();

    // Center: BIG Ticket Number & Status Badge
    QString ticketStr;
    QString ticketStyle;
    QString statusText;
    QString statusStyle;

    if (m_doctorId == 0) {
        ticketStr = "--";
        ticketStyle = "font-size: 34px; font-weight: 800; color: #D1D5DB; background: transparent; border: none;";
        statusText = "PHÒNG TRỐNG";
        statusStyle = "font-size: 12px; color: #6B7280; font-weight: 700; padding: 3px 10px; background-color: #F3F4F6; border-radius: 6px;";
    } else {
        if (currentTicket > 0) {
            ticketStr = QString("%1").arg(currentTicket, 2, 10, QChar('0'));
            ticketStyle = "font-size: 34px; font-weight: 800; color: #059669; background: transparent; border: none;";
            statusText = "Đang gọi";
            statusStyle = "font-size: 12px; color: #047857; font-weight: 700; padding: 3px 10px; background-color: #D1FAE5; border-radius: 6px;";
        } else {
            ticketStr = "--";
            ticketStyle = "font-size: 34px; font-weight: 800; color: #9CA3AF; background: transparent; border: none;";
            statusText = "Sẵn sàng";
            statusStyle = "font-size: 12px; color: #D97706; font-weight: 700; padding: 3px 10px; background-color: #FEF3C7; border-radius: 6px;";
        }
    }

    auto *lblTicketNum = new QLabel(ticketStr, this);
    lblTicketNum->setStyleSheet(ticketStyle);
    lblTicketNum->setAlignment(Qt::AlignCenter);

    auto *lblStatus = new QLabel(statusText, this);
    lblStatus->setStyleSheet(statusStyle);
    lblStatus->setAlignment(Qt::AlignCenter);

    layout->addWidget(lblTicketNum);
    layout->addWidget(lblStatus);
    layout->addStretch();

    // Bottom: Small waiting count display at the bottom
    auto *lblWaiting = new QLabel(this);
    if (m_doctorId > 0) {
        lblWaiting->setText(QString("Đang chờ: %1 bệnh nhân").arg(waitingCount));
        lblWaiting->setStyleSheet("font-size: 11px; color: #64748B; font-weight: 600; background: transparent; border: none;");
    } else {
        lblWaiting->setText("");
    }
    lblWaiting->setAlignment(Qt::AlignCenter);
    layout->addWidget(lblWaiting);
}

void RoomQueueWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_roomId, m_doctorId);
    }
    QFrame::mousePressEvent(event);
}

void RoomQueueWidget::enterEvent(QEnterEvent *event) {
    QFrame::enterEvent(event);
}

void RoomQueueWidget::leaveEvent(QEvent *event) {
    QFrame::leaveEvent(event);
}
