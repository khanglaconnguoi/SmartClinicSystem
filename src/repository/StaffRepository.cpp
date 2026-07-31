#include "StaffRepository.h"

#include <QDir>
#include <QIODevice>
#include <QSqlError>

#include "DatabaseManager.h"
#include "model/Admin.h"
#include "model/Doctor.h"
#include "model/Nurse.h"
#include "model/Receptionist.h"
#include "model/Pharmacist.h"


bool StaffRepository::insertStaffBase(const StaffInsertDTO &staff,
                                      int &staffId) {
  QString insert = R"(
        INSERT INTO staff (
            staff_code,
            password_hash,
            full_name,
            avatar,
            role,
            gender,
            date_of_birth,
            citizen_id,
            phone_number,
            email,
            address,
            department_id,
            hire_date,
            shift
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

  QVariantList params = {
      staff.staffCode,   staff.passwordHash, staff.fullName,
      staff.avatarBytes, staff.role,         staff.gender,
      staff.dateOfBirth, staff.citizenId,    staff.phoneNumber,
      staff.email,       staff.address,      staff.departmentId,
      staff.hireDate,    staff.shift};

  QSqlQuery query = DatabaseManager::getInstance().executeQuery(insert, params);
  if (query.lastError().isValid()) {
    return false;
  }

  staffId = query.lastInsertId().toInt();
  return true;
}

bool StaffRepository::insertStaff(const StaffInsertDTO &staff) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;
  int staffId = 0;
  if (!insertStaffBase(staff, staffId)) {
    db.rollbackTransaction();
    qWarning() << "StaffRepository::insertStaff - Lỗi ghi bảng staff";
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool StaffRepository::insertDoctor(const DoctorInsertDTO &doctor) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;
  int staffId = 0;
  if (!insertStaffBase(doctor, staffId)) {
    db.rollbackTransaction();
    qWarning() << "StaffRepository::insertStaff - Lỗi ghi bảng staff";
    return false;
  }

  QString insert = R"(
        INSERT INTO doctor_profiles (
            staff_id,
            specialty,
            license_number,
            experience_years,
            consultation_fee,
            bio,
            room_id
        )
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )";

  QVariantList params = {staffId,
                         doctor.specialty,
                         doctor.licenseNumber,
                         doctor.experienceYears,
                         doctor.consultationFee,
                         doctor.bio,
                         doctor.roomId > 0 ? QVariant(doctor.roomId) : QVariant(QMetaType::fromType<int>())};

  if (db.executeQuery(insert, params).lastError().isValid()) {
    db.rollbackTransaction();
    qWarning()
        << "StaffRepository::insertDoctor - Lỗi ghi bảng doctor_profiles";
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool StaffRepository::insertNurse(const NurseInsertDTO &nurse) {
  DatabaseManager &db = DatabaseManager::getInstance();

  if (!db.beginTransaction())
    return false;

  int staffId = 0;
  if (!insertStaffBase(nurse, staffId)) {
    db.rollbackTransaction();
    qWarning() << "StaffRepository::insertStaff - Lỗi ghi bảng staff";
    return false;
  }

  QString insert = R"(
        INSERT INTO nurse_profiles (
            staff_id,
            nurse_level,
            certification
        )
        VALUES (?, ?, ?)
    )";

  QVariantList params = {staffId, nurse.nurseLevel, nurse.certification};

  if (db.executeQuery(insert, params).lastError().isValid()) {
    db.rollbackTransaction();
    qWarning() << "StaffRepository::insertNurse - Lỗi ghi bảng nurse_profiles";
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool StaffRepository::insertPharmacist(const PharmacistInsertDTO &pharmacist) {
  DatabaseManager &db = DatabaseManager::getInstance();

  if (!db.beginTransaction())
    return false;

  int staffId = 0;
  if (!insertStaffBase(pharmacist, staffId)) {
    db.rollbackTransaction();
    qWarning() << "StaffRepository::insertStaff - Lỗi ghi bảng staff";
    return false;
  }

  QString insert = R"(
        INSERT INTO pharmacist_profiles (
            staff_id,
            license_number,
            pharmacy_section,
            experience_years
        )
        VALUES (?, ?, ?, ?)
    )";

  QVariantList params = {staffId, pharmacist.licenseNumber, pharmacist.pharmacySection, pharmacist.experienceYears};

  if (db.executeQuery(insert, params).lastError().isValid()) {
    db.rollbackTransaction();
    qWarning() << "StaffRepository::insertPharmacist - Lỗi ghi bảng pharmacist_profiles";
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool StaffRepository::updateStaff(const StaffUpdateDTO &staff) {
  QString sql = R"(
        UPDATE staff
        SET
            full_name = ?,
            avatar = ?,
            gender = ?,
            date_of_birth = ?,
            citizen_id = ?,
            phone_number = ?,
            email = ?,
            address = ?,
            department_id = ?,
            shift = ?
        WHERE staff_id = ?;
    )";

  QVariantList params = {
      staff.fullName,     staff.avatarBytes, staff.gender, staff.dateOfBirth,
      staff.citizenId,    staff.phoneNumber, staff.email,  staff.address,
      staff.departmentId, staff.shift,       staff.staffId};

  QSqlQuery query = DatabaseManager::getInstance().executeQuery(sql, params);
  if (query.lastError().isValid()) {
    qWarning() << "UPDATE STAFF ERROR:" << query.lastError().text();
    return false;
  }
  return true;
}

bool StaffRepository::updateDoctor(const DoctorUpdateDTO &doctor) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  if (!updateStaff(doctor)) {
    db.rollbackTransaction();
    return false;
  }

  QString sql = R"(
        UPDATE doctor_profiles
        SET specialty = ?,
            license_number = ?,
            experience_years = ?,
            consultation_fee = ?,
            bio = ?,
            room_id = ?
        WHERE staff_id = (SELECT staff_id FROM staff WHERE staff_id = ?);
    )";

  QVariantList params = {
      doctor.specialty,       doctor.licenseNumber, doctor.experienceYears,
      doctor.consultationFee, doctor.bio,           doctor.roomId > 0 ? QVariant(doctor.roomId) : QVariant(QMetaType::fromType<int>()), doctor.staffId};

  QSqlQuery query = db.executeQuery(sql, params);
  if (query.lastError().isValid()) {
    db.rollbackTransaction();
    qWarning() << "UPDATE DOCTOR PROFILE ERROR:" << query.lastError().text();
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool StaffRepository::updateNurse(const NurseUpdateDTO &nurse) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  if (!updateStaff(nurse)) {
    db.rollbackTransaction();
    return false;
  }

  QString sql = R"(
        UPDATE nurse_profiles
        SET nurse_level = ?,
            certification = ?
        WHERE staff_id = (SELECT staff_id FROM staff WHERE staff_id = ?);
    )";

  QVariantList params = {nurse.nurseLevel, nurse.certification, nurse.staffId};

  if (db.executeQuery(sql, params).lastError().isValid()) {
    db.rollbackTransaction();
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool StaffRepository::updatePharmacist(const PharmacistUpdateDTO& pharmacist) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  if (!updateStaff(pharmacist)) {
    db.rollbackTransaction();
    return false;
  }

  QString sql = R"(
        UPDATE pharmacist_profiles
        SET license_number = ?,
            pharmacy_section = ?,
            experience_years = ?
        WHERE staff_id = (SELECT staff_id FROM staff WHERE staff_id = ?);
    )";

  QVariantList params = {pharmacist.licenseNumber, pharmacist.pharmacySection, pharmacist.experienceYears, pharmacist.staffId};

  if (db.executeQuery(sql, params).lastError().isValid()) {
    db.rollbackTransaction();
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

bool StaffRepository::updateReceptionist(const ReceptionistUpdateDTO& receptionist) {
  return updateStaff(receptionist);
}

bool StaffRepository::deactivate(int staffId) {
  QString sql = "UPDATE staff SET is_active = 0 WHERE staff_id = ?";

  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;
  if (db.executeQuery(sql, {staffId}).lastError().isValid()) {
    db.rollbackTransaction();
    qWarning() << "StaffRepository::deactivate - Lỗi update staff_id:"
               << staffId;
    return false;
  }
  return db.commitTransaction();
}

bool StaffRepository::reactivate(int staffId) {
  QString sql = "UPDATE staff SET is_active = 1 WHERE staff_id = ?";

  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;
  if (db.executeQuery(sql, {staffId}).lastError().isValid()) {
    db.rollbackTransaction();
    qWarning() << "StaffRepository::reactivate - Lỗi update staff_id:"
               << staffId;
    return false;
  }
  return db.commitTransaction();
}

std::shared_ptr<SystemUser> StaffRepository::mapRowToUser(const QSqlQuery &query) const {
    int staffId = query.value("staff_id").toInt();
    QString staffCode = query.value("staff_code").toString();
    QString passwordHash = query.value("password_hash").toString();
    QString fullName = query.value("full_name").toString();
    UserRole role = userRoleFromEn(query.value("role").toString());
    bool isActive = query.value("is_active").toBool();
    bool mustChangePassword = query.value("must_change_password").toBool();
    QByteArray avatarBytes = query.value("avatar").toByteArray();
    QPixmap avatar;

    if (!avatarBytes.isEmpty()) {
        avatar.loadFromData(avatarBytes);
    } else {
        #ifdef PROJECT_ROOT_DIR
            QString defaultPath = QString::fromUtf8(PROJECT_ROOT_DIR) +
                                "/assets/images/default_avatar.png";
            avatar.load(defaultPath);
        #endif
    }

  switch (role) {
  case UserRole::Admin: {
    return std::make_shared<Admin>(staffId, staffCode, passwordHash, fullName,
                                   avatar, role, isActive, mustChangePassword);
  }
  case UserRole::Doctor: {
    return std::make_shared<Doctor>(
        staffId, staffCode, passwordHash, fullName, avatar, role, isActive,
        mustChangePassword, query.value("doctor_specialty").toString(),
        query.value("doctor_license_number").toString(),
        query.value("doctor_experience_years").toInt(),
        query.value("doctor_consultation_fee").toInt(), 
        query.value("doctor_bio").toString());
  }
  case UserRole::Nurse: {
    return std::make_shared<Nurse>(
        staffId, staffCode, passwordHash, fullName,
        avatar, role, isActive, mustChangePassword,
        query.value("nurse_level").toString(),
        query.value("nurse_certification").toString());
  }
  case UserRole::Receptionist: {
    return std::make_shared<Receptionist>(staffId, staffCode, passwordHash,
                                          fullName, avatar, role, isActive,
                                          mustChangePassword);
  }
  case UserRole::Pharmacist: {
    return std::make_shared<Pharmacist>(
        staffId, staffCode, passwordHash, fullName, 
        avatar, role, isActive, mustChangePassword,
        query.value("pharmacist_license_number").toString(),
        query.value("pharmacist_section").toString(),
        query.value("pharmacist_experience_years").toInt()
    );
  }
  default:
    qWarning() << "StaffRepository::mapRowToUser - role không hỗ trợ:"
               << query.value("role").toString();
    return nullptr;
  }
  return nullptr;
}

static const QString SELECT_STAFF_SQL = R"(
    SELECT  
        s.staff_id,
        s.staff_code,
        s.password_hash,
        s.full_name,
        s.avatar,
        s.role,
        s.gender,
        s.phone_number,
        s.email,
        s.department_id,
        s.shift,
        s.is_active,
        s.must_change_password,

        dp.specialty            AS doctor_specialty,
        dp.license_number       AS doctor_license_number,
        dp.experience_years     AS doctor_experience_years,
        dp.consultation_fee     AS doctor_consultation_fee,
        dp.bio                  AS doctor_bio,

        np.nurse_level          AS nurse_level,
        np.certification        AS nurse_certification,

        pp.license_number       AS pharmacist_license_number,
        pp.pharmacy_section     AS pharmacist_section,
        pp.experience_years     AS pharmacist_experience_years
    FROM staff s
    LEFT JOIN doctor_profiles     dp ON s.staff_id = dp.staff_id
    LEFT JOIN nurse_profiles      np ON s.staff_id = np.staff_id
    LEFT JOIN pharmacist_profiles pp ON s.staff_id = pp.staff_id
)";

static const QString SELECT_STAFF_PROFILE_SQL = R"(
    SELECT  
        s.staff_id,
        s.staff_code,
        s.full_name,
        s.avatar,
        s.role,
        s.gender,
        s.date_of_birth,
        s.citizen_id,
        s.phone_number,
        s.email,
        s.address,
        s.department_id,
        s.hire_date,
        s.shift,
        s.is_active,

        dp.specialty            AS doctor_specialty,
        dp.license_number       AS doctor_license_number,
        dp.experience_years     AS doctor_experience_years,
        dp.consultation_fee     AS doctor_consultation_fee,
        dp.bio                  AS doctor_bio,
        dp.room_id              AS doctor_room_id,
        r.room_number           AS doctor_room_number,

        np.nurse_level          AS nurse_level,
        np.certification        AS nurse_certification,

        pp.license_number       AS pharmacist_license_number,
        pp.pharmacy_section     AS pharmacist_section,
        pp.experience_years     AS pharmacist_experience_years
    FROM staff s
    LEFT JOIN doctor_profiles dp ON s.staff_id = dp.staff_id
    LEFT JOIN nurse_profiles  np ON s.staff_id = np.staff_id
    LEFT JOIN pharmacist_profiles pp ON s.staff_id = pp.staff_id
    LEFT JOIN rooms r ON dp.room_id = r.room_id

)";

static QString buildStaffWhereClause(const StaffSearchCriteria& criteria, QVariantList& outParams) {
    QString where = " WHERE 1=1";

    if (!criteria.searchKey.trimmed().isEmpty()) {
        where += " AND (LOWER(s.staff_code) LIKE ? OR LOWER(s.full_name) LIKE ?)";
        QString pattern = "%" + criteria.searchKey.trimmed().toLower() + "%";
        outParams.append(pattern);
        outParams.append(pattern);
    }

    if (criteria.role.has_value()) {
        where += " AND s.role = ?";
        outParams.append(roleToString(criteria.role.value()));
    }

    if (auto docCriteria = dynamic_cast<const DoctorSearchCriteria*>(&criteria)) {
        if (!docCriteria->specialty.trimmed().isEmpty()) {
            where += " AND LOWER(dp.specialty) LIKE ?";
            outParams.append("%" + docCriteria->specialty.trimmed().toLower() + "%");
        }
    }

    if (auto nurseCriteria = dynamic_cast<const NurseSearchCriteria*>(&criteria)) {
        if (!nurseCriteria->nurseLevel.trimmed().isEmpty()) {
            where += " AND np.nurse_level = ?";
            outParams.append(nurseCriteria->nurseLevel.trimmed());
        }
    }

    if (auto pharmCriteria = dynamic_cast<const PharmacistSearchCriteria*>(&criteria)) {
        if (!pharmCriteria->pharmacySection.trimmed().isEmpty()) {
            where += " AND pp.pharmacy_section = ?";
            outParams.append(pharmCriteria->pharmacySection.trimmed());
        }
    }

    if (criteria.departmentId != -1) {
        where += " AND s.department_id = ?";
        outParams.append(criteria.departmentId);
    }

    if (!criteria.shift.trimmed().isEmpty()) {
        where += " AND s.shift = ?";
        outParams.append(criteria.shift.trimmed());
    }

    if (criteria.onlyActive) { where += " AND s.is_active = 1"; }

    if (!criteria.includeDeleted) { where += " AND s.is_deleted = 0"; }

    return where;
}

PagedResult<std::shared_ptr<SystemUser>> StaffRepository::searchStaffPaged(
        const StaffSearchCriteria& criteria) const {
    PagedResult<std::shared_ptr<SystemUser>> result;
    result.page = qMax(1, criteria.page);
    result.pageSize = criteria.pageSize;

    const QString fromClause = R"(
        FROM staff s
        LEFT JOIN doctor_profiles     dp ON s.staff_id = dp.staff_id
        LEFT JOIN nurse_profiles      np ON s.staff_id = np.staff_id
        LEFT JOIN pharmacist_profiles pp ON s.staff_id = pp.staff_id
    )";

    // Bước 1: Đếm tổng bản ghi khớp
    QVariantList countParams;
    QString whereClause = buildStaffWhereClause(criteria, countParams);
    QString countSql = "SELECT COUNT(DISTINCT s.staff_id)" + fromClause + whereClause;

    QSqlQuery countQuery = DatabaseManager::getInstance().selectQuery(countSql, countParams);
    if (!countQuery.next()) {
        qWarning() << "StaffRepository::searchStaffPaged - Lỗi đếm tổng bản ghi";
        result.totalCount = 0;
        return result;
    }
    result.totalCount = countQuery.value(0).toInt();

    // Bước 2: Lấy dữ liệu trang hiện tại
    QVariantList dataParams;
    whereClause = buildStaffWhereClause(criteria, dataParams);

    QString dataSql = SELECT_STAFF_SQL + whereClause + " ORDER BY s.full_name ASC";

    if (criteria.pageSize > 0) {
        int offset = (result.page - 1) * criteria.pageSize;
        dataSql += " LIMIT ? OFFSET ?";
        dataParams.append(criteria.pageSize);
        dataParams.append(offset);
    }

    QSqlQuery dataQuery = DatabaseManager::getInstance().selectQuery(dataSql, dataParams);
    while (dataQuery.next()) {
        if (auto user = mapRowToUser(dataQuery)) { result.items.append(user); }
    }

    return result;
}

