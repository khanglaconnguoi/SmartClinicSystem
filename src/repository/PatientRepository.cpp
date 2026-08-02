/**
 * @file    PatientRepository.cpp
 * @brief   Implementación của PatientRepository.
 *
 *  Lưu ý về cột DB:
 *    patients                       patient_code, full_name, date_of_birth,
 *                                   gender, citizen_id, phone, email, address,
 *                                   blood_type, default_patient_type,
 *                                   emergency_contact_name,
 *                                   emergency_contact_phone
 *    out_patients                   patient_id (FK), doctor_id, status
 *    in_patients                    patient_id (FK), admitting_doctor_id,
 *                                   room_id, admission_date, discharge_date,
 *                                   reason, status
 *    emergency_patients             patient_id (FK), room_id, doctor_id,
 *                                   injury_cause, injury_description,
 *                                   admission_date, discharge_date, status
 *    patient_allergies              patient_id (FK), ingredient_id,
 *                                   allergen_name, severity, notes, is_active
 *    patient_insurance              patient_id (FK), provider_name,
 *                                   policy_number, insurance_type,
 *                                   coverage_percent, valid_from, valid_to,
 *                                   notes, is_active
 */

#include "PatientRepository.h"
#include "DatabaseManager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QtDebug>

namespace {

bool tableHasColumn(const QString &tableName, const QString &columnName) {
  QSqlQuery query(DatabaseManager::getInstance().database());
  if (!query.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
    qWarning() << "PatientRepository - unable to inspect columns for table"
               << tableName << ":" << query.lastError().text();
    return false;
  }

  while (query.next()) {
    if (query.value(1).toString().compare(columnName, Qt::CaseInsensitive) == 0) {
      return true;
    }
  }
  return false;
}

QString safeTextValue(const QString &value, const QString &fallback = QStringLiteral("N/A")) {
  const QString trimmed = value.trimmed();
  return trimmed.isEmpty() ? fallback : trimmed;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertBasePatient(const PatientInsertDTO &dto,
                                          int &patientId) {
  QStringList columns;
  QVariantList params;
  QStringList placeholders;

  auto addColumn = [&](const QString &column, const QVariant &value) {
    columns << column;
    params << value;
    placeholders << "?";
  };

  addColumn("patient_code", dto.patientCode);
  addColumn("full_name", dto.fullName);
  addColumn("date_of_birth", dto.dateOfBirth);
  addColumn("gender", dto.gender);

  if (tableHasColumn("patients", "citizen_id")) {
    addColumn("citizen_id",
              dto.citizenId.isEmpty() ? QVariant() : dto.citizenId);
  }
  if (tableHasColumn("patients", "phone_number")) {
    addColumn("phone_number",
              dto.phone.isEmpty() ? QVariant() : dto.phone);
  }
  if (tableHasColumn("patients", "email")) {
    addColumn("email", safeTextValue(dto.email, "unknown@example.com"));
  }
  if (tableHasColumn("patients", "address")) {
    addColumn("address", safeTextValue(dto.address, "N/A"));
  }
  if (tableHasColumn("patients", "blood_type")) {
    addColumn("blood_type", dto.bloodType);
  }
  if (tableHasColumn("patients", "default_patient_type")) {
    addColumn("default_patient_type", dto.type);
  }
  if (tableHasColumn("patients", "emergency_contact_name")) {
    addColumn("emergency_contact_name",
              safeTextValue(dto.emergencyContactName, "N/A"));
  }
  if (tableHasColumn("patients", "emergency_contact_phone")) {
    addColumn("emergency_contact_phone",
              safeTextValue(dto.emergencyContactPhone, "N/A"));
  }

  if (columns.isEmpty()) {
    qWarning() << "PatientRepository::insertBasePatient - không có cột nào để insert";
    return false;
  }

  const QString sql = QString("INSERT INTO patients (%1) VALUES (%2)")
                          .arg(columns.join(", "))
                          .arg(placeholders.join(", "));

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
    qWarning()
        << "PatientRepository::insertBasePatient - INSERT patients thất bại:"
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

  if (!insertAllergies(patientId, dto.allergies)) {
    db.rollbackTransaction();
    return false;
  }

  if (!insertInsurance(patientId, dto.insurance)) {
    db.rollbackTransaction();
    return false;
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
      dto.roomId <= 0 ? QVariant() : dto.roomId,
      dto.doctorId <= 0 ? QVariant() : dto.doctorId,
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

  if (!insertAllergies(patientId, dto.allergies)) {
    db.rollbackTransaction();
    return false;
  }

  if (!insertInsurance(patientId, dto.insurance)) {
    db.rollbackTransaction();
    return false;
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
      dto.roomId <= 0 ? QVariant() : dto.roomId,
      dto.doctorId <= 0 ? QVariant() : dto.doctorId,
      dto.injuryCause.isEmpty() ? QVariant() : dto.injuryCause,
      dto.injuryDescription.isEmpty() ? QVariant() : dto.injuryDescription,
      dto.admissionDate,
      dto.dischargeDate.isEmpty() ? QVariant() : dto.dischargeDate,
      dto.status,
  };

  if (!db.executeQuery(sql, params).isActive()) {
    db.rollbackTransaction();
    qWarning() << "PatientRepository::insertEmergencyPatient - INSERT "
                  "emergency_patients thất bại";
    return false;
  }

  if (!insertAllergies(patientId, dto.allergies)) {
    db.rollbackTransaction();
    return false;
  }

  if (!insertInsurance(patientId, dto.insurance)) {
    db.rollbackTransaction();
    return false;
  }

  if (!db.commitTransaction()) {
    qWarning() << "PatientRepository::insertEmergencyPatient - Commit thất bại";
    return false;
  }
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::updatePatient(const PatientUpdateDTO &dto) {
  const QString sql = R"(
    UPDATE patients
    SET
        full_name = ?,
        date_of_birth = ?,
        gender = ?,
        citizen_id = ?,
        phone_number = ?,
        email = ?,
        address = ?,
        blood_type = ?,
        emergency_contact_name = ?,
        emergency_contact_phone = ?
    WHERE patient_id = ?
  )";

  const QVariantList params = {
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
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  if (!updatePatient(dto)) {
    db.rollbackTransaction();
    return false;
  }

  const QString sql = R"(
    UPDATE out_patients
    SET
        doctor_id = ?,
        status = ?
    WHERE patient_id = ?
  )";

  const QVariantList params = {
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value()) : QVariant(),
      dto.status,
      dto.patientId,
  };

  QSqlQuery query = db.executeQuery(sql, params);
  if (query.lastError().isValid()) {
    db.rollbackTransaction();
    qWarning() << "UPDATE OUT_PATIENT ERROR:" << query.lastError().text();
    return false;
  }

  return db.commitTransaction();
}

bool PatientRepository::updateInPatient(const InPatientUpdateDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  if (!updatePatient(dto)) {
    db.rollbackTransaction();
    return false;
  }

  const QString sql = R"(
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

  const QVariantList params = {
      dto.roomId.has_value() ? QVariant(dto.roomId.value()) : QVariant(),
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value()) : QVariant(),
      dto.admissionDate,
      dto.dischargeDate.isEmpty() ? QVariant() : dto.dischargeDate,
      dto.reason.isEmpty() ? QVariant() : dto.reason,
      dto.status,
      dto.patientId,
  };

  QSqlQuery query = db.executeQuery(sql, params);
  if (query.lastError().isValid()) {
    db.rollbackTransaction();
    qWarning() << "UPDATE IN_PATIENT ERROR:" << query.lastError().text();
    return false;
  }

  return db.commitTransaction();
}

bool PatientRepository::updateEmergencyPatient(
    const EmergencyPatientUpdateDTO &dto) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  if (!updatePatient(dto)) {
    db.rollbackTransaction();
    return false;
  }

