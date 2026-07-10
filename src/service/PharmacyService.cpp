/**
 * @file    service/PharmacyService.cpp
 */
#include "PharmacyService.h"
#include "utils/StringSanitize.h"
#include <QDebug>
#include <QSet>

// ─────────────────────────────────────────────────────────────────────────────
// PRIVATE VALIDATOR
// ─────────────────────────────────────────────────────────────────────────────

void PharmacyService::normalizeMedicationInput(MedicationInputDTO& input) const {
    input.brandName      = StringSanitize::collapseSpaces(input.brandName);
    input.category       = StringSanitize::collapseSpaces(input.category);
    input.unit           = input.unit.trimmed();
    input.manufacturer   = StringSanitize::collapseSpaces(input.manufacturer);
    input.description    = input.description.trimmed();

    // Trim từng hoạt chất + loại bỏ trùng ingredientId (giữ bản ghi xuất hiện đầu tiên)
    QList<MedicationInputDTO::IngredientInput> cleaned;
    QSet<int> seenIds;
    for (auto ing : input.ingredients) {
        ing.strength = ing.strength.trimmed();
        if (seenIds.contains(ing.ingredientId)) continue;
        seenIds.insert(ing.ingredientId);
        cleaned.append(ing);
    }
    input.ingredients = cleaned;
}

QString PharmacyService::validateMedicationInput(const MedicationInputDTO& input) const {
    if (input.brandName.isEmpty())
        return "Tên thuốc không được để trống.";

    if (input.unit.isEmpty())
        return "Đơn vị tính không được để trống.";

    if (input.unitPrice < 0)
        return "Đơn giá không được âm.";

    if (input.stockQuantity < 0)
        return "Số lượng tồn kho không được âm.";

    if (input.minimumStock < 0)
        return "Ngưỡng tồn kho tối thiểu không được âm.";

    if (input.reorderThreshold < 0)
        return "Ngưỡng đặt hàng lại không được âm.";

    if (input.reorderThreshold <= input.minimumStock)
        return "Ngưỡng đặt hàng lại phải lớn hơn ngưỡng tồn kho tối thiểu.";

    if (input.expiryDate.isValid() && input.expiryDate <= QDate::currentDate())
        return "Hạn sử dụng phải sau ngày hôm nay.";

    if (input.ingredients.isEmpty())
        return "Thuốc phải có ít nhất 1 hoạt chất.";

    for (const auto& ing : input.ingredients) {
        if (ing.ingredientId <= 0)
            return "Hoạt chất không hợp lệ.";
        if (ing.strength.isEmpty())
            return QString("Hàm lượng của hoạt chất ID=%1 không được để trống.")
                          .arg(ing.ingredientId);
    }

    return "";
}


void PharmacyService::normalizePrescriptionInput(PrescriptionInputDTO& input) const {
    input.notes = input.notes.trimmed();

    QList<PrescriptionItemDTO> cleanedItems;
    QSet<int> seenMedicationIds;

    for (auto item : input.items) {
        item.dosage = StringSanitize::collapseSpaces(item.dosage);
        item.frequency = StringSanitize::collapseSpaces(item.frequency);
        item.note = item.note.trimmed();

        if (seenMedicationIds.contains(item.medicationId)) continue;
        
        seenMedicationIds.insert(item.medicationId);
        cleanedItems.append(item);
    }
    
    input.items = cleanedItems;
}


QString PharmacyService::validatePrescriptionInput(const PrescriptionInputDTO& input) const {
    if (input.recordId <= 0)
        return "Hồ sơ khám không hợp lệ.";

    if (input.doctorId <= 0)
        return "Thông tin bác sĩ không hợp lệ.";

    if (input.items.isEmpty())
        return "Đơn thuốc phải có ít nhất 1 loại thuốc.";

    for (const auto& item : input.items) {
        if (item.quantity <= 0)
            return QString("Số lượng thuốc ID=%1 phải lớn hơn 0.")
                          .arg(item.medicationId);

        if (item.dosage.isEmpty())
            return QString("Liều dùng của thuốc ID=%1 không được rỗng.")
                          .arg(item.medicationId);

        if (item.frequency.isEmpty())
            return QString("Tần suất dùng của thuốc ID=%1 không được rỗng.")
                          .arg(item.medicationId);

        if (item.durationDays <= 0)
            return QString("Ngày dùng của thuốc ID=%1 phải lớn hơn 0.")
                          .arg(item.medicationId);
    }

    return ""; // hợp lệ
}



// ─────────────────────────────────────────────────────────────────────────────
// KHO THUỐC
// ─────────────────────────────────────────────────────────────────────────────

QString PharmacyService::addMedication(MedicationInputDTO& dto) {
    normalizeMedicationInput(dto);

    QString err = validateMedicationInput(dto); // excludeMedicationId = -1 (INSERT)
    if (!err.isEmpty()) return err;

    if (!m_medicationRepo->insert(dto))
        return "Lỗi hệ thống khi thêm thuốc mới. Vui lòng thử lại.";

    return "";
}

QString PharmacyService::updateMedication(int medicationId, MedicationInputDTO& dto) {
    if (medicationId <= 0)
        return "ID thuốc không hợp lệ.";

    normalizeMedicationInput(dto);

    QString err = validateMedicationInput(dto);
    if (!err.isEmpty()) return err;

    if (!m_medicationRepo->update(medicationId, dto))
        return "Lỗi hệ thống khi cập nhật thông tin thuốc. Vui lòng thử lại.";

    return "";
}



