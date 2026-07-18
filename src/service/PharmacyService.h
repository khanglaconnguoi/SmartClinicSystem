/**
 * @file    service/PharmacyService.h
 * @brief   Business Logic cho Dược phẩm & Kê đơn
 *
 * PHẠM VI:
 *   - Quản lý kho thuốc (tìm kiếm, kiểm tra tồn kho, cảnh báo)
 *   - Tạo / hủy đơn thuốc (validate + snapshot giá)
 *   - Cấp phát thuốc (Dược sĩ/Y tá thao tác)
 *   - Cung cấp dữ liệu đơn thuốc cho BillingService
 *
 * KHÔNG THUỘC PHẠM VI:
 *   - Tạo hồ sơ khám (MedicalRecordService)
 *   - Tính hóa đơn (BillingService)
 *   - PharmacyService KHÔNG gọi MedicalRecordService và ngược lại
 *     — UI là nơi kết nối hai service này qua recordId
 *
 * VALIDATE STRATEGY:
 *   - Mỗi field có 1 hàm validate riêng (public static) -> UI gọi real-time khi editingFinished()
 *   - Validate format trước, validate business-rule phức hợp (liên quan nhiều field) sau
 *   - Aggregate validators (private) gom toàn bộ trước khi gọi Repo
 *   - Tất cả hàm validate trả về QString: "" = hợp lệ, có giá trị = thông báo lỗi
 *
 * LƯU Ý VỀ THIẾT KẾ:
 *   - Return type: QString — rỗng = thành công, có nội dung = thông báo lỗi
 *     (nhất quán với pattern của StaffService hiện tại trong project).
 */
#pragma once
#include "dto/PrescriptionDTOs.h"
#include "dto/MedicationDTOs.h"
#include "repository/MedicationRepository.h"
#include "repository/PrescriptionRepository.h"
#include <QDate>
#include <QList>
#include <memory>
#include <optional>

class PharmacyService {
private:
    std::shared_ptr<MedicationRepository>   m_medicationRepo;
    std::shared_ptr<PrescriptionRepository> m_prescriptionRepo;

    // ── Private Aggregate Validator ──────────────────────────────────

    /**
     * @brief Chuẩn hóa dữ liệu: trim khoảng trắng, viết hoa mã thuốc,
     *        loại bỏ hoạt chất trùng lặp trong cùng 1 lần submit.
     *        LUÔN gọi trước validate — đảm bảo validate trên dữ liệu đã sạch.
     */
    void normalizeMedicationInput(MedicationInputDTO& input) const;

    /**
     * @brief Validate toàn bộ field — dùng chung cho cả add và update.
     */
    QString validateMedicationInput(const MedicationInputDTO& input) const;

    void normalizePrescriptionInput(PrescriptionInputDTO& input) const;

    /**
     * @brief Validate toàn bộ đơn thuốc trước khi INSERT.
     *        Gọi tuần tự: kiểm tra record hợp lệ → kiểm tra từng item.
     *        Dừng lại và trả về lỗi ngay khi gặp item đầu tiên không hợp lệ.
     * @return "" nếu hợp lệ, chuỗi mô tả lỗi nếu không
     */
    QString validatePrescriptionInput(const PrescriptionInputDTO& input) const;

public:
    explicit PharmacyService(
        std::shared_ptr<MedicationRepository>   medicationRepo,
        std::shared_ptr<PrescriptionRepository> prescriptionRepo
    ) : 
        m_medicationRepo(medicationRepo),
        m_prescriptionRepo(prescriptionRepo)
    {}

    // ════════════════════════════════════════════════════════════════
    // FORMAT VALIDATORS — public static (Medication)
    // UI gọi real-time trên từng QLineEdit::editingFinished()
    // Không cần DB, không có side effect.
    // Trả về: "" = hợp lệ | chuỗi lỗi = không hợp lệ
    // ════════════════════════════════════════════════════════════════

    // -- Field thông tin cơ bản thuốc ─────────────────────────────────
    static QString validateBrandName(const QString& brandName);
    static QString validateUnit(const QString& unit);
    static QString validateUnitPrice(double unitPrice);
    static QString validateStockQuantity(int stockQuantity);
    static QString validateMinimumStock(int minimumStock);
    static QString validateReorderThreshold(int reorderThreshold, int minimumStock);
    static QString validateExpiryDate(const QDate& expiryDate);

