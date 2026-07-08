/**
 * @file    PatientRepository.cpp
 * @brief   Implementación của PatientRepository – chỉ INSERT.
 *
 *  Lưu ý về cột DB:
 *    patients                       patient_code, full_name, date_of_birth,
 *                                   gender, citizen_id, phone, email, address,
 *                                   blood_type, default_patient_type,
 *                                   emergency_contact_name,
 * emergency_contact_phone out_patients                   patient_id (FK),
 * doctor_id, status in_patients                    patient_id (FK),
 * admitting_doctor_id, room_id, admission_date, discharge_date, reason, status
 *    emergency_patients_admissions  patient_id (FK), room_id, doctor_id,
 *                                   injury_cause, injury_description,
 *                                   admission_date, discharge_date, status
 */

#include "PatientRepository.h"
#include "DatabaseManager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QtDebug>

// ─────────────────────────────────────────────────────────────────────────────
// Private helper
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertBasePatient(const PatientInsertDTO &dto,
                                          int &patientId) {
  const QString sql = R"(
    INSERT INTO patients (
        patient_code,
        full_name,
        date_of_birth,
        gender,
        citizen_id,
        phone,
        email,
        address,
        blood_type,
        default_patient_type,
        emergency_contact_name,
        emergency_contact_phone
    )
    VALUES (?,?,?,?,?,?,?,?,?,?,?,?)
  )";

  const QVariantList params = {
      dto.patientCode,
      dto.fullName,
      dto.dateOfBirth,
      dto.gender,
      dto.citizenId.isEmpty() ? QVariant() : dto.citizenId,
      dto.phone.isEmpty() ? QVariant() : dto.phone,
      dto.email.isEmpty() ? QVariant() : dto.email,
      dto.address.isEmpty() ? QVariant() : dto.address,
      dto.bloodType,
      dto.type,
      dto.emergencyContactName.isEmpty() ? QVariant()
                                         : dto.emergencyContactName,
      dto.emergencyContactPhone.isEmpty() ? QVariant()
                                          : dto.emergencyContactPhone,
  };

  // Dùng QSqlQuery trực tiếp để lấy lastInsertId() ngay sau exec()
  // tránh race condition khi tạo QSqlQuery mới reset last_insert_rowid()
  QSqlQuery query(DatabaseManager::getInstance().database());
  if (!query.prepare(sql)) {
    qWarning() << "PatientRepository::insertBasePatient - prepare thất bại:"
               << query.lastError().text();
    return false;
  }
  for (const QVariant &param : params) {
    query.addBindValue(param);
  }
  if (!query.exec()) {
    qWarning() << "PatientRepository::insertBasePatient - INSERT patients thất bại:"
               << query.lastError().text();
    return false;
  }
  patientId = query.lastInsertId().toInt();
  if (patientId <= 0) {
    qWarning() << "PatientRepository::insertBasePatient - lastInsertId() trả về"
               << patientId;
    return false;
  }
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public – insert out patient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertOutPatient(const OutPatientInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();

  if (!db.beginTransaction()) {
    qWarning()
        << "PatientRepository::insertOutPatient - Không mở được transaction";
    return false;
  }

  int patientId = 0;
  if (!insertBasePatient(dto, patientId)) {
    db.rollbackTransaction();
    return false;
  }

  const QString sql = R"(
    INSERT INTO out_patients (
        patient_id,
        doctor_id,
        status
    )
    VALUES (?,?,?)
  )";

  const QVariantList params = {
      patientId,
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value()) : QVariant(),
      dto.status,
  };

  if (!db.executeQuery(sql, params).isActive()) {
    db.rollbackTransaction();
    qWarning()
        << "PatientRepository::insertOutPatient - INSERT out_patients thất bại";
    return false;
  }

  // Ghi dị ứng (nếu có) – cập nhật patientId thực tế vừa được tạo
  if (!dto.allergies.isEmpty()) {
    QList<AllergyInsertDTO> items = dto.allergies;
    for (auto &item : items) item.patientId = patientId;
    if (!insertAllergies(items)) {
      db.rollbackTransaction();
      return false;
    }
  }

  // Ghi bảo hiểm (nếu có)
  if (dto.insurance.has_value()) {
    InsuranceInsertDTO ins = dto.insurance.value();
    ins.patientId = patientId;
    if (!upsertInsurance(ins)) {
      db.rollbackTransaction();
      return false;
    }
  }

  if (!db.commitTransaction()) {
    qWarning() << "PatientRepository::insertOutPatient - Commit thất bại";
    return false;
  }
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public – insert in patient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertInPatient(const InPatientInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();

  if (!db.beginTransaction()) {
    qWarning()
        << "PatientRepository::insertInPatient - Không mở được transaction";
    return false;
  }

  int patientId = 0;
  if (!insertBasePatient(dto, patientId)) {
    db.rollbackTransaction();
    return false;
  }

  const QString sql = R"(
    INSERT INTO in_patients (
        patient_id,
        room_id,
        doctor_id,
        admission_date,
        discharge_date,
        reason,
        status
    )
    VALUES (?,?,?,?,?,?,?)
  )";

  const QVariantList params = {
      patientId,
      dto.roomId.has_value() ? QVariant(dto.roomId.value()) : QVariant(),
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value()) : QVariant(),
      dto.admissionDate,
      dto.dischargeDate.isEmpty() ? QVariant() : dto.dischargeDate,
      dto.reason.isEmpty() ? QVariant() : dto.reason,
      dto.status,
  };

  if (!db.executeQuery(sql, params).isActive()) {
    db.rollbackTransaction();
    qWarning()
        << "PatientRepository::insertInPatient - INSERT in_patients thất bại";
    return false;
  }

  // Ghi dị ứng (nếu có) – cập nhật patientId thực tế
  if (!dto.allergies.isEmpty()) {
    QList<AllergyInsertDTO> items = dto.allergies;
    for (auto &item : items) item.patientId = patientId;
    if (!insertAllergies(items)) {
      db.rollbackTransaction();
      return false;
    }
  }

  // Ghi bảo hiểm (nếu có)
  if (dto.insurance.has_value()) {
    InsuranceInsertDTO ins = dto.insurance.value();
    ins.patientId = patientId;
    if (!upsertInsurance(ins)) {
      db.rollbackTransaction();
      return false;
    }
  }

  if (!db.commitTransaction()) {
    qWarning() << "PatientRepository::insertInPatient - Commit thất bại";
    return false;
  }
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public – insert emergency patient
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertEmergencyPatient(
    const EmergencyPatientInsertDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();

  if (!db.beginTransaction()) {
    qWarning() << "PatientRepository::insertEmergencyPatient - Không mở được "
                  "transaction";
    return false;
  }

  int patientId = 0;
  if (!insertBasePatient(dto, patientId)) {
    db.rollbackTransaction();
    return false;
  }

  const QString sql = R"(
    INSERT INTO emergency_patients (
        patient_id,
        room_id,
        doctor_id,
        injury_cause,
        injury_description,
        admission_date,
        discharge_date,
        status
    )
    VALUES (?,?,?,?,?,?,?,?)
  )";

  const QVariantList params = {
      patientId,
      dto.roomId.has_value() ? QVariant(dto.roomId.value()) : QVariant(),
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value()) : QVariant(),
      dto.injuryCause.isEmpty() ? QVariant() : dto.injuryCause,
      dto.injuryDescription.isEmpty() ? QVariant() : dto.injuryDescription,
      dto.admissionDate,
      dto.dischargeDate.isEmpty() ? QVariant() : dto.dischargeDate,
      dto.status,
  };

  if (!db.executeQuery(sql, params).isActive()) {
    db.rollbackTransaction();
    qWarning() << "PatientRepository::insertEmergencyPatient - INSERT "
                  "emergency_patients_admissions thất bại";
    return false;
  }

  // Ghi dị ứng (nếu có) – cập nhật patientId thực tế
  if (!dto.allergies.isEmpty()) {
    QList<AllergyInsertDTO> items = dto.allergies;
    for (auto &item : items) item.patientId = patientId;
    if (!insertAllergies(items)) {
      db.rollbackTransaction();
      return false;
    }
  }

  // Ghi bảo hiểm (nếu có)
  if (dto.insurance.has_value()) {
    InsuranceInsertDTO ins = dto.insurance.value();
    ins.patientId = patientId;
    if (!upsertInsurance(ins)) {
      db.rollbackTransaction();
      return false;
    }
  }

  if (!db.commitTransaction()) {
    qWarning() << "PatientRepository::insertEmergencyPatient - Commit thất bại";
    return false;
  }
  return true;
}

