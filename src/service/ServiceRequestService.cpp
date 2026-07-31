#include "ServiceRequestService.h"

int ServiceRequestService::createRequest(const ServiceRequestInputDTO& input) const {
    if (input.recordId <= 0 || input.roomId <= 0 || input.doctorId <= 0 || input.serviceName.trimmed().isEmpty()) {
        return -1;
    }
    return m_repository ? m_repository->createRequest(input) : -1;
}

QList<ServiceRequestDTO> ServiceRequestService::getRequestsByRoom(
    int roomId, const QString& date, std::optional<ServiceRequestStatus> status) const {
    if (!m_repository) return {};
    return m_repository->getRequestsByRoom(roomId, date, status);
}

std::optional<ServiceRequestDTO> ServiceRequestService::getById(int requestId) const {
    if (!m_repository || requestId <= 0) return std::nullopt;
    return m_repository->getById(requestId);
}

bool ServiceRequestService::checkIn(int requestId, int& outTicketNumber) const {
    if (!m_repository || requestId <= 0) return false;
    return m_repository->checkIn(requestId, outTicketNumber);
}

bool ServiceRequestService::startProcessing(int requestId) const {
    if (!m_repository || requestId <= 0) return false;
    return m_repository->startProcessing(requestId);
}

bool ServiceRequestService::completeProcessing(int requestId, const QString& resultNote) const {
    if (!m_repository || requestId <= 0) return false;
    return m_repository->completeProcessing(requestId, resultNote);
}

bool ServiceRequestService::cancelRequest(int requestId) const {
    if (!m_repository || requestId <= 0) return false;
    return m_repository->cancelRequest(requestId);
}