    // -- Field danh mục thuốc ────────────────────────────────────────
    /** @brief Kiểm tra danh sách danh mục: không rỗng, mỗi danh mục không được là chuỗi trống */
    static QString validateCategories(const QList<QString>& categories);
    /** @brief Kiểm tra 1 danh mục đơn lẻ — UI gọi real-time khi người dùng nhập tên danh mục */
    static QString validateCategoryEntry(const QString& category);

    // -- Field thành phần hoạt chất ───────────────────────────────────
    /** @brief Kiểm tra danh sách hoạt chất: ít nhất 1, ingredientId > 0, strength không rỗng */
    static QString validateIngredients(const QList<MedicationInputDTO::IngredientInput>& ingredients);
    /** @brief Kiểm tra một hoạt chất đơn lẻ — UI gọi real-time khi người dùng thêm từng dòng */
    static QString validateIngredientEntry(int ingredientId, const QString& strength);

    // ════════════════════════════════════════════════════════════════
    // FORMAT VALIDATORS — public static (Prescription Item)
    // UI gọi khi bác sĩ nhập từng thuốc trong danh sách đơn
    // ════════════════════════════════════════════════════════════════

    static QString validatePrescriptionItemQuantity(int quantity);
    static QString validatePrescriptionItemDosage(const QString& dosage);
    static QString validatePrescriptionItemFrequency(const QString& frequency);
    static QString validatePrescriptionItemDuration(int durationDays);

    // ════════════════════════════════════════════════════════════════
    // KHO THUỐC — Medication Inventory
    // ════════════════════════════════════════════════════════════════

    QString addMedication(MedicationInputDTO& dto);
    QString updateMedication(int medicationId, MedicationInputDTO& dto);


    /**
     * @brief Tìm kiếm thuốc — bác sĩ gọi khi đang lập đơn.
     *        Mặc định chỉ trả về thuốc còn hàng và chưa hết hạn.
     *
     * @note  Không phân trang — trả về toàn bộ kết quả.
     *        Dùng cho các luồng nội bộ không cần phân trang
     *        (VD: load danh sách thả xuống nhỏ).
     */
    QList<MedicationSummaryDTO> searchMedications(MedicationSearchCriteria& criteria) const;

    /**
     * @brief Tìm kiếm thuốc có phân trang — UI chính (bảng danh sánh thuốc, trang Admin).
     *
     * Các bước xử lý bên trong:
     *   1. Normalize criteria (trim, simplified)
     *   2. Guard: cưỡng chế page >= 1, pageSize trong [1, 200]
     *   3. Delegate xuống MedicationRepository::searchMedicationsPaged()
     *   4. Map domain object sang MedicationSummaryDTO
     *   5. Trả về PagedResult<MedicationSummaryDTO> có đủ totalCount để UI vẽ pagination bar
     *
     * @param criteria  UI set page (>= 1), pageSize (> 0) trước khi gọi.
     * @return          PagedResult có items, totalCount, page, pageSize.
     */
    PagedResult<MedicationSummaryDTO> searchMedicationsPaged(
        MedicationSearchCriteria& criteria) const;

    /**
     * @brief Lấy chi tiết 1 thuốc theo ID — dùng khi bác sĩ xem thông tin
     *        trước khi kê đơn (thành phần, chỉ định, hướng dẫn dùng).
     */
    std::optional<MedicationSummaryDTO> getMedicationById(int medicationId) const;

    /**
     * @brief Danh sách thuốc sắp hết hàng (stockQuantity <= reorderThreshold).
     *        Dashboard Admin/Dược sĩ hiển thị cảnh báo.
     */
    QList<MedicationSummaryDTO> getLowStockMedications() const;

    /**
     * @brief Danh sách thuốc sắp hết hạn trong vòng N ngày.
     *        Dashboard Admin/Dược sĩ hiển thị cảnh báo.
     * @param withinDays Ngưỡng cảnh báo (mặc định 30 ngày)
     */
    QList<MedicationSummaryDTO> getExpiringMedications(int withinDays = 30) const;


    QList<ActiveIngredientDTO> searchIngredients(const QString& keyword) const;

