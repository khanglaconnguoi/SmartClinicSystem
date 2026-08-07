#include "ServiceRequestRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDate>

int ServiceRequestRepository::createRequest(const ServiceRequestInputDTO& input) const {
    DatabaseManager& db = DatabaseManager::getInstance();

    QString sql = R"(
        INSERT INTO service_requests (
            record_id, room_id, doctor_id, service_name, status, prescribed_at
        ) VALUES (?, ?, ?, ?, 'PENDING', datetime('now', 'localtime'))
    )";

    QVariantList params = {
        input.recordId,
        input.roomId,
        input.doctorId,
        input.serviceName
    };

    QSqlQuery query(db.database());
    if (!query.prepare(sql)) {
        qWarning() << "ServiceRequestRepository::createRequest - prepare query thất bại:"
                   << query.lastError().text();
        return -1;
    }
    for (const QVariant &param : params) {
        query.addBindValue(param);
    }
    if (!query.exec()) {
        qWarning() << "ServiceRequestRepository::createRequest error:" << query.lastError().text();
        return -1;
    }

    int requestId = query.lastInsertId().toInt();
    if (requestId <= 0) {
        qWarning() << "ServiceRequestRepository::createRequest - lastInsertId() trả về" << requestId;
        return -1;
    }
    return requestId;
}

QList<ServiceRequestDTO> ServiceRequestRepository::getRequestsByRoom(
    int roomId, const QString& date, std::optional<ServiceRequestStatus> status) const {

    DatabaseManager& db = DatabaseManager::getInstance();
    QList<ServiceRequestDTO> list;

    QString filterDate = date.isEmpty() ? QDate::currentDate().toString("yyyy-MM-dd") : date;

    QString sql = R"(
        SELECT 
            sr.request_id, sr.record_id, sr.room_id, r.room_number,
            sr.doctor_id, doc.full_name AS doctor_name,
            mr.patient_id, p.full_name AS patient_name, p.patient_code,
            sr.service_name, sr.status, sr.ticket_number,
            sr.prescribed_at, sr.started_at, sr.completed_at, sr.result_note
        FROM service_requests sr
        JOIN rooms r ON sr.room_id = r.room_id
        JOIN staff doc ON sr.doctor_id = doc.staff_id
        JOIN medical_records mr ON sr.record_id = mr.record_id
        JOIN patients p ON mr.patient_id = p.patient_id
        WHERE 1=1
    )";

    QVariantList params;

    if (roomId > 0) {
        sql += " AND sr.room_id = ?";
        params.append(roomId);
    }

    if (!filterDate.isEmpty()) {
        sql += " AND DATE(sr.prescribed_at) = ?";
        params.append(filterDate);
    }

    if (status.has_value()) {
        sql += " AND sr.status = ?";
        params.append(serviceRequestStatusToEn(status.value()));
    }

    sql += " ORDER BY sr.status ASC, sr.ticket_number ASC, sr.request_id ASC";

    QSqlQuery query = db.selectQuery(sql, params);
    while (query.next()) {
        ServiceRequestDTO dto;
        dto.requestId    = query.value("request_id").toInt();
        dto.recordId     = query.value("record_id").toInt();
        dto.roomId       = query.value("room_id").toInt();
        dto.roomName     = query.value("room_number").toString();
        dto.doctorId     = query.value("doctor_id").toInt();
        dto.doctorName   = query.value("doctor_name").toString();
        dto.patientId    = query.value("patient_id").toInt();
        dto.patientName  = query.value("patient_name").toString();
        dto.patientCode  = query.value("patient_code").toString();
        dto.serviceName  = query.value("service_name").toString();
        dto.status       = serviceRequestStatusFromEn(query.value("status").toString());

        if (!query.value("ticket_number").isNull()) {
            dto.ticketNumber = query.value("ticket_number").toInt();
        }

        dto.prescribedAt = QDateTime::fromString(query.value("prescribed_at").toString(), Qt::ISODate);
        if (!query.value("started_at").isNull()) {
            dto.startedAt = QDateTime::fromString(query.value("started_at").toString(), Qt::ISODate);
        }
        if (!query.value("completed_at").isNull()) {
            dto.completedAt = QDateTime::fromString(query.value("completed_at").toString(), Qt::ISODate);
        }

        dto.resultNote   = query.value("result_note").toString();

        list.append(dto);
    }

    return list;
}

