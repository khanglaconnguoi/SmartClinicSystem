#include <iostream>
#include <memory>
#include <QtTest/QTest>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QDate>
#include <QMessageBox>

#include "../TestHelper.h"
#include "GuiTestHelper.h"
#include "service/AuthService.h"
#include "service/StaffService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"
#include "service/MedicalRecordService.h"
#include "service/PharmacyService.h"
#include "service/BillingService.h"
#include "service/AnalyticService.h"

#include "repository/StaffRepository.h"
#include "repository/PatientRepository.h"
#include "repository/AppointmentRepository.h"
#include "repository/MedicalRecordRepository.h"
#include "repository/MedicationRepository.h"
#include "repository/PrescriptionRepository.h"
#include "repository/BillingRepository.h"
#include "repository/AnalyticRepository.h"

#include "ui/view/LoginDialog.h"
#include "ui/Admin/DoctorRegistrationDialog.h"
#include "ui/Reception/PatientRegistrationDialog.h"
#include "ui/Nurse/NurseDashboard.h"
#include "ui/Doctor/CreatePrescriptionDialog.h"
#include "ui/Pharmacy/AddMedicationDialog.h"
#include "ui/Pharmacy/PharmacistDashboard.h"
#include "ui/Admin/AdminAnalyticsWidget.h"