/*
QList<std::shared_ptr<SystemUser>>
StaffRepository::search(const StaffSearchCriteria &criteria) const {
  ...
}
*/


std::shared_ptr<SystemUser>
StaffRepository::findByStaffCode(const QString &staffCode) const {
  QString sql =
      SELECT_STAFF_SQL +
      " WHERE s.staff_code = ? AND s.is_active = 1 AND s.is_deleted = 0";

  QSqlQuery query =
      DatabaseManager::getInstance().selectQuery(sql, {staffCode});
  if (!query.next())
    return nullptr;
  return mapRowToUser(query);
};

std::shared_ptr<SystemUser> StaffRepository::findById(int staffId) const {
  QString sql =
      SELECT_STAFF_SQL +
      " WHERE s.staff_id = ? AND s.is_active = 1 AND s.is_deleted = 0";

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {staffId});
  if (!query.next())
    return nullptr;
  return mapRowToUser(query);
}


std::unique_ptr<StaffProfileDTO> StaffRepository::queryProfile(const QString& whereClause, const QVariantList& params) const {
    QString sql = SELECT_STAFF_PROFILE_SQL + " " + whereClause;
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
    if (!query.next()) return nullptr;

    UserRole role = roleFromString(query.value("role").toString());


    auto fill = [&](StaffProfileDTO& dto) {
        dto.staffId = query.value("staff_id").toInt();
        dto.staffCode = query.value("staff_code").toString();
        dto.role = role;
        dto.isActive = query.value("is_active").toBool();
        dto.hireDate =
        QDate::fromString(query.value("hire_date").toString(), "yyyy-MM-dd");
        // dto.departmentName = query.value("department_name").toString();
        dto.fullName = query.value("full_name").toString();
        dto.gender = query.value("gender").toString();
        dto.dateOfBirth = QDate::fromString(query.value("date_of_birth").toString(), "yyyy-MM-dd");
        dto.citizenId = query.value("citizen_id").toString();
        dto.phoneNumber = query.value("phone_number").toString();
        dto.email = query.value("email").toString();
        dto.address = query.value("address").toString();
        dto.departmentId = query.value("department_id").toInt();
        dto.shift = query.value("shift").toString();
        QByteArray avatarBytes = query.value("avatar").toByteArray();
        if (!avatarBytes.isEmpty()) {
            dto.avatar.loadFromData(avatarBytes);
        } else {
            #ifdef PROJECT_ROOT_DIR
                QString defaultPath = QString::fromUtf8(PROJECT_ROOT_DIR) +
                                        "/assets/images/default_avatar.png";
                dto.avatar.load(defaultPath);
            #endif
        }
    };

    switch (role) {
    case UserRole::Doctor: {
        auto dto = std::make_unique<DoctorProfileDTO>();
        fill(*dto);
        dto->specialty       = query.value("doctor_specialty").toString();
        dto->licenseNumber   = query.value("doctor_license_number").toString();
        dto->experienceYears = query.value("doctor_experience_years").toInt();
        dto->consultationFee = query.value("doctor_consultation_fee").toDouble();
        dto->bio             = query.value("doctor_bio").toString();
        dto->roomId          = query.value("doctor_room_id").toInt();
        dto->roomNumber      = query.value("doctor_room_number").toString();
        return dto;
    }
    case UserRole::Nurse: {
        auto dto = std::make_unique<NurseProfileDTO>();
        fill(*dto);
        dto->nurseLevel    = query.value("nurse_level").toString();
        dto->certification = query.value("nurse_certification").toString();
        return dto;
    }
    case UserRole::Pharmacist: {
        auto dto = std::make_unique<PharmacistProfileDTO>();
        fill(*dto);
        dto->licenseNumber    = query.value("pharmacist_license_number").toString();
        dto->pharmacySection  = query.value("pharmacist_section").toString();
        dto->experienceYears  = query.value("pharmacist_experience_years").toInt();
        return dto;
    }
    default: {
        auto dto = std::make_unique<StaffProfileDTO>();
        fill(*dto);
        return dto;
    }
    }
}