// Update
bool PatientRepository::updatePatient(const PatientUpdateDTO &dto) {
  QString sql = R"(
    UPDATE patients
    SET
        full_name = ?,
        date_of_birth = ?,
        gender = ?,
        citizen_id = ?,
        phone = ?,
        email = ?,
        address = ?,
        blood_type = ?,
        emergency_contact_name = ?,
        emergency_contact_phone = ?
    WHERE patient_id = ?
  )";

  QVariantList params = {
      dto.fullName,
      dto.dateOfBirth,
      dto.gender,
      dto.citizenId,
      dto.phone,
      dto.email,
      dto.address,
      dto.bloodType,
      dto.emergencyContactName,
      dto.emergencyContactPhone,
      dto.patientId,
  };

  return DatabaseManager::getInstance().executeQuery(sql, params).isActive();
}
bool PatientRepository::updateOutPatient(const OutPatientUpdateDTO &dto) {
  QString sql = R"(
    UPDATE out_patients
    SET
        doctor_id = ?,
        status = ?
    WHERE patient_id = ?
  )";

  QVariantList params = {
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value()) : QVariant(),
      dto.status,
      dto.patientId,
  };

  return DatabaseManager::getInstance().executeQuery(sql, params).isActive();
}
bool PatientRepository::updateInPatient(const InPatientUpdateDTO &dto) {
  QString sql = R"(
    UPDATE in_patients
    SET
        room_id = ?,
        doctor_id = ?,
        admission_date = ?,
        discharge_date = ?,
        reason = ?,
        status = ?
    WHERE patient_id = ?
  )";

  QVariantList params = {
      dto.roomId.has_value() ? QVariant(dto.roomId.value()) : QVariant(),
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value()) : QVariant(),
      dto.admissionDate,
      dto.dischargeDate.isEmpty() ? QVariant() : dto.dischargeDate,
      dto.reason.isEmpty() ? QVariant() : dto.reason,
      dto.status,
      dto.patientId,
  };

  return DatabaseManager::getInstance().executeQuery(sql, params).isActive();
}
bool PatientRepository::updateEmergencyPatient(
    const EmergencyPatientUpdateDTO &dto) {
  QString sql = R"(
    UPDATE emergency_patients
    SET
        room_id = ?,
        doctor_id = ?,
        injury_cause = ?,
        injury_description = ?,
        admission_date = ?,
        discharge_date = ?,
        status = ?
    WHERE patient_id = ?
  )";

  QVariantList params = {
      dto.roomId.has_value() ? QVariant(dto.roomId.value()) : QVariant(),
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value()) : QVariant(),
      dto.injuryCause.isEmpty() ? QVariant() : dto.injuryCause,
      dto.injuryDescription.isEmpty() ? QVariant() : dto.injuryDescription,
      dto.admissionDate,
      dto.dischargeDate.isEmpty() ? QVariant() : dto.dischargeDate,
      dto.status,
      dto.patientId,
  };

  return DatabaseManager::getInstance().executeQuery(sql, params).isActive();
}
// ─────────────────────────────────────────────────────────────────────────────
// Search
// ─────────────────────────────────────────────────────────────────────────────

