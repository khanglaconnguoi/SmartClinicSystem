#pragma once

#include "dto/AppointmentDTOs.h"
#include "repository/AppointmentRepository.h"
#include <QList>
#include <QString>
#include <memory>

class AppointmentService {
public:
    explicit AppointmentService(std::shared_ptr<AppointmentRepository> repo)
        : m_appointmentRepository(std::move(repo)) {}

    QList<AppointmentRecordDTO> getDoctorAppointments(const QString &doctorId, const QString &date = "") const;
    QList<AppointmentRecordDTO> getPatientAppointments(int patientId) const;
    bool updateAppointmentStatus(int appointmentId, const QString &status) const;
    bool cancelAppointment(int appointmentId) const;
    bool createAppointment(const AppointmentInputDTO &input) const;

private:
    std::shared_ptr<AppointmentRepository> m_appointmentRepository;
};