// ── 2 hàm public giờ chỉ còn 1 dòng ─────────────────────────────────

std::unique_ptr<StaffProfileDTO>
StaffRepository::findProfileById(int staffId) const {
    return queryProfile(
        "WHERE s.staff_id = ? AND s.is_active = 1 AND s.is_deleted = 0",
        { staffId }
    );
}

std::unique_ptr<StaffProfileDTO>
StaffRepository::findProfileByStaffCode(const QString& staffCode) const {
    return queryProfile(
        "WHERE s.staff_code = ? AND s.is_active = 1 AND s.is_deleted = 0",
        { staffCode }
    );
}

std::optional<QString> StaffRepository::getLatestStaffCodeByYear(int year) {
  QString sql = R"(
        SELECT staff_code
        FROM staff
        WHERE substr(staff_code, 2, 2) = substr(CAST(? AS TEXT), -2)
        ORDER BY CAST(substr(staff_code, 4, 2) AS INTEGER) DESC
        LIMIT 1
    )";

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {year});

  if (!query.next())
    return std::nullopt;

  QVariant val = query.value("staff_code");
  if (val.isNull())
    return std::nullopt;
  return val.toString();
}

bool StaffRepository::existsByCitizenId(const QString &citizenId,
                                        int excludeStaffId) const {
  QString sql = "SELECT COUNT(*) FROM staff WHERE citizen_id = ?";
  QVariantList params;
  params << citizenId;

  if (excludeStaffId > 0) {
    sql += " AND staff_id != ?";
    params << excludeStaffId;
  }

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (query.next()) {
    int count = query.value(0).toInt();
    return count > 0;
  }

  return false;
}

