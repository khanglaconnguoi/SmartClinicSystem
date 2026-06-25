/**
 * @file    PatientRepository.cpp
 * @brief   Implementation cho PatientRepository — SQL queries.
 */
#include "PatientRepository.h"
#include "model/OutPatient.h"
#include "model/InPatient.h"
#include "model/EmergencyPatient.h"
#include "model/MedicalRecord.h"
#include <QDate>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

PatientRepository::PatientRepository(QSqlDatabase& db)
    : m_db(db) {
}

bool PatientRepository::insert(std::shared_ptr<Patient> patient) {
    if (!patient) return false;
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO patients "
        "(patient_code, full_name, birth_date, gender, phone_number, address, "
        " blood_type, allergies, medical_history, citizen_id, email, insurance, is_active, state, patient_type) "
        "VALUES "
        "(:patient_code, :full_name, :birth_date, :gender, :phone_number, :address, "
        " :blood_type, :allergies, :medical_history, :citizen_id, :email, :insurance, :is_active, :state, :patient_type)"
    );

    query.bindValue(":patient_code", patient->patientCode());
    query.bindValue(":full_name", patient->fullName());
    query.bindValue(":birth_date", patient->birthDate().toString("yyyy-MM-dd"));
    query.bindValue(":gender", static_cast<int>(patient->gender()));
    query.bindValue(":phone_number", patient->phoneNumber());
    query.bindValue(":address", patient->address());
    query.bindValue(":blood_type", patient->bloodType());
    query.bindValue(":allergies", patient->allergies());
    query.bindValue(":medical_history", patient->medicalHistory());
    query.bindValue(":citizen_id", patient->citizenId());
    query.bindValue(":email", patient->email());
    query.bindValue(":insurance", patient->insurance());
    query.bindValue(":is_active", patient->isActive() ? 1 : 0);
    query.bindValue(":state", static_cast<int>(patient->stateType()));
    query.bindValue(":patient_type", static_cast<int>(patient->getType()));

    if (!query.exec()) {
        qCritical() << "Failed to insert patient:"
                    << query.lastError().text();
        return false;
    }

    // Gán ID tự tăng cho đối tượng Patient
    patient->setId(query.lastInsertId().toInt());
    qDebug() << "Inserted patient with ID:" << patient->id();
    return true;
}

bool PatientRepository::update(std::shared_ptr<Patient> patient) {
    if (!patient) return false;
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE patients SET "
        "  patient_code = :patient_code,"
        "  full_name    = :full_name,"
        "  birth_date   = :birth_date,"
        "  gender       = :gender,"
        "  phone_number = :phone_number,"
        "  address      = :address,"
        "  blood_type   = :blood_type,"
        "  allergies    = :allergies,"
        "  medical_history = :medical_history,"
        "  citizen_id   = :citizen_id,"
        "  email        = :email,"
        "  insurance    = :insurance,"
        "  is_active    = :is_active,"
        "  state        = :state, "
        "  patient_type = :patient_type "
        "WHERE id = :id"
    );

    query.bindValue(":id", patient->id());
    query.bindValue(":patient_code", patient->patientCode());
    query.bindValue(":full_name", patient->fullName());
    query.bindValue(":birth_date", patient->birthDate().toString("yyyy-MM-dd"));
    query.bindValue(":gender", static_cast<int>(patient->gender()));
    query.bindValue(":phone_number", patient->phoneNumber());
    query.bindValue(":address", patient->address());
    query.bindValue(":blood_type", patient->bloodType());
    query.bindValue(":allergies", patient->allergies());
    query.bindValue(":medical_history", patient->medicalHistory());
    query.bindValue(":citizen_id", patient->citizenId());
    query.bindValue(":email", patient->email());
    query.bindValue(":insurance", patient->insurance());
    query.bindValue(":is_active", patient->isActive() ? 1 : 0);
    query.bindValue(":state", static_cast<int>(patient->stateType()));
    query.bindValue(":patient_type", static_cast<int>(patient->getType()));

    if (!query.exec()) {
        qCritical() << "Failed to update patient (ID:"
                    << patient->id() << "):"
                    << query.lastError().text();
        return false;
    }

    qDebug() << "Updated patient ID:" << patient->id();
    return true;
}

bool PatientRepository::softDelete(int patientId) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE patients SET is_active = 0 WHERE id = :id");
    query.bindValue(":id", patientId);

    if (!query.exec()) {
        qCritical() << "Failed to soft-delete patient (ID:"
                    << patientId << "):"
                    << query.lastError().text();
        return false;
    }

    qDebug() << "Soft-deleted patient ID:" << patientId;
    return true;
}

std::shared_ptr<Patient> PatientRepository::findById(int patientId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM patients WHERE id = :id");
    query.bindValue(":id", patientId);

    if (!query.exec()) {
        qCritical() << "Failed to find patient (ID:"
                    << patientId << "):"
                    << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        return mapRowToPatient(query);
    }
    return nullptr;
}

