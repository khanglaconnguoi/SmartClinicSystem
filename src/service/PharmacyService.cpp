/**
 * @file    service/PharmacyService.cpp
 */
#include "PharmacyService.h"

#include <QDebug>
#include <QMap>
#include <QSet>

#include "Validation.h"
#include "dto/PatientDTOs.h"



// ─────────────────────────────────────────────────────────────────────────────
// PRIVATE VALIDATOR
// ─────────────────────────────────────────────────────────────────────────────

void PharmacyService::normalizeMedicationInput(
    MedicationInputDTO &input) const {
  input.brandName = input.brandName.simplified();
  input.unit = input.unit.trimmed();
  input.manufacturer = input.manufacturer.simplified();
  input.description = input.description.trimmed();

  // Trim + dedup danh mục (giữ bản ghi xuất hiện đầu tiên)
  QList<QString> cleanedCats;
  QSet<QString> seenCats;
  for (const QString& cat : input.categories) {
    QString trimmed = cat.simplified();
    if (trimmed.isEmpty() || seenCats.contains(trimmed)) continue;
    seenCats.insert(trimmed);
    cleanedCats.append(trimmed);
  }
  input.categories = cleanedCats;

  // Trim từng hoạt chất + loại bỏ trùng ingredientId (giữ bản ghi xuất hiện đầu
  // tiên)
  QList<MedicationInputDTO::IngredientInput> cleaned;
  QSet<int> seenIds;
  for (auto ing : input.ingredients) {
    ing.strength = ing.strength.trimmed();
    if (seenIds.contains(ing.ingredientId))
      continue;
    seenIds.insert(ing.ingredientId);
    cleaned.append(ing);
  }
  input.ingredients = cleaned;
}

QString PharmacyService::validateMedicationInput(
    const MedicationInputDTO &input) const {
  QString err;
  if (!(err = Validation::validateTrimmedNotEmpty(input.brandName, "Tên thuốc không được để trống.")).isEmpty())
    return err;
  if (!(err = Validation::validateTrimmedNotEmpty(input.unit, "Đơn vị tính không được để trống.")).isEmpty())
    return err;
  if (!(err = validateUnitPrice(input.unitPrice)).isEmpty())
    return err;
  if (!(err = validateStockQuantity(input.stockQuantity)).isEmpty())
    return err;
  if (!(err = validateMinimumStock(input.minimumStock)).isEmpty())
    return err;
  if (!(err = validateReorderThreshold(input.reorderThreshold,
                                       input.minimumStock))
           .isEmpty())
    return err;
  if (!(err = validateExpiryDate(input.expiryDate)).isEmpty())
    return err;
  if (!(err = validateCategories(input.categories)).isEmpty())
    return err;
  if (!(err = validateIngredients(input.ingredients)).isEmpty())
    return err;
  return "";
}

// ───────────────────────────────────────────────────────────────────────────────
// FORMAT VALIDATORS — public static (Medication)
// ───────────────────────────────────────────────────────────────────────────────



/** @brief Đơn giá (unitPrice): không được âm */
QString PharmacyService::validateUnitPrice(double unitPrice) {
  if (unitPrice < 0)
    return "Đơn giá không được âm.";
  return "";
}

/** @brief Số lượng tồn kho (stockQuantity): không được âm */
QString PharmacyService::validateStockQuantity(int stockQuantity) {
  if (stockQuantity < 0)
    return "Số lượng tồn kho không được âm.";
  return "";
}

/** @brief Ngưỡng tồn kho tối thiểu (minimumStock): không được âm */
QString PharmacyService::validateMinimumStock(int minimumStock) {
  if (minimumStock < 0)
    return "Ngưỡng tồn kho tối thiểu không được âm.";
  return "";
}

/**
 * @brief Ngưỡng đặt hàng lại (reorderThreshold): không được âm và phải lớn hơn
 * minimumStock.
 * @note  UI nên gọi hàm này sau khi cả hai field minimumStock và
 * reorderThreshold đã được điền xong.
 */
QString PharmacyService::validateReorderThreshold(int reorderThreshold,
                                                  int minimumStock) {
  if (reorderThreshold < 0)
    return "Ngưỡng đặt hàng lại không được âm.";
  if (reorderThreshold <= minimumStock)
    return "Ngưỡng đặt hàng lại phải lớn hơn ngưỡng tồn kho tối thiểu.";
  return "";
}

