#include "StaffRepository.h"
#include "DatabaseManager.h"
#include "model/Doctor.h"
#include <QDir>
#include <QIODevice>
#include <QDir>


bool StaffRepository::insertStaffBase(const StaffInsertDTO& staff, int& staffId) {
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
        staff.staffCode,
        staff.passwordHash,
        staff.fullName,
        staff.avatarBytes,
        staff.role,
        staff.gender,
        staff.dateOfBirth,
        staff.citizenId,
        staff.phoneNumber,
        staff.email,
        staff.address,
        staff.departmentId,
        staff.hireDate,
        staff.shift
    };

    if (!DatabaseManager::getInstance().executeQuery(insert, params)) { return false; }

    QSqlQuery lastId = DatabaseManager::getInstance().selectQuery("SELECT last_insert_rowid()");
    if (!lastId.next()) return false;
    staffId = lastId.value(0).toInt();
    return true;
}

bool StaffRepository::insertStaff(const StaffInsertDTO& staff) {
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;
    int staffId = 0;
    if (!insertStaffBase(staff, staffId)) {
        db.rollbackTransaction();
        qWarning() << "StaffRepository::insertStaff - Lỗi ghi bảng staff";
        return false;
    }

    if (!db.commitTransaction()) return false;
    return true;
}

bool StaffRepository::insertDoctor(const DoctorInsertDTO& doctor) {
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;
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
            bio
        )
        VALUES (?, ?, ?, ?, ?, ?)
    )";

    QVariantList params = {
        staffId, 
        doctor.specialty, 
        doctor.licenseNumber, 
        doctor.experienceYears,
        doctor.consultationFee, 
        doctor.bio
    };

    if (!db.executeQuery(insert, params)) {
        db.rollbackTransaction();
        qWarning() << "StaffRepository::insertDoctor - Lỗi ghi bảng doctor_profiles";
        return false;
    }

    if (!db.commitTransaction()) return false;
    return true;
}

bool StaffRepository::insertNurse(const NurseInsertDTO& nurse) {
    DatabaseManager& db = DatabaseManager::getInstance();

    if (!db.beginTransaction()) return false;

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

    QVariantList params = {
        staffId, 
        nurse.nurseLevel, 
        nurse.certification
    };

    if (!db.executeQuery(insert, params)) {
        db.rollbackTransaction();
        qWarning() << "StaffRepository::insertNurse - Lỗi ghi bảng nurse_profiles";
        return false;
    }

    if (!db.commitTransaction()) return false;
    return true;
}

bool StaffRepository::updateStaff(const StaffUpdateDTO& staff) {
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
        staff.fullName, 
        staff.avatarBytes,
        staff.gender, 
        staff.dateOfBirth, 
        staff.citizenId, 
        staff.phoneNumber,
        staff.email, 
        staff.address, 
        staff.departmentId, 
        staff.shift,
        staff.staffId
    };

    if (!DatabaseManager::getInstance().executeQuery(sql, params)) return false;
    return true;
}

bool StaffRepository::updateDoctor(const DoctorUpdateDTO& doctor) {
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;

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
            bio = ?
        WHERE staff_id = (SELECT staff_id FROM staff WHERE staff_id = ?);
    )";

    QVariantList params = {
        doctor.specialty, 
        doctor.licenseNumber, 
        doctor.experienceYears,
        doctor.consultationFee, 
        doctor.bio, 
        doctor.staffId
    };

    if (!db.executeQuery(sql, params)) {
        db.rollbackTransaction();
        return false;
    }

    if (!db.commitTransaction()) return false;
    return true;
}

bool StaffRepository::updateNurse(const NurseUpdateDTO& nurse) {
DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;

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

    if (!db.executeQuery(sql, params)) {
        db.rollbackTransaction();
        return false;
    }

    if (!db.commitTransaction()) return false;
    return true;
}



bool StaffRepository::deactivate(int staffId) {
    QString sql = "UPDATE staff SET is_active = 0 WHERE staff_id = ?";
 
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;
    if (!db.executeQuery(sql, { staffId })) {
        db.rollbackTransaction();
        qWarning() << "StaffRepository::deactivate - Lỗi update staff_id:" << staffId;
        return false;
    }
    return db.commitTransaction();
}

