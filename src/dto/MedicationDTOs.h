#pragma once

#include <QDate>
#include <QList>
#include <QString>
#include <QVariant>


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