#include "RoomQueueDialog.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include "../../model/CommonEnums.h"

RoomQueueDialog::RoomQueueDialog(int roomId, const QString& roomName, const QList<RoomQueueItemDTO>& queueItems, QWidget *parent)
    : QDialog(parent), m_roomId(roomId), m_roomName(roomName), m_queueItems(queueItems)
{
    setupUi();
    loadData();
}

void RoomQueueDialog::setupUi() {
    setWindowTitle(QString("Hàng đợi - %1").arg(m_roomName));
    setMinimumSize(600, 400);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // Add a container for border/shadow if frameless
    QFrame* container = new QFrame(this);
    container->setStyleSheet("QFrame { background-color: white; border: 1px solid #E5E7EB; border-radius: 8px; }");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(container);

    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(20, 20, 20, 20);

    m_lblTitle = new QLabel(QString("Danh sách bệnh nhân đang đợi tại %1").arg(m_roomName), this);
    m_lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #1F2937; margin-bottom: 10px;");
    layout->addWidget(m_lblTitle);

    m_tableQueue = new QTableWidget(this);
    m_tableQueue->setColumnCount(4);
    m_tableQueue->setHorizontalHeaderLabels({"STT", "Tên Bệnh Nhân", "Giờ Khám", "Trạng Thái"});
    m_tableQueue->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableQueue->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableQueue->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableQueue->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tableQueue->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableQueue->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableQueue->setAlternatingRowColors(true);
    m_tableQueue->setFocusPolicy(Qt::NoFocus);
    m_tableQueue->setStyleSheet(
        "QTableWidget { border: 1px solid #E5E7EB; border-radius: 4px; background-color: #FFFFFF; color: #111827; outline: none; }"
        "QTableWidget::item { padding: 4px; color: #111827; outline: none; }"
        "QTableWidget::item:focus { outline: none; border: none; }"
        "QHeaderView::section { background-color: #3B82F6; color: white; padding: 6px; font-weight: bold; border: none; border-bottom: 1px solid #E5E7EB; }"
    );
    layout->addWidget(m_tableQueue);

    auto *btnClose = new QPushButton("Đóng", this);
    btnClose->setStyleSheet(
        "QPushButton { background-color: #E5E7EB; padding: 8px 16px; border-radius: 4px; font-weight: bold; color: #374151; border: none; }"
        "QPushButton:hover { background-color: #D1D5DB; }"
    );
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnClose);
    layout->addLayout(btnLayout);
}

void RoomQueueDialog::loadData() {
    m_tableQueue->setRowCount(0);
    int row = 0;
    for (const auto& item : m_queueItems) {
        m_tableQueue->insertRow(row);
        
        auto *itemTicket = new QTableWidgetItem(QString::number(item.ticketNumber));
        itemTicket->setTextAlignment(Qt::AlignCenter);
        
        m_tableQueue->setItem(row, 0, itemTicket);
        m_tableQueue->setItem(row, 1, new QTableWidgetItem(item.patientName));
        m_tableQueue->setItem(row, 2, new QTableWidgetItem(item.startTime));
        m_tableQueue->setItem(row, 3, new QTableWidgetItem(AppointmentStatusText::toVi(item.status)));

        row++;
    }
}