/**
 * @brief Hạn sử dụng (expiryDate): nếu có giá trị thì phải sau ngày hôm nay.
 * @note  Cho phép expiryDate không hợp lệ (isValid() == false) tức là không
 * điền — không phải lỗi.
 */
QString PharmacyService::validateExpiryDate(const QDate &expiryDate) {
  if (expiryDate.isValid() && expiryDate <= QDate::currentDate())
    return "Hạn sử dụng phải sau ngày hôm nay.";
  return "";
}

/**
 * @brief Kiểm tra toàn bộ danh sách hoạt chất: ít nhất 1 phần tử, mỗi phần tử
 * hợp lệ. Dùng trước khi submit form — kiểm tra tổng thể.
 */
QString PharmacyService::validateIngredients(
    const QList<MedicationInputDTO::IngredientInput> &ingredients) {
  if (ingredients.isEmpty())
    return "Thuốc phải có ít nhất 1 hoạt chất.";
  for (const auto &ing : ingredients) {
    QString err = validateIngredientEntry(ing.ingredientId, ing.strength);
    if (!err.isEmpty())
      return err;
  }
  return "";
}

/**
 * @brief Kiểm tra 1 hoạt chất đơn lẻ: ingredientId > 0 và strength không rỗng.
 *        UI gọi real-time ngay sau khi người dùng chọn hoạt chất / nhập hàm
 * lượng.
 */
QString PharmacyService::validateIngredientEntry(int ingredientId,
                                                 const QString &strength) {
  QString err;
  if (!(err = Validation::validateValidId(ingredientId, "Hoạt chất không hợp lệ.")).isEmpty())
    return err;
  if (!(err = Validation::validateTrimmedNotEmpty(strength, QString("Hàm lượng của hoạt chất ID=%1 không được để trống.").arg(ingredientId))).isEmpty())
    return err;
  return "";
}

/**
 * @brief Kiểm tra toàn bộ danh sách danh mục: ít nhất 1 phần tử, mỗi phần tử
 *        không được là chuỗi trống.
 *        Dùng trước khi submit form — kiểm tra tổng thể.
 */
QString PharmacyService::validateCategories(const QList<QString>& categories) {
  if (categories.isEmpty())
    return "Thuốc phải có ít nhất 1 danh mục.";
  for (const QString& cat : categories) {
    QString err = Validation::validateTrimmedNotEmpty(cat, "Tên danh mục không được để trống.");
    if (!err.isEmpty()) return err;
  }
  return "";
}



// ───────────────────────────────────────────────────────────────────────────────
// FORMAT VALIDATORS — public static (Prescription Item)
// ───────────────────────────────────────────────────────────────────────────────

/** @brief Số lượng thuốc trong đơn: phải > 0 */
QString PharmacyService::validatePrescriptionItemQuantity(int quantity) {
  if (quantity <= 0)
    return "Số lượng thuốc phải lớn hơn 0.";
  return "";
}



/** @brief Số ngày dùng (durationDays): phải > 0 */
QString PharmacyService::validatePrescriptionItemDuration(int durationDays) {
  if (durationDays <= 0)
    return "Số ngày dùng phải lớn hơn 0.";
  return "";
}

void PharmacyService::normalizePrescriptionInput(
    PrescriptionInputDTO &input) const {
  input.notes = input.notes.trimmed();

  QList<PrescriptionItemDTO> cleanedItems;
  QSet<int> seenMedicationIds;

  for (auto item : input.items) {
    item.dosage = item.dosage.simplified();
    item.frequency = item.frequency.simplified();
    item.note = item.note.trimmed();

    if (seenMedicationIds.contains(item.medicationId))
      continue;

    seenMedicationIds.insert(item.medicationId);
    cleanedItems.append(item);
  }

  input.items = cleanedItems;
}

QString PharmacyService::validatePrescriptionInput(
    const PrescriptionInputDTO &input) const {
    QString err;
  if (!(err = Validation::validateValidId(input.recordId, "Hồ sơ khám không hợp lệ.")).isEmpty())
    return err;
  if (!(err = Validation::validateValidId(input.doctorId, "Thông tin bác sĩ không hợp lệ.")).isEmpty())
    return err;

  if (input.items.isEmpty())
    return "Đơn thuốc phải có ít nhất 1 loại thuốc.";

  for (const auto &item : input.items) {
    if (!(err = validatePrescriptionItemQuantity(item.quantity)).isEmpty())
      return err;
    if (!(err = Validation::validateTrimmedNotEmpty(item.dosage, "Liều dùng không được để trống.")).isEmpty())
      return err;
    if (!(err = Validation::validateTrimmedNotEmpty(item.frequency, "Tần suất dùng không được để trống.")).isEmpty())
      return err;
    if (!(err = validatePrescriptionItemDuration(item.durationDays)).isEmpty())
      return err;
  }

  return ""; // hợp lệ
}

