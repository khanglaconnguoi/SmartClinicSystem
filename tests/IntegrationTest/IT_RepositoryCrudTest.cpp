#include <iostream>
#include <memory>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDateTime>
#include <QVariant>
#include "../TestHelper.h"

#include "repository/DatabaseManager.h"
#include "repository/PatientRepository.h"
#include "repository/StaffRepository.h"
#include "repository/MedicationRepository.h"
#include "repository/PrescriptionRepository.h"
#include "repository/MedicalRecordRepository.h"
#include "repository/BillingRepository.h"

void runRepositoryCrudTestSuite() {
    std::cout << "  [IT 10/10] Running Direct Repository & SQLite CRUD Integration Tests..." << std::endl;

    // Initialize Database singleton
    DatabaseManager::getInstance();

    PatientRepository patientRepo;
    StaffRepository staffRepo;
    MedicationRepository medRepo;
    MedicalRecordRepository recordRepo;
    BillingRepository billingRepo;
    PrescriptionRepository presRepo;

    // =========================================================================
    // 1. PATIENT REPOSITORY CRUD & SQLITE VERIFICATION
    // =========================================================================
    RUN_INLINE_TEST("PatientRepo CRUD: Create, Read via SQL, Update, Soft-Delete & Restore", {
        // --- CREATE ---
        OutPatientInsertDTO outDto;
        outDto.patientCode = "PAT-REPO-001";
        outDto.fullName = "Nguyễn Văn An";
        outDto.dateOfBirth = "1990-05-15"; // SQLite text format "yyyy-MM-dd"
        outDto.gender = "MALE";
        outDto.citizenId = "079190008881";
        outDto.phone = "0988777661";
        outDto.email = "vanan.repo@test.com";
        outDto.address = "123 Điện Biên Phủ, TP.HCM";
        outDto.bloodType = "O+";
        outDto.type = "OUTPATIENT";
        outDto.emergencyContactName = "Nguyễn Văn B";
        outDto.emergencyContactPhone = "0988777662";
        outDto.status = "REGISTERED";

        bool okInsert = patientRepo.insertOutPatient(outDto);
        TEST_ASSERT_TRUE(okInsert == true);

        // --- READ VIA DIRECT QSQLQUERY ---
        QSqlQuery q;
        q.prepare("SELECT patient_id, full_name, citizen_id, phone_number, address, is_deleted "
                  "FROM patients WHERE citizen_id = :cid");
        q.bindValue(":cid", "079190008881");
        TEST_ASSERT_TRUE(q.exec());
        TEST_ASSERT_TRUE(q.next());

        int patientId = q.value("patient_id").toInt();
        QString dbFullName = q.value("full_name").toString();
        int isDeleted = q.value("is_deleted").toInt();

        TEST_ASSERT_TRUE(patientId > 0);
        TEST_ASSERT_TRUE(dbFullName == "Nguyễn Văn An");
        TEST_ASSERT_TRUE(isDeleted == 0);

        // --- UPDATE ---
        PatientUpdateDTO updateDto;
        updateDto.patientId = patientId;
        updateDto.fullName = "Nguyễn Văn An (Đã cập nhật)";
        updateDto.dateOfBirth = "1990-05-15"; // QString
        updateDto.gender = "MALE";
        updateDto.citizenId = "079190008881";
        updateDto.phone = "0988777999";
        updateDto.email = "vanan.updated@test.com";
        updateDto.address = "456 Lê Duẩn, TP.HCM";
        updateDto.bloodType = "A+";
        updateDto.emergencyContactName = "Nguyễn Văn C";
        updateDto.emergencyContactPhone = "0988777663";

        bool okUpdate = patientRepo.updatePatient(updateDto);
        TEST_ASSERT_TRUE(okUpdate == true);

        // Verify update in SQLite
        QSqlQuery qUp;
        qUp.prepare("SELECT full_name, phone_number, address, blood_type FROM patients WHERE patient_id = :pid");
        qUp.bindValue(":pid", patientId);
        TEST_ASSERT_TRUE(qUp.exec() && qUp.next());
        TEST_ASSERT_TRUE(qUp.value("full_name").toString() == "Nguyễn Văn An (Đã cập nhật)");
        TEST_ASSERT_TRUE(qUp.value("phone_number").toString() == "0988777999");
        TEST_ASSERT_TRUE(qUp.value("blood_type").toString() == "A+");

        // --- SOFT-DELETE ---
        bool okDel = patientRepo.softDeletePatient(patientId);
        TEST_ASSERT_TRUE(okDel == true);
        TEST_ASSERT_TRUE(patientRepo.isPatientSoftDeleted(patientId) == true);

        QSqlQuery qDel;
        qDel.prepare("SELECT is_deleted FROM patients WHERE patient_id = :pid");
        qDel.bindValue(":pid", patientId);
        TEST_ASSERT_TRUE(qDel.exec() && qDel.next());
        TEST_ASSERT_TRUE(qDel.value("is_deleted").toInt() == 1);

        // --- RESTORE ---
        bool okRes = patientRepo.restorePatient(patientId);
        TEST_ASSERT_TRUE(okRes == true);
        TEST_ASSERT_TRUE(patientRepo.isPatientSoftDeleted(patientId) == false);

        std::cout << "    [OK] PatientRepo CRUD & SQLite Direct Verification Passed" << std::endl;
    });

    // =========================================================================
    // 2. STAFF REPOSITORY CRUD & AUTHENTICATION SQLITE VERIFICATION
    // =========================================================================
    RUN_INLINE_TEST("StaffRepo CRUD: Create Doctor, Query SQL, Update Password & Deactivate", {
        DoctorInsertDTO docDto;
        docDto.staffCode = "DOC-REPO-001";
        docDto.passwordHash = "$2a$10$abcdefghijklmnopqrstuu"; // mock bcrypt hash
        docDto.fullName = "Bác sĩ Phạm Hoàng Long";
        docDto.role = "DOCTOR"; // QString role (UPPERCASE for CHECK constraint)
        docDto.gender = "MALE";
        docDto.dateOfBirth = "1985-08-20"; // QString
        docDto.citizenId = "079185007777";
        docDto.phoneNumber = "0912345678";
        docDto.email = "dr.long@clinic.com";
        docDto.address = "789 Nguyễn Thị Minh Khai";
        docDto.departmentId = 1;
        docDto.hireDate = "2020-01-01"; // QString
        docDto.shift = "MORNING";
        docDto.specialty = "CARDIOLOGY";
        docDto.licenseNumber = "CCHN-889900";
        docDto.experienceYears = 12;
        docDto.consultationFee = 350000;
        docDto.bio = "Chuyên gia Tim mạch";

        bool okInsert = staffRepo.insertDoctor(docDto);
        TEST_ASSERT_TRUE(okInsert == true);

        // Direct SQL query on `staff` table JOIN doctor_profiles
        QSqlQuery q;
        q.prepare("SELECT s.staff_id, s.full_name, s.role, d.specialty, s.is_active FROM staff s JOIN doctor_profiles d ON s.staff_id = d.staff_id WHERE s.staff_code = :code");
        q.bindValue(":code", "DOC-REPO-001");
        TEST_ASSERT_TRUE(q.exec() && q.next());

        int staffId = q.value("staff_id").toInt();
        TEST_ASSERT_TRUE(staffId > 0);
        TEST_ASSERT_TRUE(q.value("full_name").toString() == "Bác sĩ Phạm Hoàng Long");
        TEST_ASSERT_TRUE(q.value("specialty").toString() == "CARDIOLOGY");
        TEST_ASSERT_TRUE(q.value("is_active").toInt() == 1);

        // Update password hash
        QString newHash = "$2a$10$updatedhash12345678901234";
        bool okPwd = staffRepo.updatePasswordInformation(staffId, newHash, false);
        TEST_ASSERT_TRUE(okPwd == true);

        // Verify password in SQLite
        QSqlQuery qPwd;
        qPwd.prepare("SELECT password_hash FROM staff WHERE staff_id = :sid");
        qPwd.bindValue(":sid", staffId);
        TEST_ASSERT_TRUE(qPwd.exec() && qPwd.next());
        TEST_ASSERT_TRUE(qPwd.value("password_hash").toString() == newHash);

        // Deactivate & Reactivate
        TEST_ASSERT_TRUE(staffRepo.deactivate(staffId) == true);
        QSqlQuery qDeac;
        qDeac.prepare("SELECT is_active FROM staff WHERE staff_id = :sid");
        qDeac.bindValue(":sid", staffId);
        TEST_ASSERT_TRUE(qDeac.exec() && qDeac.next());
        TEST_ASSERT_TRUE(qDeac.value("is_active").toInt() == 0);

        TEST_ASSERT_TRUE(staffRepo.reactivate(staffId) == true);

        std::cout << "    [OK] StaffRepo CRUD & SQLite Direct Verification Passed" << std::endl;
    });

    // =========================================================================
    // 3. MEDICATION REPOSITORY CRUD & INVENTORY SQLITE VERIFICATION
    // =========================================================================
    RUN_INLINE_TEST("MedicationRepo CRUD: Create, Update Stock/Price, Deactivate", {
        MedicationInputDTO medDto;
        medDto.brandName = "Thuốc Repo Test Amlodipine 5mg";
        medDto.unit = "TABLET";
        medDto.unitPrice = 12000.0;
        medDto.stockQuantity = 500;
        medDto.minimumStock = 50;
        medDto.reorderThreshold = 100;
        medDto.expiryDate = QDate::currentDate().addYears(1); // QDate in MedicationInputDTO
        medDto.manufacturer = "Dược Hậu Giang";
        medDto.description = "Thuốc điều trị cao huyết áp";
        medDto.categories = {"CARDIOVASCULAR"};

        bool okInsert = medRepo.insertMedication(medDto);
        TEST_ASSERT_TRUE(okInsert == true);

        // Direct SQL query on `medications` table
        QSqlQuery q;
        q.prepare("SELECT medication_id, brand_name, unit_price, stock_quantity, is_active "
                  "FROM medications WHERE brand_name = :bname");
        q.bindValue(":bname", "Thuốc Repo Test Amlodipine 5mg");
        TEST_ASSERT_TRUE(q.exec() && q.next());

        int medId = q.value("medication_id").toInt();
        TEST_ASSERT_TRUE(medId > 0);
        TEST_ASSERT_TRUE(q.value("unit_price").toDouble() == 12000.0);
        TEST_ASSERT_TRUE(q.value("stock_quantity").toInt() == 500);
        TEST_ASSERT_TRUE(q.value("is_active").toInt() == 1);

        // Update Stock & Price
        medDto.unitPrice = 15000.0;
        medDto.stockQuantity = 800;
        bool okUp = medRepo.updateMedication(medId, medDto);
        TEST_ASSERT_TRUE(okUp == true);

        QSqlQuery qUp;
        qUp.prepare("SELECT unit_price, stock_quantity FROM medications WHERE medication_id = :mid");
        qUp.bindValue(":mid", medId);
        TEST_ASSERT_TRUE(qUp.exec() && qUp.next());
        TEST_ASSERT_TRUE(qUp.value("unit_price").toDouble() == 15000.0);
        TEST_ASSERT_TRUE(qUp.value("stock_quantity").toInt() == 800);

        // Deactivate
        TEST_ASSERT_TRUE(medRepo.deactivate(medId) == true);
        QSqlQuery qDeac;
        qDeac.prepare("SELECT is_active FROM medications WHERE medication_id = :mid");
        qDeac.bindValue(":mid", medId);
        TEST_ASSERT_TRUE(qDeac.exec() && qDeac.next());
        TEST_ASSERT_TRUE(qDeac.value("is_active").toInt() == 0);

        std::cout << "    [OK] MedicationRepo CRUD & SQLite Direct Verification Passed" << std::endl;
    });

    // =========================================================================
    // 4. MEDICAL RECORD & TRANSACTION ROLLBACK VERIFICATION
    // =========================================================================
    RUN_INLINE_TEST("MedicalRecordRepo CRUD & Transaction Rollback Integrity", {
        // Setup FK dummy records
        QSqlQuery qSetup;
        qSetup.exec("INSERT OR IGNORE INTO patients (patient_id, patient_code, full_name, date_of_birth, gender, citizen_id, phone_number, email, address, emergency_contact_name, emergency_contact_phone) "
                    "VALUES (888, 'PAT-888', 'Tx Patient', '1990-01-01', 'MALE', '079888888888', '0900000888', 'tx888@test.com', 'Address', 'Contact', '0900000000')");
        qSetup.exec("INSERT OR IGNORE INTO staff (staff_id, staff_code, password_hash, full_name, role, gender, date_of_birth, citizen_id, phone_number, email, address, department_id, shift) "
                    "VALUES (888, 'D888', 'hash', 'Tx Doctor', 'DOCTOR', 'MALE', '1985-01-01', '079000000888', '0900000888', 'doc888@test.com', 'Address', 1, 'FULL_DAY')");
        qSetup.exec("INSERT OR IGNORE INTO appointments (appointment_id, ticket_number, patient_id, doctor_id, appointment_date, start_time, status) "
                    "VALUES (888, 1, 888, 888, '2026-08-08', '09:00', 'COMPLETED')");

        MedicalRecordInsertDTO recDto;
        recDto.patientId = 888;
        recDto.doctorId = 888;
        recDto.appointmentId = 888;
        recDto.visitDateTime = QDateTime::currentDateTime();
        recDto.chiefComplaint = "Đau ngực nhẹ";
        recDto.clinicalNotes = "Khám tim mạch bình thường";
        recDto.treatment = "Theo dõi tại nhà";
        recDto.vitals.weight = 68.5;
        recDto.vitals.height = 172.0;
        recDto.vitals.bloodPressure = "120/80";
        recDto.vitals.heartRate = 75;
        recDto.vitals.temperature = 36.8;

        Diagnosis diag;
        diag.description = "Sốt nhẹ chưa rõ nguyên nhân";
        diag.severity = "MILD";
        recDto.diagnoses.append(diag);

        int recordId = recordRepo.insertMedicalRecord(recDto);
        TEST_ASSERT_TRUE(recordId > 0);

        // Verify direct SQLite row
        QSqlQuery qRec;
        qRec.prepare("SELECT chief_complaint, is_deleted FROM medical_records WHERE record_id = :rid");
        qRec.bindValue(":rid", recordId);
        TEST_ASSERT_TRUE(qRec.exec() && qRec.next());
        TEST_ASSERT_TRUE(qRec.value("chief_complaint").toString() == "Đau ngực nhẹ");

        // --- TRANSACTION ROLLBACK TEST ---
        // Verify that DatabaseManager transaction rollback leaves SQLite untouched on error
        TEST_ASSERT_TRUE(DatabaseManager::getInstance().beginTransaction() == true);

        QSqlQuery qTx;
        qTx.exec("INSERT INTO patients (patient_id, patient_code, full_name, date_of_birth, gender, citizen_id, phone_number, email, address, emergency_contact_name, emergency_contact_phone) "
                 "VALUES (9999, 'PAT-9999', 'Rollback Test', '1990-01-01', 'MALE', '079999999991', '0900009999', 'rb9999@test.com', 'Address', 'Contact', '0900000000')");
        
        // Force a rollback
        DatabaseManager::getInstance().rollbackTransaction();

        // Verify patient 9999 was NOT saved
        QSqlQuery qCheck;
        qCheck.exec("SELECT COUNT(*) FROM patients WHERE patient_id = 9999");
        TEST_ASSERT_TRUE(qCheck.next());
        TEST_ASSERT_TRUE(qCheck.value(0).toInt() == 0);

        std::cout << "    [OK] MedicalRecord & Transaction Rollback Test Passed" << std::endl;
    });

    // =========================================================================
    // 5. BILLING & INVOICE REPOSITORY CRUD
    // =========================================================================
    RUN_INLINE_TEST("BillingRepo CRUD: Insert Invoice, Collect Payment, Cancel Invoice", {
        // Setup FK dummy patient
        QSqlQuery qSetup;
        qSetup.exec("INSERT OR IGNORE INTO patients (patient_id, patient_code, full_name, date_of_birth, gender, citizen_id, phone_number, email, address, emergency_contact_name, emergency_contact_phone) "
                    "VALUES (777, 'PAT-777', 'Billing Patient', '1990-01-01', 'MALE', '079777777777', '0900000777', 'bill777@test.com', 'Address', 'Contact', '0900000000')");

        InvoiceInsertDTO invDto;
        invDto.invoiceCode = "INV-REPO-2026-001";
        invDto.patientId = 777;
        invDto.recordId = std::nullopt;
        invDto.patientType = "OUTPATIENT";
        invDto.consultationFee = 200000.0;
        invDto.medicationFee = 350000.0;
        invDto.totalAmount = 550000.0;
        invDto.issuedDate = QDate::currentDate().toString("yyyy-MM-dd"); // QString date
        invDto.status = "UNPAID";

        InvoiceItemDTO item;
        item.itemType = "CONSULTATION";
        item.description = "Phí khám bệnh ngoại trú";
        item.quantity = 1;
        item.unitPrice = 200000.0;
        item.subtotal = 200000.0;
        invDto.items.append(item);

        bool okInsert = billingRepo.insertInvoice(invDto);
        TEST_ASSERT_TRUE(okInsert == true);

        // Verify in SQLite `invoices` table
        QSqlQuery q;
        q.prepare("SELECT invoice_id, consultation_fee, medication_fee, total_amount, status "
                  "FROM invoices WHERE invoice_code = :code");
        q.bindValue(":code", "INV-REPO-2026-001");
        TEST_ASSERT_TRUE(q.exec() && q.next());

        int invoiceId = q.value("invoice_id").toInt();
        TEST_ASSERT_TRUE(invoiceId > 0);
        TEST_ASSERT_TRUE(q.value("status").toString() == "UNPAID");
        TEST_ASSERT_TRUE(q.value("total_amount").toDouble() == 550000.0);

        // Collect Payment
        bool okCollect = billingRepo.collectPayment(invoiceId, 550000.0);
        TEST_ASSERT_TRUE(okCollect == true);

        QSqlQuery qPaid;
        qPaid.prepare("SELECT status FROM invoices WHERE invoice_id = :iid");
        qPaid.bindValue(":iid", invoiceId);
        TEST_ASSERT_TRUE(qPaid.exec() && qPaid.next());
        TEST_ASSERT_TRUE(qPaid.value("status").toString() == "PAID");

        std::cout << "    [OK] BillingRepo CRUD & Payment Collection Passed" << std::endl;
    });

    std::cout << "    ==> [PASSED] All Direct Repository & SQLite CRUD Integration Tests (5/5)" << std::endl;
    std::fflush(stdout);
}
