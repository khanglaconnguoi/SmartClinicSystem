#pragma once
#include "model/CommonEnums.h"
#include "dto/Pagination.h"
#include <QString>
#include <QDateTime>
#include <QList>
#include <optional>

// ── INPUT DTO ────────────────────────────────────────────────────────────────
// Bác sĩ điền thông tin khi chỉ định dịch vụ xét nghiệm
struct ServiceRequestInputDTO {
    int     recordId    = 0;  // FK → medical_records
    int     roomId      = 0;  // FK → rooms (LAB type)
    int     doctorId    = 0;  // bác sĩ chỉ định
    QString serviceName;      // "Xét nghiệm máu", "X-quang ngực"...
};

// ── RESULT DTO ───────────────────────────────────────────────────────────────
// Y tá xem danh sách hàng đợi
struct ServiceRequestDTO {
    int       requestId     = 0;
    int       recordId      = 0;
    int       roomId        = 0;
    QString   roomName;       // JOIN từ rooms
    int       doctorId      = 0;
    QString   doctorName;     // JOIN từ staff
    int       patientId     = 0;
    QString   patientName;    // JOIN qua medical_records → appointments → patients
    QString   patientCode;

    QString   serviceName;
    ServiceRequestStatus status = ServiceRequestStatus::Pending;

    std::optional<int> ticketNumber;

    QDateTime prescribedAt;
    std::optional<QDateTime> startedAt;
    std::optional<QDateTime> completedAt;
    QString   resultNote;
};

// ── SEARCH CRITERIA ──────────────────────────────────────────────────────────
struct ServiceRequestSearchCriteria {
    int     roomId      = -1;    // -1 = tất cả phòng
    std::optional<ServiceRequestStatus> status;  // nullopt = tất cả
    QString date;                // "yyyy-MM-dd", rỗng = hôm nay
    int     page     = 1;
    int     pageSize = 50;
};
