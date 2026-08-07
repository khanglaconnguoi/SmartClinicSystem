#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include <QSqlQuery>
#include "service/PharmacyService.h"
#include "repository/MedicationRepository.h"
#include "repository/PrescriptionRepository.h"

void runPharmacyServiceTestSuite() {
    std::cout << "  [IT 5/9] Running Pharmacy Service Integration Tests..." << std::endl;
    auto medRepo = std::make_shared<MedicationRepository>();
    auto presRepo = std::make_shared<PrescriptionRepository>();
    PharmacyService pharmacyService(medRepo, presRepo);

    // Setup: Ensure active_ingredients table has ingredient_id = 1 for test
    {
        QSqlQuery q;
        q.exec("INSERT OR IGNORE INTO active_ingredients (ingredient_id, ingredient_name, description) VALUES (1, 'Paracetamol', 'Analgesic')");
    }

    int createdMedId = -1;

    // --- HAPPY: Add medication to inventory ---
    RUN_INLINE_TEST("HAPPY: Add medication to inventory", {
        MedicationInputDTO dto;
        dto.brandName = "IT Paracetamol 500mg";
        dto.unit = "TABLET";
        dto.unitPrice = 5000.0;
        dto.stockQuantity = 1000;
        dto.minimumStock = 100;
        dto.reorderThreshold = 200;
        dto.expiryDate = QDate::currentDate().addYears(2);
        dto.manufacturer = "IT Pharma Corp";
        dto.description = "Pain reliever for integration tests";
        dto.categories = {"ANALGESICS"};
        
        MedicationInputDTO::IngredientInput ing;
        ing.ingredientId = 1;
        ing.strength = "500mg";
        dto.ingredients.append(ing);

        QString err = pharmacyService.addMedication(dto);
        TEST_ASSERT_TRUE(err.isEmpty());
        std::cout << "    [OK] addMedication -> success" << std::endl;
    });

    // --- HAPPY: Search medication ---
    RUN_INLINE_TEST("HAPPY: Search medication", {
        MedicationSearchCriteria criteria;
        criteria.keyword = "IT Paracetamol";
        criteria.page = 1;
        criteria.pageSize = 10;
        auto results = pharmacyService.searchMedicationsPaged(criteria);
        TEST_ASSERT_TRUE(results.totalCount >= 1);
        createdMedId = results.items.first().medicationId;
        std::cout << "    [OK] searchMedicationsPaged -> found " << results.totalCount << " medication(s), ID=" << createdMedId << std::endl;
    });

    // --- HAPPY: Get medication by ID ---
    RUN_INLINE_TEST("HAPPY: Get medication by ID", {
        auto med = pharmacyService.getMedicationById(createdMedId);
        TEST_ASSERT_TRUE(med.has_value());
        TEST_ASSERT_TRUE(med->brandName.contains("Paracetamol"));
        std::cout << "    [OK] getMedicationById -> " << med->brandName.toStdString() << std::endl;
    });

    // --- HAPPY: Check low stock (should be empty initially) ---
    RUN_INLINE_TEST("HAPPY: Check low stock (should be empty initially)", {
        auto lowStock = pharmacyService.getLowStockMedications();
        // Our test medication has stock=1000, threshold=200, so it should NOT be in low stock
        bool found = false;
        for (const auto &med : lowStock) {
            if (med.medicationId == createdMedId) found = true;
        }
        TEST_ASSERT_TRUE(found == false);
        std::cout << "    [OK] getLowStockMedications -> test med not in low stock (correct)" << std::endl;
    });

    // --- HAPPY: Validate static fields ---
    RUN_INLINE_TEST("HAPPY: Validate static fields", {
        TEST_ASSERT_TRUE(PharmacyService::validateUnitPrice(5000.0).isEmpty());
        TEST_ASSERT_TRUE(!PharmacyService::validateUnitPrice(-100.0).isEmpty());
        TEST_ASSERT_TRUE(PharmacyService::validateStockQuantity(100).isEmpty());
        TEST_ASSERT_TRUE(!PharmacyService::validateStockQuantity(-1).isEmpty());
        TEST_ASSERT_TRUE(PharmacyService::validateExpiryDate(QDate::currentDate().addDays(30)).isEmpty());
        std::cout << "    [OK] Static validators -> all working" << std::endl;
    });

    // --- HAPPY: Add Prescription and Dispense (Stock Deduction) ---
    RUN_INLINE_TEST("HAPPY: Add Prescription and Dispense (Stock Deduction)", {
        // 1. Setup minimal dummy data for Foreign Keys
        QSqlQuery q;
        q.exec("INSERT OR IGNORE INTO patients (patient_id, patient_code, full_name, date_of_birth, gender, citizen_id, phone_number, email, address, emergency_contact_name, emergency_contact_phone) "
               "VALUES (99, 'PAT-99', 'Pharmacy Test Patient', '1990-01-01', 'MALE', '079999999999', '0900000099', 'pharm99@test.com', 'Address', 'Contact', '0900000000')");
        q.exec("INSERT OR IGNORE INTO staff (staff_id, staff_code, password_hash, full_name, role, gender, date_of_birth, citizen_id, phone_number, email, address, department_id, shift) "
               "VALUES (99, 'D9901', 'hash', 'Pharmacy Test Doctor', 'DOCTOR', 'MALE', '1985-01-01', '079000000099', '0900000099', 'doc99@test.com', 'Address', 1, 'FULL_DAY')");
        q.exec("INSERT OR IGNORE INTO appointments (appointment_id, patient_id, doctor_id, appointment_date, appointment_time, status) VALUES (99, 99, 99, '2026-01-01', '08:00', 'COMPLETED')");
        q.exec("INSERT OR IGNORE INTO medical_records (record_id, patient_id, doctor_id, appointment_id, status) VALUES (99, 99, 99, 99, 'OPEN')");

        // 2. Create Prescription
        PrescriptionInputDTO presDto;
        presDto.recordId = 99;
        presDto.doctorId = 99;
        presDto.notes = "Test prescription notes";

        PrescriptionItemDTO item;
        item.medicationId = createdMedId;
        item.quantity = 5;
        item.dosage = "1 tablet/day";
        item.frequency = "Once daily";
        item.durationDays = 5;
        item.unitPrice = 5000.0;
        presDto.items.append(item);

        QString err = pharmacyService.createPrescription(presDto);
        TEST_ASSERT_TRUE(err.isEmpty());
        
        // 3. Find created prescription
        QList<PrescriptionResultDTO> patPrescs = presRepo->findByPatientId(99);
        TEST_ASSERT_TRUE(patPrescs.size() > 0);
        int presId = patPrescs.last().prescriptionId;
        TEST_ASSERT_TRUE(patPrescs.last().status == PrescriptionStatus::Pending);

        // 4. Dispense Prescription
        err = pharmacyService.dispensePrescription(presId, 99);
        TEST_ASSERT_TRUE(err.isEmpty());
        
        // 5. Verify status is DISPENSED
        auto updatedPresc = presRepo->findById(presId);
        TEST_ASSERT_TRUE(updatedPresc.has_value());
        TEST_ASSERT_TRUE(updatedPresc->status == PrescriptionStatus::Dispensed);

        // 6. Verify Stock Deduction
        auto updatedMed = pharmacyService.getMedicationById(createdMedId);
        TEST_ASSERT_TRUE(updatedMed.has_value());
        TEST_ASSERT_TRUE(updatedMed->stockQuantity == 1000 - 5); // Original 1000 - 5 dispensed
        
        std::cout << "    [OK] dispensePrescription -> success, stock deducted" << std::endl;
    });

    std::cout << "    ==> [PASSED] Pharmacy Service Integration Tests (6/6)" << std::endl;
    std::fflush(stdout);
}