// ─────────────────────────────────────────────────────────────────────────────
// KHO THUỐC
// ─────────────────────────────────────────────────────────────────────────────

QString PharmacyService::addMedication(MedicationInputDTO dto) {
    normalizeMedicationInput(dto);

    QString err = validateMedicationInput(dto);  // excludeMedicationId = -1 (INSERT)
    if (!err.isEmpty()) return err;

    if (!m_medicationRepo->insertMedication(dto))
        return "Lỗi hệ thống khi thêm thuốc mới. Vui lòng thử lại.";

    return "";
}

QString PharmacyService::updateMedication(int medicationId, MedicationInputDTO dto) {
    QString err;
    if (!(err = Validation::validateValidId(medicationId, "ID thuốc không hợp lệ.")).isEmpty())
        return err;

    normalizeMedicationInput(dto);

    if (!(err = validateMedicationInput(dto)).isEmpty()) return err;

    if (!m_medicationRepo->updateMedication(medicationId, dto))
        return "Lỗi hệ thống khi cập nhật thông tin thuốc. Vui lòng thử lại.";

    return "";
}

// QList<MedicationSummaryDTO>
// PharmacyService::searchMedications(MedicationSearchCriteria& criteria) const {
//   criteria.keyword = criteria.keyword.simplified();
//   criteria.manufacturer = criteria.manufacturer.simplified();
//   // selectedCategories: UI đã gửi chuỗi sạch, không cần normalize tại đây
//   QList<std::shared_ptr<Medication>> models =
//       m_medicationRepo->searchMedications(criteria);
//   QList<MedicationSummaryDTO> dtos;

//   dtos.reserve(models.size());

//   for (const auto &model : models) {
//     if (model) {
//       dtos.append(model->toSummary());
//     }
//   }

//   return dtos;
// }

std::optional<MedicationSummaryDTO>
PharmacyService::getMedicationById(int medicationId) const {
  std::shared_ptr<Medication> model = m_medicationRepo->findById(medicationId);
  if (model == nullptr)
    return std::nullopt;
  return model->toSummary();
}

QList<MedicationSummaryDTO> PharmacyService::getLowStockMedications() const {
  QList<std::shared_ptr<Medication>> models = m_medicationRepo->findLowStock();
  QList<MedicationSummaryDTO> dtos;

  dtos.reserve(models.size());

  for (const auto &model : models) {
    if (model) {
      dtos.append(model->toSummary());
    }
  }

  return dtos;
}

QList<MedicationSummaryDTO>
PharmacyService::getExpiringMedications(int withinDays) const {
  QDate threshold = QDate::currentDate().addDays(withinDays);
  QList<std::shared_ptr<Medication>> models =
      m_medicationRepo->findExpiringBefore(threshold);
  QList<MedicationSummaryDTO> dtos;

  dtos.reserve(models.size());

  for (const auto &model : models) {
    if (model) {
      dtos.append(model->toSummary());
    }
  }

  return dtos;
}

// QList<ActiveIngredientDTO>
// PharmacyService::searchIngredients(const QString &keyword) const {
//   if (keyword.trimmed().isEmpty()) {
//     return QList<ActiveIngredientDTO>();
//   }

//   QString cleanKeyword = keyword.trimmed().simplified();

//   return m_medicationRepo->searchIngredients(cleanKeyword);
// }

// ── Phân trang: Medication ────────────────────────────────────────────────────