  const QString sql = R"(
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

  const QVariantList params = {
      dto.roomId.has_value() ? QVariant(dto.roomId.value()) : QVariant(),
      dto.doctorId.has_value() ? QVariant(dto.doctorId.value()) : QVariant(),
      dto.injuryCause.isEmpty() ? QVariant() : dto.injuryCause,
      dto.injuryDescription.isEmpty() ? QVariant() : dto.injuryDescription,
      dto.admissionDate,
      dto.dischargeDate.isEmpty() ? QVariant() : dto.dischargeDate,
      dto.status,
      dto.patientId,
  };

  QSqlQuery query = db.executeQuery(sql, params);
  if (query.lastError().isValid()) {
    db.rollbackTransaction();
    qWarning() << "UPDATE EMERGENCY_PATIENT ERROR:" << query.lastError().text();
    return false;
  }

  return db.commitTransaction();
}

// ─────────────────────────────────────────────────────────────────────────────
// Search helpers
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

  // searchKey: so khớp LIKE trên full_name / patient_code / citizen_id / phone_number
  if (!criteria.searchKey.trimmed().isEmpty()) {
    conditions << "(LOWER(p.full_name) LIKE ? OR LOWER(p.patient_code) LIKE ? OR "
                  "LOWER(p.citizen_id) LIKE ? OR LOWER(p.phone_number) LIKE ?)";
    QString pattern = "%" + criteria.searchKey.trimmed().toLower() + "%";
    outParams << pattern << pattern << pattern << pattern;
  }


  // roomId: chỉ áp dụng được với bảng có cột room_id (in_patients /
  // emergency_patients).
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

