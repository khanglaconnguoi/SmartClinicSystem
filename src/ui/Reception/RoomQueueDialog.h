#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QString>

#include "../../dto/AppointmentDTOs.h"

class RoomQueueDialog : public QDialog {
    Q_OBJECT
public:
    explicit RoomQueueDialog(int roomId, const QString& roomName, const QList<RoomQueueItemDTO>& queueItems, QWidget *parent = nullptr);
    ~RoomQueueDialog() override = default;

private:
    void setupUi();
    void loadData();

    int m_roomId;
    QString m_roomName;
    QList<RoomQueueItemDTO> m_queueItems;

    QTableWidget *m_tableQueue;
    QLabel *m_lblTitle;
};