PagedResult<MedicationSummaryDTO>
PharmacyService::searchMedicationsPaged(MedicationSearchCriteria criteria) const {
  // Bước 1: Normalize text fields
  criteria.keyword      = criteria.keyword.simplified();
  criteria.manufacturer = criteria.manufacturer.simplified();

  // Bước 2: Clamp page/pageSize vào range hợp lệ
  //   - page     : tối thiểu 1 (UI không thể gửi page = 0 hay âm)
  //   - pageSize : [1, 200] để tránh truy vấn quá nặng
  criteria.page     = qMax(1, criteria.page);
  criteria.pageSize = qBound(1, criteria.pageSize, 200);

  // Bước 3: Delegate xuống Repository
  PagedResult<std::shared_ptr<Medication>> repoResult =
      m_medicationRepo->searchMedicationsPaged(criteria);

  // Bước 4: Map domain objects → DTOs (không thay đổi metadata phân trang)
  PagedResult<MedicationSummaryDTO> dtoResult;
  dtoResult.totalCount = repoResult.totalCount;
  dtoResult.page       = repoResult.page;
  dtoResult.pageSize   = repoResult.pageSize;
  dtoResult.items.reserve(repoResult.items.size());

  for (const auto &model : repoResult.items) {
    if (model) {
      dtoResult.items.append(model->toSummary());
    }
  }

  return dtoResult;
}

// ── Phân trang: Active Ingredient ────────────────────────────────────────────

PagedResult<ActiveIngredientDTO>
PharmacyService::searchIngredientsPaged(IngredientSearchCriteria criteria) const {
  // Bước 1: Normalize keyword
  criteria.keyword = criteria.keyword.simplified();

  // Bước 2: Clamp page/pageSize vào range hợp lệ
  criteria.page     = qMax(1, criteria.page);
  criteria.pageSize = qBound(1, criteria.pageSize, 200);

  // Bước 3: Delegate xuống Repository (kết quả là DTO trực tiếp, không cần map)
  return m_medicationRepo->searchIngredientsPaged(criteria);
}

// ─────────────────────────────────────────────────────────────────────────────
// KÊ ĐƠN & KIỂM TRA AN TOÀN
// ─────────────────────────────────────────────────────────────────────────────

QMap<int, MedicationSummaryDTO>
PharmacyService::buildMedicationMap(const QList<PrescriptionItemDTO>& items) const {
  QMap<int, MedicationSummaryDTO> medMap;
  for (const auto& item : items) {
    if (medMap.contains(item.medicationId)) continue;
    auto medOpt = getMedicationById(item.medicationId);
    if (medOpt.has_value()) {
      medMap[item.medicationId] = medOpt.value();
    }
  }
  return medMap;
}

PrescriptionSafetyReport
PharmacyService::checkPrescriptionSafety(const QList<PrescriptionItemDTO>& items,
                                         const QList<AllergyResultDTO>& allergies) const {
  PrescriptionSafetyReport report;

  // Bước 0: Load tất cả MedicationSummaryDTO liên quan
  QMap<int, MedicationSummaryDTO> medMap = buildMedicationMap(items);

  // Bước 1: Gom nhóm ingredientId -> danh sách thuốc {medicationId, brandName}
  QMap<int, QList<QPair<int, QString>>> ingredientToMeds;
  for (auto it = medMap.begin(); it != medMap.end(); ++it) {
    for (const auto& ing : it.value().ingredients) {
      ingredientToMeds[ing.ingredientId].append({it.key(), it.value().brandName});
    }
  }

  // Bước 2: Phát hiện trùng hoạt chất (ingredientId xuất hiện >= 2 thuốc)
  for (auto it = ingredientToMeds.begin(); it != ingredientToMeds.end(); ++it) {
    if (it.value().size() >= 2) {
      IngredientDuplicationWarning w;
      w.ingredientId = it.key();
      // Tìm tên hoạt chất từ medication đầu tiên chứa nó
      const auto& firstMed = medMap[it.value().first().first];
      for (const auto& ing : firstMed.ingredients) {
        if (ing.ingredientId == it.key()) {
          w.ingredientName = ing.ingredientName;
          break;
        }
      }
      w.conflictingMedications = it.value();
      report.duplications.append(w);
    }
  }

  // Bước 3: Kiểm tra dị ứng hoạt chất với bệnh nhân
  for (const auto& allergy : allergies) {
    if (!allergy.ingredientId.has_value()) continue; // Dị ứng tự do tên -> bỏ qua tự động
    if (!allergy.isActive) continue;
    int allergyIngId = allergy.ingredientId.value();
    if (!ingredientToMeds.contains(allergyIngId)) continue;

    AllergyConflictWarning w;
    w.ingredientId = allergyIngId;
    w.ingredientName = allergy.allergenName;
    w.severity = allergy.severity;
    w.allergyNotes = allergy.notes;
    w.conflictingMedications = ingredientToMeds[allergyIngId];
    report.allergyConflicts.append(w);
  }

  return report;
}

