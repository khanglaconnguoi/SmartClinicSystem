#pragma once

#include <QList>
#include <QPair>
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
    QList<AppointmentRecordDTO> getAppointmentsByDate(const QString& date) const;
    bool updateAppointmentStatus(int appointmentId, const QString& status) const;
    QPair<QString, int> checkInPatient(int appointmentId) const;
    bool cancelAppointment(int appointmentId) const;
    int cancelAppointmentsForDoctor(int doctorId, const QString& startDate, const QString& endDate) const;
    bool createAppointment(const AppointmentInputDTO& input) const;
    void autoMarkNoShowAppointments() const;

    // Room Queue Methods
    QList<RoomQueueStatusDTO> getRoomQueueStatuses(const QString& date) const;
    QList<RoomQueueItemDTO> getDoctorQueue(int doctorId, const QString& date) const;

    bool checkInPatientByTicket(int doctorId, const QString &date, int ticketNumber) const;
  
    QPair<int, QString> callSpecificPatient(int appointmentId) const;

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
