#include "AppointmentRepository.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariantList>

QList<AppointmentRecordDTO> AppointmentRepository::getDoctorAppointments(int doctorId, const QString &date) const {
  QList<AppointmentRecordDTO> list;

  QString sql = R"(
        SELECT a.appointment_id, a.patient_id, a.doctor_id, a.appointment_date, a.start_time, a.end_time, a.status, a.reason, a.notes, p.full_name, p.patient_code, r.room_number, a.ticket_number, a.called_at, a.started_at, a.completed_at
        FROM appointments a
        JOIN patients p ON a.patient_id = p.patient_id
        LEFT JOIN rooms r ON a.room_id = r.room_id
        WHERE a.doctor_id = ?
    )";
  QVariantList params = {doctorId};
  if (!date.isEmpty()) {
    sql += " AND a.appointment_date = ?";
    params << date;
  }
  sql += " ORDER BY a.appointment_date DESC, a.start_time ASC";

  qDebug() << "getDoctorAppointments - Executing query for doctorId:"
           << doctorId << "| Date:" << (date.isEmpty() ? "All" : date);
  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);

  int rowCount = 0;
  while (query.next()) {
    rowCount++;
    AppointmentRecordDTO rec;
    rec.appointmentId = query.value(0).toInt();
    rec.patientId = query.value(1).toInt();
    rec.doctorId = query.value(2).toString();
    rec.appointmentDate = query.value(3).toString();
    rec.startTime = query.value(4).toString();
    rec.endTime = query.value(5).toString();
    rec.status = query.value(6).toString();
    rec.reason = query.value(7).toString();
    rec.notes = query.value(8).toString();
    rec.patientName = query.value(9).toString();
    rec.patientCode = query.value(10).toString();
    rec.roomNumber = query.value(11).toString();
    if (rec.roomNumber.isEmpty())
      rec.roomNumber = "N/A";
    rec.ticketNumber = query.value(12).toInt();
    rec.calledAt = query.value(13).toString();
    rec.startedAt = query.value(14).toString();
    rec.completedAt = query.value(15).toString();

    qDebug() << "  -> Row" << rowCount << ":" << rec.startTime << "|"
             << rec.patientName << "|" << rec.status;
    list.append(rec);
  }
  qDebug() << "getDoctorAppointments - Loaded" << rowCount << "records.";
  return list;
}

// QList<AppointmentRecordDTO> AppointmentRepository::getDoctorAppointments(const QString &doctorId, const QString &date) const {
//   bool ok = false;
//   int id = doctorId.toInt(&ok);
//   if (ok && id > 0) {
//     return getDoctorAppointments(id, date);
//   }

//   // Fallback query if doctorId string parameter is a staff_code
//   QList<AppointmentRecordDTO> list;
//   QString sql = R"(
//         SELECT a.appointment_id, a.patient_id, a.doctor_id, a.appointment_date, a.start_time, a.end_time, a.status, a.reason, a.notes, p.full_name, p.patient_code, r.room_number, a.ticket_number, a.called_at, a.started_at, a.completed_at
//         FROM appointments a
//         JOIN patients p ON a.patient_id = p.patient_id
//         LEFT JOIN rooms r ON a.room_id = r.room_id
//         JOIN staff s ON a.doctor_id = s.staff_id
//         WHERE s.staff_code = ?
//     )";
//   QVariantList params = {doctorId};
//   if (!date.isEmpty()) {
//     sql += " AND a.appointment_date = ?";
//     params << date;
//   }
//   sql += " ORDER BY a.appointment_date DESC, a.start_time ASC";

//   QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
//   while (query.next()) {
//     AppointmentRecordDTO rec;
//     rec.appointmentId = query.value(0).toInt();
//     rec.patientId = query.value(1).toInt();
//     rec.doctorId = query.value(2).toString();
//     rec.appointmentDate = query.value(3).toString();
//     rec.startTime = query.value(4).toString();
//     rec.endTime = query.value(5).toString();
//     rec.status = query.value(6).toString();
//     rec.reason = query.value(7).toString();
//     rec.notes = query.value(8).toString();
//     rec.patientName = query.value(9).toString();
//     rec.patientCode = query.value(10).toString();
//     rec.roomNumber = query.value(11).toString();
//     if (rec.roomNumber.isEmpty())
//       rec.roomNumber = "N/A";
//     rec.ticketNumber = query.value(12).toInt();
//     rec.calledAt = query.value(13).toString();
//     rec.startedAt = query.value(14).toString();
//     rec.completedAt = query.value(15).toString();
//     list.append(rec);
//   }
//   return list;
// }