QString
PharmacyService::createPrescription(PrescriptionInputDTO &prescription) {
  normalizePrescriptionInput(prescription);
  QString err = validatePrescriptionInput(prescription);
  if (!err.isEmpty())
    return err;

  for (PrescriptionItemDTO &item : prescription.items) {
    std::shared_ptr<Medication> med =
        m_medicationRepo->findById(item.medicationId);
    if (!med)
      return QString("Thuốc ID=%1 không tồn tại.").arg(item.medicationId);

    if (!med->isEligibleForPrescription(item.quantity)) {
      if (!med->isActive())
        return QString("\"%1\" đã ngừng kinh doanh.").arg(med->getBrandName());
      if (med->isExpired())
        return QString("\"%1\" đã hết hạn sử dụng.").arg(med->getBrandName());
      return QString("\"%1\" không đủ tồn kho (cần %2, còn %3).")
          .arg(med->getBrandName())
          .arg(item.quantity)
          .arg(med->getStockQuantity());
    }
    // Snapshot
    item.brandName = med->getBrandName();
    item.unitPrice = med->getUnitPrice();
  }

  // ── Bước 3: Gọi Repository (INSERT trong 1 transaction) ───────
  if (!m_prescriptionRepo->insert(prescription)) {
    return "Lỗi hệ thống khi lưu đơn thuốc. Vui lòng thử lại.";
  }

  return ""; // thành công
}

QString PharmacyService::cancelPrescription(int prescriptionId, int cancelledBy,
                                            const QString &reason) {
  auto prescriptionOpt = m_prescriptionRepo->findById(prescriptionId);
  if (!prescriptionOpt.has_value())
    return "Không tìm thấy đơn thuốc.";

  PrescriptionResultDTO prescription = prescriptionOpt.value();
  if (prescription.status != PrescriptionStatus::Pending)
    return QString("Không thể hủy đơn thuốc có trạng thái \"%1\".")
        .arg(prescriptionStatusToVi(prescription.status));

  if (!m_prescriptionRepo->cancel(prescriptionId, cancelledBy, reason))
    return "Lỗi hệ thống khi hủy đơn thuốc.";

  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// CẤP PHÁT THUỐC
// ─────────────────────────────────────────────────────────────────────────────

QString PharmacyService::dispensePrescription(int prescriptionId,
                                              int dispensedBy) {
  auto prescriptionOpt = m_prescriptionRepo->findById(prescriptionId);
  if (!prescriptionOpt.has_value())
    return "Không tìm thấy đơn thuốc.";

  PrescriptionResultDTO prescription = prescriptionOpt.value();

  if (prescription.status != PrescriptionStatus::Pending)
    return QString("Đơn thuốc có trạng thái \"%1\" — không thể cấp phát.")
        .arg(prescriptionStatusToVi(prescription.status));

  if (!m_prescriptionRepo->dispense(prescriptionId, dispensedBy))
    return "Lỗi hệ thống khi cập nhật trạng thái đơn thuốc.";

  return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// TRUY VẤN
// ─────────────────────────────────────────────────────────────────────────────



QList<PrescriptionResultDTO> PharmacyService::searchPrescriptions(
    PrescriptionSearchCriteria &criteria) const {
  // Làm sạch dữ liệu chữ trước khi tìm kiếm gần đúng (LIKE)
  criteria.patientName = criteria.patientName.simplified();
  criteria.doctorName = criteria.doctorName.simplified();
  criteria.keyword = criteria.keyword.simplified();
  criteria.status = criteria.status.trimmed();

  return m_prescriptionRepo->search(criteria);
}

std::optional<PrescriptionResultDTO>
PharmacyService::getPrescriptionByRecordId(int recordId) const {
  return m_prescriptionRepo->findByRecordId(recordId);
}

QList<PrescriptionResultDTO>
PharmacyService::getPrescriptionsByPatient(int patientId) const {
  return m_prescriptionRepo->findByPatientId(patientId);
}

PagedResult<PrescriptionResultDTO> PharmacyService::searchPrescriptionsPaged(
    PrescriptionSearchCriteria criteria) const {
  criteria.patientName = criteria.patientName.simplified();
  criteria.doctorName = criteria.doctorName.simplified();
  criteria.keyword = criteria.keyword.simplified();
  criteria.status = criteria.status.trimmed();

  if (criteria.page < 1) criteria.page = 1;
  if (criteria.pageSize < 1) criteria.pageSize = 20;

  return m_prescriptionRepo->searchPaged(criteria);
}


