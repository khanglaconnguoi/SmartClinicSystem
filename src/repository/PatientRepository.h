/**
 * @file    PatientRepository.h
 * @brief   Repository chỉ chứa các hàm INSERT bệnh nhân.
 *
 *  Luồng ghi dữ liệu:
 *    Service  →  (InsertDTO)  →  PatientRepository  →  DB
 *
 *  Mỗi hàm insert chạy trong một transaction:
 *    1. insertBasePatient()  → INSERT INTO patients
 *    2. INSERT INTO out_patients / in_patients / emergency_patients_admissions
 */

#pragma once

#include "dto/patientDTOs.h"
#include <QString>

class PatientRepository {
private:
  /**
   * @brief Ghi bản ghi vào bảng `patients`.
   *        Được gọi bên trong mỗi insertXxxPatient() TRƯỚC khi ghi bảng con.
   * @param[in]  dto       Dữ liệu bệnh nhân cơ bản (PatientInsertDTO hoặc dẫn xuất).
   * @param[out] patientId Nhận về ROWID vừa được sinh bởi SQLite (last_insert_rowid).
   * @return true nếu INSERT thành công.
   */
  bool insertBasePatient(const PatientInsertDTO &dto, int &patientId);

public:
  /**
   * @brief Đăng ký bệnh nhân ngoại trú.
   *        Ghi vào `patients` + `out_patients` trong cùng 1 transaction.
   * @return true nếu cả hai INSERT thành công và transaction được commit.
   */
  bool insertOutPatient(const OutPatientInsertDTO &dto);

  /**
   * @brief Nhập viện bệnh nhân nội trú.
   *        Ghi vào `patients` + `in_patients` trong cùng 1 transaction.
   * @return true nếu cả hai INSERT thành công và transaction được commit.
   */
  bool insertInPatient(const InPatientInsertDTO &dto);

  /**
   * @brief Tiếp nhận bệnh nhân cấp cứu.
   *        Ghi vào `patients` + `emergency_patients_admissions` trong cùng 1 transaction.
   * @return true nếu cả hai INSERT thành công và transaction được commit.
   */
  bool insertEmergencyPatient(const EmergencyPatientInsertDTO &dto);
};
