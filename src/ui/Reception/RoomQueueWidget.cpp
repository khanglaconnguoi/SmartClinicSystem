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
    setFixedSize(200, 175);
    setStyleSheet(
        "RoomQueueWidget {"
        "    background-color: #FFFFFF;"
        "    border-radius: 12px;"
        "    border: 1px solid #E5E7EB;"
        "}"
        "RoomQueueWidget:hover {"
        "    border: 2px solid #3B82F6;"
        "    background-color: #F8FAFC;"
        "}"
    );
    setCursor(Qt::PointingHandCursor);

    // Drop shadow
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(4);

    // Room Name
    auto *lblRoom = new QLabel(roomName, this);
    lblRoom->setStyleSheet("font-size: 18px; font-weight: bold; color: #1E3A8A;");
    lblRoom->setAlignment(Qt::AlignCenter);

    // Doctor Name
    auto *lblDoctor = new QLabel(doctorName.isEmpty() ? "Trống" : doctorName, this);
    lblDoctor->setStyleSheet("font-size: 14px; color: #4B5563; font-weight: 500;");
    lblDoctor->setAlignment(Qt::AlignCenter);
    lblDoctor->setWordWrap(true);

    // Remaining waiting count
    auto *lblWaiting = new QLabel(this);
    if (m_doctorId > 0) {
        lblWaiting->setText(QString("Đang chờ: %1 bệnh nhân").arg(waitingCount));
        lblWaiting->setStyleSheet("font-size: 12px; color: #475569; font-weight: 600; background: transparent; padding: 2px 0px;");
    } else {
        lblWaiting->setText("");
    }
    lblWaiting->setAlignment(Qt::AlignCenter);

    // Status / Queue Ticket
    QString queueText;
    QString queueStyle;
    
    if (m_doctorId == 0) {
        queueText = "PHÒNG TRỐNG";
        queueStyle = "font-size: 14px; color: #9CA3AF; font-weight: bold; padding: 6px; background-color: #F3F4F6; border-radius: 6px;";
    } else {
        if (currentTicket > 0) {
            queueText = QString("Đang gọi: Số %1").arg(currentTicket, 2, 10, QChar('0'));
            queueStyle = "font-size: 14px; color: #10B981; font-weight: bold; padding: 6px; background-color: #D1FAE5; border-radius: 6px;";
        } else {
            queueText = "Đang chờ bệnh nhân";
            queueStyle = "font-size: 14px; color: #F59E0B; font-weight: bold; padding: 6px; background-color: #FEF3C7; border-radius: 6px;";
        }
    }

    auto *lblQueue = new QLabel(queueText, this);
    lblQueue->setStyleSheet(queueStyle);
    lblQueue->setAlignment(Qt::AlignCenter);
    lblQueue->setWordWrap(true);

    layout->addWidget(lblRoom);
    layout->addWidget(lblDoctor);
    if (m_doctorId > 0) {
        layout->addWidget(lblWaiting);
    }
    layout->addStretch();
    layout->addWidget(lblQueue);
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
