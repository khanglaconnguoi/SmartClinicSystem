#pragma once
#include <QString>
#include <memory>

#include "repository/StaffRepository.h"

class StaffService {
   private:
    std::shared_ptr<StaffRepository> m_staffRepository;

    // Validation helpers — trả về chuỗi lỗi rỗng nếu hợp lệ
    QString validateBaseInput(const QString& fullName,
        const QString& phone,
        const QString& username,
        int experienceYears,
        int consultationFee,
        int departmentId) const;

   public:
    explicit StaffService(std::shared_ptr<StaffRepository> staffRepository)
        : m_staffRepository(staffRepository) {}

    bool hireNewDoctor(const QString& username,
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
        const QString& bio);

    bool hireNewNurse(/*...*/);
    bool hireNewReceptionist(/*...*/);
};