QList<std::shared_ptr<Patient>> PatientRepository::findAllActive() {
    QList<std::shared_ptr<Patient>> patients;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM patients WHERE is_active = 1 ORDER BY id DESC");

    if (!query.exec()) {
        qCritical() << "Failed to fetch active patients:"
                    << query.lastError().text();
        return patients;
    }

    while (query.next()) {
        patients.append(mapRowToPatient(query));
    }

    qDebug() << "Loaded" << patients.size() << "active patients";
    return patients;
}

QList<std::shared_ptr<Patient>> PatientRepository::searchByName(const QString& keyword) {
    QList<std::shared_ptr<Patient>> patients;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT * FROM patients "
        "WHERE is_active = 1 AND full_name LIKE :keyword "
        "ORDER BY id DESC"
    );
    query.bindValue(":keyword", "%" + keyword + "%");

    if (!query.exec()) {
        qCritical() << "Failed to search patients:"
                    << query.lastError().text();
        return patients;
    }

    while (query.next()) {
        patients.append(mapRowToPatient(query));
    }

    qDebug() << "Search found" << patients.size()
             << "patients for keyword:" << keyword;
    return patients;
}

std::vector<std::shared_ptr<Patient>> PatientRepository::searchPatients(const PatientSearchCriteria& criteria) const {
    std::vector<std::shared_ptr<Patient>> results;
    
    QString queryString = "SELECT * FROM patients WHERE 1=1 ";
    
    if (!criteria.nameKeyword.isEmpty()) {
        queryString += " AND full_name LIKE :name ";
    }
    if (!criteria.phoneNumber.isEmpty()) {
        queryString += " AND phone_number LIKE :phone ";
    }
    if (!criteria.patientCode.isEmpty()) {
        queryString += " AND patient_code LIKE :code ";
    }
    if (!criteria.bloodType.isEmpty()) {
        queryString += " AND blood_type = :blood ";
    }
    if (!criteria.citizenId.isEmpty()) {
        queryString += " AND citizen_id LIKE :citizen ";
    }
    
    // Xử lý tuổi: SQLite lưu birth_date dạng chuỗi, nhưng ta có thể lấy theo năm
    // Cách tốt nhất là filter in-memory cho phần age nếu dùng SQLite đơn giản, hoặc dùng strftime
    // Để tối ưu và an toàn, ta lấy dữ liệu thô và filter in-memory cho những thứ phức tạp như tuổi.
    // Dưới đây chỉ lọc theo LIKE.
    
    QSqlQuery query(m_db);
    query.prepare(queryString);
    
    if (!criteria.nameKeyword.isEmpty()) {
        query.bindValue(":name", "%" + criteria.nameKeyword + "%");
    }
    if (!criteria.phoneNumber.isEmpty()) {
        query.bindValue(":phone", "%" + criteria.phoneNumber + "%");
    }
    if (!criteria.patientCode.isEmpty()) {
        query.bindValue(":code", "%" + criteria.patientCode + "%");
    }
    if (!criteria.bloodType.isEmpty()) {
        query.bindValue(":blood", criteria.bloodType);
    }
    if (!criteria.citizenId.isEmpty()) {
        query.bindValue(":citizen", "%" + criteria.citizenId + "%");
    }
    
    if (!query.exec()) {
        qWarning() << "searchPatients error:" << query.lastError().text();
        return results;
    }
    
    while (query.next()) {
        auto p = mapRowToPatient(query);
        if (p) {
            // Lọc theo độ tuổi (tính toán sau khi query do hạn chế SQLite date/time format tuỳ biến)
            bool matchAge = true;
            if (criteria.minAge >= 0 && p->getAge() < criteria.minAge) matchAge = false;
            if (criteria.maxAge >= 0 && p->getAge() > criteria.maxAge) matchAge = false;
            
            if (matchAge) {
                results.push_back(p);
            }
        }
    }
    
    return results;
}