bool StaffRepository::existsByPhoneNumber(const QString &phoneNumber,
                                          int excludeStaffId) const {
  QString sql = "SELECT COUNT(*) FROM staff WHERE phone_number = ?";
  QVariantList params;
  params << phoneNumber;

  if (excludeStaffId > 0) {
    sql += " AND staff_id != ?";
    params << excludeStaffId;
  }

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (query.next()) {
    int count = query.value(0).toInt();
    return count > 0;
  }

  return false;
}

bool StaffRepository::existsByEmail(const QString &email,
                                    int excludeStaffId) const {
  QString sql = "SELECT COUNT(*) FROM staff WHERE email = ?";
  QVariantList params;
  params << email;

  if (excludeStaffId > 0) {
    sql += " AND staff_id != ?";
    params << excludeStaffId;
  }

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (query.next()) {
    int count = query.value(0).toInt();
    return count > 0;
  }

  return false;
}

bool StaffRepository::existsByLicenseNumber(const QString &licenseNumber, int excludeStaffId) const {
  QString sql = R"(
        SELECT COUNT(*) FROM (
            SELECT staff_id FROM doctor_profiles WHERE license_number = ?
            UNION ALL
            SELECT staff_id FROM pharmacist_profiles WHERE license_number = ?
        ) combined
    )";
    QVariantList params = {
        licenseNumber,
        licenseNumber,
    };

  if (excludeStaffId > 0) {
    sql += " WHERE staff_id != ?";
    params << excludeStaffId;
  }

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (query.next()) {
    int count = query.value(0).toInt();
    return count > 0;
  }

  return false;
}

