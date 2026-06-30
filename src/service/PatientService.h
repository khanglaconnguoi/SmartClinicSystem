/**
 * @file    PatientService.h
 * @brief   Service layer cho module Patient.
 *
 *  Các hàm validate là private — chỉ dùng nội bộ trong class này.
 *  Mỗi hàm validate trả về chuỗi rỗng "" nếu hợp lệ,
 *  hoặc chuỗi mô tả lỗi nếu không hợp lệ.
 */

#pragma once

#include "model/CommonEnums.h"
#include <QDate>
#include <QString>
#include <memory>

class PatientRepository;

class PatientService {
private:
  std::shared_ptr<PatientRepository> m_patientRepository;

  // ── Validate nhóm trường theo loại bệnh nhân ────────────────────────────

  /**
   * @brief Kiểm tra toàn bộ trường cơ bản của bảng `patients`.
   *        Bao gồm: patientId, patientCode, fullName, dateOfBirth, gender,
   *                 citizenId, phone, email, address, bloodType, allergies,
   *                 insurance, type, emergencyContactName,
   * emergencyContactPhone.
   */
  static QString
  validateBaseInput(int patientId, const QString &patientCode,
                    const QString &fullName, const QDate &dateOfBirth,
                    const QString &gender, const QString &citizenId,
                    const QString &phone, const QString &email,
                    const QString &address, const QString &bloodType,
                    const QString &allergies, const QString &insurance,
                    const QString &type, const QString &emergencyContactName,
                    const QString &emergencyContactPhone);

  /**
   * @brief Kiểm tra trường đặc thù của bệnh nhân nội trú (`in_patients`).
   *        Bao gồm: roomId, doctorId,
   *                 admissionDate, dischargeDate, reason.
   */
  static QString validateInPatientInput(const QString &roomId,
                                        const QString &doctorId,
                                        const QDate &admissionDate,
                                        const QDate &dischargeDate,
                                        const QString &reason);

  /**
   * @brief Kiểm tra trường đặc thù của bệnh nhân cấp cứu
   *        (`emergency_patients`).
   *        Bao gồm: roomId, doctorId,
   *                 injuryCause, injuryDescription,
   *                 admissionDate, dischargeDate.
   */
  static QString validateEmergencyPatientInput(const QString &roomId,
                                               const QString &doctorId,
                                               const QString &injuryCause,
                                               const QString &injuryDescription,
                                               const QDate &admissionDate,
                                               const QDate &dischargeDate);

public:
  explicit PatientService(std::shared_ptr<PatientRepository> patientRepository)
      : m_patientRepository(patientRepository) {}
  ~PatientService() {}

  /**
   * @brief Tạo mã bệnh nhân theo loại (OUT-yyyyMMdd-NNNN / IN-… / EMER-…).
   */
  QString generatePatientCode(PatientType type);

  /**
   * @brief Đăng ký bệnh nhân ngoại trú.
   */
  bool AddOutPatient(int patientId, int doctorId, const QString &fullName,
                     const QDate &dateOfBirth, const QString &gender,
                     const QString &citizenId, const QString &phone,
                     const QString &email, const QString &address,
                     const QString &bloodType, const QString &allergies,
                     const QString &insurance, PatientType type,
                     const QString &emergencyContactName,
                     const QString &emergencyContactPhone);

  /**
   * @brief Nhập viện bệnh nhân nội trú.
   */
  bool AddInPatient(int patientId, const QString &fullName,
                    const QDate &dateOfBirth, const QString &gender,
                    const QString &citizenId, const QString &phone,
                    const QString &email, const QString &address,
                    const QString &bloodType, const QString &allergies,
                    const QString &insurance, PatientType type,
                    const QString &emergencyContactName,
                    const QString &emergencyContactPhone, const QString &roomId,
                    const QString &doctorId, const QDate &admissionDate,
                    const QDate &dischargeDate, const QString &reason);

  /**
   * @brief Tiếp nhận bệnh nhân cấp cứu.
   */
  bool AddEmergencyPatient(
      int patientId, const QString &fullName, const QDate &dateOfBirth,
      const QString &gender, const QString &citizenId, const QString &phone,
      const QString &email, const QString &address, const QString &bloodType,
      const QString &allergies, const QString &insurance, PatientType type,
      const QString &emergencyContactName, const QString &emergencyContactPhone,
      const QString &roomId, const QString &doctorId,
      const QString &injuryCause, const QString &injuryDescription,
      const QDate &admissionDate, const QDate &dischargeDate);
};