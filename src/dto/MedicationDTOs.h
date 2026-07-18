#pragma once

#include <QDate>
#include <QList>
#include <QString>
#include <QVariant>

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


struct ActiveIngredientDTO {
  int ingredientId;
  QString ingredientName;
  QString description;
};

struct MedicationIngredientDTO {
  int ingredientId;
  QString ingredientName;
  QString strength;
};

// INPUT — Admin thêm thuốc vào kho (UI → PharmacyService →
// MedicationRepository)
struct MedicationInputDTO {
  QString brandName;
  QList<QString> categories; // Một thuốc có thể thuộc nhiều danh mục
  QString unit;
  double unitPrice;
  int stockQuantity;
  int minimumStock;
  int reorderThreshold;
  QDate expiryDate;
  QString manufacturer;
  QString description;

  struct IngredientInput {
    int ingredientId;
    QString strength;
  };
  QList<IngredientInput> ingredients;
};

// ── MEDICATION SEARCH ─────────────────────────────────────────────────
struct MedicationSearchCriteria {
  QString keyword;
  QList<QString> selectedCategories; // Lọc theo nhiều danh mục (dùng IN)
  QList<int> selectedIngredientIds;
  bool inStockOnly = true;
  bool excludeExpired = true;
  bool lowStockOnly = false;
  bool criticalStockOnly = false;
  QVariant maxUnitPrice;
  QString manufacturer = "";
  // ── Phân trang ─────────────────────────────────────────────────────
  int page     = 1;  ///< Trang hiện tại (1-indexed). UI set trước khi gọi Service.
  int pageSize = 20; ///< Số kết quả mỗi trang. 0 = không phân trang (trả về tất cả).
};

// ── INGREDIENT SEARCH ─────────────────────────────────────────────────
/**
 * @brief Tiêu chí tìm kiếm hoạt chất (active_ingredients).
 *        Dùng khi Admin thêm thuốc và cần autocomplete hoạt chất.
 */
struct IngredientSearchCriteria {
  QString keyword;   ///< Tìm theo tên hoạt chất (LIKE %keyword%)
  int page     = 1;  ///< Trang hiện tại (1-indexed)
  int pageSize = 20; ///< Số kết quả mỗi trang. 0 = không phân trang.
};

// ── MEDICATION SUMMARY — bác sĩ chọn thuốc khi lập đơn ──────────────
struct MedicationSummaryDTO {
  int medicationId;
  QString brandName;
  QList<QString> categories; // Danh sách danh mục thuốc
  QString unit;
  double unitPrice;
  int stockQuantity;
  QDate expiryDate;
  bool isLowStock;      // stock <= reorderThreshold → màu vàng
  bool isCriticalStock; // stock <= minimumStock     → màu đỏ
  bool isExpiringSoon;  // còn < 30 ngày hết hạn     → màu cam
  bool isActive;
  QList<MedicationIngredientDTO> ingredients;
};