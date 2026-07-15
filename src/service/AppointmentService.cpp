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

bool AppointmentService::createAppointment(int patientId, const QString &doctorCode, int createdBy, const QString &date, const QString &startTime, const QString &reason) const {
    return m_appointmentRepository->createAppointment(patientId, doctorCode, createdBy, date, startTime, reason);
}

bool AppointmentService::cancelAppointment(int appointmentId) const {
    return m_appointmentRepository->cancelAppointment(appointmentId);
}
