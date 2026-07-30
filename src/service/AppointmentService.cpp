#include "AppointmentService.h"
#include "../model/CommonEnums.h"

#include <QDate>
#include <QDebug>
#include <QRegularExpression>

#include "Validation.h"

void AppointmentService::normalizeAppointmentInput(AppointmentInputDTO &input) {
  input.reason = input.reason.simplified();
}

QString AppointmentService::validateAppointmentDate(const QDate &date) {
  if (!date.isValid()) {
    return "Ngày khám bắt buộc và phải là ngày hợp lệ.";
  }
  if (date < QDate::currentDate()) {
    return "Ngày hẹn khám không thể nằm trong quá khứ (phải từ ngày hôm nay trở đi).";
  }
  return "";
}

QString AppointmentService::validateStartTime(const QTime &startTime) {
  if (!startTime.isValid()) {
    return "Start time is required and must be a valid time.";
  }
  return "";
}

QString AppointmentService::validateEndTime(const QTime &endTime,
                                            const QTime &startTime) {
  if (endTime.isValid()) {
    if (startTime.isValid() && endTime <= startTime) {
      return "End time must be strictly after start time.";
    }
  }
  return "";
}

QString AppointmentService::validateAppointmentStatus(const QString &status) {
  if (status.trimmed().isEmpty()) {
    return "Appointment status is required.";
  }
  if (!AppointmentStatusText::isValid(status)) {
    return "Invalid appointment status value.";
  }
  return "";
}

QString AppointmentService::validatePatientExists(int patientId) const {
  QString err =
      Validation::validateValidId(patientId, "Please select a valid patient.");
  if (!err.isEmpty())
    return err;

  if (m_patientRepository &&
      !m_patientRepository->getPatientById(patientId).has_value()) {
    return "Patient does not exist in the system.";
  }
  return "";
}

QString AppointmentService::validateDoctorExists(int doctorId) const {
  QString err =
      Validation::validateValidId(doctorId, "Please select a valid doctor.");
  if (!err.isEmpty())
    return err;

  if (m_staffRepository && !m_staffRepository->existsByStaffId(doctorId)) {
    return "Doctor does not exist in the system.";
  }
  return "";
}

