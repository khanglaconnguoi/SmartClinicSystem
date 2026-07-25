#pragma once

#include <QString>

struct AppointmentRecordDTO {
    int appointmentId;
    int patientId;
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
};

struct AppointmentInputDTO {
    int patientId;
    QString doctorCode;
    int createdBy;
    QString date;
    QString startTime;
    QString reason;
};
