#include "StaffRepository.h"
#include "../model/Doctor.h"


bool StaffRepository::insertStaffBase(const StaffInsertDTO& staff, int& staffId) {
    QString insert = R"(
        INSERT INTO staff (
            staff_code,
            password_hash,
            full_name,
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
        roleToString(staff.role),
        genderToString(staff.gender),
        staff.dateOfBirth,
        staff.nationalId,
        staff.phoneNumber,
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







std::shared_ptr<SystemUser> StaffRepository::mapRowToUser(const QSqlQuery& query) const {
    int      staffId      = query.value("staff_id").toInt();
    QString  staffCode    = query.value("staff_code").toString();
    QString  username     = query.value("username").toString();
    QString  passwordHash = query.value("password_hash").toString();
    QString  fullName     = query.value("full_name").toString();
    UserRole role         = roleFromString(query.value("role").toString());
    bool     isActive     = query.value("is_active").toBool();

    switch(role) {
        case UserRole::Doctor: {
            QString specialty       = query.value("specialty").toString();
            QString licenseNumber   = query.value("license_number").toString();
            int     experienceYears = query.value("experience_years").toInt();
            double  consultationFee = query.value("consultation_fee").toDouble();

            return std::make_shared<Doctor>(
                staffId, staffCode, username, passwordHash, fullName, isActive,
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

}

std::optional<std::shared_ptr<SystemUser>> StaffRepository::findByStaffCode(const QString& staffCode) const {
    QString sql = R"(
        SELECT
            s.staff_id,
            s.staff_code,
            s.password_hash,
            s.full_name,
            s.role,
            s.gender,
            s.phoneNumber,
            s.email,
            s.departmentId,
            s.shift,
            s.is_active,

            d.specialty,
            d.license_number,
            d.experience_years,
            d.consultation_fee,
            d.bio,

            n.nurse_level,
            n.certification

        FROM Staffs s
        LEFT JOIN doctor_profiles dp ON s.staff_id = dp.staff_id
        LEFT JOIN nurse_profiles  np ON s.staff_id = np.staff_id

        WHERE s.staff_code  = ?
          AND s.is_deleted = 0
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { staffCode });

    if (!query.next()) {
        return std::nullopt; // không tìm thấy
    }

    return mapRowToUser(query);
}

std::optional<std::shared_ptr<SystemUser>> StaffRepository::findById(int staffId) const {
    QString sql = R"(
        SELECT
            s.staff_id,
            s.staff_code,
            s.password_hash,
            s.full_name,
            s.role,
            s.gender,
            s.phoneNumber,
            s.email,
            s.departmentId,
            s.shift,
            s.is_active,

            d.specialty,
            d.license_number,
            d.experience_years,
            d.consultation_fee,
            d.bio,

            n.nurse_level,
            n.certification

        FROM Staffs s
        LEFT JOIN doctor_profiles dp ON s.staff_id = dp.staff_id
        LEFT JOIN nurse_profiles  np ON s.staff_id = np.staff_id

        WHERE s.staff_id  = ?
          AND s.is_deleted = 0
    )";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, { staffId });

    if (!query.next()) {
        return std::nullopt; // không tìm thấy
    }

    return mapRowToUser(query);
}