QString AppointmentService::validateDoctorConflict(int doctorId,
                                                   const QDate &date,
                                                   const QTime &startTime,
                                                   const QTime &endTime,
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
                                                    const QDate &date,
                                                    const QTime &startTime,
                                                    const QTime &endTime,
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

QString
AppointmentService::validateAppointmentInput(const AppointmentInputDTO &input,
                                             int excludeId) const {
  QString err;
  if (!(err = validatePatientExists(input.patientId)).isEmpty())
    return err;
  if (!(err = validateDoctorExists(input.doctorId)).isEmpty())
    return err;
  if (!(err = validateAppointmentDate(input.date)).isEmpty())
    return err;
  if (!(err = validateStartTime(input.startTime)).isEmpty())
    return err;
  if (!(err = validateEndTime(input.endTime, input.startTime)).isEmpty())
    return err;
  if (!(err = Validation::validateTrimmedNotEmpty(
            input.reason, "Appointment reason is required."))
           .isEmpty())
    return err;

  // Check if doctor is on leave
  if (m_staffRepository &&
      m_staffRepository->isStaffOnLeave(input.doctorId, input.date)) {
    return "Bác sĩ đang trong thời gian nghỉ phép vào ngày này.";
  }

  // Doctor conflict
  if (!(err = validateDoctorConflict(input.doctorId, input.date,
                                     input.startTime, input.endTime, excludeId))
           .isEmpty())
    return err;
  if (!(err =
            validatePatientConflict(input.patientId, input.date,
                                    input.startTime, input.endTime, excludeId))
           .isEmpty())
    return err;

  return "";
}

QList<AppointmentRecordDTO>
AppointmentService::getDoctorAppointments(int doctorId,
                                          const QDate &date) const {
  QString dateStr = date.isValid() ? date.toString("yyyy-MM-dd") : "";
  return m_appointmentRepository->getDoctorAppointments(doctorId, dateStr);
}

// QList<AppointmentRecordDTO> AppointmentService::getDoctorAppointments(
//         const QString& doctorId, const QDate& date) const {
//     QString dateStr = date.isValid() ? date.toString("yyyy-MM-dd") : "";
//     return m_appointmentRepository->getDoctorAppointments(doctorId, dateStr);
// }

QList<AppointmentRecordDTO>
AppointmentService::getAppointmentsByDate(const QDate &date) const {
  QString dateStr = date.isValid() ? date.toString("yyyy-MM-dd") : "";
  return m_appointmentRepository->getAppointmentsByDate(dateStr);
}

QList<AppointmentRecordDTO>
AppointmentService::getPatientAppointments(int patientId) const {
  return m_appointmentRepository->getPatientAppointments(patientId);
}

QString
AppointmentService::updateAppointmentStatus(int appointmentId,
                                            const QString &status) const {
  QString err =
      Validation::validateValidId(appointmentId, "Invalid appointment ID.");
  if (!err.isEmpty())
    return err;
  err = validateAppointmentStatus(status);
  if (!err.isEmpty())
    return err;

  QString normalizedStatus = AppointmentStatusText::toEn(status);
  if (!m_appointmentRepository->updateAppointmentStatus(appointmentId,
                                                        normalizedStatus)) {
    return "Failed to update appointment status in database.";
  }
  return "";
}

QPair<QString, int>
AppointmentService::checkInPatient(int appointmentId) const {
  QString err =
      Validation::validateValidId(appointmentId, "Invalid appointment ID.");
  if (!err.isEmpty())
    return qMakePair(err, -1);

  if (m_appointmentRepository &&
      !m_appointmentRepository->existsById(appointmentId)) {
    return qMakePair(QString("Appointment does not exist in the system."), -1);
  }

  return m_appointmentRepository->checkInPatient(appointmentId);
}

QString AppointmentService::cancelAppointment(int appointmentId) const {
  QString err =
      Validation::validateValidId(appointmentId, "Invalid appointment ID.");
  if (!err.isEmpty())
    return err;

  if (!m_appointmentRepository->cancelAppointment(appointmentId)) {
    return "Failed to cancel appointment in database.";
  }
  return "";
}

int AppointmentService::cancelAppointmentsForDoctor(
    int doctorId, const QDate &startDate, const QDate &endDate) const {
  if (doctorId <= 0 || !startDate.isValid() || !endDate.isValid() ||
      startDate > endDate) {
    return 0;
  }
  return m_appointmentRepository->cancelAppointmentsForDoctor(
      doctorId, startDate.toString("yyyy-MM-dd"),
      endDate.toString("yyyy-MM-dd"));
}

QString AppointmentService::createAppointment(AppointmentInputDTO input) const {
  normalizeAppointmentInput(input);
  QString err = validateAppointmentInput(input);
  if (!err.isEmpty()) {
    qWarning() << "AppointmentService::createAppointment validation failed:"
               << err;
    return err;
  }

  if (!m_appointmentRepository->createAppointment(input)) {
    return "Failed to create appointment in database.";
  }
  return "";
}

// bool AppointmentService::createAppointment(const AppointmentInputDTO& input)
// const {
//     return createAppointmentWithReason(input).isEmpty();
// }

QStringList AppointmentService::getAvailableTimeSlots(int doctorId,
                                                      const QDate &date) const {
  QStringList availableSlots;
  if (doctorId <= 0 || !date.isValid()) {
    return availableSlots;
  }

  // 0. Check if Doctor is on leave
  if (m_staffRepository && m_staffRepository->isStaffOnLeave(doctorId, date)) {
    return availableSlots; // Return empty if on leave
  }

  // 1. Get Doctor's Shift
  QString shift = "FULL_DAY"; // default
  if (m_staffRepository) {
    auto profile = m_staffRepository->findProfileById(doctorId);
    if (profile) {
      shift = profile->shift;
    }
  }

  // 2. Generate Base Slots based on shift
  QList<QPair<QTime, QTime>> baseSlots;
  auto addSlots = [&baseSlots](int startHour, int endHour) {
    for (int h = startHour; h < endHour; ++h) {
      baseSlots.append(qMakePair(QTime(h, 0), QTime(h, 30)));
      baseSlots.append(qMakePair(QTime(h, 30), QTime(h + 1, 0)));
    }
  };

  if (shift == "MORNING" || shift == "FULL_DAY") {
    addSlots(8, 12); // 08:00 - 12:00
  }
  if (shift == "AFTERNOON" || shift == "FULL_DAY") {
    addSlots(13, 17); // 13:00 - 17:00
  }
  if (shift == "NIGHT") {
    addSlots(18, 22); // 18:00 - 22:00
  }

  // 3. Fetch Booked Appointments
  QList<AppointmentRecordDTO> bookedAppts =
      getDoctorAppointments(doctorId, date);

  // 4. Filter out overlapping slots
  for (const auto &slot : baseSlots) {
    bool isBooked = false;
    QTime slotStart = slot.first;
    QTime slotEnd = slot.second;

    // Skip slots in the past if it's today
    if (date == QDate::currentDate() && slotStart <= QTime::currentTime()) {
      continue;
    }

    for (const auto &appt : bookedAppts) {
      if (appt.status == AppointmentStatusText::CANCELLED)
        continue;

      QTime apptStart = QTime::fromString(appt.startTime, "HH:mm");
      if (!apptStart.isValid())
        apptStart = QTime::fromString(appt.startTime, "HH:mm:ss");

      QTime apptEnd = QTime::fromString(appt.endTime, "HH:mm");
      if (!apptEnd.isValid())
        apptEnd = QTime::fromString(appt.endTime, "HH:mm:ss");

      if (!apptStart.isValid() || !apptEnd.isValid())
        continue;

      // Overlap condition:
      if (slotStart < apptEnd && slotEnd > apptStart) {
        isBooked = true;
        break;
      }
    }

    if (!isBooked) {
      QString slotStr =
          slotStart.toString("HH:mm") + " - " + slotEnd.toString("HH:mm");
      availableSlots.append(slotStr);
    }
  }

  return availableSlots;
}

QList<RoomQueueStatusDTO>
AppointmentService::getRoomQueueStatuses(const QDate &date) const {
  if (!date.isValid())
    return {};
  return m_appointmentRepository->getRoomQueueStatuses(
      date.toString("yyyy-MM-dd"));
}

QList<RoomQueueItemDTO>
AppointmentService::getDoctorQueue(int doctorId, const QDate &date) const {
  if (doctorId <= 0 || !date.isValid())
    return {};
  return m_appointmentRepository->getDoctorQueue(doctorId,
                                                 date.toString("yyyy-MM-dd"));
}


QPair<int, QString>
AppointmentService::callSpecificPatient(int appointmentId) const {
  return m_appointmentRepository->callSpecificPatient(appointmentId);
}