bool PatientRepository::addMedicalRecord(const MedicalRecord& record) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO medical_records "
        "(patient_id, doctor_id, visit_date, vitals, chief_complaint, "
        " clinical_notes, treatment, test_results, next_visit) "
        "VALUES "
        "(:patient_id, :doctor_id, :visit_date, :vitals, :chief_complaint, "
        " :clinical_notes, :treatment, :test_results, :next_visit)"
    );
    
    query.bindValue(":patient_id", record.getPatientId());
    query.bindValue(":doctor_id", record.getDoctorId());
    query.bindValue(":visit_date", record.getVisitDateTime().toString(Qt::ISODate));
    
    // Serialize vitals
    VitalSigns v = record.getVitals();
    QString vitalsStr = QString("%1|%2|%3|%4|%5|%6")
        .arg(v.temperature).arg(v.bloodPressureSystolic).arg(v.bloodPressureDiastolic)
        .arg(v.heartRate).arg(v.weight).arg(v.height);
    query.bindValue(":vitals", vitalsStr);
    
    query.bindValue(":chief_complaint", record.getChiefComplaint());
    query.bindValue(":clinical_notes", record.getClinicalNotes());
    // (Lưu ý: chưa serialize diagnoses vào DB cho đơn giản MVP, hoặc có thể nối chuỗi. MVP tạm lược bớt cột diagnoses trong DB)
    query.bindValue(":treatment", record.getTreatment());
    query.bindValue(":test_results", record.getTestResults());
    query.bindValue(":next_visit", record.getNextVisitDate().isValid() ? record.getNextVisitDate().toString(Qt::ISODate) : QVariant());
    
    if (!query.exec()) {
        qWarning() << "Failed to insert medical_record:" << query.lastError().text();
        return false;
    }
    return true;
}

std::vector<MedicalRecord> PatientRepository::getRecordsByPatientId(int patientId) const {
    std::vector<MedicalRecord> records;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM medical_records WHERE patient_id = :pid ORDER BY visit_date DESC");
    query.bindValue(":pid", patientId);
    
    if (!query.exec()) {
        qWarning() << "Failed to fetch medical_records:" << query.lastError().text();
        return records;
    }
    
    while (query.next()) {
        MedicalRecord rec;
        rec.setRecordId(query.value("id").toInt());
        // setVisitDateTime is needed in MedicalRecord.h!
        rec.setVisitDateTime(QDateTime::fromString(query.value("visit_date").toString(), Qt::ISODate));
        rec.setChiefComplaint(query.value("chief_complaint").toString());
        rec.setClinicalNotes(query.value("clinical_notes").toString());
        rec.setTreatment(query.value("treatment").toString());
        rec.setTestResults(query.value("test_results").toString());
        
        QVariant nextVisitVar = query.value("next_visit");
        if (!nextVisitVar.isNull()) {
            rec.setNextVisitDate(QDateTime::fromString(nextVisitVar.toString(), Qt::ISODate));
        }
        
        // Deserialize vitals
        QStringList vList = query.value("vitals").toString().split('|');
        if (vList.size() == 6) {
            VitalSigns v;
            v.temperature = vList[0].toDouble();
            v.bloodPressureSystolic = vList[1].toInt();
            v.bloodPressureDiastolic = vList[2].toInt();
            v.heartRate = vList[3].toInt();
            v.weight = vList[4].toDouble();
            v.height = vList[5].toDouble();
            rec.setVitals(v);
        }
        
        // (Khôi phục diagnoses nếu có lưu)
        records.push_back(rec);
    }
    return records;
}

std::shared_ptr<Patient> PatientRepository::mapRowToPatient(const QSqlQuery& query) const {
    PatientType type = PatientType::OutPatient; // Default
    if (query.record().indexOf("patient_type") != -1) {
        type = static_cast<PatientType>(query.value("patient_type").toInt());
    }

    std::shared_ptr<Patient> p;
    switch (type) {
        case PatientType::InPatient: {
            auto inP = std::make_shared<InPatient>();
            if (query.record().indexOf("room_no") != -1) {
                inP->setRoomNo(query.value("room_no").toString());
            }
            if (query.record().indexOf("admit_date") != -1) {
                inP->setAdmitDate(QDate::fromString(query.value("admit_date").toString(), "yyyy-MM-dd"));
            }
            p = inP;
            break;
        }
        case PatientType::Emergency: {
            auto emP = std::make_shared<EmergencyPatient>();
            if (query.record().indexOf("severity") != -1) {
                emP->setSeverity(query.value("severity").toInt());
            }
            p = emP;
            break;
        }
        case PatientType::OutPatient:
        default:
            p = std::make_shared<OutPatient>();
            break;
    }

    p->setId(query.value("id").toInt());
    p->setPatientCode(query.value("patient_code").toString());
    p->setFullName(query.value("full_name").toString());
    p->setBirthDate(QDate::fromString(
        query.value("birth_date").toString(), "yyyy-MM-dd"));
    p->setGender(static_cast<Gender>(query.value("gender").toInt()));
    p->setPhoneNumber(query.value("phone_number").toString());
    p->setAddress(query.value("address").toString());
    p->setBloodType(query.value("blood_type").toString());
    p->setAllergies(query.value("allergies").toString());
    p->setMedicalHistory(query.value("medical_history").toString());
    p->setCitizenId(query.value("citizen_id").toString());
    p->setEmail(query.value("email").toString());
    p->setInsurance(query.value("insurance").toString());
    p->setIsActive(query.value("is_active").toInt() == 1);
    p->setState(static_cast<PatientStateType>(query.value("state").toInt()));
    return p;
}
