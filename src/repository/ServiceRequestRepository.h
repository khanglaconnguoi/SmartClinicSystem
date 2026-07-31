#pragma once

#include <QList>
#include <QString>
#include <optional>
#include "dto/ServiceRequestDTOs.h"

class ServiceRequestRepository {
public:
    ServiceRequestRepository() = default;
    ~ServiceRequestRepository() = default;

    int createRequest(const ServiceRequestInputDTO& input) const;
    QList<ServiceRequestDTO> getRequestsByRoom(int roomId, const QString& date = "", std::optional<ServiceRequestStatus> status = std::nullopt) const;
    std::optional<ServiceRequestDTO> getById(int requestId) const;

    bool checkIn(int requestId, int& outTicketNumber) const;
    bool startProcessing(int requestId) const;
    bool completeProcessing(int requestId, const QString& resultNote) const;
    bool cancelRequest(int requestId) const;
};
