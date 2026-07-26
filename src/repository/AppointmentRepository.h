#pragma once

#include <QList>
#include <QString>

#include "dto/AppointmentDTOs.h"

class AppointmentRepository {
   public:
    AppointmentRepository() = default;
    ~AppointmentRepository() = default;

    QList<AppointmentRecordDTO> getDoctorAppointments(int doctorId, const QString& date = "") const;
    // QList<AppointmentRecordDTO> getDoctorAppointments(
    //         const QString& doctorId, const QString& date = "") const;
    QList<AppointmentRecordDTO> getPatientAppointments(int patientId) const;
    bool updateAppointmentStatus(int appointmentId, const QString& status) const;
    bool cancelAppointment(int appointmentId) const;
    bool createAppointment(const AppointmentInputDTO& input) const;

    // Helper methods for Service validation
    bool existsById(int appointmentId) const;
    bool hasDoctorScheduleConflict(int doctorId,
            const QString& date,
            const QString& startTime,
            const QString& endTime = "",
            int excludeAppointmentId = -1) const;
    bool hasPatientScheduleConflict(int patientId,
            const QString& date,
            const QString& startTime,
            const QString& endTime = "",
            int excludeAppointmentId = -1) const;
};