// ─────────────────────────────────────────────────────────────────────────────
// getPatientById
// ─────────────────────────────────────────────────────────────────────────────

std::optional<PatientDetailDTO>
PatientRepository::getPatientById(int patientId) {
  const QString sql = R"(
    SELECT
      p.patient_id, p.patient_code, p.full_name, p.date_of_birth, p.gender,
      p.citizen_id, p.phone_number, p.email, p.address, p.blood_type,
      p.default_patient_type, p.emergency_contact_name,
      p.emergency_contact_phone, p.is_deleted, p.created_at, p.updated_at,

      'OUTPATIENT' AS current_type, o.status,
      NULL AS room_id, o.doctor_id,
      NULL AS admission_date, NULL AS discharge_date,
      NULL AS reason, NULL AS injury_cause, NULL AS injury_description
    FROM patients p JOIN out_patients o ON p.patient_id = o.patient_id
    WHERE p.patient_id = ?

    UNION ALL

    SELECT
      p.patient_id, p.patient_code, p.full_name, p.date_of_birth, p.gender,
      p.citizen_id, p.phone_number, p.email, p.address, p.blood_type,
      p.default_patient_type, p.emergency_contact_name,
      p.emergency_contact_phone, p.is_deleted, p.created_at, p.updated_at,

      'INPATIENT' AS current_type, i.status,
      i.room_id, i.doctor_id, i.admission_date, i.discharge_date,
      i.reason, NULL AS injury_cause, NULL AS injury_description
    FROM patients p JOIN in_patients i ON p.patient_id = i.patient_id
    WHERE p.patient_id = ?

    UNION ALL

    SELECT
      p.patient_id, p.patient_code, p.full_name, p.date_of_birth, p.gender,
      p.citizen_id, p.phone_number, p.email, p.address, p.blood_type,
      p.default_patient_type, p.emergency_contact_name,
      p.emergency_contact_phone, p.is_deleted, p.created_at, p.updated_at,

      'EMERGENCY' AS current_type, e.status,
      e.room_id, e.doctor_id, e.admission_date, e.discharge_date,
      NULL AS reason, e.injury_cause, e.injury_description
    FROM patients p JOIN emergency_patients e ON p.patient_id = e.patient_id
    WHERE p.patient_id = ?

    UNION ALL

    SELECT
      p.patient_id, p.patient_code, p.full_name, p.date_of_birth, p.gender,
      p.citizen_id, p.phone_number, p.email, p.address, p.blood_type,
      p.default_patient_type, p.emergency_contact_name,
      p.emergency_contact_phone, p.is_deleted, p.created_at, p.updated_at,

      'OUTPATIENT' AS current_type, 'ACTIVE' AS status,
      NULL AS room_id, NULL AS doctor_id,
      NULL AS admission_date, NULL AS discharge_date,
      NULL AS reason, NULL AS injury_cause, NULL AS injury_description
    FROM patients p
    WHERE p.patient_id = ?
      AND NOT EXISTS (SELECT 1 FROM out_patients WHERE patient_id = p.patient_id)
      AND NOT EXISTS (SELECT 1 FROM in_patients WHERE patient_id = p.patient_id)
      AND NOT EXISTS (SELECT 1 FROM emergency_patients WHERE patient_id = p.patient_id)
  )";

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(
      sql, {patientId, patientId, patientId, patientId});

  if (!query.next())
    return std::nullopt;

  PatientDetailDTO dto;
  dto.patientId = query.value("patient_id").toInt();
  dto.patientCode = query.value("patient_code").toString();
  dto.fullName = query.value("full_name").toString();
  dto.dateOfBirth =
      QDate::fromString(query.value("date_of_birth").toString(), "yyyy-MM-dd");
  dto.gender = query.value("gender").toString();
  dto.citizenId = query.value("citizen_id").toString();
  dto.phone = query.value("phone_number").toString();
  dto.email = query.value("email").toString();
  dto.address = query.value("address").toString();
  dto.bloodType = query.value("blood_type").toString();
  dto.defaultPatientType =
      patientTypeFromEn(query.value("default_patient_type").toString());
  dto.emergencyContactName = query.value("emergency_contact_name").toString();
  dto.emergencyContactPhone = query.value("emergency_contact_phone").toString();
  dto.isDeleted = query.value("is_deleted").toBool();
  dto.createdAt =
      QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
  dto.updatedAt =
      QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);

  dto.currentType = patientTypeFromEn(query.value("current_type").toString());
  dto.status = query.value("status").toString();

  if (!query.isNull("room_id"))
    dto.roomId = query.value("room_id").toInt();
  if (!query.isNull("doctor_id"))
    dto.doctorId = query.value("doctor_id").toInt();
  if (!query.isNull("admission_date"))
    dto.admissionDate = QDate::fromString(
        query.value("admission_date").toString(), "yyyy-MM-dd");
  if (!query.isNull("discharge_date"))
    dto.dischargeDate = QDate::fromString(
        query.value("discharge_date").toString(), "yyyy-MM-dd");

  dto.reason = query.value("reason").toString();
  dto.injuryCause = query.value("injury_cause").toString();
  dto.injuryDescription = query.value("injury_description").toString();

  // Load dị ứng & bảo hiểm
  dto.allergies = getAllergiesByPatientId(dto.patientId);
  dto.insurance = getInsuranceByPatientId(dto.patientId);

  return dto;
}

