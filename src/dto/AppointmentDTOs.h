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
