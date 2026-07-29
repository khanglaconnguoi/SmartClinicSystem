#include "PrescriptionRepository.h"
#include <QSqlError>

bool PrescriptionRepository::insertHeader(const PrescriptionInputDTO& prescription, int& prescriptionId) {
    DatabaseManager& db = DatabaseManager::getInstance();

    QString insert = R"(
        INSERT INTO prescriptions (
            record_id,
            doctor_id,
            notes
        )
        VALUES (?, ?, ?)
    )";

    QVariantList params = {
        prescription.recordId,
        prescription.doctorId,
        prescription.notes
    };

    QSqlQuery query = db.executeQuery(insert, params);

    if (!query.isActive()) { 
        qWarning() << "PrescriptionRepository::insertPrescription - Lỗi ghi bảng prescription";
        return false; 
    }

    prescriptionId = query.lastInsertId().toInt();

    return true;
}

bool PrescriptionRepository::insertItems(int prescriptionId, const QList<PrescriptionItemDTO>& items) {
    DatabaseManager& db = DatabaseManager::getInstance();

    for(auto item: items) {
        QString insertItems = R"(
            INSERT INTO prescription_items (
                prescription_id,
                medication_id,
                brand_name,
                unit_price,
                quantity,
                dosage,
                frequency,
                duration_days,
                note
            )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        )";

        QVariantList params = {
            prescriptionId,
            item.medicationId,
            item.brandName,
            item.unitPrice,
            item.quantity,
            item.dosage,
            item.frequency,
            item.durationDays,
            item.note
        };


        if (!db.executeQuery(insertItems, params).isActive()) { 
            qWarning() << "PrescriptionRepository::insertPrescription - Lỗi ghi bảng prescription_items";
            return false; 
        }
    }

    return true;
}


bool PrescriptionRepository::insert(const PrescriptionInputDTO& prescription) {
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;

    int prescriptionId = 0;
    if (!insertHeader(prescription, prescriptionId)){
        db.rollbackTransaction();
        return false;
    }
    if (!insertItems(prescriptionId, prescription.items)){
        db.rollbackTransaction();
        return false;
    }
    if (!db.commitTransaction()) return false;
    return true;
}


bool PrescriptionRepository::cancel(int prescriptionId, int cancelledBy, const QString& reason) {
    QString sql = R"(
        UPDATE prescriptions
        SET status        = 'CANCELLED',
            cancelled_by  = ?,
            cancelled_at  = datetime('now', 'localtime'),
            cancel_reason = ?
        WHERE prescription_id = ?
          AND status != 'DISPENSED'
    )";

    DatabaseManager& db = DatabaseManager::getInstance();
    

    QSqlQuery query = db.executeQuery(sql, { cancelledBy, reason, prescriptionId });

    if (!query.isActive()) {
        qCritical() << "PrescriptionRepository::cancel - Lỗi thực thi SQL:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "PrescriptionRepository::cancel - "
                      "Đơn thuốc đã DISPENSED hoặc không tồn tại, ID:" << prescriptionId;
        return false;
    }

    return true;
}

bool PrescriptionRepository::dispense(int prescriptionId, int dispensedByStaffId) {
    QString sql = R"(
        UPDATE prescriptions
        SET status       = 'DISPENSED',
            dispensed_by = ?,
            dispensed_at = datetime('now', 'localtime')
        WHERE prescription_id = ?
          AND status = 'PENDING' 
    )";

    DatabaseManager& db = DatabaseManager::getInstance();
    
    QSqlQuery query = db.executeQuery(sql, { dispensedByStaffId, prescriptionId });

    if (!query.isActive()) {
        qCritical() << "PrescriptionRepository::dispense - Lỗi thực thi SQL:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "PrescriptionRepository::dispense - "
                      "Đơn thuốc không ở trạng thái hợp lệ hoặc không tồn tại, ID:" << prescriptionId;
        return false;
    }

    return true;
}