bool StaffRepository::existsByStaffId(int staffId) const {
  QString sql = R"(SELECT COUNT(*) FROM staff WHERE staff_id = ?;)";
  QVariantList params;
  params << staffId;

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (query.next()) {
    int count = query.value(0).toInt();
    return count > 0;
  }

  return false;
}

bool StaffRepository::updatePasswordInformation(int userId,
                                                const QString &newHash,
                                                bool mustChangePassword) {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!db.beginTransaction())
    return false;

  if (!existsByStaffId(userId)) {
    db.rollbackTransaction();
    return false;
  }

  QString sql = R"(
        UPDATE staff 
        SET 
            password_hash = ?,
            must_change_password = ?
        WHERE staff_id = ?;
    )";
  QVariantList params;
  params << newHash;
  params << (mustChangePassword ? 1 : 0);
  params << userId;

  if (db.executeQuery(sql, params).lastError().isValid()) {
    db.rollbackTransaction();
    qWarning()
        << "StaffRepository::updatePasswordInformation - Lỗi ghi bảng staff";
    return false;
  }

  if (!db.commitTransaction())
    return false;
  return true;
}

// --- Leave Management ---

LeaveBalanceDTO StaffRepository::getLeaveBalance(int staffId, int year) const {
    LeaveBalanceDTO dto;
    dto.staffId = staffId;
    dto.year = year;
    dto.totalDays = 12; // Default
    dto.usedDays = 0;

    QString sql = "SELECT total_days, used_days FROM leave_balances WHERE staff_id = ? AND year = ?";
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {staffId, year});
    
    if (query.next()) {
        dto.totalDays = query.value("total_days").toInt();
        dto.usedDays = query.value("used_days").toInt();
    } else {
        // If not exists, insert default balance
        QString insertSql = "INSERT INTO leave_balances (staff_id, year, total_days, used_days) VALUES (?, ?, 12, 0)";
        DatabaseManager::getInstance().executeQuery(insertSql, {staffId, year});
    }

    return dto;
}

