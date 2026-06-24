/**
 * @file    PatientRepository.cpp
 * @brief   Implementation cho PatientRepository — SQL queries.
 */
#include "PatientRepository.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

PatientRepository::PatientRepository(QSqlDatabase& db)
    : m_db(db) {
}

bool PatientRepository::insert(Patient& patient) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO patients "
        "(full_name, birth_date, gender, phone_number, address, "
        " citizen_id, email, insurance, is_active, state) "
        "VALUES "
        "(:full_name, :birth_date, :gender, :phone_number, :address, "
        " :citizen_id, :email, :insurance, :is_active, :state)"
    );

    query.bindValue(":full_name", patient.fullName());
    query.bindValue(":birth_date", patient.birthDate().toString("yyyy-MM-dd"));
    query.bindValue(":gender", static_cast<int>(patient.gender()));
    query.bindValue(":phone_number", patient.phoneNumber());
    query.bindValue(":address", patient.address());
    query.bindValue(":citizen_id", patient.citizenId());
    query.bindValue(":email", patient.email());
    query.bindValue(":insurance", patient.insurance());
    query.bindValue(":is_active", patient.isActive() ? 1 : 0);
    query.bindValue(":state", static_cast<int>(patient.stateType()));

    if (!query.exec()) {
        qCritical() << "Failed to insert patient:"
                    << query.lastError().text();
        return false;
    }

    // Gán ID tự tăng cho đối tượng Patient
    patient.setId(query.lastInsertId().toInt());
    qDebug() << "Inserted patient with ID:" << patient.id();
    return true;
}

bool PatientRepository::update(const Patient& patient) {
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE patients SET "
        "  full_name    = :full_name,"
        "  birth_date   = :birth_date,"
        "  gender       = :gender,"
        "  phone_number = :phone_number,"
        "  address      = :address,"
        "  citizen_id   = :citizen_id,"
        "  email        = :email,"
        "  insurance    = :insurance,"
        "  is_active    = :is_active,"
        "  state        = :state "
        "WHERE id = :id"
    );

    query.bindValue(":id", patient.id());
    query.bindValue(":full_name", patient.fullName());
    query.bindValue(":birth_date", patient.birthDate().toString("yyyy-MM-dd"));
    query.bindValue(":gender", static_cast<int>(patient.gender()));
    query.bindValue(":phone_number", patient.phoneNumber());
    query.bindValue(":address", patient.address());
    query.bindValue(":citizen_id", patient.citizenId());
    query.bindValue(":email", patient.email());
    query.bindValue(":insurance", patient.insurance());
    query.bindValue(":is_active", patient.isActive() ? 1 : 0);
    query.bindValue(":state", static_cast<int>(patient.stateType()));

    if (!query.exec()) {
        qCritical() << "Failed to update patient (ID:"
                    << patient.id() << "):"
                    << query.lastError().text();
        return false;
    }

    qDebug() << "Updated patient ID:" << patient.id();
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

std::optional<Patient> PatientRepository::findById(int patientId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM patients WHERE id = :id");
    query.bindValue(":id", patientId);

    if (!query.exec()) {
        qCritical() << "Failed to find patient (ID:"
                    << patientId << "):"
                    << query.lastError().text();
        return std::nullopt;
    }

    if (query.next()) {
        return mapRowToPatient(query);
    }
    return std::nullopt;
}

QList<Patient> PatientRepository::findAllActive() {
    QList<Patient> patients;
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

QList<Patient> PatientRepository::searchByName(const QString& keyword) {
    QList<Patient> patients;
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

Patient PatientRepository::mapRowToPatient(const QSqlQuery& query) {
    Patient p;
    p.setId(query.value("id").toInt());
    p.setFullName(query.value("full_name").toString());
    p.setBirthDate(QDate::fromString(
        query.value("birth_date").toString(), "yyyy-MM-dd"));
    p.setGender(static_cast<Gender>(query.value("gender").toInt()));
    p.setPhoneNumber(query.value("phone_number").toString());
    p.setAddress(query.value("address").toString());
    p.setCitizenId(query.value("citizen_id").toString());
    p.setEmail(query.value("email").toString());
    p.setInsurance(query.value("insurance").toString());
    p.setIsActive(query.value("is_active").toInt() == 1);
    p.setState(static_cast<PatientStateType>(query.value("state").toInt()));
    return p;
}
