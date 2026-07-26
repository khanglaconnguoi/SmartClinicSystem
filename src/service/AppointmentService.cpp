#include "AppointmentService.h"

#include <QDate>
#include <QDebug>
#include <QRegularExpression>

#include "Validation.h"

void AppointmentService::normalizeAppointmentInput(AppointmentInputDTO& input) {
    input.reason = input.reason.simplified();
}

QString AppointmentService::validateAppointmentDate(const QDate& date) {
    if (!date.isValid()) { return "Appointment date is required and must be a valid date."; }
    if (date < QDate::currentDate()) { return "Appointment date cannot be in the past."; }
    return "";
}

QString AppointmentService::validateStartTime(const QTime& startTime) {
    if (!startTime.isValid()) { return "Start time is required and must be a valid time."; }
    return "";
}

QString AppointmentService::validateEndTime(const QTime& endTime, const QTime& startTime) {
    if (endTime.isValid()) {
        if (startTime.isValid() && endTime <= startTime) {
            return "End time must be strictly after start time.";
        }
    }
    return "";
}

QString AppointmentService::validateAppointmentStatus(const QString& status) {
    if (status.trimmed().isEmpty()) { return "Appointment status is required."; }
    if (!AppointmentStatusText::isValid(status)) { return "Invalid appointment status value."; }
    return "";
}

QString AppointmentService::validatePatientExists(int patientId) const {
    QString err = Validation::validateValidId(patientId, "Please select a valid patient.");
    if (!err.isEmpty()) return err;

    if (m_patientRepository && !m_patientRepository->getPatientById(patientId).has_value()) {
        return "Patient does not exist in the system.";
    }
    return "";
}

QString AppointmentService::validateDoctorExists(int doctorId) const {
    QString err = Validation::validateValidId(doctorId, "Please select a valid doctor.");
    if (!err.isEmpty()) return err;

    if (m_staffRepository && !m_staffRepository->existsByStaffId(doctorId)) {
        return "Doctor does not exist in the system.";
    }
    return "";
}

QString AppointmentService::validateDoctorConflict(int doctorId,
        const QDate& date,
        const QTime& startTime,
        const QTime& endTime,
        int excludeId) const {
    QString dateStr = date.toString("yyyy-MM-dd");
    QString startStr = startTime.isValid() ? startTime.toString("HH:mm") : "";
    QString endStr = endTime.isValid() ? endTime.toString("HH:mm") : "";
    if (m_appointmentRepository->hasDoctorScheduleConflict(
                doctorId, dateStr, startStr, endStr, excludeId)) {
        return "Doctor already has an appointment scheduled at this time slot.";
    }
    return "";
}

QString AppointmentService::validatePatientConflict(int patientId,
        const QDate& date,
        const QTime& startTime,
        const QTime& endTime,
        int excludeId) const {
    QString dateStr = date.toString("yyyy-MM-dd");
    QString startStr = startTime.isValid() ? startTime.toString("HH:mm") : "";
    QString endStr = endTime.isValid() ? endTime.toString("HH:mm") : "";
    if (m_appointmentRepository->hasPatientScheduleConflict(
                patientId, dateStr, startStr, endStr, excludeId)) {
        return "Patient already has an appointment scheduled at this time slot.";
    }
    return "";
}

QString AppointmentService::validateAppointmentInput(
        const AppointmentInputDTO& input, int excludeId) const {
    QString err;
    if (!(err = validatePatientExists(input.patientId)).isEmpty()) return err;
    if (!(err = validateDoctorExists(input.doctorId)).isEmpty()) return err;
    if (!(err = validateAppointmentDate(input.date)).isEmpty()) return err;
    if (!(err = validateStartTime(input.startTime)).isEmpty()) return err;
    if (!(err = validateEndTime(input.endTime, input.startTime)).isEmpty()) return err;
    if (!(err = Validation::validateTrimmedNotEmpty(
                  input.reason, "Appointment reason is required."))
                    .isEmpty())
        return err;

    if (!(err = validateDoctorConflict(
                  input.doctorId, input.date, input.startTime, input.endTime, excludeId))
                    .isEmpty())
        return err;
    if (!(err = validatePatientConflict(
                  input.patientId, input.date, input.startTime, input.endTime, excludeId))
                    .isEmpty())
        return err;

    return "";
}

QList<AppointmentRecordDTO> AppointmentService::getDoctorAppointments(
        int doctorId, const QDate& date) const {
    QString dateStr = date.isValid() ? date.toString("yyyy-MM-dd") : "";
    return m_appointmentRepository->getDoctorAppointments(doctorId, dateStr);
}

// QList<AppointmentRecordDTO> AppointmentService::getDoctorAppointments(
//         const QString& doctorId, const QDate& date) const {
//     QString dateStr = date.isValid() ? date.toString("yyyy-MM-dd") : "";
//     return m_appointmentRepository->getDoctorAppointments(doctorId, dateStr);
// }

QList<AppointmentRecordDTO> AppointmentService::getAppointmentsByDate(const QDate& date) const {
    QString dateStr = date.isValid() ? date.toString("yyyy-MM-dd") : "";
    return m_appointmentRepository->getAppointmentsByDate(dateStr);
}

QList<AppointmentRecordDTO> AppointmentService::getPatientAppointments(int patientId) const {
    return m_appointmentRepository->getPatientAppointments(patientId);
}

bool AppointmentService::updateAppointmentStatus(int appointmentId, const QString& status) const {
    if (!Validation::validateValidId(appointmentId, "Invalid appointment ID.").isEmpty())
        return false;
    if (!validateAppointmentStatus(status).isEmpty()) return false;

    QString normalizedStatus = AppointmentStatusText::toEn(status);
    return m_appointmentRepository->updateAppointmentStatus(appointmentId, normalizedStatus);
}

QPair<QString, int> AppointmentService::checkInPatient(int appointmentId) const {
    QString err = Validation::validateValidId(appointmentId, "Invalid appointment ID.");
    if (!err.isEmpty()) return qMakePair(err, -1);

    if (m_appointmentRepository && !m_appointmentRepository->existsById(appointmentId)) {
        return qMakePair(QString("Appointment does not exist in the system."), -1);
    }

    return m_appointmentRepository->checkInPatient(appointmentId);
}

bool AppointmentService::cancelAppointment(int appointmentId) const {
    if (!Validation::validateValidId(appointmentId, "Invalid appointment ID.").isEmpty())
        return false;
    return m_appointmentRepository->cancelAppointment(appointmentId);
}

QString AppointmentService::createAppointment(AppointmentInputDTO input) const {
    normalizeAppointmentInput(input);
    QString err = validateAppointmentInput(input);
    if (!err.isEmpty()) {
        qWarning() << "AppointmentService::createAppointment validation failed:" << err;
        return err;
    }

    if (!m_appointmentRepository->createAppointment(input)) {
        return "Failed to create appointment in database.";
    }
    return "";
}

// bool AppointmentService::createAppointment(const AppointmentInputDTO& input) const {
//     return createAppointmentWithReason(input).isEmpty();
// }
