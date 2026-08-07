#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "service/MedicalRecordService.h"
#include "service/PatientService.h"
#include "service/StaffService.h"
#include "repository/MedicalRecordRepository.h"
#include "repository/PatientRepository.h"
#include "repository/StaffRepository.h"
#include "repository/AppointmentRepository.h"

void runMedicalRecordServiceTestSuite() {
    std::cout << "  [IT 4/9] Running MedicalRecord Service Integration Tests..." << std::endl;
    auto patientRepo = std::make_shared<PatientRepository>();
    auto mrRepo = std::make_shared<MedicalRecordRepository>();
    auto staffRepo = std::make_shared<StaffRepository>();
    auto apptRepo = std::make_shared<AppointmentRepository>();
    auto patientService = std::make_shared<PatientService>(patientRepo);
    StaffService staffService(staffRepo);
    MedicalRecordService mrService(mrRepo, patientService);

    int patientId = -1;
    int doctorId = -1;
    int appointmentId = -1;

    // --- Setup 1: Create a patient ---
    RUN_INLINE_TEST("Setup 1: Create a patient", {
        OutPatientInputDTO dto;
        dto.fullName = "MR Test Patient";
        dto.dateOfBirth = QDate(1990, 6, 15);
        dto.gender = "FEMALE";
        dto.citizenId = "079190002001";
        dto.phone = "0904000001";
        dto.email = "mr_test@test.com";
        dto.address = "123 Medical St";
        dto.bloodType = "O+";
        dto.emergencyContactName = "Test Emergency Contact";
        dto.emergencyContactPhone = "0904000002";
        dto.type = PatientType::Outpatient;
        dto.doctorId = std::nullopt;
        QString err = patientService->addOutPatient(dto);
        TEST_ASSERT_TRUE(err.isEmpty());

        PatientSearchCriteria criteria;
        criteria.searchKey = "MR Test Patient";
        criteria.page = 1;
        criteria.pageSize = 1;
        auto res = patientService->searchPatientsPaged(criteria);
        TEST_ASSERT_TRUE(res.totalCount >= 1);
        patientId = res.items.first().patientId;
    });

    // --- Setup 2: Create a doctor (satisfies FK doctor_id -> staff) ---
    RUN_INLINE_TEST("Setup 2: Create a doctor (satisfies FK doctor_id -> staff)", {
        DoctorInputDTO docDto;
        docDto.fullName = "MR Doctor Test";
        docDto.gender = "MALE";
        docDto.dateOfBirth = QDate(1980, 1, 1);
        docDto.citizenId = "079180009999";
        docDto.phoneNumber = "0909999999";
        docDto.email = "mr_doc@test.com";
        docDto.address = "Hospital";
        docDto.departmentId = 1;
        docDto.shift = "MORNING";
        docDto.specialty = "KHOA_NOI";
        docDto.licenseNumber = "CCHNMRDOC1";
        docDto.experienceYears = 10;
        docDto.consultationFee = 200000;
        docDto.bio = "Bio";

        StaffHireResult hireRes = staffService.hireNewDoctor(docDto);
        TEST_ASSERT_TRUE(hireRes.errorMessage.isEmpty());
        
        StaffSearchCriteria docCriteria;
        docCriteria.searchKey = "MR Doctor Test";
        docCriteria.page = 1;
        docCriteria.pageSize = 1;
        auto docSearchRes = staffService.searchStaffPaged(docCriteria);
        TEST_ASSERT_TRUE(docSearchRes.totalCount >= 1);
        doctorId = docSearchRes.items.first()->getAccountId();
    });

    // --- Setup 3: Create an appointment (satisfies FK appointment_id -> appointments) ---
    RUN_INLINE_TEST("Setup 3: Create an appointment (satisfies FK appointment_id -> appointments)", {
        AppointmentInputDTO apptDto;
        apptDto.patientId = patientId;
        apptDto.doctorId = doctorId;
        apptDto.createdBy = doctorId;
        apptDto.ticketNumber = 1;
        apptDto.date = QDate::currentDate();
        apptDto.startTime = QTime(9, 0);
        apptDto.endTime = QTime(9, 30);
        apptDto.reason = "Examination";

        bool apptCreated = apptRepo->createAppointment(apptDto);
        TEST_ASSERT_TRUE(apptCreated == true);

        auto appts = apptRepo->getPatientAppointments(patientId);
        TEST_ASSERT_TRUE(!appts.isEmpty());
        appointmentId = appts.first().appointmentId;
    });

    int createdRecordId = -1;

    // --- HAPPY: Create medical record ---
    RUN_INLINE_TEST("HAPPY: Create medical record", {
        MedicalRecordInsertDTO dto;
        dto.patientId = patientId;
        dto.doctorId = doctorId;
        dto.appointmentId = appointmentId;
        dto.visitDateTime = QDateTime::currentDateTime();
        dto.vitals.bloodPressure = "120/80";
        dto.vitals.heartRate = 75;
        dto.vitals.temperature = 36.5;
        dto.vitals.weight = 60.0;
        dto.vitals.height = 165.0;
        dto.chiefComplaint = "Headache and fever";
        dto.clinicalNotes = "Patient presents with mild headache";
        dto.treatment = "Paracetamol 500mg";

        Diagnosis diag;
        diag.icdCode = "R51";
        diag.description = "Headache";
        diag.severity = severityToEn(Severity::Mild);
        dto.diagnoses.append(diag);

        QString err = mrService.createMedicalRecord(dto, &createdRecordId);
        TEST_ASSERT_TRUE(err.isEmpty());
        TEST_ASSERT_TRUE(createdRecordId > 0);
        std::cout << "    [OK] createMedicalRecord -> recordId=" << createdRecordId << std::endl;
    });

    // --- HAPPY: Get medical history ---
    RUN_INLINE_TEST("HAPPY: Get medical history", {
        auto history = mrService.getMedicalHistory(patientId);
        TEST_ASSERT_TRUE(history.size() >= 1);
        TEST_ASSERT_TRUE(history.first().chiefComplaint.contains("Headache"));
        std::cout << "    [OK] getMedicalHistory -> " << history.size() << " record(s)" << std::endl;
    });

    // --- HAPPY: Search records ---
    RUN_INLINE_TEST("HAPPY: Search records", {
        MedicalRecordSearchCriteria criteria;
        criteria.searchKey = "Headache";
        criteria.patientId = patientId;
        criteria.page = 1;
        criteria.pageSize = 10;
        auto results = mrService.searchMedicalRecordsPaged(criteria);
        TEST_ASSERT_TRUE(results.totalCount >= 1);
        std::cout << "    [OK] searchMedicalRecordsPaged -> found " << results.totalCount << " result(s)" << std::endl;
    });

    // --- HAPPY: Soft delete ---
    RUN_INLINE_TEST("HAPPY: Soft delete", {
        bool deleted = mrService.softDeleteMedicalRecord(createdRecordId);
        TEST_ASSERT_TRUE(deleted == true);
        std::cout << "    [OK] softDeleteMedicalRecord -> success" << std::endl;
    });

    // --- NEGATIVE: Create record with missing chief complaint ---
    RUN_INLINE_TEST("NEGATIVE: Create record with missing chief complaint", {
        MedicalRecordInsertDTO dto;
        dto.patientId = patientId;
        dto.doctorId = doctorId;
        dto.appointmentId = appointmentId;
        dto.visitDateTime = QDateTime::currentDateTime();
        dto.vitals.temperature = 36.5;
        dto.vitals.heartRate = 75;
        dto.vitals.weight = 60.0;
        dto.vitals.height = 165.0;
        dto.chiefComplaint = ""; // Empty - should fail validation
        dto.clinicalNotes = "Notes";
        dto.treatment = "Treatment";

        Diagnosis diag;
        diag.description = "Test";
        diag.severity = "MILD";
        dto.diagnoses.append(diag);

        QString err = mrService.createMedicalRecord(dto);
        TEST_ASSERT_TRUE(!err.isEmpty());
        std::cout << "    [OK] createMedicalRecord (empty complaint) -> rejected" << std::endl;
    });

    std::cout << "    ==> [PASSED] MedicalRecord Service Integration Tests (5/5)" << std::endl;
    std::fflush(stdout);
}