bool StaffRepository::reactivate(int staffId) {
    QString sql = "UPDATE staff SET is_active = 1 WHERE staff_id = ?";
 
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) return false;
    if (!db.executeQuery(sql, { staffId })) {
        db.rollbackTransaction();
        qWarning() << "StaffRepository::reactivate - Lỗi update staff_id:" << staffId;
        return false;
    }
    return db.commitTransaction();
}



std::shared_ptr<SystemUser> StaffRepository::mapRowToUser(const QSqlQuery& query) const {
    int         staffId        = query.value("staff_id").toInt();
    QString     staffCode      = query.value("staff_code").toString();
    QString     passwordHash   = query.value("password_hash").toString();
    QString     fullName       = query.value("full_name").toString();
    UserRole    role           = roleFromString(query.value("role").toString());
    bool        isActive       = query.value("is_active").toBool();
    QByteArray  avatarBytes    = query.value("avatar").toByteArray();
    QPixmap     avatar;

    if (!avatarBytes.isEmpty()) {
        avatar.loadFromData(avatarBytes);
    } 
    else {
        #ifdef PROJECT_ROOT_DIR
            QString defaultPath = QString::fromUtf8(PROJECT_ROOT_DIR) + "/assets/images/default_avatar.png";
            avatar.load(defaultPath);
        #endif
    }
    
    switch(role) {
        case UserRole::Doctor: {
            return std::make_shared<Doctor>(
                staffId, staffCode, passwordHash, fullName, avatar, role, isActive,
                query.value("specialty").toString(),
                query.value("license_number").toString(),
                query.value("experience_years").toInt(),
                query.value("consultation_fee").toInt(),
                query.value("bio").toString()
            );
        }
        // case UserRole::Nurse: {
        //     return std::make_shared<Nurse>(
        //         staffId, staffCode, passwordHash, fullName, avatar, isActive,
        //         query.value("nurse_level").toString(),
        //         query.value("certification").toString()
        //     );
        // }
        // case UserRole::Receptionist: {
        //     return std::make_shared<Receptionist>(
        //         staffId, staffCode, passwordHash, fullName, avatar, isActive
        //     );
        // }
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

        dp.specialty,
        dp.license_number,
        dp.experience_years,
        dp.consultation_fee,
        dp.bio,

        np.nurse_level,
        np.certification
    FROM staff s
    LEFT JOIN doctor_profiles dp ON s.staff_id = dp.staff_id
    LEFT JOIN nurse_profiles  np ON s.staff_id = np.staff_id
)";

static const QString SELECT_STAFF_PROFILE_SQL = R"(
    SELECT  
        s.staff_id,
        s.staff_code,
        s.full_name,
        s.avatar,
        s.role,
        s.gender,
        s.date_of_birth
        s.citizen_id
        s.phone_number,
        s.email,
        s.address,
        s.department_id,
        s.hire_date
        s.shift,
        s.is_active,

        dp.specialty,
        dp.license_number,
        dp.experience_years,
        dp.consultation_fee,
        dp.bio,

        np.nurse_level,
        np.certification
    FROM staff s
    LEFT JOIN doctor_profiles dp ON s.staff_id = dp.staff_id
    LEFT JOIN nurse_profiles  np ON s.staff_id = np.staff_id
)";

QList<std::shared_ptr<SystemUser>> StaffRepository::search(const StaffSearchCriteria& criteria) const {
    QString search = SELECT_STAFF_SQL + " WHERE 1=1";

    QVariantList params;

    // Chỉ thêm điều kiện khi field thực sự có giá trị
    if (!criteria.searchKey.trimmed().isEmpty()) {
        search += " AND (s.staff_code LIKE ? OR s.full_name LIKE ?)";
        QString keyword = "%" + criteria.searchKey + "%";
        params << keyword << keyword;
    }

    if (criteria.role.has_value()) {
        search += " AND s.role = ?";
        params << roleToString(criteria.role.value());
    }

    if (!criteria.specialty.trimmed().isEmpty()) {
        search += " AND dp.specialty LIKE ?";
        params << "%" + criteria.specialty.trimmed() + "%";
    }

    if (criteria.departmentId != -1) {
        search += " AND s.department_id = ?";
        params << criteria.departmentId;
    }

    if (!criteria.shift.trimmed().isEmpty()) {
        search += " AND s.shift = ?";
        params << criteria.shift.trimmed();
    }

    if (criteria.onlyActive) {
        search += " AND s.is_active = 1";
    }

    if (!criteria.includeDeleted) {
        search += " AND s.is_deleted = 0";
    }

    search += " ORDER BY s.full_name ASC";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(search, params);

    QList<std::shared_ptr<SystemUser>> result;
    while (query.next()) {
        if (auto user = mapRowToUser(query)) {
            result.append(user);
        }
    }
    return result;
}