PrescriptionResultDTO PrescriptionRepository::mapRowToPrescriptionHeader(const QSqlQuery& query) const {
    PrescriptionResultDTO dto;
    dto.prescriptionId = query.value("prescription_id").toInt();
    dto.recordId       = query.value("record_id").toInt();
    dto.doctorId       = query.value("doctor_id").toInt();
    dto.doctorCode     = query.value("doctor_code").toString();
    dto.doctorName     = query.value("doctor_name").toString();
    dto.status         = prescriptionStatusFromEn(query.value("status").toString());
    dto.notes          = query.value("notes").toString();
    dto.prescribedAt   = query.value("prescribed_at").toDateTime();
    dto.totalAmount    = 0.0; // Khởi tạo để tính toán sau

    if (!query.value("dispensed_by").isNull()) {
        PrescriptionActionInfoDTO info;
        info.staffId   = query.value("dispensed_by").toInt();
        info.staffCode = query.value("dispensed_by_code").toString();
        info.staffName = query.value("dispensed_by_name").toString();
        info.actionAt  = query.value("dispensed_at").toDateTime();
        
        dto.dispensedInfo = info;
    } else {
        dto.dispensedInfo = std::nullopt;
    }


    if (!query.value("cancelled_by").isNull()) {
        PrescriptionActionInfoDTO info;
        info.staffId   = query.value("cancelled_by").toInt();
        info.staffCode = query.value("cancelled_by_code").toString();
        info.staffName = query.value("cancelled_by_name").toString();
        info.actionAt  = query.value("cancelled_at").toDateTime();
        info.reason  = query.value("cancel_reason").toString();
        
        dto.cancelledInfo = info;
    } else {
        dto.cancelledInfo = std::nullopt;
    }

    return dto;
}

PrescriptionItemDTO PrescriptionRepository::mapRowToPrescriptionItem(const QSqlQuery& query) const {
    PrescriptionItemDTO item;
    item.medicationId   = query.value("medication_id").toInt();
    item.brandName      = query.value("brand_name").toString();
    item.unitPrice      = query.value("unit_price").toDouble();
    item.quantity       = query.value("quantity").toInt();
    item.dosage         = query.value("dosage").toString();
    item.frequency      = query.value("frequency").toString();
    item.durationDays   = query.value("duration_days").toInt();
    item.note           = query.value("item_note").toString();
    return item;
}


static const QString SELECT_PRESCRIPTION_SQL = R"(
    SELECT 
        p.prescription_id, 
        p.record_id, 
        p.doctor_id, 
        d.staff_code AS doctor_code,
        d.full_name AS doctor_name, 
        p.status, 
        p.notes, 
        p.prescribed_at,
        p.dispensed_by,
        s_disp.staff_code AS dispensed_by_code,
        s_disp.full_name AS dispensed_by_name,
        p.dispensed_at,
        p.cancelled_by,
        s_canc.staff_code AS cancelled_by_code,
        s_canc.full_name AS cancelled_by_name,
        p.cancelled_at,
        p.cancel_reason,
        pi.medication_id, 
        pi.brand_name, 
        pi.unit_price, 
        pi.quantity, 
        pi.dosage, 
        pi.frequency, 
        pi.duration_days, 
        pi.note AS item_note
    FROM prescriptions p
    INNER JOIN staffs d ON p.doctor_id = d.staff_id
    INNER JOIN prescription_items pi ON p.prescription_id = pi.prescription_id
    LEFT JOIN staffs s_disp ON p.dispensed_by = s_disp.staff_id
    LEFT JOIN staffs s_canc ON p.cancelled_by = s_canc.staff_id
    WHERE 1=1
)";