bool StaffRepository::createLeaveRequest(int staffId, const QDate& startDate, const QDate& endDate, const QString& reason) const {
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;

    // 1. Insert leave_requests
    QString insertSql = R"(
        INSERT INTO leave_requests (staff_id, start_date, end_date, reason, status)
        VALUES (?, ?, ?, ?, 'PENDING')
    )";
    QSqlQuery insertQuery = db.executeQuery(insertSql, {
        staffId, 
        startDate.toString("yyyy-MM-dd"), 
        endDate.toString("yyyy-MM-dd"), 
        reason
    });

    if (insertQuery.lastError().isValid()) {
        qWarning() << "Failed to insert leave_requests:" << insertQuery.lastError().text();
        db.rollbackTransaction();
        return false;
    }

    // 2. Update leave_balances
    int days = startDate.daysTo(endDate) + 1;
    int year = startDate.year();
    
    QString updateSql = R"(
        UPDATE leave_balances 
        SET used_days = used_days + ?
        WHERE staff_id = ? AND year = ?
    )";
    QSqlQuery updateQuery = db.executeQuery(updateSql, {days, staffId, year});
    
    if (updateQuery.lastError().isValid() || updateQuery.numRowsAffected() == 0) {
        // Just in case it wasn't inserted yet
        QString insertBalSql = "INSERT INTO leave_balances (staff_id, year, total_days, used_days) VALUES (?, ?, 12, ?)";
        QSqlQuery insertBalQuery = db.executeQuery(insertBalSql, {staffId, year, days});
        if (insertBalQuery.lastError().isValid()) {
            qWarning() << "Failed to update/insert leave_balances:" << insertBalQuery.lastError().text();
            db.rollbackTransaction();
            return false;
        }
    }

    if (!db.commitTransaction()) return false;
    return true;
}

