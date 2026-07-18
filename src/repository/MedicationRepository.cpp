#include "MedicationRepository.h"
#include "DatabaseManager.h"
#include <QSqlError>

bool MedicationRepository::insertMedicationBase(const MedicationInputDTO& medication,
                                                int& outMedicationId) {
    QString sql = R"(
        INSERT INTO medications (
            brand_name,
            unit,
            unit_price,
            stock_quantity,
            minimum_stock,
            reorder_threshold,
            description,
            manufacturer,
            expiry_date,
            is_active
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 1)
    )";

    QVariantList params = {
        medication.brandName,
        medication.unit,
        medication.unitPrice,
        medication.stockQuantity,
        medication.minimumStock,
        medication.reorderThreshold,
        medication.description,
        medication.manufacturer,
        medication.expiryDate.toString("yyyy-MM-dd")
    };

    QSqlQuery query = DatabaseManager::getInstance().executeQuery(sql, params);
    if (!query.isActive()) {
        qWarning() << "MedicationRepository::insertMedicationBase - Lỗi:"
                   << query.lastError().text();
        return false;
    }

    outMedicationId = query.lastInsertId().toInt();
    return true;
}

bool MedicationRepository::insertMedicationIngredients(int medicationId, const QList<MedicationInputDTO::IngredientInput>& ingredients) {
    QString sql = R"(
        INSERT INTO medication_ingredients (
            medication_id,
            ingredient_id,
            strength
        )
        VALUES (?, ?, ?)
    )";

    DatabaseManager& db = DatabaseManager::getInstance();

    for (const auto& ing : ingredients) {
        QVariantList params = {
            medicationId,
            ing.ingredientId,
            ing.strength
        };

        QSqlQuery query = db.executeQuery(sql, params);
        if (!query.isActive()) {
            qWarning() << "MedicationRepository::insertMedicationIngredients - Lỗi hoạt chất ID" 
                       << ing.ingredientId << ":" << query.lastError().text();
            return false;
        }
    }

    return true;
}

bool MedicationRepository::insertMedicationCategories(int medicationId,
                                                     const QList<QString>& categories) {
    if (categories.isEmpty()) return true; // không có danh mục nào — không phải lỗi

    QString sql = R"(
        INSERT INTO medication_categories (medication_id, category_name)
        VALUES (?, ?)
    )";

    DatabaseManager& db = DatabaseManager::getInstance();
    for (const QString& cat : categories) {
        QVariantList params = { medicationId, cat };
        QSqlQuery query = db.executeQuery(sql, params);
        if (!query.isActive()) {
            qWarning() << "MedicationRepository::insertMedicationCategories - Lỗi danh mục"
                       << cat << ":" << query.lastError().text();
            return false;
        }
    }
    return true;
}

bool MedicationRepository::insertMedication(const MedicationInputDTO& medication) {
    DatabaseManager& dbManager = DatabaseManager::getInstance();

    if (!dbManager.beginTransaction()) {
        return false;
    }

    int medicationId = 0;
    if (!insertMedicationBase(medication, medicationId)) {
        dbManager.rollbackTransaction();
        return false;
    }

    if (!insertMedicationIngredients(medicationId, medication.ingredients)) {
        dbManager.rollbackTransaction();
        return false;
    }

    if (!insertMedicationCategories(medicationId, medication.categories)) {
        dbManager.rollbackTransaction();
        return false;
    }

    if (!dbManager.commitTransaction()) {
        return false;
    }

    return true;
}

