#pragma once
#include <QList>

// ── PHÂN TRANG — Generic wrapper ─────────────────────────────────────────
/**
 * @brief Wrapper trả về kết quả phân trang cho bất kỳ kiểu T nào.
 *        Repository tính sẵn totalCount bằng COUNT(*) riêng để UI
 *        biết tổng số trang mà không cần load toàn bộ data.
 *
 * @tparam T  Kiểu phần tử trong danh sách (VD: MedicationSummaryDTO,
 *            ActiveIngredientDTO, shared_ptr<Medication>, ...)
 */
template<typename T>
struct PagedResult {
    QList<T> items;   ///< Dữ liệu của trang hiện tại
    int totalCount;   ///< Tổng số bản ghi khớp điều kiện (bỏ qua LIMIT/OFFSET)
    int page;         ///< Trang hiện tại (1-indexed)
    int pageSize;     ///< Số bản ghi mỗi trang

    /** Tổng số trang, làm tròn lên (trả về 0 nếu pageSize <= 0) */
    int totalPages() const {
        if (pageSize <= 0) return 0;
        return (totalCount + pageSize - 1) / pageSize;
    }
    bool hasNext() const { return page < totalPages(); }
    bool hasPrev() const { return page > 1; }
    bool isEmpty() const { return items.isEmpty(); }
};