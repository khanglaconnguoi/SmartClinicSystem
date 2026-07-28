#include "AppointmentRepository.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariantList>

QList<AppointmentRecordDTO> AppointmentRepository::getDoctorAppointments(const QString &doctorId, const QString &date) const {
  QList<AppointmentRecordDTO> list;

  QString sql = R"(
        SELECT a.appointment_id, a.patient_id, a.doctor_id, a.appointment_date, a.start_time, a.end_time, a.status, a.reason, a.notes, p.full_name, p.patient_code, r.room_number
        FROM appointments a
        JOIN patients p ON a.patient_id = p.patient_id
        LEFT JOIN rooms r ON a.room_id = r.room_id
        JOIN staff s ON (a.doctor_id = s.staff_id OR a.doctor_id = s.staff_code)
        WHERE s.staff_id = ?
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

    qDebug() << "  -> Row" << rowCount << ":" << rec.startTime << "|"
             << rec.patientName << "|" << rec.status;
    list.append(rec);
  }
  qDebug() << "getDoctorAppointments - Loaded" << rowCount << "records.";
  return list;
}

QList<AppointmentRecordDTO> AppointmentRepository::getPatientAppointments(int patientId) const {
  QList<AppointmentRecordDTO> list;

  QString sql = R"(
        SELECT a.appointment_id, a.patient_id, a.doctor_id, a.appointment_date, a.start_time, a.end_time, a.status, a.reason, a.notes, s.full_name, dp.specialty, r.room_number
        FROM appointments a
        JOIN staff s ON (a.doctor_id = s.staff_id OR a.doctor_id = s.staff_code)
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
    list.append(rec);
  }

  return list;
}

bool AppointmentRepository::updateAppointmentStatus(int appointmentId, const QString &status) const {
  QSqlQuery query = DatabaseManager::getInstance().executeQuery(
      "UPDATE appointments SET status = ? WHERE appointment_id = ?",
      {status, appointmentId});
  return !query.lastError().isValid();
}

bool AppointmentRepository::createAppointment(const AppointmentInputDTO &input) const {
  QString sql = R"(
        INSERT INTO appointments (patient_id, doctor_id, created_by, appointment_date, start_time, status, reason)
        VALUES (?, ?, ?, ?, ?, 'SCHEDULED', ?)
    )";
  QSqlQuery query = DatabaseManager::getInstance().executeQuery(
      sql, {input.patientId, input.doctorCode, input.createdBy, input.date, input.startTime, input.reason});

  if (!query.isActive()) {
    qWarning() << "PatientRepository::createAppointment - Lỗi:"
               << query.lastError().text();
    return false;
  }
  return true;
}

bool AppointmentRepository::cancelAppointment(int appointmentId) const {
  QSqlQuery query = DatabaseManager::getInstance().executeQuery(
      "UPDATE appointments SET status = 'CANCELLED' WHERE appointment_id = ?",
      {appointmentId});
  return !query.lastError().isValid();
}
