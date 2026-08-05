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
  std::shared_ptr<Medication> mapRowToMedication(const QSqlQuery &query) const;
  ActiveIngredientDTO mapRowToIngredient(const QSqlQuery &query) const;
  // mapRowToSummary: dùng cho các hàm trả về DTO hiển thị
  // MedicationSummaryDTO mapRowToSummary(const QSqlQuery& q) const;
  QList<MedicationIngredientDTO>
  getIngredientsForMedication(int medicationId) const;
  QList<QString> getCategoriesForMedication(int medicationId) const;

  bool insertMedicationBase(const MedicationInputDTO &medication,
                            int &outMedicationId);
  bool insertMedicationIngredients(
      int medicationId,
      const QList<MedicationInputDTO::IngredientInput> &ingredients);
  bool insertMedicationCategories(int medicationId,
                                  const QList<QString> &categories);

public:
  // ── DÙNG TRONG PharmacyService::createPrescription() ─────────────
  // Trả về domain object để Service có thể gọi isEligibleForPrescription()
  std::shared_ptr<Medication> findById(int medicationId) const;

  // ── DÙNG CHO UI (bác sĩ chọn thuốc khi lập đơn)
  // ─────────────────────────────
  //QList<std::shared_ptr<Medication>> searchMedications(const MedicationSearchCriteria &criteria) const;
  QList<std::shared_ptr<Medication>> findLowStock() const;
  QList<std::shared_ptr<Medication>>
  findExpiringBefore(const QDate &date) const;
  double getTotalInventoryValue() const;
  int getOutOfStockCount() const;

  //QList<ActiveIngredientDTO> searchIngredients(const QString &keyword) const;

  // ── PHÂN TRANG — các hàm dưới đây dùng LIMIT/OFFSET + COUNT(*) ────────────

  /**
   * @brief Tìm kiếm thuốc có phân trang.
   *        Chạy 2 query SQL: COUNT(*) để lấy tổng bản ghi,
   *        rồi LIMIT/OFFSET để lấy dữ liệu trang hiện tại.
   * @param criteria  Phải set sẵn page (>= 1) và pageSize (> 0).
   *                  pageSize = 0 → không phân trang, trả về tất cả.
   * @return PagedResult chứa items, totalCount, page, pageSize.
   */
  PagedResult<std::shared_ptr<Medication>>
  searchMedicationsPaged(const MedicationSearchCriteria &criteria) const;

  /**
   * @brief Tìm kiếm hoạt chất có phân trang.
   *        Dùng khi Admin nhập phần của tên hoạt chất và cần dropdown gợi ý.
   * @param criteria  keyword (LIKE), page, pageSize.
   * @return PagedResult chứa items, totalCount, page, pageSize.
   */
  PagedResult<ActiveIngredientDTO>
  searchIngredientsPaged(const IngredientSearchCriteria &criteria) const;

  // ── QUẢN LÝ KHO (Admin) ───────────────────────────────────────────
  bool insertMedication(const MedicationInputDTO &medication);
  bool updateMedication(int medicationId, const MedicationInputDTO &dto);
  bool deactivate(int medicationId);
  bool reactivate(int medicationId);

  // atomic stock update — tránh race condition khi 2 bác sĩ kê cùng lúc
  // bool adjustStock(int medicationId, int delta);
};