QList<LeaveRequestDTO> StaffRepository::getPendingLeaveRequests() const {
    QList<LeaveRequestDTO> list;
    QString sql = R"(
        SELECT lr.request_id, lr.staff_id, s.staff_code, s.full_name, lr.start_date, lr.end_date, lr.reason, lr.status
        FROM leave_requests lr
        JOIN staff s ON lr.staff_id = s.staff_id
        WHERE lr.status = 'PENDING'
        ORDER BY lr.created_at DESC
    )";
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {});
    while (query.next()) {
        LeaveRequestDTO dto;
        dto.requestId = query.value("request_id").toInt();
        dto.staffId = query.value("staff_id").toInt();
        dto.staffCode = query.value("staff_code").toString();
        dto.fullName = query.value("full_name").toString();
        dto.startDate = QDate::fromString(query.value("start_date").toString(), "yyyy-MM-dd");
        dto.endDate = QDate::fromString(query.value("end_date").toString(), "yyyy-MM-dd");
        dto.reason = query.value("reason").toString();
        dto.status = query.value("status").toString();
        list.append(dto);
    }
    return list;
}

QList<LeaveRequestDTO> StaffRepository::getLeaveHistory(int staffId) const {
    QList<LeaveRequestDTO> list;
    QString sql = R"(
        SELECT lr.request_id, lr.staff_id, s.staff_code, s.full_name, lr.start_date, lr.end_date, lr.reason, lr.status
        FROM leave_requests lr
        JOIN staff s ON lr.staff_id = s.staff_id
        WHERE lr.staff_id = ?
        ORDER BY lr.created_at DESC
    )";
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {staffId});
    while (query.next()) {
        LeaveRequestDTO dto;
        dto.requestId = query.value("request_id").toInt();
        dto.staffId = query.value("staff_id").toInt();
        dto.staffCode = query.value("staff_code").toString();
        dto.fullName = query.value("full_name").toString();
        dto.startDate = QDate::fromString(query.value("start_date").toString(), "yyyy-MM-dd");
        dto.endDate = QDate::fromString(query.value("end_date").toString(), "yyyy-MM-dd");
        dto.reason = query.value("reason").toString();
        dto.status = query.value("status").toString();
        list.append(dto);
    }
    return list;
}

