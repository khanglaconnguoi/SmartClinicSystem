#include "InPatient.h"

InPatient::InPatient(const QString &fullName, const QDate &birthDate, Gender gender,
                     const QString &phoneNumber, const QString &address,
                     const QString &citizenId, const QString &email,
                     const QString &insurance, const QString &roomNo)
    : Patient(fullName, birthDate, gender, phoneNumber, address, citizenId, email, insurance),
      m_roomNo(roomNo), m_admitDate(QDate::currentDate()) {}

PatientPriority InPatient::getPriority() const {
    return PatientPriority::High; // Nội trú ưu tiên cao hơn ngoại trú
}

QString InPatient::getBillingType() const {
    return QStringLiteral("Nội trú");
}

QString InPatient::getStatusLabel() const {
    return QStringLiteral("Bệnh nhân nội trú");
}

double InPatient::getBaseFee() const {
    return 500000.0; // Phí giường bệnh/ngày
}

PatientType InPatient::getType() const {
    return PatientType::InPatient;
}

QString InPatient::roomNo() const { return m_roomNo; }
void InPatient::setRoomNo(const QString &roomNo) { m_roomNo = roomNo; }

QDate InPatient::admitDate() const { return m_admitDate; }
void InPatient::setAdmitDate(const QDate &date) { m_admitDate = date; }
