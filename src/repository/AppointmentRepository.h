#pragma once

#include "dto/AppointmentDTOs.h"
#include <QList>
#include <QString>

class AppointmentRepository {
public:
    AppointmentRepository() = default;
    ~AppointmentRepository() = default;

    QList<AppointmentRecordDTO> getDoctorAppointments(const QString &doctorId, const QString &date = "") const;
    QList<AppointmentRecordDTO> getPatientAppointments(int patientId) const;
    bool updateAppointmentStatus(int appointmentId, const QString &status) const;
    bool cancelAppointment(int appointmentId) const;
    bool createAppointment(int patientId, const QString &doctorCode, int createdBy, const QString &date, const QString &startTime, const QString &reason) const;
};
