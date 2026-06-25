#include "StaffRepository.h"
#include "../model/Doctor.h"
#include <QBuffer>
#include <QIODevice>

bool StaffRepository::insertStaffBase(const StaffInsertDTO& staff, int& staffId) {
    QByteArray avatarBytes;
    if (!staff.avatar.isNull()) {
        QBuffer buffer(&avatarBytes);
        buffer.open(QIODevice::WriteOnly);
        staff.avatar.save(&buffer, "PNG"); // Lưu dạng PNG để giữ chất lượng
    }

    QString insert = R"(
        INSERT INTO staff (
            staff_code,
            password_hash,
            full_name,
            avatar,
            role,
            gender,
            date_of_birth,
            national_id,
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
        avatarBytes,
        roleToString(staff.role),
        genderToString(staff.gender),
        staff.dateOfBirth,
        staff.nationalId,
        staff.phoneNumber,
        staff.email,
        staff.address,
        staff.departmentId,
        staff.hireDate,
        staff.shift
    };

    if(!DatabaseManager::getInstance().executeQuery(insert, params)) {
        return false;
    }

    QSqlQuery lastId = DatabaseManager::getInstance().selectQuery("SELECT last_insert_rowid()");
    if (!lastId.next()) return false;
    staffId = lastId.value(0).toInt();
    return true;

}

bool StaffRepository::insertStaff(const StaffInsertDTO& staff) {
    DatabaseManager& db = DatabaseManager::getInstance();
    if(!db.beginTransaction()) return false;
    int staffId = 0;
    if(!insertStaffBase(staff, staffId)){
        db.rollbackTransaction();
        qWarning() << "StaffRepository::insertStaff - Lỗi ghi bảng staff";
        return false;
    }

    if(!db.commitTransaction()) return false;
    return true;
}

bool StaffRepository::insertDoctor(const DoctorInsertDTO& doctor) {
    DatabaseManager& db = DatabaseManager::getInstance();
    if(!db.beginTransaction()) return false;
    int staffId = 0;
    if(!insertStaffBase(doctor, staffId)){
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

    if(!db.executeQuery(insert, params)){
        db.rollbackTransaction();
        qWarning() << "StaffRepository::insertDoctor - Lỗi ghi bảng doctor_profiles";
        return false;
    }

    if(!db.commitTransaction()) return false;
    return true;
}


bool StaffRepository::insertNurse(const NurseInsertDTO& nurse) {
    DatabaseManager& db = DatabaseManager::getInstance();
    
    if(!db.beginTransaction()) return false;

    int staffId = 0;
    if(!insertStaffBase(nurse, staffId)){
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

    if(!db.executeQuery(insert, params)){
        db.rollbackTransaction();
        qWarning() << "StaffRepository::insertNurse - Lỗi ghi bảng nurse_profiles";
        return false;
    }

    if(!db.commitTransaction()) return false;
    return true;
}


bool StaffRepository::deactivate(int staffId){
    DatabaseManager& db = DatabaseManager::getInstance();
    QString deactivate = R"(
        UPDATE  staff
        SET     active = 0
        WHERE   staff_id = ?
    )";

    if(!db.beginTransaction()) return false;
    if(!db.executeQuery(deactivate, { staffId })){
        db.rollbackTransaction();
        qWarning() << "StaffRepository::deactivate - Lỗi update bảng staff";
        return false;
    }
    if(!db.commitTransaction()) return false;
    return true;
}

bool StaffRepository::reactivate(int staffId){
    DatabaseManager& db = DatabaseManager::getInstance();
    QString deactivate = R"(
        UPDATE  staff
        SET     active = 1
        WHERE   staff_id = ?
    )";

    if(!db.beginTransaction()) return false;
    if(!db.executeQuery(deactivate, { staffId })){
        db.rollbackTransaction();
        qWarning() << "StaffRepository::deactivate - Lỗi update bảng staff";
        return false;
    }
    if(!db.commitTransaction()) return false;
    return true;
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
    } else {
        avatar.load(":/assets/images/default_avatar.png");
    }

    
    switch(role) {
        case UserRole::Doctor: {
            QString specialty       = query.value("specialty").toString();
            QString licenseNumber   = query.value("license_number").toString();
            int     experienceYears = query.value("experience_years").toInt();
            double  consultationFee = query.value("consultation_fee").toDouble();

            return std::make_shared<Doctor>(
                staffId, staffCode, passwordHash, fullName, avatar, role, isActive,
                specialty, licenseNumber, experienceYears, consultationFee
            );
        }
        // case UserRole::Nurse: {
        //     //return std::make_shared<Nurse>(id, username, passwordHash, full_name, role);
        // }
        // case UserRole::Receptionist: {
        // //     return std::make_shared<Receptionist>(id, username, passwordHash, full_name, role);
        // }
    }
    return nullptr;
}


QList<std::shared_ptr<SystemUser>> StaffRepository::search(const StaffSearchCriteria& criteria) const {
    QString search = R"(
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
        WHERE 1 = 1
    )";

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


std::optional<std::shared_ptr<SystemUser>> StaffRepository::findByStaffCode(const QString& staffCode) const {
    QString find = R"(
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

        WHERE s.staff_code  = ?
          AND s.is_deleted = 0
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(find, { staffCode });

    if (!query.next()) {
        return std::nullopt; // không tìm thấy
    }

    return mapRowToUser(query);
}

std::optional<std::shared_ptr<SystemUser>> StaffRepository::findById(int staffId) const {
    QString find = R"(
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

        WHERE s.staff_id  = ?
          AND s.is_deleted = 0
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(find, { staffId });

    if (!query.next()) {
        return std::nullopt; // không tìm thấy
    }

    return mapRowToUser(query);
}