bool MedicationRepository::updateMedication(int medicationId, const MedicationInputDTO& dto) {
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;

    QString sql = R"(
        UPDATE medications SET
            brand_name        = ?,
            unit              = ?,
            unit_price        = ?,
            stock_quantity    = ?,
            minimum_stock     = ?,
            reorder_threshold = ?,
            expiry_date       = ?,
            manufacturer      = ?,
            description       = ?
        WHERE medication_id = ?
    )";

    QVariant expiryValue = dto.expiryDate.isValid()
        ? QVariant(dto.expiryDate.toString("yyyy-MM-dd"))
        : QVariant(QMetaType::fromType<QString>()); // NULL

    QVariantList params = {
        dto.brandName, dto.unit,
        dto.unitPrice, dto.stockQuantity, dto.minimumStock, dto.reorderThreshold,
        expiryValue, dto.manufacturer, dto.description,
        medicationId
    };

    QSqlQuery query = db.executeQuery(sql, params);
    if (!query.isActive()) {
        db.rollbackTransaction();
        qWarning() << "MedicationRepository::update - Lỗi cập nhật bảng medications:"
                   << query.lastError().text();
        return false;
    }

    // Xóa hoạt chất cũ và INSERT lại
    QSqlQuery delIngQuery = db.executeQuery(
        "DELETE FROM medication_ingredients WHERE medication_id = ?",
        { medicationId }
    );
    if (!delIngQuery.isActive()) {
        db.rollbackTransaction();
        qWarning() << "MedicationRepository::update - Lỗi xóa hoạt chất cũ:"
                   << delIngQuery.lastError().text();
        return false;
    }
    if (!insertMedicationIngredients(medicationId, dto.ingredients)) {
        db.rollbackTransaction();
        return false;
    }

    // Xóa danh mục cũ và INSERT lại (cùng pattern với hoạt chất)
    QSqlQuery delCatQuery = db.executeQuery(
        "DELETE FROM medication_categories WHERE medication_id = ?",
        { medicationId }
    );
    if (!delCatQuery.isActive()) {
        db.rollbackTransaction();
        qWarning() << "MedicationRepository::update - Lỗi xóa danh mục cũ:"
                   << delCatQuery.lastError().text();
        return false;
    }
    if (!insertMedicationCategories(medicationId, dto.categories)) {
        db.rollbackTransaction();
        return false;
    }

    return db.commitTransaction();
}

bool MedicationRepository::deactivate(int medicationId) {
    QString sql = R"(
        UPDATE medications
        SET is_active = 0
        WHERE medication_id = ?
    )";

    QSqlQuery query = DatabaseManager::getInstance().executeQuery(sql, { medicationId });

    if (!query.isActive()) {
        qWarning() << "MedicationRepository::deactivate - Lỗi thực thi SQL cho ID:" << medicationId;
        return false;
    }
    return true;
}

bool MedicationRepository::reactivate(int medicationId) {
    QString sql = R"(
        UPDATE medications
        SET is_active = 1
        WHERE medication_id = ?
    )";

    QSqlQuery query = DatabaseManager::getInstance().executeQuery(sql, { medicationId });

    if (!query.isActive()) {
        qWarning() << "MedicationRepository::deactivate - Lỗi thực thi SQL cho ID:" << medicationId;
        return false;
    }
    return true;
}


QList<QString> MedicationRepository::getCategoriesForMedication(int medicationId) const {
    QList<QString> list;
    QString sql = R"(
        SELECT category_name
        FROM medication_categories
        WHERE medication_id = ?
        ORDER BY category_name ASC
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { medicationId });
    while (query.next()) {
        list.append(query.value("category_name").toString());
    }
    return list;
}

QList<MedicationIngredientDTO> MedicationRepository::getIngredientsForMedication(int medicationId) const {
    QList<MedicationIngredientDTO> list;
    QString sql = R"(
        SELECT ai.ingredient_id, ai.ingredient_name, mi.strength
        FROM medication_ingredients mi
        INNER JOIN active_ingredients ai ON mi.ingredient_id = ai.ingredient_id
        WHERE mi.medication_id = ?
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { medicationId });
    while (query.next()) {
        MedicationIngredientDTO dto;
        dto.ingredientId = query.value("ingredient_id").toInt();
        dto.ingredientName = query.value("ingredient_name").toString();
        dto.strength = query.value("strength").toString();
        list.append(dto);
    }
    return list;
}

std::shared_ptr<Medication> MedicationRepository::mapRowToMedication(const QSqlQuery& query) const {
    int medicationId = query.value("medication_id").toInt();
    return std::make_shared<Medication>(
        medicationId,
        query.value("brand_name").toString(),
        getIngredientsForMedication(medicationId),
        getCategoriesForMedication(medicationId),  // thay thế query.value("category")
        query.value("unit").toString(),
        query.value("unit_price").toDouble(),
        query.value("stock_quantity").toInt(),
        query.value("minimum_stock").toInt(),
        query.value("reorder_threshold").toInt(),
        QDate::fromString(query.value("expiry_date").toString(), "yyyy-MM-dd"),
        query.value("manufacturer").toString(),
        query.value("description").toString(),
        query.value("is_active").toBool()
    );
}

