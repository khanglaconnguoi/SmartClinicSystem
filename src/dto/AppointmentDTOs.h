#pragma once

#include <QDate>
#include <QString>
#include <QTime>

struct AppointmentRecordDTO {
    int appointmentId = 0;
    int ticketNumber = 0;
    int patientId = 0;
    QString doctorId;
    QString appointmentDate;
    QString startTime;
    QString endTime;
    QString status;
    QString reason;
    QString notes;
    QString patientName;
    QString patientCode;
    QString roomNumber;
    QString doctorName;
    QString doctorSpecialty;
    QString checkedInAt;
    QString startedAt;
    QString completedAt;
};

struct AppointmentInputDTO {
    int patientId = 0;
    int doctorId = 0;
    int createdBy = 0;
    int ticketNumber = 0;
    QDate date;
    QTime startTime;
    QTime endTime;
    QString reason;
};

struct RoomQueueItemDTO {
    int ticketNumber = 0;
    QString patientName;
    QString startTime;
    QString status;
};

struct RoomQueueStatusDTO {
    int roomId = 0;
    QString roomNumber;
    int doctorId = 0;
    QString doctorName;
    int currentTicketNumber = 0; // The one currently STARTED
    int nextTicketNumber = 0;    // The next one CHECKED_IN
    int waitingCount = 0;        // Count of patients CHECKED_IN waiting
};