QList<AppointmentRecordDTO> AppointmentRepository::getPatientAppointments(int patientId) const {
  QList<AppointmentRecordDTO> list;

  QString sql = R"(
        SELECT a.appointment_id, a.patient_id, a.doctor_id, a.appointment_date, a.start_time, a.end_time, a.status, a.reason, a.notes, s.full_name, dp.specialty, r.room_number, a.ticket_number, a.called_at, a.started_at, a.completed_at
        FROM appointments a
        JOIN staff s ON a.doctor_id = s.staff_id
        LEFT JOIN doctor_profiles dp ON s.staff_id = dp.staff_id
        LEFT JOIN rooms r ON a.room_id = r.room_id
        WHERE a.patient_id = ?
        ORDER BY a.appointment_date DESC, a.start_time ASC
    )";
  QVariantList params = {patientId};

  qDebug() << "getPatientAppointments - Executing query for patientId:" << patientId;
  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);

  while (query.next()) {
    AppointmentRecordDTO rec;
    rec.appointmentId = query.value(0).toInt();
    rec.patientId = query.value(1).toInt();
    rec.doctorId = query.value(2).toString();
    rec.appointmentDate = query.value(3).toString();
    rec.startTime = query.value(4).toString();
    rec.endTime = query.value(5).toString();
    rec.status = query.value(6).toString();
    rec.reason = query.value(7).toString();
    rec.notes = query.value(8).toString();
    rec.doctorName = query.value(9).toString();
    rec.doctorSpecialty = query.value(10).toString();
    rec.roomNumber = query.value(11).toString();
    if (rec.roomNumber.isEmpty())
      rec.roomNumber = "N/A";
    rec.ticketNumber = query.value(12).toInt();
    rec.calledAt = query.value(13).toString();
    rec.startedAt = query.value(14).toString();
    rec.completedAt = query.value(15).toString();
    list.append(rec);
  }

  return list;
}

bool AppointmentRepository::updateAppointmentStatus(int appointmentId, const QString &status) const {
  QString sql = R"(
        UPDATE appointments
        SET status = ?,
            called_at = CASE WHEN ? = 'CALLED' AND called_at IS NULL THEN datetime('now') ELSE called_at END,
            started_at = CASE WHEN ? IN ('CHECKED_IN', 'IN_PROGRESS') AND started_at IS NULL THEN datetime('now') ELSE started_at END,
            completed_at = CASE WHEN ? = 'COMPLETED' AND completed_at IS NULL THEN datetime('now') ELSE completed_at END,
            updated_at = datetime('now')
        WHERE appointment_id = ?
    )";
  QSqlQuery query = DatabaseManager::getInstance().executeQuery(
      sql, {status, status, status, status, appointmentId});
  return !query.lastError().isValid();
}

QPair<QString, int> AppointmentRepository::checkInPatient(int appointmentId) const {
  if (!DatabaseManager::getInstance().beginTransaction()) {
    return qMakePair(QString("Failed to begin database transaction."), -1);
  }

  // 1. Fetch doctor_id and appointment_date for the appointment
  QString findSql = "SELECT doctor_id, appointment_date FROM appointments WHERE appointment_id = ?";
  QSqlQuery findQuery = DatabaseManager::getInstance().selectQuery(findSql, {appointmentId});
  if (!findQuery.next()) {
    DatabaseManager::getInstance().rollbackTransaction();
    return qMakePair(QString("Appointment with ID %1 was not found.").arg(appointmentId), -1);
  }

  int doctorId = findQuery.value(0).toInt();
  QString appointmentDate = findQuery.value(1).toString();

  // 2. Compute MAX(ticket_number) + 1 for this doctor on this date
  QString ticketSql = R"(
        SELECT COALESCE(MAX(ticket_number), 0) + 1
        FROM appointments
        WHERE doctor_id = ? AND appointment_date = ? AND status != 'CANCELLED'
    )";
  QSqlQuery ticketQuery = DatabaseManager::getInstance().selectQuery(ticketSql, {doctorId, appointmentDate});
  int nextTicket = 1;
  if (ticketQuery.next()) {
    nextTicket = ticketQuery.value(0).toInt();
  }

  // 3. Update appointment status to CHECKED_IN and set ticket_number
  QString updateSql = R"(
        UPDATE appointments
        SET status = 'CHECKED_IN',
            ticket_number = ?,
            updated_at = datetime('now')
        WHERE appointment_id = ?
    )";
  QSqlQuery updateQuery = DatabaseManager::getInstance().executeQuery(updateSql, {nextTicket, appointmentId});

  if (updateQuery.lastError().isValid()) {
    DatabaseManager::getInstance().rollbackTransaction();
    return qMakePair(QString("Failed to update check-in status in database: %1").arg(updateQuery.lastError().text()), -1);
  }

  if (!DatabaseManager::getInstance().commitTransaction()) {
    return qMakePair(QString("Failed to commit check-in transaction."), -1);
  }

  return qMakePair(QString(""), nextTicket);
}