ActiveIngredientDTO MedicationRepository::mapRowToIngredient(const QSqlQuery& query) const {
    ActiveIngredientDTO dto;
    dto.ingredientId = query.value("ingredient_id").toInt();
    dto.ingredientName = query.value("ingredient_name").toString();
    dto.description = query.value("description").toString();
    return dto;
}


std::shared_ptr<Medication> MedicationRepository::findById(int medicationId) const {
    QString sql = R"(
        SELECT 
            medication_id,
            brand_name,
            unit,
            unit_price,
            stock_quantity,
            minimum_stock,
            reorder_threshold,
            expiry_date,
            manufacturer,
            description,
            is_active
        FROM medications
        WHERE medication_id = ? AND is_active = 1
    )";
 
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { medicationId });
    if (!query.next()) {
        return nullptr;
    }
    return mapRowToMedication(query);
}


QList<std::shared_ptr<Medication>> MedicationRepository::searchMedications(const MedicationSearchCriteria& criteria) const {
    QList<std::shared_ptr<Medication>> result;
    
    QString sql = R"(
        SELECT DISTINCT m.medication_id, m.brand_name, m.unit, m.unit_price,
                        m.stock_quantity, m.minimum_stock, m.reorder_threshold, m.expiry_date,
                        m.manufacturer, m.description, m.is_active
        FROM medications m
        LEFT JOIN medication_ingredients mi ON m.medication_id = mi.medication_id
        LEFT JOIN active_ingredients ai ON mi.ingredient_id = ai.ingredient_id
        WHERE m.is_active = 1
    )";
    
    QVariantList params;

    if (!criteria.keyword.isEmpty()) {
        sql += " AND (LOWER(m.brand_name) LIKE ? OR LOWER(ai.ingredient_name) LIKE ?)";
        QString pattern = "%" + criteria.keyword.toLower() + "%";
        params.append(pattern);
        params.append(pattern);
    }

    if (!criteria.selectedIngredientIds.isEmpty()) {
        QStringList placeholders;
        for (int id : criteria.selectedIngredientIds) {
            placeholders.append("?");
            params.append(id);
        }
        sql += QString(" AND mi.ingredient_id IN (%1)").arg(placeholders.join(","));
    } 

    if (!criteria.selectedCategories.isEmpty()) {
        QStringList placeholders;
        for (const QString& cat : criteria.selectedCategories) {
            placeholders.append("?");
            params.append(cat);
        }
        // EXISTS đảm bảo thuốc khớp ít nhất 1 danh mục trong selectedCategories
        sql += QString(
            " AND EXISTS ("
            "   SELECT 1 FROM medication_categories mc"
            "   WHERE mc.medication_id = m.medication_id"
            "   AND mc.category_name IN (%1)"
            ")"
        ).arg(placeholders.join(","));
    }

    if (criteria.inStockOnly) {
        sql += " AND m.stock_quantity > 0";
    }

    if (criteria.excludeExpired) {
        sql += " AND (m.expiry_date IS NULL OR m.expiry_date >= ?)";
        params.append(QDate::currentDate().toString("yyyy-MM-dd"));
    }

    if (criteria.criticalStockOnly) {
        sql += " AND m.stock_quantity <= m.minimum_stock";
    } 
    else if (criteria.lowStockOnly) {
        sql += " AND m.stock_quantity <= m.reorder_threshold AND m.stock_quantity > m.minimum_stock";
    }

    if (!criteria.maxUnitPrice.isNull() && criteria.maxUnitPrice.isValid()) {
        sql += " AND m.unit_price <= ?";
        params.append(criteria.maxUnitPrice.toDouble());
    }

    if (!criteria.manufacturer.isEmpty()) {
        sql += " AND LOWER(m.manufacturer) LIKE ?";
        QString pattern = "%" + criteria.manufacturer.toLower() + "%";
        params.append(pattern);
    }

    sql += " ORDER BY m.brand_name ASC";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
    while (query.next()) {
        result.append(mapRowToMedication(query));
    }
    
    return result;
}



QList<std::shared_ptr<Medication>> MedicationRepository::findLowStock() const {
    QList<std::shared_ptr<Medication>> result;
    QString sql = R"(
        SELECT medication_id, brand_name, unit, unit_price, stock_quantity,
               minimum_stock, reorder_threshold, expiry_date, manufacturer, description, is_active
        FROM medications 
        WHERE is_active = 1 AND stock_quantity <= reorder_threshold
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {});
    while (query.next()) {
        result.append(mapRowToMedication(query));
    }
    return result;
}