std::optional<ServiceRequestDTO> ServiceRequestRepository::getById(int requestId) const {
    DatabaseManager& db = DatabaseManager::getInstance();

    QString sql = R"(
        SELECT 
            sr.request_id, sr.record_id, sr.room_id, r.room_number,
            sr.doctor_id, doc.full_name AS doctor_name,
            mr.patient_id, p.full_name AS patient_name, p.patient_code,
            sr.service_name, sr.status, sr.ticket_number,
            sr.prescribed_at, sr.started_at, sr.completed_at, sr.result_note
        FROM service_requests sr
        JOIN rooms r ON sr.room_id = r.room_id
        JOIN staff doc ON sr.doctor_id = doc.staff_id
        JOIN medical_records mr ON sr.record_id = mr.record_id
        JOIN patients p ON mr.patient_id = p.patient_id
        WHERE sr.request_id = ?
    )";

    QSqlQuery query = db.selectQuery(sql, {requestId});
    if (query.next()) {
        ServiceRequestDTO dto;
        dto.requestId    = query.value("request_id").toInt();
        dto.recordId     = query.value("record_id").toInt();
        dto.roomId       = query.value("room_id").toInt();
        dto.roomName     = query.value("room_number").toString();
        dto.doctorId     = query.value("doctor_id").toInt();
        dto.doctorName   = query.value("doctor_name").toString();
        dto.patientId    = query.value("patient_id").toInt();
        dto.patientName  = query.value("patient_name").toString();
        dto.patientCode  = query.value("patient_code").toString();
        dto.serviceName  = query.value("service_name").toString();
        dto.status       = serviceRequestStatusFromEn(query.value("status").toString());

        if (!query.value("ticket_number").isNull()) {
            dto.ticketNumber = query.value("ticket_number").toInt();
        }

        dto.prescribedAt = QDateTime::fromString(query.value("prescribed_at").toString(), Qt::ISODate);
        if (!query.value("started_at").isNull()) {
            dto.startedAt = QDateTime::fromString(query.value("started_at").toString(), Qt::ISODate);
        }
        if (!query.value("completed_at").isNull()) {
            dto.completedAt = QDateTime::fromString(query.value("completed_at").toString(), Qt::ISODate);
        }

        dto.resultNote   = query.value("result_note").toString();
        return dto;
    }

    return std::nullopt;
}

bool ServiceRequestRepository::checkIn(int requestId, int& outTicketNumber) const {
    DatabaseManager& db = DatabaseManager::getInstance();

    if (!db.beginTransaction()) return false;

    // Get current room_id for this request
    auto optReq = getById(requestId);
    if (!optReq.has_value()) {
        db.rollbackTransaction();
        return false;
    }

    int roomId = optReq->roomId;
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    // Calculate max ticket number for today in this room
    QString maxSql = R"(
        SELECT MAX(ticket_number) FROM service_requests
        WHERE room_id = ? AND DATE(prescribed_at) = ?
    )";

    QSqlQuery maxQuery = db.selectQuery(maxSql, {roomId, today});
    int nextTicket = 1;
    if (maxQuery.next() && !maxQuery.value(0).isNull()) {
        nextTicket = maxQuery.value(0).toInt() + 1;
    }

    QString updateSql = R"(
        UPDATE service_requests
        SET status = 'CHECKED_IN', ticket_number = ?
        WHERE request_id = ? AND status = 'PENDING'
    )";

    QSqlQuery updateQuery = db.executeQuery(updateSql, {nextTicket, requestId});
    if (updateQuery.lastError().isValid() || updateQuery.numRowsAffected() <= 0) {
        db.rollbackTransaction();
        return false;
    }

    if (!db.commitTransaction()) return false;

    outTicketNumber = nextTicket;
    return true;
}

bool ServiceRequestRepository::startProcessing(int requestId) const {
    DatabaseManager& db = DatabaseManager::getInstance();

    QString sql = R"(
        UPDATE service_requests
        SET status = 'PROCESSING', started_at = datetime('now', 'localtime')
        WHERE request_id = ? AND status = 'CHECKED_IN'
    )";

    QSqlQuery query = db.executeQuery(sql, {requestId});
    return !query.lastError().isValid() && query.numRowsAffected() > 0;
}

bool ServiceRequestRepository::completeProcessing(int requestId, const QString& resultNote) const {
    DatabaseManager& db = DatabaseManager::getInstance();

    QString sql = R"(
        UPDATE service_requests
        SET status = 'COMPLETED', completed_at = datetime('now', 'localtime'), result_note = ?
        WHERE request_id = ? AND status = 'PROCESSING'
    )";

    QSqlQuery query = db.executeQuery(sql, {resultNote, requestId});
    return !query.lastError().isValid() && query.numRowsAffected() > 0;
}

bool ServiceRequestRepository::cancelRequest(int requestId) const {
    DatabaseManager& db = DatabaseManager::getInstance();

    QString sql = R"(
        UPDATE service_requests
        SET status = 'CANCELLED'
        WHERE request_id = ? AND status != 'COMPLETED'
    )";

    QSqlQuery query = db.executeQuery(sql, {requestId});
    return !query.lastError().isValid() && query.numRowsAffected() > 0;
}
