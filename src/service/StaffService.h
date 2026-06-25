#pragma once
#include <QString>
#include <memory>

#include "repository/StaffRepository.h"

class StaffService {
private:
    std::shared_ptr<StaffRepository> m_staffRepository;

    static QString validatePlainPassword(const QString& plainPassword);
    static QString validatePhoneNumber(const QString& phoneNumber);
    static QString validateNationalId(const QString& nationalId);

    // Validation helpers — trả về chuỗi lỗi rỗng nếu hợp lệ
    static QString validateBaseInput(
        const QString& staffCode,
        const QString& plainPassword,
        const QString& fullName,
        const QDate& dateOfBirth,
        const QString& nationalId,
        const QString& phoneNumber,
        const QString& email,
        const QString& address,
        int departmentId,
        const QDate& hireDate,
        const QString& shift,
        const QString& specialty,
        const QString& licenseNumber,
        int experienceYears,
        int consultationFee,
        const QString& bio
    );

public:
    explicit StaffService(std::shared_ptr<StaffRepository> staffRepository)
        : m_staffRepository(staffRepository) {}

    bool hireNewDoctor(
        const QString& staffCode,
        const QString& plainPassword,
        const QString& fullName,
        Gender gender,
        const QDate& dateOfBirth,
        const QString& nationalId,
        const QString& phoneNumber,
        const QString& email,
        const QString& address,
        int departmentId,
        const QDate& hireDate,
        const QString& shift,
        const QString& specialty,
        const QString& licenseNumber,
        int experienceYears,
        int consultationFee,
        const QString& bio
    );

    bool hireNewNurse(/*...*/);
    bool hireNewReceptionist(/*...*/);

    //std::vector<std::shared_ptr<SystemUser>> searchDoctors(const )
};