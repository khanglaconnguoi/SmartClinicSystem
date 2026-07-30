#include "AppointmentRepository.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariantList>

void AppointmentRepository::autoMarkNoShowAppointments() const {
  QString sql = R"(
        UPDATE appointments
        SET status = 'NO_SHOW',
            updated_at = datetime('now')
        WHERE status = 'SCHEDULED' AND appointment_date < date('now', 'localtime')
    )";
  DatabaseManager::getInstance().executeQuery(sql);
}

QList<AppointmentRecordDTO> AppointmentRepository::getDoctorAppointments(int doctorId, const QString &date) const {
  autoMarkNoShowAppointments();
  QList<AppointmentRecordDTO> list;

  QString sql = R"(
        SELECT a.appointment_id, a.patient_id, a.doctor_id, a.appointment_date, a.start_time, a.end_time, a.status, a.reason, a.notes, p.full_name, p.patient_code, r.room_number, a.ticket_number, a.checked_in_at, a.started_at, a.completed_at
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
  sql += R"(
    ORDER BY 
        CASE a.status 
            WHEN 'STARTED' THEN 1 
            WHEN 'CHECKED_IN' THEN 2 
            WHEN 'SCHEDULED' THEN 3 
            ELSE 4 
        END ASC,
        a.appointment_date ASC, 
        a.start_time ASC
  )";

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
    rec.checkedInAt = query.value(13).toString();
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

QList<AppointmentRecordDTO> AppointmentRepository::getAppointmentsByDate(const QString &date) const {
  autoMarkNoShowAppointments();
  QList<AppointmentRecordDTO> list;

  QString sql = R"(
        SELECT a.appointment_id, a.patient_id, a.doctor_id, a.appointment_date, a.start_time, a.end_time, a.status, a.reason, a.notes, p.full_name, p.patient_code, r.room_number, a.ticket_number, a.checked_in_at, a.started_at, a.completed_at, s.full_name, dp.specialty
        FROM appointments a
        JOIN patients p ON a.patient_id = p.patient_id
        LEFT JOIN rooms r ON a.room_id = r.room_id
        LEFT JOIN staff s ON a.doctor_id = s.staff_id
        LEFT JOIN doctor_profiles dp ON s.staff_id = dp.staff_id
        WHERE a.appointment_date = ?
        ORDER BY a.start_time ASC
    )";
  QVariantList params = {date};

  qDebug() << "getAppointmentsByDate - Executing query for date:" << date;
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
    rec.patientName = query.value(9).toString();
    rec.patientCode = query.value(10).toString();
    rec.roomNumber = query.value(11).toString();
    if (rec.roomNumber.isEmpty())
      rec.roomNumber = "N/A";
    rec.ticketNumber = query.value(12).toInt();
    rec.checkedInAt = query.value(13).toString();
    rec.startedAt = query.value(14).toString();
    rec.completedAt = query.value(15).toString();
    rec.doctorName = query.value(16).toString();
    rec.doctorSpecialty = query.value(17).toString();
    list.append(rec);
  }

  return list;
}

