#include "AppointmentService.h"

QList<AppointmentRecordDTO> AppointmentService::getDoctorAppointments(const QString &doctorId, const QString &date) const {
    return m_appointmentRepository->getDoctorAppointments(doctorId, date);
}

QList<AppointmentRecordDTO> AppointmentService::getPatientAppointments(int patientId) const {
    return m_appointmentRepository->getPatientAppointments(patientId);
}

bool AppointmentService::updateAppointmentStatus(int appointmentId, const QString &status) const {
    return m_appointmentRepository->updateAppointmentStatus(appointmentId, status);
}

bool AppointmentService::createAppointment(const AppointmentInputDTO &input) const {
    return m_appointmentRepository->createAppointment(input);
}

bool AppointmentService::cancelAppointment(int appointmentId) const {
    return m_appointmentRepository->cancelAppointment(appointmentId);
}