// ─────────────────────────────────────────────────────────────────────────────
// searchPatients
// ─────────────────────────────────────────────────────────────────────────────

PagedResult<PatientSearchResultDTO>
PatientRepository::searchPatientsPaged(const PatientSearchCriteria &criteria) const {
    PagedResult<PatientSearchResultDTO> result;
    result.page = qMax(1, criteria.page);
    result.pageSize = criteria.pageSize;

    const bool wantOut =
        !criteria.type.has_value() || criteria.type == PatientType::Outpatient;
    const bool wantIn =
        !criteria.type.has_value() || criteria.type == PatientType::Inpatient;
    const bool wantEmergency =
        !criteria.type.has_value() || criteria.type == PatientType::Emergency;

    // Bước 1: Đếm tổng số bản ghi khớp tiêu chí
    QStringList countBranches;
    QVariantList countParams;

    if (wantOut) {
      QVariantList branchParams;
      const QString where =
          buildSearchWhereClause(criteria, /*hasRoomColumn=*/false, branchParams);
      countBranches << QString(R"(
        SELECT p.patient_id FROM patients p
        JOIN out_patients o ON p.patient_id = o.patient_id
        WHERE %1
      )").arg(where);
      countParams << branchParams;
    }

    if (wantIn) {
      QVariantList branchParams;
      const QString where =
          buildSearchWhereClause(criteria, /*hasRoomColumn=*/true, branchParams);
      countBranches << QString(R"(
        SELECT p.patient_id FROM patients p
        JOIN in_patients i ON p.patient_id = i.patient_id
        WHERE %1
      )").arg(where);
      countParams << branchParams;
    }

    if (wantEmergency) {
      QVariantList branchParams;
      const QString where =
          buildSearchWhereClause(criteria, /*hasRoomColumn=*/true, branchParams);
      countBranches << QString(R"(
        SELECT p.patient_id FROM patients p
        JOIN emergency_patients e ON p.patient_id = e.patient_id
        WHERE %1
      )").arg(where);
      countParams << branchParams;
    }

    if (countBranches.isEmpty()) {
      result.totalCount = 0;
      return result;
    }

    const QString countSql =
        "SELECT COUNT(*) FROM (" + countBranches.join(" UNION ALL ") + ")";
    QSqlQuery countQuery = DatabaseManager::getInstance().selectQuery(countSql, countParams);
    if (!countQuery.isActive() || !countQuery.next()) {
      qWarning() << "PatientRepository::searchPatientsPaged - Lỗi đếm tổng bản ghi";
      result.totalCount = 0;
      return result;
    }
    result.totalCount = countQuery.value(0).toInt();

    // Bước 2: Lấy dữ liệu trang hiện tại
    QStringList dataBranches;
    QVariantList dataParams;

    if (wantOut) {
      QVariantList branchParams;
      const QString where =
          buildSearchWhereClause(criteria, /*hasRoomColumn=*/false, branchParams);
      dataBranches << QString(R"(
        SELECT p.patient_id, p.patient_code, p.full_name, p.date_of_birth,
              p.gender, p.phone_number, 'OUTPATIENT' AS type, o.status AS status_label,
              NULL AS room_id
        FROM patients p
        JOIN out_patients o ON p.patient_id = o.patient_id
        WHERE %1
      )").arg(where);
      dataParams << branchParams;
    }

    if (wantIn) {
      QVariantList branchParams;
      const QString where =
          buildSearchWhereClause(criteria, /*hasRoomColumn=*/true, branchParams);
      dataBranches << QString(R"(
        SELECT p.patient_id, p.patient_code, p.full_name, p.date_of_birth,
              p.gender, p.phone_number, 'INPATIENT' AS type, i.status AS status_label,
              i.room_id AS room_id
        FROM patients p
        JOIN in_patients i ON p.patient_id = i.patient_id
        WHERE %1
      )").arg(where);
      dataParams << branchParams;
    }

    if (wantEmergency) {
      QVariantList branchParams;
      const QString where =
          buildSearchWhereClause(criteria, /*hasRoomColumn=*/true, branchParams);
      dataBranches << QString(R"(
        SELECT p.patient_id, p.patient_code, p.full_name, p.date_of_birth,
              p.gender, p.phone_number, 'EMERGENCY' AS type, e.status AS status_label,
              e.room_id AS room_id
        FROM patients p
        JOIN emergency_patients e ON p.patient_id = e.patient_id
        WHERE %1
      )").arg(where);
      dataParams << branchParams;
    }

    QString dataSql = dataBranches.join(" UNION ALL ") + " ORDER BY full_name ASC";
    if (criteria.pageSize > 0) {
      int offset = (result.page - 1) * criteria.pageSize;
      dataSql += " LIMIT ? OFFSET ?";
      dataParams.append(criteria.pageSize);
      dataParams.append(offset);
    }

    QSqlQuery dataQuery = DatabaseManager::getInstance().selectQuery(dataSql, dataParams);
    if (!dataQuery.isActive()) {
        qWarning() << "PatientRepository::searchPatientsPaged - Data query thất bại";
        return result;
    }

    while (dataQuery.next()) {
        PatientSearchResultDTO row;
    row.patientId = dataQuery.value("patient_id").toInt();
    row.patientCode = dataQuery.value("patient_code").toString();
    row.fullName = dataQuery.value("full_name").toString();
    row.dateOfBirth = QDate::fromString(dataQuery.value("date_of_birth").toString(), "yyyy-MM-dd");
    row.gender = dataQuery.value("gender").toString();
    row.phone = dataQuery.value("phone_number").toString();
    row.type = patientTypeFromEn(dataQuery.value("type").toString());
    row.statusLabel = dataQuery.value("status_label").toString();
    row.roomId = dataQuery.isNull("room_id") ? QString() : dataQuery.value("room_id").toString();
    result.items.append(row);
  }

  return result;
}