QList<AppointmentRecordDTO> AppointmentRepository::getPatientAppointments(int patientId) const {
  autoMarkNoShowAppointments();
  QList<AppointmentRecordDTO> list;

  QString sql = R"(
        SELECT a.appointment_id, a.patient_id, a.doctor_id, a.appointment_date, a.start_time, a.end_time, a.status, a.reason, a.notes, s.full_name, dp.specialty, r.room_number, a.ticket_number, a.checked_in_at, a.started_at, a.completed_at
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
    rec.checkedInAt = query.value(13).toString();
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
            checked_in_at = CASE WHEN ? = 'CHECKED_IN' AND checked_in_at IS NULL THEN datetime('now') ELSE checked_in_at END,
            started_at = CASE WHEN ? = 'STARTED' AND started_at IS NULL THEN datetime('now') ELSE started_at END,
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

  QString todayStr = QDate::currentDate().toString("yyyy-MM-dd");
  if (appointmentDate != todayStr) {
    DatabaseManager::getInstance().rollbackTransaction();
    if (appointmentDate < todayStr) {
      return qMakePair(QString("Không thể check-in lịch hẹn trong quá khứ."), -1);
    } else {
      return qMakePair(QString("Chưa đến ngày hẹn, không thể check-in."), -1);
    }
  }

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

  // 3. Update appointment status to CHECKED_IN, set ticket_number and checked_in_at
  QString updateSql = R"(
        UPDATE appointments
        SET status = 'CHECKED_IN',
            ticket_number = ?,
            checked_in_at = CASE WHEN checked_in_at IS NULL THEN datetime('now') ELSE checked_in_at END,
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
  QString dateStr = input.date.isValid() ? input.date.toString("yyyy-MM-dd") : QDate::currentDate().toString("yyyy-MM-dd");
  bool isToday = (dateStr == QDate::currentDate().toString("yyyy-MM-dd"));
  int ticketNum = input.ticketNumber;
  QString initialStatus = "SCHEDULED";
  
  if (ticketNum <= 0 && isToday) {
      // Calculate next ticket number for today
      QString getNextTicketSql = R"(
          SELECT COALESCE(MAX(ticket_number), 0) + 1 AS next_ticket
          FROM appointments
          WHERE doctor_id = ? AND appointment_date = ? AND status != 'CANCELLED'
      )";
      QSqlQuery nextQuery = DatabaseManager::getInstance().selectQuery(getNextTicketSql, {input.doctorId, dateStr});
      if (nextQuery.next()) {
          ticketNum = nextQuery.value("next_ticket").toInt();
      } else {
          ticketNum = 1;
      }
  } else if (ticketNum <= 0) {
      ticketNum = 1; // Default for future dates
  }

  QString sql = R"(
        INSERT INTO appointments (ticket_number, patient_id, doctor_id, room_id, created_by, appointment_date, start_time, end_time, status, reason)
        VALUES (?, ?, ?, (SELECT room_id FROM doctor_profiles WHERE staff_id = ?), ?, ?, ?, ?, ?, ?)
    )";
    
  QVariant createdByVar = (input.createdBy > 0) ? QVariant(input.createdBy) : QVariant();
  QString startStr = input.startTime.isValid() ? input.startTime.toString("HH:mm") : "";
  QVariant endVar = input.endTime.isValid() ? QVariant(input.endTime.toString("HH:mm")) : QVariant();

  QSqlQuery query = DatabaseManager::getInstance().executeQuery(
      sql, {ticketNum, input.patientId, input.doctorId, input.doctorId, createdByVar, dateStr, startStr, endVar, initialStatus, input.reason});

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

int AppointmentRepository::cancelAppointmentsForDoctor(int doctorId, const QString& startDate, const QString& endDate) const {
    QString sql = R"(
        UPDATE appointments 
        SET status = 'CANCELLED', updated_at = datetime('now') 
        WHERE doctor_id = ? 
          AND appointment_date >= ? AND appointment_date <= ?
          AND status NOT IN ('CANCELLED', 'COMPLETED')
    )";
    QSqlQuery query = DatabaseManager::getInstance().executeQuery(sql, {doctorId, startDate, endDate});
    
    if (query.lastError().isValid()) {
        qWarning() << "Failed to cancel appointments for doctor:" << query.lastError().text();
        return 0;
    }
    return query.numRowsAffected();
}

bool AppointmentRepository::existsById(int appointmentId) const {
  QSqlQuery query = DatabaseManager::getInstance().selectQuery(
      "SELECT 1 FROM appointments WHERE appointment_id = ?", {appointmentId});
  return query.next();
}

bool AppointmentRepository::hasDoctorScheduleConflict(int doctorId, const QString &date, const QString &startTime, const QString &endTime, int excludeAppointmentId) const {
  // First, check if doctor is on leave
  QString leaveSql = "SELECT 1 FROM leave_requests WHERE staff_id = ? AND status = 'APPROVED' AND start_date <= ? AND end_date >= ?";
  QSqlQuery leaveQuery = DatabaseManager::getInstance().selectQuery(leaveSql, {doctorId, date, date});
  if (leaveQuery.next()) {
      return true; // Conflict because doctor is on leave
  }

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

QList<RoomQueueStatusDTO> AppointmentRepository::getRoomQueueStatuses(const QString& date) const {
    QList<RoomQueueStatusDTO> statuses;
    DatabaseManager& db = DatabaseManager::getInstance();
    
    QString sql = R"(
        SELECT r.room_id, r.room_number, d.staff_id AS doctor_id, s.full_name AS doctor_name
        FROM rooms r
        LEFT JOIN doctor_profiles d ON r.room_id = d.room_id
        LEFT JOIN staff s ON d.staff_id = s.staff_id
        WHERE r.room_type = 'EXAM'
        ORDER BY r.room_id ASC
    )";

    QSqlQuery query = db.selectQuery(sql);
    while (query.next()) {
        RoomQueueStatusDTO dto;
        dto.roomId = query.value("room_id").toInt();
        dto.roomNumber = query.value("room_number").toString();
        dto.doctorId = query.value("doctor_id").toInt();
        dto.doctorName = query.value("doctor_name").toString();

        if (dto.doctorId > 0) {
            // Get currently STARTED ticket
            QString currentSql = R"(
                SELECT ticket_number FROM appointments
                WHERE doctor_id = ? AND appointment_date = ? AND status = 'STARTED'
                ORDER BY ticket_number ASC LIMIT 1
            )";
            QSqlQuery currentQ = db.selectQuery(currentSql, {dto.doctorId, date});
            if (currentQ.next()) {
                dto.currentTicketNumber = currentQ.value("ticket_number").toInt();
            }

            // Get next CHECKED_IN ticket
            QString nextSql = R"(
                SELECT ticket_number FROM appointments
                WHERE doctor_id = ? AND appointment_date = ? AND status = 'CHECKED_IN'
                ORDER BY ticket_number ASC LIMIT 1
            )";
            QSqlQuery nextQ = db.selectQuery(nextSql, {dto.doctorId, date});
            if (nextQ.next()) {
                dto.nextTicketNumber = nextQ.value("ticket_number").toInt();
            }
        }
        statuses.append(dto);
    }
    return statuses;
}