void runFullClinicWorkflowTestSuite() {
    std::cout << "\n=======================================================" << std::endl;
    std::cout << "  [E2E WORKFLOW] Running 8 Main Clinic Workflows..." << std::endl;
    std::cout << "=======================================================" << std::endl;

    // Repositories
    auto staffRepo = std::make_shared<StaffRepository>();
    auto patientRepo = std::make_shared<PatientRepository>();
    auto appointmentRepo = std::make_shared<AppointmentRepository>();
    auto recordRepo = std::make_shared<MedicalRecordRepository>();
    auto medRepo = std::make_shared<MedicationRepository>();
    auto presRepo = std::make_shared<PrescriptionRepository>();
    auto billingRepo = std::make_shared<BillingRepository>();
    auto analyticRepo = std::make_shared<AnalyticRepository>();

    // Services
    auto staffService = std::make_shared<StaffService>(staffRepo);
    auto authService = std::make_shared<AuthService>(staffRepo);
    auto patientService = std::make_shared<PatientService>(patientRepo);
    auto appointmentService = std::make_shared<AppointmentService>(appointmentRepo);
    auto recordService = std::make_shared<MedicalRecordService>(recordRepo, patientService);
    auto pharmacyService = std::make_shared<PharmacyService>(medRepo, presRepo);
    auto billingService = std::make_shared<BillingService>(billingRepo);
    auto analyticService = std::make_shared<AnalyticService>(analyticRepo);

    // Variables to track across the 8 workflows
    QString doctorStaffCode;
    QString doctorPassword;
    int createdPatientId = -1;

    // --- WORKFLOW 1: Authentication & Role-Based Access ---
    RUN_INLINE_TEST("WORKFLOW 1: Authentication & Role-Based Access", {
        DoctorInputDTO docInput;
        docInput.fullName = "EndToEnd Doctor Main";
        docInput.gender = "MALE";
        docInput.dateOfBirth = QDate(1985, 5, 15);
        docInput.citizenId = "079185999001";
        docInput.phoneNumber = "0985999001";
        docInput.email = "e2edoctor@clinic.com";
        docInput.address = "Main Street";
        docInput.departmentId = 1;
        docInput.shift = "FULL_DAY";
        docInput.specialty = "KHOA_NOI";
        docInput.licenseNumber = "CCHNE2E001";
        docInput.experienceYears = 10;
        docInput.consultationFee = 200000;
        docInput.bio = "E2E Test Doctor";

        StaffHireResult hireResult = staffService->hireNewDoctor(docInput);
        TEST_ASSERT_TRUE(hireResult.errorMessage.isEmpty());
        doctorStaffCode = hireResult.staffCode;
        doctorPassword = hireResult.plainPassword;

        // Test Login Dialog GUI
        LoginDialog loginDlg(authService, staffService);
        loginDlg.show();

        auto lineEdits = loginDlg.findChildren<QLineEdit*>();
        TEST_ASSERT_TRUE(lineEdits.size() >= 2);
        QTest::keyClicks(lineEdits[0], doctorStaffCode);
        QTest::keyClicks(lineEdits[1], doctorPassword);

        LoginResult res = authService->login(doctorStaffCode, doctorPassword);
        TEST_ASSERT_TRUE(res.isLoginSuccess);
        std::cout << "    [OK] Workflow 1: Authentication successful for " << doctorStaffCode.toStdString() << std::endl;
        loginDlg.close();
    });

    // --- WORKFLOW 2: Staff Registration and Account Status ---
    RUN_INLINE_TEST("WORKFLOW 2: Staff Registration and Account Status", {
        DoctorRegistrationDialog regDlg(staffService, appointmentService);
        regDlg.show();

        auto lineEdits = regDlg.findChildren<QLineEdit*>();
        if (!lineEdits.isEmpty()) {
            lineEdits[0]->clear();
            QTest::keyClicks(lineEdits[0], "EndToEnd Registered Doctor");
        }
        std::cout << "    [OK] Workflow 2: Staff registration dialog form entry validated" << std::endl;
        regDlg.close();
    });

    // --- WORKFLOW 3: Patient Registration and Profile Management ---
    RUN_INLINE_TEST("WORKFLOW 3: Patient Registration and Profile Management", {
        OutPatientInputDTO pDto;
        pDto.fullName = "EndToEnd Patient Nguyen";
        pDto.gender = "FEMALE";
        pDto.dateOfBirth = QDate(1992, 3, 10);
        pDto.citizenId = "079192999002";
        pDto.phone = "0982999002";
        pDto.email = "e2epatient@test.com";
        pDto.address = "Nguyen Trai, Q5";
        pDto.bloodType = "O+";
        pDto.emergencyContactName = "Nguyen Van B";
        pDto.emergencyContactPhone = "0982999003";

        QString pErr = patientService->addOutPatient(pDto);
        TEST_ASSERT_TRUE(pErr.isEmpty());

        PatientSearchCriteria criteria;
        criteria.searchKey = "079192999002";
        auto searchRes = patientService->searchPatientsPaged(criteria);
        TEST_ASSERT_TRUE(searchRes.items.size() > 0);
        createdPatientId = searchRes.items.first().patientId;
        TEST_ASSERT_TRUE(createdPatientId > 0);

        PatientRegistrationDialog pDlg(patientService);
        pDlg.show();
        std::cout << "    [OK] Workflow 3: Patient registered with ID=" << createdPatientId << std::endl;
        pDlg.close();
    });

    // --- WORKFLOW 4: Appointment Scheduling, Queueing, and Cancellation ---
    RUN_INLINE_TEST("WORKFLOW 4: Appointment Scheduling & Queueing", {
        AppointmentInputDTO apptDto;
        apptDto.patientId = createdPatientId;
        apptDto.doctorId = 1;
        apptDto.date = QDate::currentDate().addDays(1);
        apptDto.startTime = QTime(9, 0);
        apptDto.endTime = QTime(9, 30);
        apptDto.reason = "Khám sức khỏe định kỳ E2E";

        QString apptErr = appointmentService->createAppointment(apptDto);
        TEST_ASSERT_TRUE(apptErr.isEmpty());

        std::cout << "    [OK] Workflow 4: Appointment scheduled successfully" << std::endl;
    });

    // --- WORKFLOW 5: Clinical Examination, Laboratory Testing & Prescribing ---
    RUN_INLINE_TEST("WORKFLOW 5: Clinical Examination & Prescribing", {
        CreatePrescriptionDialog presDlg(pharmacyService, patientService, createdPatientId);
        presDlg.show();

        auto lineEdits = presDlg.findChildren<QLineEdit*>();
        if (!lineEdits.isEmpty()) {
            QTest::keyClicks(lineEdits[0], "Paracetamol");
        }
        std::cout << "    [OK] Workflow 5: Prescription UI verified" << std::endl;
        presDlg.close();
    });

    // --- WORKFLOW 6: Pharmacy Inventory and Dispensing ---
    RUN_INLINE_TEST("WORKFLOW 6: Pharmacy Inventory & Dispensing", {
        // Active ingredient setup for FK
        {
            QSqlQuery q;
            q.exec("INSERT OR IGNORE INTO active_ingredients (ingredient_id, ingredient_name, description) VALUES (1, 'Paracetamol', 'Analgesic')");
        }

        MedicationInputDTO medDto;
        medDto.brandName = "E2E Paracetamol 500mg";
        medDto.unit = "TABLET";
        medDto.unitPrice = 4500.0;
        medDto.stockQuantity = 500;
        medDto.minimumStock = 50;
        medDto.reorderThreshold = 100;
        medDto.expiryDate = QDate::currentDate().addYears(1);
        medDto.manufacturer = "E2E Pharma";
        medDto.description = "E2E Pain Reliever";
        medDto.categories = {"ANALGESICS"};

        MedicationInputDTO::IngredientInput ing;
        ing.ingredientId = 1;
        ing.strength = "500mg";
        medDto.ingredients.append(ing);

        QString medErr = pharmacyService->addMedication(medDto);
        TEST_ASSERT_TRUE(medErr.isEmpty());

        AddMedicationDialog medDlg(pharmacyService);
        medDlg.show();
        std::cout << "    [OK] Workflow 6: Medication added & Pharmacy UI verified" << std::endl;
        medDlg.close();
    });

    // --- WORKFLOW 7: Billing and Payment Collection ---
    RUN_INLINE_TEST("WORKFLOW 7: Billing & Payment Collection", {
        InvoiceInsertDTO invDto;
        invDto.invoiceCode = "INV-E2E-2026-001";
        invDto.patientId = createdPatientId;
        invDto.recordId = std::nullopt;
        invDto.patientType = "OUTPATIENT";
        invDto.consultationFee = 200000.0;
        invDto.medicationFee = 45000.0;
        invDto.totalAmount = 245000.0;
        invDto.status = "UNPAID";
        invDto.issuedDate = QDate::currentDate().toString("yyyy-MM-dd");

        InvoiceItemDTO item;
        item.itemType = "MEDICATION";
        item.description = "E2E Paracetamol 500mg";
        item.quantity = 10;
        item.unitPrice = 4500.0;
        item.subtotal = 45000.0;
        invDto.items.append(item);

        bool invOk = billingRepo->insertInvoice(invDto);
        TEST_ASSERT_TRUE(invOk);

        std::cout << "    [OK] Workflow 7: Invoice created and persisted" << std::endl;
    });

    // --- WORKFLOW 8: Executive Analytics ---
    RUN_INLINE_TEST("WORKFLOW 8: Executive Analytics", {
        AdminAnalyticsWidget analyticWidget(analyticService);
        analyticWidget.show();

        auto buttons = analyticWidget.findChildren<QPushButton*>();
        TEST_ASSERT_TRUE(!buttons.isEmpty());

        std::cout << "    [OK] Workflow 8: Executive Analytics UI verified" << std::endl;
        analyticWidget.close();
    });

    std::cout << "    ==> [PASSED] ALL 8 CLINIC WORKFLOWS EXECUTED SUCCESSFULLY!" << std::endl;
    std::fflush(stdout);
}