QString
PatientRepository::buildSearchWhereClause(const PatientSearchCriteria &criteria,
                                          bool hasRoomColumn,
                                          QVariantList &outParams) const {
  QStringList conditions;

  // is_deleted: mặc định chỉ lấy bệnh nhân chưa xoá mềm.
  if (!criteria.includeDeleted) {
    conditions << "p.is_deleted = 0";
  }

  // searchKey: so khớp LIKE trên full_name / patient_code / citizen_id / phone
  if (!criteria.searchKey.trimmed().isEmpty()) {
    conditions << "(p.full_name LIKE ? OR p.patient_code LIKE ? OR "
                  "p.citizen_id LIKE ? OR p.phone LIKE ?)";
    // Escape ký tự đặc biệt của LIKE ('%', '_') trước khi thêm wildcard,
    // tránh user nhập "%" hoặc "_" làm sai kết quả tìm kiếm.
    QString escaped = criteria.searchKey.trimmed();
    escaped.replace('%', "\\%").replace('_', "\\_");
    const QString likeValue = "%" + escaped + "%";
    outParams << likeValue << likeValue << likeValue << likeValue;
  }

  // roomId: chỉ áp dụng được với bảng có cột room_id (in_patients /
  // emergency_patients). Nếu bảng không có cột này (out_patients) mà
  // roomId được chỉ định, coi như KHÔNG có kết quả khớp.
  if (criteria.roomId != -1) {
    if (!hasRoomColumn) {
      conditions << "1 = 0"; // out_patients không có room_id → loại trừ
    } else {
      conditions << "room_id = ?";
      outParams << criteria.roomId;
    }
  }

  // status: so khớp chính xác với cột status của bảng con tương ứng.
  if (!criteria.status.trimmed().isEmpty()) {
    conditions << "status = ?";
    outParams << criteria.status.trimmed();
  }

  // Khoảng ngày: lọc theo admission_date (in_patients / emergency_patients).
  // out_patients không có admission_date nên bỏ qua điều kiện này.
  if (hasRoomColumn) {
    if (criteria.fromDate.has_value() && criteria.fromDate->isValid()) {
      conditions << "admission_date >= ?";
      outParams << criteria.fromDate->toString("yyyy-MM-dd");
    }
    if (criteria.toDate.has_value() && criteria.toDate->isValid()) {
      conditions << "admission_date <= ?";
      outParams << criteria.toDate->toString("yyyy-MM-dd");
    }
  }

  if (conditions.isEmpty())
    return "1 = 1"; // không có điều kiện nào → lấy tất cả

  return conditions.join(" AND ");
}