QList<PrescriptionResultDTO> PrescriptionRepository::search(const PrescriptionSearchCriteria& criteria) const {
    DatabaseManager& db = DatabaseManager::getInstance();
    QList<PrescriptionResultDTO> results;
    QHash<int, int> idToIdxMap;

    QString sql = SELECT_PRESCRIPTION_SQL;
    QVariantList params;

    // --- NỐI ĐIỀU KIỆN ĐỘNG (Dynamic Filters) ---
    if (criteria.recordId > 0) {
        sql += " AND p.record_id = ?";
        params.append(criteria.recordId);
    }
    if (!criteria.status.isEmpty()) {
        sql += " AND p.status = ?";
        params.append(criteria.status);
    }
    if (criteria.fromDate.isValid()) {
        sql += " AND p.prescribed_at >= ?";
        params.append(criteria.fromDate.toString("yyyy-MM-dd HH:mm:ss"));
    }
    if (criteria.toDate.isValid()) {
        sql += " AND p.prescribed_at <= ?";
        params.append(criteria.toDate.toString("yyyy-MM-dd HH:mm:ss"));
    }

    sql += " ORDER BY p.prescribed_at DESC, p.prescription_id ASC";

    QSqlQuery query = db.executeQuery(sql, params);
    if (!query.isActive()) return results;

    // --- THUẬT TOÁN GOM NHÓM DỮ LIỆU (Grouping Flat Table) ---
    while (query.next()) {
        int pId = query.value("prescription_id").toInt();

        // 1. Nếu đơn thuốc chưa tồn tại trong danh sách kết quả -> Tạo mới phần thông tin chung
        if (!idToIdxMap.contains(pId)) {
            PrescriptionResultDTO dto;
            dto.prescriptionId   = pId;
            dto.recordId         = query.value("record_id").toInt();
            dto.doctorId         = query.value("doctor_id").toInt();
            dto.doctorCode       = query.value("doctor_code").toString();
            dto.doctorName       = query.value("doctor_name").toString();
            dto.status           = prescriptionStatusFromEn(query.value("status").toString());
            dto.notes            = query.value("notes").toString();
            dto.prescribedAt     = query.value("prescribed_at").toDateTime();
            dto.totalAmount      = 0.0;

            // Nạp thông tin phát thuốc dựa theo cấu trúc PrescriptionActionInfoDTO mới
            if (!query.value("dispensed_by").isNull()) {
                PrescriptionActionInfoDTO dispInfo;
                dispInfo.staffId   = query.value("dispensed_by").toInt();
                dispInfo.staffCode = query.value("dispensed_by_code").toString();
                dispInfo.staffName = query.value("dispensed_by_name").toString();
                dispInfo.actionAt  = query.value("dispensed_at").toDateTime();
                dispInfo.reason    = ""; // Phát thuốc không cần lý do
                dto.dispensedInfo  = dispInfo;
            }

            // Nạp thông tin hủy đơn dựa theo cấu trúc PrescriptionActionInfoDTO mới
            if (!query.value("cancelled_by").isNull()) {
                PrescriptionActionInfoDTO cancInfo;
                cancInfo.staffId   = query.value("cancelled_by").toInt();
                cancInfo.staffCode = query.value("cancelled_by_code").toString();
                cancInfo.staffName = query.value("cancelled_by_name").toString();
                cancInfo.actionAt  = query.value("cancelled_at").toDateTime();
                cancInfo.reason    = query.value("cancel_reason").toString(); // Lý do hủy đơn
                dto.cancelledInfo  = cancInfo;
            }

            results.append(dto);
            idToIdxMap.insert(pId, results.size() - 1);
        }

        // 2. Trích xuất vị trí index và nạp tiếp mảng danh sách thuốc (Items) đi kèm
        int targetIdx = idToIdxMap.value(pId);
        
        PrescriptionItemDTO item;
        item.medicationId = query.value("medication_id").toInt();
        item.brandName    = query.value("brand_name").toString();
        item.unitPrice    = query.value("unit_price").toDouble();
        item.quantity     = query.value("quantity").toInt();
        item.dosage       = query.value("dosage").toString();
        item.frequency    = query.value("frequency").toString();
        item.durationDays = query.value("duration_days").toInt();
        item.note         = query.value("item_note").toString();

        // Tích lũy tổng tiền và đẩy item vào đơn thuốc đích
        results[targetIdx].totalAmount += (item.quantity * item.unitPrice);
        results[targetIdx].items.append(item);
    }

    return results;
}


