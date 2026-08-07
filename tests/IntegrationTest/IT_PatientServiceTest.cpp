#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "service/PatientService.h"
#include "repository/PatientRepository.h"

void runPatientServiceTestSuite() {
    std::cout << "  [IT 3/9] Running Patient Service Integration Tests..." << std::endl;
    auto patientRepo = std::make_shared<PatientRepository>();
    PatientService patientService(patientRepo);

    int createdPatientId = -1;

    // --- HAPPY: Add OutPatient ---
    RUN_INLINE_TEST("HAPPY: Add OutPatient", {
        OutPatientInputDTO dto;
        dto.fullName = "IT Patient Nguyen";
        dto.dateOfBirth = QDate(1995, 3, 20);
        dto.gender = "MALE";
        dto.citizenId = "079195001001";
        dto.phone = "0903000001";
        dto.email = "it_patient@test.com";
        dto.address = "456 Patient Rd";
        dto.bloodType = "A+";
        dto.type = PatientType::Outpatient;
        dto.emergencyContactName = "Tran Van B";
        dto.emergencyContactPhone = "0903000002";
        dto.doctorId = std::nullopt;

        QString err = patientService.addOutPatient(dto);
        TEST_ASSERT_TRUE(err.isEmpty());
        std::cout << "    [OK] addOutPatient -> success" << std::endl;
    });

    // --- HAPPY: Search patient ---
    RUN_INLINE_TEST("HAPPY: Search patient", {
        PatientSearchCriteria criteria;
        criteria.searchKey = "IT Patient";
        criteria.page = 1;
        criteria.pageSize = 10;
        auto results = patientService.searchPatientsPaged(criteria);
        TEST_ASSERT_TRUE(results.totalCount >= 1);
        createdPatientId = results.items.first().patientId;
        std::cout << "    [OK] searchPatientsPaged -> found " << results.totalCount << " result(s), ID=" << createdPatientId << std::endl;
    });

    // --- HAPPY: Get patient by ID ---
    RUN_INLINE_TEST("HAPPY: Get patient by ID", {
        auto detail = patientService.getPatientById(createdPatientId);
        TEST_ASSERT_TRUE(detail.has_value());
        TEST_ASSERT_TRUE(detail->fullName == "IT Patient Nguyen");
        std::cout << "    [OK] getPatientById -> " << detail->fullName.toStdString() << std::endl;
    });

    // --- HAPPY: Add allergy ---
    RUN_INLINE_TEST("HAPPY: Add allergy", {
        QList<AllergyInputDTO> allergies;
        AllergyInputDTO allergy;
        allergy.allergenName = "Penicillin";
        allergy.severity = Severity::Severe;
        allergy.notes = "Integration test allergy";
        allergies.append(allergy);

        QString err = patientService.addAllergiesToPatient(createdPatientId, allergies);
        TEST_ASSERT_TRUE(err.isEmpty());
        
        auto loadedAllergies = patientService.getAllergies(createdPatientId);
        TEST_ASSERT_TRUE(loadedAllergies.size() >= 1);
        std::cout << "    [OK] addAllergiesToPatient -> " << loadedAllergies.size() << " allergy(s)" << std::endl;
    });

    // --- HAPPY: Add InPatient ---
    RUN_INLINE_TEST("HAPPY: Add InPatient", {
        InPatientInputDTO dto;
        dto.fullName = "IT InPatient Tran";
        dto.dateOfBirth = QDate(1988, 7, 10);
        dto.gender = "FEMALE";
        dto.citizenId = "079188001003";
        dto.phone = "0903000003";
        dto.email = "inpatient@test.com";
        dto.address = "789 Hospital Blvd";
        dto.bloodType = "O+";
        dto.type = PatientType::Inpatient;
        dto.emergencyContactName = "Le Van C";
        dto.emergencyContactPhone = "0903000004";
        dto.roomId = 1;
        dto.doctorId = 1;
        dto.admissionDate = QDate::currentDate();
        dto.reason = "Integration Test";

        QString err = patientService.addInPatient(dto);
        TEST_ASSERT_TRUE(err.isEmpty());
        std::cout << "    [OK] addInPatient -> success" << std::endl;
    });

    // --- HAPPY: Add EmergencyPatient ---
    RUN_INLINE_TEST("HAPPY: Add EmergencyPatient", {
        EmergencyPatientInputDTO dto;
        dto.fullName = "IT Emergency Le";
        dto.dateOfBirth = QDate(2000, 12, 1);
        dto.gender = "MALE";
        dto.citizenId = "079200001005";
        dto.phone = "0903000005";
        dto.email = "emergency@test.com";
        dto.address = "Emergency Dept";
        dto.bloodType = "B-";
        dto.type = PatientType::Emergency;
        dto.emergencyContactName = "Pham Van D";
        dto.emergencyContactPhone = "0903000006";
        dto.roomId = 1;
        dto.doctorId = 1;
        dto.injuryCause = "Fall";
        dto.injuryDescription = "Broken arm";
        dto.admissionDate = QDate::currentDate();

        QString err = patientService.addEmergencyPatient(dto);
        TEST_ASSERT_TRUE(err.isEmpty());
        std::cout << "    [OK] addEmergencyPatient -> success" << std::endl;
    });

    // --- NEGATIVE: Add patient with duplicate citizenId ---
    RUN_INLINE_TEST("NEGATIVE: Add patient with duplicate citizenId", {
        OutPatientInputDTO dto;
        dto.fullName = "Duplicate Citizen";
        dto.dateOfBirth = QDate(1995, 1, 1);
        dto.gender = "MALE";
        dto.citizenId = "079195001001"; // Same as first patient
        dto.phone = "0903999999";
        dto.email = "duplicate@test.com";
        dto.address = "123 Dup St";
        dto.bloodType = "O+";
        dto.emergencyContactName = "Dup Contact";
        dto.emergencyContactPhone = "0903999998";
        dto.type = PatientType::Outpatient;
        dto.doctorId = std::nullopt;

        QString err = patientService.addOutPatient(dto);
        TEST_ASSERT_TRUE(!err.isEmpty()); // Should fail with duplicate citizenId
        std::cout << "    [OK] addOutPatient (duplicate citizenId) -> rejected" << std::endl;
    });

    // --- HAPPY: Soft delete and restore ---
    RUN_INLINE_TEST("HAPPY: Soft delete and restore", {
        bool deleted = patientService.softDeletePatient(createdPatientId);
        TEST_ASSERT_TRUE(deleted == true);
        
        bool restored = patientService.restorePatient(createdPatientId);
        TEST_ASSERT_TRUE(restored == true);
        std::cout << "    [OK] softDeletePatient + restorePatient -> success" << std::endl;
    });

    std::cout << "    ==> [PASSED] Patient Service Integration Tests (8/8)" << std::endl;
    std::fflush(stdout);
}
