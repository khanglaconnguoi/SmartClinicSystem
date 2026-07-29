#pragma once

#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QEnterEvent>

class RoomQueueWidget : public QFrame {
    Q_OBJECT
public:
    explicit RoomQueueWidget(int roomId, const QString& roomName, int doctorId, const QString& doctorName, int currentTicket, int nextTicket, QWidget *parent = nullptr);
    ~RoomQueueWidget() override = default;

    int getRoomId() const { return m_roomId; }
    int getDoctorId() const { return m_doctorId; }

signals:
    void clicked(int roomId, int doctorId);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupUi(const QString& roomName, const QString& doctorName, int currentTicket, int nextTicket);

    int m_roomId;
    int m_doctorId;
};