std::shared_ptr<SystemUser> StaffRepository::findByStaffCode(const QString& staffCode) const {
    QString sql = SELECT_STAFF_SQL + " WHERE s.staff_code = ? AND s.is_active = 1 AND s.is_deleted = 0";
 
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { staffCode });
    if (!query.next()) return nullptr;
    return mapRowToUser(query);
};

std::shared_ptr<SystemUser> StaffRepository::findById(int staffId) const {
    QString sql = SELECT_STAFF_SQL + " WHERE s.staff_id = ? AND s.is_active = 1 AND s.is_deleted = 0";
 
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { staffId });
    if (!query.next()) return nullptr;
    return mapRowToUser(query);
}

std::unique_ptr<StaffProfileDTO> StaffRepository::findProfileById(int staffId) const {
    QString sql = SELECT_STAFF_PROFILE_SQL + " WHERE s.staff_id = ? AND s.is_active = 1 AND s.is_deleted = 0";
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(SELECT_STAFF_PROFILE_SQL, { staffId });
    if (!query.next()) return nullptr;

    UserRole role = roleFromString(query.value("role").toString());
    // Helper lambda fill base fields — tránh lặp code
    auto fillBase = [&](StaffProfileDTO& dto) {
        dto.staffId        = query.value("staff_id").toInt();
        dto.staffCode      = query.value("staff_code").toString();
        dto.role           = role;
        dto.isActive       = query.value("is_active").toBool();
        dto.hireDate       = QDate::fromString(query.value("hire_date").toString(), "yyyy-MM-dd");
        //dto.departmentName = query.value("department_name").toString();
        dto.fullName       = query.value("full_name").toString();
        dto.gender         = genderFromString(query.value("gender").toString());
        dto.dateOfBirth    = QDate::fromString(query.value("date_of_birth").toString(), "yyyy-MM-dd");
        dto.citizenId      = query.value("citizen_id").toString();
        dto.phoneNumber    = query.value("phone_number").toString();
        dto.email          = query.value("email").toString();
        dto.address        = query.value("address").toString();
        dto.departmentId   = query.value("department_id").toInt();
        dto.shift          = query.value("shift").toString();
        QByteArray avatarBytes = query.value("avatar").toByteArray();
        if (!avatarBytes.isEmpty()) {
        dto.avatar.loadFromData(avatarBytes);
        } 
        else {
            #ifdef PROJECT_ROOT_DIR
                QString defaultPath = QString::fromUtf8(PROJECT_ROOT_DIR) + "/assets/images/default_avatar.png";
                dto.avatar.load(defaultPath);
            #endif
        }
    };

    // Tạo đúng subtype dựa trên role → không bị slicing
    switch (role) {
        case UserRole::Doctor: {
            auto dto = std::make_unique<DoctorProfileDTO>();
            fillBase(*dto);
            dto->specialty       = query.value("specialty").toString();
            dto->licenseNumber   = query.value("license_number").toString();
            dto->experienceYears = query.value("experience_years").toInt();
            dto->consultationFee = query.value("consultation_fee").toInt();
            dto->bio             = query.value("bio").toString();
            return dto;
        }
        case UserRole::Nurse: {
            auto dto = std::make_unique<NurseProfileDTO>();
            fillBase(*dto);
            dto->nurseLevel    = query.value("nurse_level").toString();
            dto->certification = query.value("certification").toString();
            return dto;
        }
        default: {
            // auto dto = std::make_unique<ReceptionistProfileDTO>();
            // fillBase(*dto);
            // return dto;
        }
    }
}