std::optional<PrescriptionResultDTO> PrescriptionRepository::findById(int prescriptionId) const {
    QString sql = SELECT_PRESCRIPTION_SQL + " AND p.prescription_id = ?";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { prescriptionId });
    
    if (!query.next()) return std::nullopt;

    PrescriptionResultDTO dto = mapRowToPrescriptionHeader(query);

    do {
        if (!query.value("medication_id").isNull()) {
            PrescriptionItemDTO item = mapRowToPrescriptionItem(query);

            dto.totalAmount += item.unitPrice * item.quantity;
            
            dto.items.append(item);
        }
    } while (query.next());

    return dto;
}

std::optional<PrescriptionResultDTO> PrescriptionRepository::findByRecordId(int recordId) const {
    QString sql = SELECT_PRESCRIPTION_SQL + " AND p.record_id = ?";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { recordId });
    
    if (!query.next()) return std::nullopt;

    PrescriptionResultDTO dto = mapRowToPrescriptionHeader(query);

    do {
        if (!query.value("medication_id").isNull()) {
            PrescriptionItemDTO item = mapRowToPrescriptionItem(query);
            
            dto.totalAmount += item.unitPrice * item.quantity;
            
            dto.items.append(item);
        }
    } while (query.next());

    return dto;
}

QList<PrescriptionResultDTO> PrescriptionRepository::findByPatientId(int patientId) const {
    QString sql = R"(
        SELECT 
            p.prescription_id, p.record_id, p.doctor_id, d.staff_code AS doctor_code, d.full_name AS doctor_name, 
            p.status, p.notes, p.prescribed_at, p.dispensed_by, s_disp.staff_code AS dispensed_by_code,
            s_disp.full_name AS dispensed_by_name, p.dispensed_at, p.cancelled_by, s_canc.staff_code AS cancelled_by_code,
            s_canc.full_name AS cancelled_by_name, p.cancelled_at, p.cancel_reason,
            pi.medication_id, pi.brand_name, pi.unit_price, pi.quantity, pi.dosage, pi.frequency, pi.duration_days, pi.note AS item_note
        FROM prescriptions p
        INNER JOIN medical_records mr ON p.record_id = mr.record_id    -- ✅ Đưa phép JOIN lên trước WHERE
        INNER JOIN staffs d ON p.doctor_id = d.staff_id
        INNER JOIN prescription_items pi ON p.prescription_id = pi.prescription_id
        LEFT JOIN staffs s_disp ON p.dispensed_by = s_disp.staff_id
        LEFT JOIN staffs s_canc ON p.cancelled_by = s_canc.staff_id
        WHERE mr.patient_id = ?
        ORDER BY p.prescribed_at DESC, p.prescription_id ASC
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { patientId });
    
    QHash<int, PrescriptionResultDTO> prescriptionHash;
    QList<int> orderedIds;

    while (query.next()) {
        int pId = query.value("prescription_id").toInt();

        if (!prescriptionHash.contains(pId)) {
            prescriptionHash[pId] = mapRowToPrescriptionHeader(query);
            orderedIds.append(pId);
        }

        if (!query.value("medication_id").isNull()) {
            PrescriptionItemDTO item = mapRowToPrescriptionItem(query);
            prescriptionHash[pId].totalAmount += item.unitPrice * item.quantity;
            prescriptionHash[pId].items.append(item);
        }
    }

    QList<PrescriptionResultDTO> results;
    results.reserve(orderedIds.size());
    for (int id : orderedIds) {
        results.append(prescriptionHash.value(id));
    }

    return results;
}

std::optional<PrescriptionStatus> PrescriptionRepository::getStatus(int prescriptionId) const {
    QString sql = "SELECT status FROM prescriptions WHERE prescription_id = ?";
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { prescriptionId });

    if (!query.next()) return std::nullopt;
    return prescriptionStatusFromEn(query.value("status").toString());
}