/*
QList<PatientSearchResultDTO>
PatientRepository::searchPatients(const PatientSearchCriteria &criteria) { ... }
int PatientRepository::countSearchResults(const PatientSearchCriteria &criteria) { ... }
*/


// ─────────────────────────────────────────────────────────────────────────────
// Soft delete / Restore
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::softDeletePatient(int patientId) {
  const QString sql = R"(
    UPDATE patients SET is_deleted = 1 WHERE patient_id = ?
  )";
  return DatabaseManager::getInstance()
      .executeQuery(sql, {patientId})
      .isActive();
}

bool PatientRepository::restorePatient(int patientId) {
  const QString sql = R"(
    UPDATE patients SET is_deleted = 0 WHERE patient_id = ?
  )";
  return DatabaseManager::getInstance()
      .executeQuery(sql, {patientId})
      .isActive();
}

bool PatientRepository::isPatientSoftDeleted(int patientId) {
  const QString sql = R"(
    SELECT is_deleted FROM patients WHERE patient_id = ?
  )";
  QSqlQuery query =
      DatabaseManager::getInstance().selectQuery(sql, {patientId});
  if (!query.isActive() || !query.next()) {
    qWarning() << "PatientRepository::isPatientSoftDeleted - Query thất bại";
    return false;
  }
  return query.value(0).toInt() == 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Allergies
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertAllergies(int patientId,
                                        const QList<AllergyInputDTO> &items) {
  if (items.isEmpty())
    return true;

  const QString sql = R"(
    INSERT OR IGNORE INTO patient_allergies
        (patient_id, ingredient_id, allergen_name, severity, notes)
    VALUES (?, ?, ?, ?, ?)
  )";

  DatabaseManager &db = DatabaseManager::getInstance();
  for (const AllergyInputDTO &item : items) {
    const QVariantList params = {
        patientId,
        item.ingredientId.has_value() ? QVariant(item.ingredientId.value())
                                      : QVariant(),
        item.allergenName,
        severityToEn(item.severity),
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
  return DatabaseManager::getInstance()
      .executeQuery(sql, {patientId})
      .isActive();
}

QList<AllergyResultDTO>
PatientRepository::getAllergiesByPatientId(int patientId) {
  QList<AllergyResultDTO> results;

  const QString sql = R"(
    SELECT allergy_id, ingredient_id, allergen_name, severity, notes,
           is_active, recorded_at, updated_at
    FROM patient_allergies
    WHERE patient_id = ? AND is_active = 1
    ORDER BY recorded_at ASC
  )";

  QSqlQuery query =
      DatabaseManager::getInstance().selectQuery(sql, {patientId});
  while (query.next()) {
    AllergyResultDTO dto;
    dto.allergyId = query.value("allergy_id").toInt();
    dto.ingredientId =
        query.isNull("ingredient_id")
            ? std::optional<int>{}
            : std::optional<int>{query.value("ingredient_id").toInt()};
    dto.allergenName = query.value("allergen_name").toString();
    dto.severity = severityFromEn(query.value("severity").toString());
    dto.notes = query.value("notes").toString();
    dto.isActive = query.value("is_active").toInt() == 1;
    dto.recordedAt = query.value("recorded_at").toString();
    dto.updatedAt = query.value("updated_at").toString();
    results.append(dto);
  }
  return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// Insurance
// ─────────────────────────────────────────────────────────────────────────────

bool PatientRepository::insertInsurance(int patientId,
                                        const InsuranceInsertDTO &dto) {
  // Return early if no insurance data is provided
  if (dto.providerName.isEmpty() && dto.policyNumber.isEmpty() &&
      dto.insuranceType.isEmpty()) {
    return true;
  }

  QStringList columns;
  QVariantList params;
  QStringList placeholders;

  auto addColumn = [&](const QString &column, const QVariant &value) {
    columns << column;
    params << value;
    placeholders << "?";
  };

  addColumn("patient_id", patientId);
  if (tableHasColumn("patient_insurance", "provider_name")) {
    addColumn("provider_name", dto.providerName);
  }
  if (tableHasColumn("patient_insurance", "policy_number")) {
    addColumn("policy_number", dto.policyNumber);
  }
  if (tableHasColumn("patient_insurance", "insurance_type")) {
    addColumn("insurance_type", dto.insuranceType);
  }
  if (tableHasColumn("patient_insurance", "coverage_percent")) {
    addColumn("coverage_percent", dto.coveragePercent);
  }
  if (tableHasColumn("patient_insurance", "valid_from")) {
    addColumn("valid_from", dto.validFrom.isEmpty() ? QVariant() : dto.validFrom);
  }
  if (tableHasColumn("patient_insurance", "valid_to")) {
    addColumn("valid_to", dto.validTo.isEmpty() ? QVariant() : dto.validTo);
  }
  if (tableHasColumn("patient_insurance", "notes")) {
    addColumn("notes", dto.notes.isEmpty() ? QVariant() : dto.notes);
  }
  if (tableHasColumn("patient_insurance", "is_active")) {
    addColumn("is_active", 1);
  }

  if (columns.isEmpty()) {
    return true;
  }

  const QString sql = QString("INSERT INTO patient_insurance (%1) VALUES (%2)")
                          .arg(columns.join(", "))
                          .arg(placeholders.join(", "));

  if (!DatabaseManager::getInstance().executeQuery(sql, params).isActive()) {
    qWarning() << "PatientRepository::insertInsurance - INSERT thất bại cho "
                  "patient_id:"
               << patientId;
    return false;
  }
  return true;
}

bool PatientRepository::updateInsurance(int patientId,
                                        const InsuranceInsertDTO &dto) {
  // Return early if no insurance data is provided
  if (dto.providerName.isEmpty() && dto.policyNumber.isEmpty() &&
      dto.insuranceType.isEmpty()) {
    return true;
  }

  const QString sql = R"(
    UPDATE patient_insurance SET
        provider_name    = ?,
        policy_number    = ?,
        insurance_type   = ?,
        coverage_percent = ?,
        valid_from       = ?,
        valid_to         = ?,
        notes            = ?,
        is_active        = 1,
        updated_at       = datetime('now')
    WHERE patient_id = ?
  )";

  const QVariantList params = {
      dto.providerName,
      dto.policyNumber,
      dto.insuranceType,
      dto.coveragePercent,
      dto.validFrom.isEmpty() ? QVariant() : dto.validFrom,
      dto.validTo.isEmpty() ? QVariant() : dto.validTo,
      dto.notes.isEmpty() ? QVariant() : dto.notes,
      patientId,
  };

  if (!DatabaseManager::getInstance().executeQuery(sql, params).isActive()) {
    qWarning() << "PatientRepository::updateInsurance - UPDATE thất bại cho "
                  "patient_id:"
               << patientId;
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

  QSqlQuery query =
      DatabaseManager::getInstance().selectQuery(sql, {patientId});
  if (!query.next())
    return std::nullopt;

  InsuranceResultDTO result;
  result.insuranceId = query.value("insurance_id").toInt();
  result.providerName = query.value("provider_name").toString();
  result.policyNumber = query.value("policy_number").toString();
  result.insuranceType = query.value("insurance_type").toString();
  result.coveragePercent = query.value("coverage_percent").toDouble();
  result.validFrom = query.value("valid_from").toString();
  result.validTo = query.value("valid_to").toString();
  result.notes = query.value("notes").toString();
  result.isActive = query.value("is_active").toInt() == 1;
  result.createdAt = query.value("created_at").toString();
  result.updatedAt = query.value("updated_at").toString();
  return result;
}

std::optional<PatientShortDTO>
PatientRepository::getPatientByPhoneOrCitizenId(
    const QString &phone, const QString &citizenId) const {
  QString sql = "SELECT patient_id, patient_code, full_name, phone_number FROM "
                "patients WHERE is_deleted = 0 AND (";
  QVariantList params;
  QStringList conditions;
  if (!phone.isEmpty()) {
    conditions << "phone_number = ?";
    params << phone;
  }
  if (!citizenId.isEmpty()) {
    conditions << "citizen_id = ?";
    params << citizenId;
  }
  if (conditions.isEmpty())
    return std::nullopt;
  sql += conditions.join(" OR ") + ")";

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (query.next()) {
    PatientShortDTO rec;
    rec.patientId = query.value(0).toInt();
    rec.patientCode = query.value(1).toString();
    rec.fullName = query.value(2).toString();
    rec.phone = query.value(3).toString();
    return rec;
  }
  return std::nullopt;
}