QList<std::shared_ptr<Medication>> MedicationRepository::findExpiringBefore(const QDate& date) const {
    QList<std::shared_ptr<Medication>> result;
    QString sql = R"(
        SELECT medication_id, brand_name, unit, unit_price, stock_quantity,
               minimum_stock, reorder_threshold, expiry_date, manufacturer, description, is_active
        FROM medications 
        WHERE is_active = 1 
          AND expiry_date IS NOT NULL 
          AND expiry_date <= ?
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { date.toString("yyyy-MM-dd") });
    while (query.next()) {
        result.append(mapRowToMedication(query));
    }
    return result;
}


QList<ActiveIngredientDTO> MedicationRepository::searchIngredients(const QString& keyword) const {
    QString sql = R"(
        SELECT ingredient_id, ingredient_name, description
        FROM active_ingredients
        WHERE LOWER(ingredient_name) LIKE ?
        ORDER BY ingredient_name ASC
    )";

    QString pattern = "%" + keyword.toLower() + "%";
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { pattern });
    QList<ActiveIngredientDTO> result;

    while (query.next()) {
        result.append(mapRowToIngredient(query));
    }
    
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// PHÂN TRANG
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @internal
 * @brief Xây dựng phần WHERE và danh sách params cho Medication search
 *        để tái sử dụng cho cả query COUNT và query SELECT.
 *
 *        Chiến lược tách riêng WHERE: tránh lặp code giữa hàm cũ và hàm phân trang.
 *        Caller ghép vào sau FROM để tạo hoàn chỉnh 2 câu SQL khác nhau.
 *
 * @param criteria  Tiêu chí tìm kiếm (đã được normalize bởi Service).
 * @param outParams [out] Danh sách tham số bind cho QSqlQuery.
 * @return          Chuỗi WHERE hoàn chỉnh (bắt đầu bằng "WHERE m.is_active = 1").
 */
static QString buildMedicationWhereClause(const MedicationSearchCriteria& criteria,
                                          QVariantList& outParams) {
    QString where = " WHERE m.is_active = 1";

    if (!criteria.keyword.isEmpty()) {
        where += " AND (LOWER(m.brand_name) LIKE ? OR LOWER(ai.ingredient_name) LIKE ?)";
        QString pattern = "%" + criteria.keyword.toLower() + "%";
        outParams.append(pattern);
        outParams.append(pattern);
    }

    if (!criteria.selectedIngredientIds.isEmpty()) {
        QStringList placeholders;
        for (int id : criteria.selectedIngredientIds) {
            placeholders.append("?");
            outParams.append(id);
        }
        where += QString(" AND mi.ingredient_id IN (%1)").arg(placeholders.join(","));
    }

    if (!criteria.selectedCategories.isEmpty()) {
        QStringList placeholders;
        for (const QString& cat : criteria.selectedCategories) {
            placeholders.append("?");
            outParams.append(cat);
        }
        where += QString(
            " AND EXISTS ("
            "   SELECT 1 FROM medication_categories mc"
            "   WHERE mc.medication_id = m.medication_id"
            "   AND mc.category_name IN (%1)"
            ")"
        ).arg(placeholders.join(","));
    }

    if (criteria.inStockOnly) {
        where += " AND m.stock_quantity > 0";
    }

    if (criteria.excludeExpired) {
        where += " AND (m.expiry_date IS NULL OR m.expiry_date >= ?)";
        outParams.append(QDate::currentDate().toString("yyyy-MM-dd"));
    }

    if (criteria.criticalStockOnly) {
        where += " AND m.stock_quantity <= m.minimum_stock";
    } else if (criteria.lowStockOnly) {
        where += " AND m.stock_quantity <= m.reorder_threshold AND m.stock_quantity > m.minimum_stock";
    }

    if (!criteria.maxUnitPrice.isNull() && criteria.maxUnitPrice.isValid()) {
        where += " AND m.unit_price <= ?";
        outParams.append(criteria.maxUnitPrice.toDouble());
    }

    if (!criteria.manufacturer.isEmpty()) {
        where += " AND LOWER(m.manufacturer) LIKE ?";
        outParams.append("%" + criteria.manufacturer.toLower() + "%");
    }

    return where;
}