std::optional<PatientDetailDTO> PatientRepository::getPatientById(int patientId) {
  QString sql = R"(
    SELECT 
      p.patient_id, p.patient_code, p.full_name, p.date_of_birth, p.gender,
      p.citizen_id, p.phone, p.email, p.address, p.blood_type, p.default_patient_type,
      p.emergency_contact_name, p.emergency_contact_phone, p.is_deleted,
      p.created_at, p.updated_at,
      
      'OUTPATIENT' AS current_type, o.status,
      NULL AS room_id, o.doctor_id, NULL AS admission_date, NULL AS discharge_date,
      NULL AS reason, NULL AS injury_cause, NULL AS injury_description
    FROM patients p JOIN out_patients o ON p.patient_id = o.patient_id
    WHERE p.patient_id = ?
    
    UNION ALL
    
    SELECT 
      p.patient_id, p.patient_code, p.full_name, p.date_of_birth, p.gender,
      p.citizen_id, p.phone, p.email, p.address, p.blood_type, p.default_patient_type,
      p.emergency_contact_name, p.emergency_contact_phone, p.is_deleted,
      p.created_at, p.updated_at,
      
      'INPATIENT' AS current_type, i.status,
      i.room_id, i.doctor_id, i.admission_date, i.discharge_date,
      i.reason, NULL AS injury_cause, NULL AS injury_description
    FROM patients p JOIN in_patients i ON p.patient_id = i.patient_id
    WHERE p.patient_id = ?
    
    UNION ALL
    
    SELECT 
      p.patient_id, p.patient_code, p.full_name, p.date_of_birth, p.gender,
      p.citizen_id, p.phone, p.email, p.address, p.blood_type, p.default_patient_type,
      p.emergency_contact_name, p.emergency_contact_phone, p.is_deleted,
      p.created_at, p.updated_at,
      
      'EMERGENCY' AS current_type, e.status,
      e.room_id, e.doctor_id, e.admission_date, e.discharge_date,
      NULL AS reason, e.injury_cause, e.injury_description
    FROM patients p JOIN emergency_patients e ON p.patient_id = e.patient_id
    WHERE p.patient_id = ?
  )";

  QVariantList params = {patientId, patientId, patientId};
  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);

  if (query.next()) {
    PatientDetailDTO dto;
    dto.patientId = query.value("patient_id").toInt();
    dto.patientCode = query.value("patient_code").toString();
    dto.fullName = query.value("full_name").toString();
    dto.dateOfBirth = QDate::fromString(query.value("date_of_birth").toString(), "yyyy-MM-dd");
    dto.gender = stringToGender(query.value("gender").toString());
    dto.citizenId = query.value("citizen_id").toString();
    dto.phone = query.value("phone").toString();
    dto.email = query.value("email").toString();
    dto.address = query.value("address").toString();
    dto.bloodType = query.value("blood_type").toString();
    dto.defaultPatientType = stringToPatientType(query.value("default_patient_type").toString());
    dto.emergencyContactName = query.value("emergency_contact_name").toString();
    dto.emergencyContactPhone = query.value("emergency_contact_phone").toString();
    dto.isDeleted = query.value("is_deleted").toBool();
    dto.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    dto.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);

    dto.currentType = stringToPatientType(query.value("current_type").toString());
    dto.status = query.value("status").toString();

    if (!query.isNull("room_id")) dto.roomId = query.value("room_id").toInt();
    if (!query.isNull("doctor_id")) dto.doctorId = query.value("doctor_id").toInt();
    if (!query.isNull("admission_date")) dto.admissionDate = QDate::fromString(query.value("admission_date").toString(), "yyyy-MM-dd");
    if (!query.isNull("discharge_date")) dto.dischargeDate = QDate::fromString(query.value("discharge_date").toString(), "yyyy-MM-dd");

    dto.reason = query.value("reason").toString();
    dto.injuryCause = query.value("injury_cause").toString();
    dto.injuryDescription = query.value("injury_description").toString();

    // Load dị ứng & bảo hiểm
    dto.allergies = getAllergiesByPatientId(dto.patientId);
    dto.insurance = getInsuranceByPatientId(dto.patientId);

    return dto;
  }

  return std::nullopt;
}