std::unique_ptr<StaffProfileDTO> StaffRepository::findProfileByStaffCode(const QString& staffCode) const {
    QString sql = SELECT_STAFF_PROFILE_SQL + " WHERE s.staff_code = ? AND s.is_active = 1 AND s.is_deleted = 0";
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(SELECT_STAFF_PROFILE_SQL, { staffCode });
    if (!query.next()) return nullptr;

    UserRole role = roleFromString(query.value("role").toString());
    // Helper lambda fill base fields — tránh lặp code
    auto fillBase = [&](StaffProfileDTO& dto) {
        dto.staffId        = query.value("staff_id").toInt();
        dto.staffCode      = query.value("staff_code").toString();
        dto.role           = role;
        dto.isActive       = query.value("is_active").toBool();
        dto.hireDate       = QDate::fromString(query.value("hire_date").toString(), "yyyy-MM-dd");
        //dto.departmentName = query.value("department_name").toString();
        dto.fullName       = query.value("full_name").toString();
        dto.gender         = genderFromString(query.value("gender").toString());
        dto.dateOfBirth    = QDate::fromString(query.value("date_of_birth").toString(), "yyyy-MM-dd");
        dto.citizenId      = query.value("citizen_id").toString();
        dto.phoneNumber    = query.value("phone_number").toString();
        dto.email          = query.value("email").toString();
        dto.address        = query.value("address").toString();
        dto.departmentId   = query.value("department_id").toInt();
        dto.shift          = query.value("shift").toString();
        QByteArray avatarBytes = query.value("avatar").toByteArray();
        if (!avatarBytes.isEmpty()) {
        dto.avatar.loadFromData(avatarBytes);
        } 
        else {
            #ifdef PROJECT_ROOT_DIR
                QString defaultPath = QString::fromUtf8(PROJECT_ROOT_DIR) + "/assets/images/default_avatar.png";
                dto.avatar.load(defaultPath);
            #endif
        }
    };

    // Tạo đúng subtype dựa trên role → không bị slicing
    switch (role) {
        case UserRole::Doctor: {
            auto dto = std::make_unique<DoctorProfileDTO>();
            fillBase(*dto);
            dto->specialty       = query.value("specialty").toString();
            dto->licenseNumber   = query.value("license_number").toString();
            dto->experienceYears = query.value("experience_years").toInt();
            dto->consultationFee = query.value("consultation_fee").toInt();
            dto->bio             = query.value("bio").toString();
            return dto;
        }
        case UserRole::Nurse: {
            auto dto = std::make_unique<NurseProfileDTO>();
            fillBase(*dto);
            dto->nurseLevel    = query.value("nurse_level").toString();
            dto->certification = query.value("certification").toString();
            return dto;
        }
        default: {
            // auto dto = std::make_unique<ReceptionistProfileDTO>();
            // fillBase(*dto);
            // return dto;
        }
    }
}

std::optional<QString> StaffRepository::getLatestStaffCodeByYear(int year) {
    QString sql = R"(
        SELECT staff_code
        FROM staff
        WHERE substr(staff_code, 2, 2) = substr(CAST(? AS TEXT), -2)
        ORDER BY CAST(substr(staff_code, 4, 2) AS INTEGER) DESC
        LIMIT 1
    )";
 
    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { year });
 
    if (!query.next()) return std::nullopt;
 
    QVariant val = query.value("staff_code");
    if (val.isNull()) return std::nullopt;
    return val.toString();
}


bool StaffRepository::existsByCitizenId(const QString& citizenId, int excludeStaffId) const {
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

bool StaffRepository::existsByPhoneNumber(const QString& phoneNumber, int excludeStaffId) const {
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

bool StaffRepository::existsByEmail(const QString& email, int excludeStaffId) const {
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

bool StaffRepository::existsByLicenseNumber(const QString& licenseNumber, int excludeStaffId) const {
    QString sql = "SELECT COUNT(*) FROM doctor_profiles WHERE license_number = ?";
    QVariantList params;
    params << licenseNumber;

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


    