    /**
     * @brief Tìm kiếm hoạt chất có phân trang.
     *
     * Dùng khi Admin cần chọn hoạt chất để thêm vào thuốc:
     *   • Gõi ý autocomplete với danh sách có phân trang (không load hết 1 lần)
     *   • Trang phân trang trong màn hình quản lý hoạt chất
     *
     * Các bước xử lý bên trong:
     *   1. Normalize keyword (trim, toLower)
     *   2. Guard: page >= 1, pageSize trong [1, 200]
     *   3. Delegate xuống MedicationRepository::searchIngredientsPaged()
     *
     * @param criteria  keyword, page (>= 1), pageSize (> 0).
     * @return          PagedResult có items, totalCount, page, pageSize.
     */
    PagedResult<ActiveIngredientDTO> searchIngredientsPaged(
        IngredientSearchCriteria& criteria) const;

    // ════════════════════════════════════════════════════════════════
    // KÊ ĐƠN — Prescription Creation
    // ════════════════════════════════════════════════════════════════

    /**
     * @brief Tạo đơn thuốc mới sau khi bác sĩ hoàn thành khám.
     *
     * Các bước xử lý bên trong:
     *   1. Validate input (recordId hợp lệ, items không rỗng)
     *   2. Với mỗi item: load Medication → kiểm tra isEligibleForPrescription()
     *   3. Snapshot medicationName và unitPrice từ Medication object
     *   4. Gọi PrescriptionRepository::insert() trong 1 transaction:
     *        INSERT prescriptions → INSERT prescription_items → deduct stock
     *
     * @param input  Thông tin đơn thuốc từ UI (items chưa có snapshot)
     * @return "" nếu thành công, chuỗi lỗi nếu thất bại
     *
     * @note recordId phải là kết quả trả về từ
     *       MedicalRecordService::createMedicalRecord() trong cùng ca khám.
     *       UI chịu trách nhiệm truyền đúng recordId này.
     */
    QString createPrescription(PrescriptionInputDTO& input);

    /**
     * @brief Hủy đơn thuốc — chỉ hủy được khi status = "PENDING".
     *        Hoàn trả tồn kho cho tất cả items trong đơn bị hủy.
     * @param prescriptionId  Đơn thuốc cần hủy
     * @param cancelledBy     staff_id của người hủy (bác sĩ hoặc dược sĩ)
     * @return "" nếu thành công, chuỗi lỗi nếu thất bại
     */
    QString cancelPrescription(int prescriptionId, int cancelledBy, const QString& reason);

    // ════════════════════════════════════════════════════════════════
    // CẤP PHÁT THUỐC — Dispensing (Dược sĩ / Y tá thực hiện)
    // ════════════════════════════════════════════════════════════════

    /**
     * @brief Dược sĩ xác nhận đã cấp phát thuốc cho bệnh nhân.
     *        Chuyển trạng thái: PENDING → DISPENSED.
     *        Tồn kho đã bị trừ lúc createPrescription() — không trừ thêm.
     * @param prescriptionId  Đơn cần cấp phát
     * @param dispensedBy     staff_id của dược sĩ/y tá thực hiện
     * @return "" nếu thành công, chuỗi lỗi nếu thất bại
     */
    QString dispensePrescription(int prescriptionId, int dispensedBy);

    // ════════════════════════════════════════════════════════════════
    // TRUY VẤN — Queries
    // ════════════════════════════════════════════════════════════════

    QList<PrescriptionResultDTO> searchPrescriptions(PrescriptionSearchCriteria& criteria) const;

    /**
     * @brief Lấy đơn thuốc theo hồ sơ khám.
     *        BillingService gọi hàm này để lấy danh sách thuốc + giá snapshot
     *        khi tổng hợp hóa đơn.
     *        UI gọi để hiển thị đơn thuốc trong màn hình xem hồ sơ.
     */
    std::optional<PrescriptionResultDTO> getPrescriptionByRecordId(int recordId) const;

    /**
     * @brief Lấy lịch sử đơn thuốc của 1 bệnh nhân qua tất cả lần khám.
     *        Dùng để kiểm tra tiền sử dùng thuốc — hỗ trợ cảnh báo dị ứng.
     */
    QList<PrescriptionResultDTO> getPrescriptionsByPatient(int patientId) const;
};