bool AppointmentRepository::createAppointment(const AppointmentInputDTO &input) const {
  QString sql = R"(
        INSERT INTO appointments (ticket_number, patient_id, doctor_id, created_by, appointment_date, start_time, end_time, status, reason)
        VALUES (?, ?, ?, ?, ?, ?, ?, 'SCHEDULED', ?)
    )";
  int ticketNum = (input.ticketNumber > 0) ? input.ticketNumber : 1;
  QVariant createdByVar = (input.createdBy > 0) ? QVariant(input.createdBy) : QVariant();

  QString dateStr = input.date.isValid() ? input.date.toString("yyyy-MM-dd") : QDate::currentDate().toString("yyyy-MM-dd");
  QString startStr = input.startTime.isValid() ? input.startTime.toString("HH:mm") : "";
  QVariant endVar = input.endTime.isValid() ? QVariant(input.endTime.toString("HH:mm")) : QVariant();

  QSqlQuery query = DatabaseManager::getInstance().executeQuery(
      sql, {ticketNum, input.patientId, input.doctorId, createdByVar, dateStr, startStr, endVar, input.reason});

  if (!query.isActive()) {
    qWarning() << "AppointmentRepository::createAppointment - Lỗi:"
               << query.lastError().text();
    return false;
  }
  return true;
}

bool AppointmentRepository::cancelAppointment(int appointmentId) const {
  QSqlQuery query = DatabaseManager::getInstance().executeQuery(
      "UPDATE appointments SET status = 'CANCELLED', updated_at = datetime('now') WHERE appointment_id = ?",
      {appointmentId});
  return !query.lastError().isValid();
}

bool AppointmentRepository::existsById(int appointmentId) const {
  QSqlQuery query = DatabaseManager::getInstance().selectQuery(
      "SELECT 1 FROM appointments WHERE appointment_id = ?", {appointmentId});
  return query.next();
}

bool AppointmentRepository::hasDoctorScheduleConflict(int doctorId, const QString &date, const QString &startTime, const QString &endTime, int excludeAppointmentId) const {
  QString sql;
  QVariantList params;
  if (!endTime.isEmpty()) {
    sql = R"(
      SELECT 1 FROM appointments
      WHERE doctor_id = ? AND appointment_date = ? AND status NOT IN ('CANCELLED', 'NO_SHOW')
        AND (
          (end_time IS NOT NULL AND start_time < ? AND end_time > ?)
          OR (end_time IS NULL AND start_time = ?)
        )
    )";
    params = {doctorId, date, endTime, startTime, startTime};
  } else {
    sql = "SELECT 1 FROM appointments WHERE doctor_id = ? AND appointment_date = ? AND start_time = ? AND status NOT IN ('CANCELLED', 'NO_SHOW')";
    params = {doctorId, date, startTime};
  }
  if (excludeAppointmentId > 0) {
    sql += " AND appointment_id != ?";
    params << excludeAppointmentId;
  }
  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  return query.next();
}

bool AppointmentRepository::hasPatientScheduleConflict(int patientId, const QString &date, const QString &startTime, const QString &endTime, int excludeAppointmentId) const {
  QString sql;
  QVariantList params;
  if (!endTime.isEmpty()) {
    sql = R"(
      SELECT 1 FROM appointments
      WHERE patient_id = ? AND appointment_date = ? AND status NOT IN ('CANCELLED', 'NO_SHOW')
        AND (
          (end_time IS NOT NULL AND start_time < ? AND end_time > ?)
          OR (end_time IS NULL AND start_time = ?)
        )
    )";
    params = {patientId, date, endTime, startTime, startTime};
  } else {
    sql = "SELECT 1 FROM appointments WHERE patient_id = ? AND appointment_date = ? AND start_time = ? AND status NOT IN ('CANCELLED', 'NO_SHOW')";
    params = {patientId, date, startTime};
  }
  if (excludeAppointmentId > 0) {
    sql += " AND appointment_id != ?";
    params << excludeAppointmentId;
  }
  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  return query.next();
}