QList<RoomQueueItemDTO> AppointmentRepository::getDoctorQueue(int doctorId, const QString& date) const {
    QList<RoomQueueItemDTO> queue;
    if (doctorId <= 0) return queue;

    QString sql = R"(
        SELECT a.ticket_number, p.full_name, a.start_time, a.status 
        FROM appointments a
        JOIN patients p ON a.patient_id = p.patient_id
        WHERE a.doctor_id = ? AND a.appointment_date = ? 
          AND a.status IN ('CHECKED_IN', 'STARTED', 'COMPLETED')
        ORDER BY a.ticket_number ASC
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {doctorId, date});
    while (query.next()) {
        RoomQueueItemDTO dto;
        dto.ticketNumber = query.value("ticket_number").toInt();
        dto.patientName = query.value("full_name").toString();
        dto.startTime = query.value("start_time").toString();
        
        QString status = query.value("status").toString();
        if (status == "STARTED") dto.status = "Đang khám";
        else if (status == "CHECKED_IN") dto.status = "Đang chờ";
        else if (status == "COMPLETED") dto.status = "Đã khám xong";
        else dto.status = status;
        
        queue.append(dto);
    }
    return queue;
}

QPair<int, QString> AppointmentRepository::callSpecificPatient(int appointmentId) const {
    DatabaseManager& db = DatabaseManager::getInstance();
    
    if (!db.beginTransaction()) {
        return {0, ""};
    }

    // 1. Lấy thông tin lịch khám hiện tại (doctorId, date, ticket_number, tên)
    QString getInfoSql = R"(
        SELECT a.doctor_id, a.appointment_date, a.ticket_number, p.full_name 
        FROM appointments a
        JOIN patients p ON a.patient_id = p.patient_id
        WHERE a.appointment_id = ?
    )";
    QSqlQuery infoQuery = db.selectQuery(getInfoSql, {appointmentId});
    if (!infoQuery.next()) {
        db.rollbackTransaction();
        return {0, ""};
    }

    int doctorId = infoQuery.value("doctor_id").toInt();
    QString date = infoQuery.value("appointment_date").toString();
    int nextTicket = infoQuery.value("ticket_number").toInt();
    QString patientName = infoQuery.value("full_name").toString();

    // 2. Mark current STARTED as COMPLETED cho bác sĩ này trong ngày hôm nay
    QString completeSql = R"(
        UPDATE appointments 
        SET status = 'COMPLETED', updated_at = CURRENT_TIMESTAMP
        WHERE doctor_id = ? AND appointment_date = ? AND status = 'STARTED'
    )";
    QSqlQuery completeQuery = db.executeQuery(completeSql, {doctorId, date});
    if (completeQuery.lastError().isValid()) {
        db.rollbackTransaction();
        return {0, ""};
    }

    // 3. Mark lịch hẹn được chọn là STARTED
    QString startSql = R"(
        UPDATE appointments 
        SET status = 'STARTED', updated_at = CURRENT_TIMESTAMP
        WHERE appointment_id = ?
    )";
    QSqlQuery startQuery = db.executeQuery(startSql, {appointmentId});
    if (startQuery.lastError().isValid()) {
        db.rollbackTransaction();
        return {0, ""};
    }

    if (!db.commitTransaction()) {
        return {0, ""};
    }

    return {nextTicket, patientName};
}

QList<QPair<int, QString>> AppointmentRepository::getExaminationRooms() const {
    QList<QPair<int, QString>> rooms;
    DatabaseManager& db = DatabaseManager::getInstance();
    QString sql =
            "SELECT room_id, room_number FROM rooms WHERE room_type = 'EXAM' ORDER BY room_id ASC";
    QSqlQuery query = db.selectQuery(sql);
    while (query.next()) {
        rooms.append({query.value("room_id").toInt(), query.value("room_number").toString()});
    }
    return rooms;
}
