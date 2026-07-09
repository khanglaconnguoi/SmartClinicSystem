// ── repository/MedicationRepository.h ───────────────────────────────
#pragma once
#include "dto/MedicationDTOs.h"
#include "model/Medication.h"
#include <QList>
#include <QSqlQuery>
#include <QString>
#include <memory>


class MedicationRepository {
private:
    // mapRowToMedication: dùng nội bộ cho các hàm trả về domain object
    std::shared_ptr<Medication> mapRowToMedication(const QSqlQuery& query) const;
    // mapRowToSummary: dùng cho các hàm trả về DTO hiển thị
    // MedicationSummaryDTO mapRowToSummary(const QSqlQuery& q) const;
    QList<MedicationIngredientDTO> getIngredientsForMedication(int medicationId) const;

    bool insertMedicationBase(const MedicationInputDTO& medication, int& outMedicationId);
    bool insertMedicationIngredients(int medicationId, const QList<MedicationInputDTO::IngredientInput>& ingredients);

public:
    // ── DÙNG TRONG PharmacyService::createPrescription() ─────────────
    // Trả về domain object để Service có thể gọi isEligibleForPrescription()
    std::shared_ptr<Medication> findById(int medicationId) const;

    // ── DÙNG CHO UI (bác sĩ chọn thuốc khi lập đơn) ──────────────────
    QList<std::shared_ptr<Medication>> search(const MedicationSearchCriteria& criteria) const;
    QList<std::shared_ptr<Medication>> findLowStock() const;
    QList<std::shared_ptr<Medication>> findExpiringBefore(const QDate& date) const;


    // ── QUẢN LÝ KHO (Admin) ───────────────────────────────────────────
    bool insert(const MedicationInputDTO& medication);
    bool update(int medicationId, const MedicationInputDTO& dto);
    bool deactivate(int medicationId);
    bool reactivate(int medicationId);

    // atomic stock update — tránh race condition khi 2 bác sĩ kê cùng lúc
    // bool adjustStock(int medicationId, int delta);
};