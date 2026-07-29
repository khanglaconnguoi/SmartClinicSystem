#pragma once
#include "dto/PrescriptionDTOs.h"
#include "DatabaseManager.h"
#include <QString>
#include <QList>
#include <optional>

class PrescriptionRepository {
private:
    //std::shared_ptr<Prescription> mapRowToMedication(int prescriptionId) const;
    bool insertHeader(const PrescriptionInputDTO& prescription, int& prescriptionId);
    bool insertItems(int prescriptionId, const QList<PrescriptionItemDTO>& items);


    PrescriptionResultDTO mapRowToPrescriptionHeader(const QSqlQuery& query) const;
    PrescriptionItemDTO mapRowToPrescriptionItem(const QSqlQuery& query) const;
public:
    PrescriptionRepository() = default;
    ~PrescriptionRepository() = default;

    bool insert(const PrescriptionInputDTO& prescription);
    bool cancel(int prescriptionId, int cancelledBy, const QString& reason);
    bool dispense(int prescriptionId, int dispensedBy);

    QList<PrescriptionResultDTO> search(const PrescriptionSearchCriteria& criteria) const;
    std::optional<PrescriptionResultDTO> findById(int prescriptionId) const;
    std::optional<PrescriptionResultDTO> findByRecordId(int recordId) const;
    QList<PrescriptionResultDTO> findByPatientId(int patientId) const;

    std::optional<PrescriptionStatus> getStatus(int prescriptionId) const;
};