#include "MedicationRepository.h"
#include "DatabaseManager.h"
#include <QSqlError>

bool MedicationRepository::insertMedicationBase(const MedicationInputDTO& medication, int& outMedicationId) {
    QString sql = R"(
        INSERT INTO medications (
            brand_name,
            category,
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
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 1)
    )";

    QVariantList params = {
        medication.brandName,
        medication.category,
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
        qWarning() << "MedicationRepository::insertParentMedication - Lỗi:" << query.lastError().text();
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

bool MedicationRepository::insert(const MedicationInputDTO& medication) {
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

    if (!dbManager.commitTransaction()) {
        return false;
    }

    return true;
}

bool MedicationRepository::update(int medicationId, const MedicationInputDTO& dto) {
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;

    QString sql = R"(
        UPDATE medications SET
            brand_name        = ?,
            category          = ?,
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

    // ✅ Ghi NULL thật sự khi expiryDate không hợp lệ — tránh lưu "" (empty string)
    // gây sai lệch khi query "WHERE expiry_date IS NOT NULL" ở findExpiringBefore()
    QVariant expiryValue = dto.expiryDate.isValid()
        ? QVariant(dto.expiryDate.toString("yyyy-MM-dd"))
        : QVariant(QMetaType::fromType<QString>()); // NULL

    QVariantList params = {
        dto.brandName, dto.category, dto.unit,
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

    QSqlQuery delQuery = db.executeQuery(
        "DELETE FROM medication_ingredients WHERE medication_id = ?",
        { medicationId }
    );
    if (!delQuery.isActive()) {
        db.rollbackTransaction();
        qWarning() << "MedicationRepository::update - Lỗi xóa hoạt chất cũ:"
                   << delQuery.lastError().text();
        return false;
    }

    if (!insertMedicationIngredients(medicationId, dto.ingredients)) {
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
        query.value("category").toString(),
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

std::shared_ptr<Medication> MedicationRepository::findById(int medicationId) const {
    QString sql = R"(
        SELECT 
            medication_id,
            brand_name,
            category,
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


QList<std::shared_ptr<Medication>> MedicationRepository::search(const MedicationSearchCriteria& criteria) const {
    QList<std::shared_ptr<Medication>> result;
    
    QString sql = R"(
        SELECT DISTINCT m.medication_id, m.brand_name, m.category, m.unit, m.unit_price, 
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

    if (!criteria.category.isEmpty()) {
        sql += " AND m.category = ?";
        params.append(criteria.category.trimmed());
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
        sql += " AND m.manufacturer LIKE ?";
        QString pattern = "%" + criteria.manufacturer.trimmed() + "%";
        params.append(pattern);
    }

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
    while (query.next()) {
        result.append(mapRowToMedication(query));
    }
    
    return result;
}



QList<std::shared_ptr<Medication>> MedicationRepository::findLowStock() const {
    QList<std::shared_ptr<Medication>> result;
    QString sql = R"(
        SELECT medication_id, brand_name, category, unit, unit_price, stock_quantity,
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
        SELECT medication_id, brand_name, category, unit, unit_price, stock_quantity,
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