bool StaffRepository::updateLeaveRequestStatus(int requestId, const QString& status) const {
    QString sql = "UPDATE leave_requests SET status = ?, updated_at = datetime('now') WHERE request_id = ?";
    QSqlQuery query = DatabaseManager::getInstance().executeQuery(sql, {status, requestId});
    return !query.lastError().isValid();
}

bool StaffRepository::approveLeaveRequest(int requestId) const {
    return updateLeaveRequestStatus(requestId, "APPROVED");
}

bool StaffRepository::rejectLeaveRequest(int requestId, int staffId, int year, int days) const {
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;

    if (!updateLeaveRequestStatus(requestId, "REJECTED")) {
        db.rollbackTransaction();
        return false;
    }

    QString updateSql = R"(
        UPDATE leave_balances 
        SET used_days = used_days - ?
        WHERE staff_id = ? AND year = ?
    )";
    QSqlQuery updateQuery = db.executeQuery(updateSql, {days, staffId, year});
    if (updateQuery.lastError().isValid()) {
        db.rollbackTransaction();
        return false;
    }

    return db.commitTransaction();
}

std::optional<LeaveRequestDTO> StaffRepository::getLeaveRequestById(int requestId) const {
    QString sql = R"(
        SELECT lr.request_id, lr.staff_id, s.staff_code, s.full_name, lr.start_date, lr.end_date, lr.reason, lr.status
        FROM leave_requests lr
        JOIN staff s ON lr.staff_id = s.staff_id
        WHERE lr.request_id = ?
    )";
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {requestId});
    if (query.next()) {
        LeaveRequestDTO dto;
        dto.requestId = query.value("request_id").toInt();
        dto.staffId = query.value("staff_id").toInt();
        dto.staffCode = query.value("staff_code").toString();
        dto.fullName = query.value("full_name").toString();
        dto.startDate = QDate::fromString(query.value("start_date").toString(), "yyyy-MM-dd");
        dto.endDate = QDate::fromString(query.value("end_date").toString(), "yyyy-MM-dd");
        dto.reason = query.value("reason").toString();
        dto.status = query.value("status").toString();
        return dto;
    }
    return std::nullopt;
}

bool StaffRepository::isStaffOnLeave(int staffId, const QDate& date) const {
    QString sql = R"(
        SELECT 1 FROM leave_requests 
        WHERE staff_id = ? AND status = 'APPROVED'
          AND start_date <= ? AND end_date >= ?
    )";
    QString dateStr = date.toString("yyyy-MM-dd");
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {staffId, dateStr, dateStr});
    
    return query.next();
}