QList<MedicationSummaryDTO> PharmacyService::searchMedications(MedicationSearchCriteria& criteria) const {
    criteria.keyword      = StringSanitize::collapseSpaces(criteria.keyword);
    criteria.category     = StringSanitize::collapseSpaces(criteria.category);
    criteria.manufacturer = StringSanitize::collapseSpaces(criteria.manufacturer);
    QList<std::shared_ptr<Medication>> models = m_medicationRepo->search(criteria);
    QList<MedicationSummaryDTO> dtos;
    
    dtos.reserve(models.size());
    
    for (const auto& model : models) {
        if (model) {
            dtos.append(model->toSummary());
        }
    }
    
    return dtos;
}

std::optional<MedicationSummaryDTO> PharmacyService::getMedicationById(int medicationId) const {
    std::shared_ptr<Medication> model = m_medicationRepo->findById(medicationId);
    if(model == nullptr) return std::nullopt; 
    return model->toSummary();
}

QList<MedicationSummaryDTO> PharmacyService::getLowStockMedications() const {
    QList<std::shared_ptr<Medication>> models = m_medicationRepo->findLowStock();
    QList<MedicationSummaryDTO> dtos;
    
    dtos.reserve(models.size());
    
    for (const auto& model : models) {
        if (model) {
            dtos.append(model->toSummary());
        }
    }
    
    return dtos;
}

QList<MedicationSummaryDTO> PharmacyService::getExpiringMedications(int withinDays) const {
    QDate threshold = QDate::currentDate().addDays(withinDays);
    QList<std::shared_ptr<Medication>> models = m_medicationRepo->findExpiringBefore(threshold);
    QList<MedicationSummaryDTO> dtos;
    
    dtos.reserve(models.size());
    
    for (const auto& model : models) {
        if (model) {
            dtos.append(model->toSummary());
        }
    }
    
    return dtos;
}

// ─────────────────────────────────────────────────────────────────────────────
// KÊ ĐƠN
// ─────────────────────────────────────────────────────────────────────────────

QString PharmacyService::createPrescription(PrescriptionInputDTO& prescription) {
    normalizePrescriptionInput(prescription);
    QString err = validatePrescriptionInput(prescription);
    if (!err.isEmpty()) return err;

    for (PrescriptionItemDTO& item : prescription.items) {
        std::shared_ptr<Medication> med = m_medicationRepo->findById(item.medicationId);
        if (!med)
            return QString("Thuốc ID=%1 không tồn tại.").arg(item.medicationId);

        if (!med->isEligibleForPrescription(item.quantity)) {
            if (!med->isActive())
                return QString("\"%1\" đã ngừng kinh doanh.").arg(med->getBrandName());
            if (med->isExpired())
                return QString("\"%1\" đã hết hạn sử dụng.").arg(med->getBrandName());
            return QString("\"%1\" không đủ tồn kho (cần %2, còn %3).")
                .arg(med->getBrandName()).arg(item.quantity).arg(med->getStockQuantity());
        }
        // Snapshot
        item.brandName      = med->getBrandName();
        item.unitPrice      = med->getUnitPrice();
    }

    // ── Bước 3: Gọi Repository (INSERT trong 1 transaction) ───────
    if (!m_prescriptionRepo->insert(prescription)) {
        return "Lỗi hệ thống khi lưu đơn thuốc. Vui lòng thử lại.";
    }

    return ""; // thành công
}

QString PharmacyService::cancelPrescription(int prescriptionId, int cancelledBy, const QString& reason)
{
    auto prescriptionOpt = m_prescriptionRepo->findById(prescriptionId);
    if (!prescriptionOpt.has_value())
        return "Không tìm thấy đơn thuốc.";

    PrescriptionResultDTO prescription = prescriptionOpt.value();
    if (prescription.status != "PENDING")
        return QString("Không thể hủy đơn thuốc có trạng thái \"%1\".")
                      .arg(prescription.status);

    if (!m_prescriptionRepo->cancel(prescriptionId, cancelledBy, reason))
        return "Lỗi hệ thống khi hủy đơn thuốc.";

    return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// CẤP PHÁT THUỐC
// ─────────────────────────────────────────────────────────────────────────────

QString PharmacyService::dispensePrescription(int prescriptionId, int dispensedBy)
{
    auto prescriptionOpt = m_prescriptionRepo->findById(prescriptionId);
    if (!prescriptionOpt.has_value())
        return "Không tìm thấy đơn thuốc.";

    PrescriptionResultDTO prescription = prescriptionOpt.value();
    
    if (prescription.status != "PENDING")
        return QString("Đơn thuốc có trạng thái \"%1\" — không thể cấp phát.")
                      .arg(prescription.status);

    if (!m_prescriptionRepo->dispense(prescriptionId, dispensedBy))
        return "Lỗi hệ thống khi cập nhật trạng thái đơn thuốc.";

    return "";
}   

// ─────────────────────────────────────────────────────────────────────────────
// TRUY VẤN
// ─────────────────────────────────────────────────────────────────────────────

QList<PrescriptionResultDTO> PharmacyService::searchPrescriptions(PrescriptionSearchCriteria& criteria) const {
    // Làm sạch dữ liệu chữ trước khi tìm kiếm gần đúng (LIKE)
    criteria.patientName   = StringSanitize::collapseSpaces(criteria.patientName);
    criteria.doctorName    = StringSanitize::collapseSpaces(criteria.doctorName);
    criteria.keyword       = StringSanitize::collapseSpaces(criteria.keyword);
    criteria.status        = criteria.status.trimmed();

    return m_prescriptionRepo->search(criteria);
}


std::optional<PrescriptionResultDTO> PharmacyService::getPrescriptionByRecordId(int recordId) const {
    return m_prescriptionRepo->findByRecordId(recordId);
}

QList<PrescriptionResultDTO> PharmacyService::getPrescriptionsByPatient(int patientId) const {
    return m_prescriptionRepo->findByPatientId(patientId);
}