PagedResult<std::shared_ptr<Medication>>
MedicationRepository::searchMedicationsPaged(const MedicationSearchCriteria& criteria) const {
    PagedResult<std::shared_ptr<Medication>> result;
    result.page     = qMax(1, criteria.page);
    result.pageSize = criteria.pageSize;

    // ── Xây dựng phần FROM + JOIN dùng chung ─────────────────────────────
    const QString fromClause = R"(
        FROM medications m
        LEFT JOIN medication_ingredients mi ON m.medication_id = mi.medication_id
        LEFT JOIN active_ingredients     ai ON mi.ingredient_id = ai.ingredient_id
    )";

    // ── Bước 1: Đếm tổng bản ghi khớp (không phân trang) ─────────────────
    QVariantList countParams;
    QString whereClause = buildMedicationWhereClause(criteria, countParams);

    QString countSql = "SELECT COUNT(DISTINCT m.medication_id)" + fromClause + whereClause;

    QSqlQuery countQuery = DatabaseManager::getInstance().selectQuery(countSql, countParams);
    if (!countQuery.next()) {
        qWarning() << "MedicationRepository::searchMedicationsPaged - Lỗi đếm tổng bản ghi";
        result.totalCount = 0;
        return result;
    }
    result.totalCount = countQuery.value(0).toInt();

    // ── Bước 2: Lấy dữ liệu trang hiện tại ──────────────────────────────
    QVariantList dataParams;
    whereClause = buildMedicationWhereClause(criteria, dataParams); // build lại params mới

    QString dataSql = R"(
        SELECT DISTINCT m.medication_id, m.brand_name, m.unit, m.unit_price,
                        m.stock_quantity, m.minimum_stock, m.reorder_threshold,
                        m.expiry_date, m.manufacturer, m.description, m.is_active
    )" + fromClause + whereClause + " ORDER BY m.brand_name ASC";

    if (criteria.pageSize > 0) {
        // Phân trang: tính OFFSET từ page 1-indexed
        int offset = (result.page - 1) * criteria.pageSize;
        dataSql += " LIMIT ? OFFSET ?";
        dataParams.append(criteria.pageSize);
        dataParams.append(offset);
    }
    // pageSize == 0 → không thêm LIMIT/OFFSET → trả về tất cả (dùng nội bộ)

    QSqlQuery dataQuery = DatabaseManager::getInstance().selectQuery(dataSql, dataParams);
    while (dataQuery.next()) {
        result.items.append(mapRowToMedication(dataQuery));
    }

    return result;
}


PagedResult<ActiveIngredientDTO>
MedicationRepository::searchIngredientsPaged(const IngredientSearchCriteria& criteria) const {
    PagedResult<ActiveIngredientDTO> result;
    result.page     = qMax(1, criteria.page);
    result.pageSize = criteria.pageSize;

    // Chuẩn bị pattern LIKE
    const bool hasKeyword  = !criteria.keyword.trimmed().isEmpty();
    const QString pattern  = "%" + criteria.keyword.toLower() + "%";

    // ── Bước 1: Đếm tổng bản ghi ─────────────────────────────────────────
    QString countSql = "SELECT COUNT(*) FROM active_ingredients";
    QVariantList countParams;
    if (hasKeyword) {
        countSql += " WHERE LOWER(ingredient_name) LIKE ?";
        countParams.append(pattern);
    }

    QSqlQuery countQuery = DatabaseManager::getInstance().selectQuery(countSql, countParams);
    if (!countQuery.next()) {
        qWarning() << "MedicationRepository::searchIngredientsPaged - Lỗi đếm tổng bản ghi";
        result.totalCount = 0;
        return result;
    }
    result.totalCount = countQuery.value(0).toInt();

    // ── Bước 2: Lấy dữ liệu trang hiện tại ──────────────────────────────
    QString dataSql = R"(
        SELECT ingredient_id, ingredient_name, description
        FROM active_ingredients
    )";
    QVariantList dataParams;
    if (hasKeyword) {
        dataSql += " WHERE LOWER(ingredient_name) LIKE ?";
        dataParams.append(pattern);
    }
    dataSql += " ORDER BY ingredient_name ASC";

    if (criteria.pageSize > 0) {
        int offset = (result.page - 1) * criteria.pageSize;
        dataSql += " LIMIT ? OFFSET ?";
        dataParams.append(criteria.pageSize);
        dataParams.append(offset);
    }

    QSqlQuery dataQuery = DatabaseManager::getInstance().selectQuery(dataSql, dataParams);
    while (dataQuery.next()) {
        result.items.append(mapRowToIngredient(dataQuery));
    }

    return result;
}