QVector<PatientSearchResultDTO>
PatientRepository::searchPatients(const PatientSearchCriteria &criteria) {
  QVector<PatientSearchResultDTO> results;

  // Mỗi nhánh UNION ALL ứng với 1 loại bệnh nhân. Nếu criteria.type được
  // set, chỉ build nhánh tương ứng để tránh quét thừa 2 bảng còn lại.
  const bool wantOut =
      !criteria.type.has_value() || criteria.type == PatientType::OUTPATIENT;
  const bool wantIn =
      !criteria.type.has_value() || criteria.type == PatientType::INPATIENT;
  const bool wantEmergency =
      !criteria.type.has_value() || criteria.type == PatientType::EMERGENCY;

  QStringList branches;
  QVariantList params;

  if (wantOut) {
    QVariantList branchParams;
    const QString where =
        buildSearchWhereClause(criteria, /*hasRoomColumn=*/false, branchParams);
    branches << QString(R"(
      SELECT p.patient_id, p.patient_code, p.full_name, p.date_of_birth,
             p.gender, p.phone, 'OUTPATIENT' AS type, o.status AS status_label,
             NULL AS room_id
      FROM patients p
      JOIN out_patients o ON p.patient_id = o.patient_id
      WHERE %1
    )")
                    .arg(where);
    params << branchParams;
  }

  if (wantIn) {
    QVariantList branchParams;
    const QString where =
        buildSearchWhereClause(criteria, /*hasRoomColumn=*/true, branchParams);
    branches << QString(R"(
      SELECT p.patient_id, p.patient_code, p.full_name, p.date_of_birth,
             p.gender, p.phone, 'INPATIENT' AS type, i.status AS status_label,
             i.room_id AS room_id
      FROM patients p
      JOIN in_patients i ON p.patient_id = i.patient_id
      WHERE %1
    )")
                    .arg(where);
    params << branchParams;
  }

  if (wantEmergency) {
    QVariantList branchParams;
    const QString where =
        buildSearchWhereClause(criteria, /*hasRoomColumn=*/true, branchParams);
    branches << QString(R"(
      SELECT p.patient_id, p.patient_code, p.full_name, p.date_of_birth,
             p.gender, p.phone, 'EMERGENCY' AS type, e.status AS status_label,
             e.room_id AS room_id
      FROM patients p
      JOIN emergency_patients e ON p.patient_id = e.patient_id
      WHERE %1
    )")
                    .arg(where);
    params << branchParams;
  }

  if (branches.isEmpty())
    return results; // không có nhánh nào để query (không nên xảy ra)

  QString sql =
      branches.join(" UNION ALL ") + " ORDER BY full_name LIMIT ? OFFSET ?";
  params << criteria.limit << criteria.offset;

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);

  if (!query.isActive()) {
    qWarning() << "PatientRepository::searchPatients - Query thất bại";
    return results;
  }

  while (query.next()) {
    PatientSearchResultDTO row;
    row.patientId = query.value("patient_id").toInt();
    row.patientCode = query.value("patient_code").toString();
    row.fullName = query.value("full_name").toString();
    row.dateOfBirth = QDate::fromString(query.value("date_of_birth").toString(),
                                        "yyyy-MM-dd");
    row.gender = stringToGender(query.value("gender").toString());
    row.phone = query.value("phone").toString();
    row.type = stringToPatientType(query.value("type").toString());
    row.statusLabel = query.value("status_label").toString();
    row.roomId = query.value("room_id").isNull()
                     ? QString()
                     : query.value("room_id").toString();
    results.append(row);
  }

  return results;
}

int PatientRepository::countSearchResults(
    const PatientSearchCriteria &criteria) {
  const bool wantOut =
      !criteria.type.has_value() || criteria.type == PatientType::OUTPATIENT;
  const bool wantIn =
      !criteria.type.has_value() || criteria.type == PatientType::INPATIENT;
  const bool wantEmergency =
      !criteria.type.has_value() || criteria.type == PatientType::EMERGENCY;

  QStringList branches;
  QVariantList params;

  if (wantOut) {
    QVariantList branchParams;
    const QString where =
        buildSearchWhereClause(criteria, /*hasRoomColumn=*/false, branchParams);
    branches << QString(R"(
      SELECT p.patient_id FROM patients p
      JOIN out_patients o ON p.patient_id = o.patient_id
      WHERE %1
    )")
                    .arg(where);
    params << branchParams;
  }

  if (wantIn) {
    QVariantList branchParams;
    const QString where =
        buildSearchWhereClause(criteria, /*hasRoomColumn=*/true, branchParams);
    branches << QString(R"(
      SELECT p.patient_id FROM patients p
      JOIN in_patients i ON p.patient_id = i.patient_id
      WHERE %1
    )")
                    .arg(where);
    params << branchParams;
  }

  if (wantEmergency) {
    QVariantList branchParams;
    const QString where =
        buildSearchWhereClause(criteria, /*hasRoomColumn=*/true, branchParams);
    branches << QString(R"(
      SELECT p.patient_id FROM patients p
      JOIN emergency_patients e ON p.patient_id = e.patient_id
      WHERE %1
    )")
                    .arg(where);
    params << branchParams;
  }

  if (branches.isEmpty())
    return 0;

  const QString sql =
      "SELECT COUNT(*) FROM (" + branches.join(" UNION ALL ") + ")";

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (!query.isActive() || !query.next()) {
    qWarning() << "PatientRepository::countSearchResults - Query thất bại";
    return 0;
  }
  return query.value(0).toInt();
}

bool PatientRepository::softDeletePatient(int patientId) {
  QString sql = R"(
    UPDATE patients
    SET
        is_deleted = 1
    WHERE patient_id = ?
  )";

  QVariantList params = {patientId};

  return DatabaseManager::getInstance().executeQuery(sql, params).isActive();
}
bool PatientRepository::restorePatient(int patientId) {
  QString sql = R"(
    UPDATE patients
    SET
        is_deleted = 0
    WHERE patient_id = ?
  )";

  QVariantList params = {patientId};

  return DatabaseManager::getInstance().executeQuery(sql, params).isActive();
}
bool PatientRepository::isPatientSoftDeleted(int patientId) {
  QString sql = R"(
    SELECT is_deleted FROM patients WHERE patient_id = ?
  )";

  QVariantList params = {patientId};

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (!query.isActive() || !query.next()) {
    qWarning() << "PatientRepository::isPatientSoftDeleted - Query thất bại";
    return false;
  }
  return query.value(0).toInt() == 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Allergies
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertAllergies(const QList<AllergyInsertDTO> &items) {
  if (items.isEmpty()) return true; // không có dị ứng → bỏ qua, không lỗi

  const QString sql = R"(
    INSERT OR IGNORE INTO patient_allergies (patient_id, allergen_name, severity, notes)
    VALUES (?, ?, ?, ?)
  )";

  DatabaseManager &db = DatabaseManager::getInstance();
  for (const AllergyInsertDTO &item : items) {
    const QVariantList params = {
        item.patientId,
        item.allergenName.trimmed(),
        item.severity.isEmpty() ? "MODERATE" : item.severity,
        item.notes.isEmpty() ? QVariant() : item.notes,
    };
    if (!db.executeQuery(sql, params).isActive()) {
      qWarning() << "PatientRepository::insertAllergies - INSERT thất bại cho:"
                 << item.allergenName;
      return false;
    }
  }
  return true;
}

bool PatientRepository::deactivateAllergies(int patientId) {
  const QString sql = R"(
    UPDATE patient_allergies SET is_active = 0 WHERE patient_id = ?
  )";
  return DatabaseManager::getInstance().executeQuery(sql, {patientId}).isActive();
}

QList<AllergyResultDTO> PatientRepository::getAllergiesByPatientId(int patientId) {
  QList<AllergyResultDTO> results;

  const QString sql = R"(
    SELECT allergy_id, allergen_name, severity, notes, is_active, recorded_at, updated_at
    FROM patient_allergies
    WHERE patient_id = ? AND is_active = 1
    ORDER BY recorded_at ASC
  )";

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {patientId});
  while (query.next()) {
    AllergyResultDTO dto;
    dto.allergyId    = query.value("allergy_id").toInt();
    dto.allergenName = query.value("allergen_name").toString();
    dto.severity     = query.value("severity").toString();
    dto.notes        = query.value("notes").toString();
    dto.isActive     = query.value("is_active").toInt() == 1;
    dto.recordedAt   = query.value("recorded_at").toString();
    dto.updatedAt    = query.value("updated_at").toString();
    results.append(dto);
  }
  return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// Insurance
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::upsertInsurance(const InsuranceInsertDTO &dto) {
  // INSERT OR REPLACE dựa trên UNIQUE(patient_id)
  const QString sql = R"(
    INSERT INTO patient_insurance (
        patient_id, provider_name, policy_number, insurance_type,
        coverage_percent, valid_from, valid_to, notes, is_active
    )
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, 1)
    ON CONFLICT(patient_id) DO UPDATE SET
        provider_name    = excluded.provider_name,
        policy_number    = excluded.policy_number,
        insurance_type   = excluded.insurance_type,
        coverage_percent = excluded.coverage_percent,
        valid_from       = excluded.valid_from,
        valid_to         = excluded.valid_to,
        notes            = excluded.notes,
        is_active        = 1,
        updated_at       = datetime('now')
  )";

  const QVariantList params = {
      dto.patientId,
      dto.providerName.trimmed(),
      dto.policyNumber.trimmed(),
      dto.insuranceType.isEmpty() ? "BHYT" : dto.insuranceType,
      dto.coveragePercent,
      dto.validFrom.isEmpty()  ? QVariant() : dto.validFrom,
      dto.validTo.isEmpty()    ? QVariant() : dto.validTo,
      dto.notes.isEmpty()      ? QVariant() : dto.notes,
  };

  if (!DatabaseManager::getInstance().executeQuery(sql, params).isActive()) {
    qWarning() << "PatientRepository::upsertInsurance - UPSERT thất bại cho patient_id:"
               << dto.patientId;
    return false;
  }
  return true;
}

std::optional<InsuranceResultDTO>
PatientRepository::getInsuranceByPatientId(int patientId) {
  const QString sql = R"(
    SELECT insurance_id, provider_name, policy_number, insurance_type,
           coverage_percent, valid_from, valid_to, notes, is_active,
           created_at, updated_at
    FROM patient_insurance
    WHERE patient_id = ? AND is_active = 1
    LIMIT 1
  )";

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {patientId});
  if (!query.next()) return std::nullopt;

  InsuranceResultDTO dto;
  dto.insuranceId      = query.value("insurance_id").toInt();
  dto.providerName     = query.value("provider_name").toString();
  dto.policyNumber     = query.value("policy_number").toString();
  dto.insuranceType    = query.value("insurance_type").toString();
  dto.coveragePercent  = query.value("coverage_percent").toDouble();
  dto.validFrom        = query.value("valid_from").toString();
  dto.validTo          = query.value("valid_to").toString();
  dto.notes            = query.value("notes").toString();
  dto.isActive         = query.value("is_active").toInt() == 1;
  dto.createdAt        = query.value("created_at").toString();
  dto.updatedAt        = query.value("updated_at").toString();
  return dto;
}

QString PatientRepository::getAllergiesStringByPatientId(int patientId) {
  QString sql = "SELECT allergies FROM patients WHERE patient_id = ? AND is_deleted = 0";
  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {patientId});
  if (query.next()) {
    return query.value(0).toString();
  }
  return "";
}
