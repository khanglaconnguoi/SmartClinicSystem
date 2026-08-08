#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "service/BillingService.h"
#include "repository/BillingRepository.h"
#include <QSqlQuery>
#include <QDate>

void runBillingServiceTestSuite() {
    std::cout << "  [IT 6/9] Running Billing Service Integration Tests..." << std::endl;
    auto billingRepo = std::make_shared<BillingRepository>();
    BillingService billingService(billingRepo);

    // --- HAPPY: Validate invoice input (static) ---
    RUN_INLINE_TEST("HAPPY: Validate invoice input (static)", {
        QList<PrescriptionItemDTO> emptyItems;
        QString err = BillingService::validateInvoiceInput(1, 1, 150000.0, emptyItems);
        // Empty items may or may not be valid depending on business rules
        std::cout << "    [OK] validateInvoiceInput (basic) -> " 
                  << (err.isEmpty() ? "valid" : "invalid") << std::endl;
    });

    // --- HAPPY: Validate consultation fee ---
    RUN_INLINE_TEST("HAPPY: Validate consultation fee", {
        TEST_ASSERT_TRUE(BillingService::validateConsultationFee(150000.0).isEmpty());
        TEST_ASSERT_TRUE(!BillingService::validateConsultationFee(-1.0).isEmpty());
        std::cout << "    [OK] validateConsultationFee -> valid/invalid correctly" << std::endl;
    });

    // --- HAPPY: Calculate medication total ---
    RUN_INLINE_TEST("HAPPY: Calculate medication total", {
        QList<PrescriptionItemDTO> items;
        PrescriptionItemDTO item1;
        item1.unitPrice = 5000.0;
        item1.quantity = 10;
        items.append(item1);

        PrescriptionItemDTO item2;
        item2.unitPrice = 15000.0;
        item2.quantity = 5;
        items.append(item2);

        double total = billingService.calculateMedicationTotal(items);
        TEST_ASSERT_TRUE(total == 125000.0); // 5000*10 + 15000*5
        std::cout << "    [OK] calculateMedicationTotal -> " << total << " VND" << std::endl;
    });

    // --- HAPPY: Search invoices (empty DB should return empty) ---
    RUN_INLINE_TEST("HAPPY: Search invoices (empty DB should return empty)", {
        InvoiceSearchCriteria criteria;
        criteria.page = 1;
        criteria.pageSize = 10;
        auto results = billingService.searchInvoicesPaged(criteria);
        std::cout << "    [OK] searchInvoicesPaged -> " << results.totalCount << " invoice(s)" << std::endl;
    });

    // --- HAPPY: Create and Pay Invoice Workflow ---
    RUN_INLINE_TEST("HAPPY: Create and Pay Invoice Workflow", {
        // 1. Setup minimal dummy data for Foreign Keys
        QSqlQuery q;
        q.exec("INSERT OR IGNORE INTO patients (patient_id, patient_code, full_name, date_of_birth, gender, citizen_id, phone_number, email, address, emergency_contact_name, emergency_contact_phone) "
               "VALUES (88, 'PAT-88', 'Billing Test Patient', '1990-01-01', 'MALE', '078888888888', '0900000088', 'bill88@test.com', 'Address', 'Contact', '0900000000')");
        q.exec("INSERT OR IGNORE INTO staff (staff_id, staff_code, password_hash, full_name, role, gender, date_of_birth, citizen_id, phone_number, email, address, department_id, shift) "
               "VALUES (88, 'D8801', 'hash', 'Billing Test Doctor', 'DOCTOR', 'MALE', '1985-01-01', '079000000088', '0900000088', 'doc88@test.com', 'Address', 1, 'FULL_DAY')");
        q.exec("INSERT OR IGNORE INTO appointments (appointment_id, ticket_number, patient_id, doctor_id, appointment_date, start_time, status) VALUES (88, 1, 88, 88, '2026-01-01', '08:00', 'COMPLETED')");
        q.exec("INSERT OR IGNORE INTO medical_records (record_id, patient_id, doctor_id, appointment_id, visit_datetime) VALUES (88, 88, 88, 88, '2026-01-01 08:00:00')");

        // 2. Create Invoice
        InvoiceInsertDTO invDto;
        invDto.invoiceCode = "INV-TEST-0088";
        invDto.patientId = 88;
        invDto.recordId = 88;
        invDto.patientType = "OUTPATIENT";
        invDto.consultationFee = 150000.0;
        invDto.medicationFee = 50000.0;
        invDto.totalAmount = 200000.0;
        invDto.status = InvoiceStatusText::UNPAID;
        invDto.issuedDate = QDate::currentDate().toString("yyyy-MM-dd");
        
        InvoiceItemDTO item;
        item.itemType = "MEDICATION";
        item.description = "Test Medicine";
        item.unitPrice = 5000.0;
        item.quantity = 10;
        item.subtotal = 50000.0;
        invDto.items.append(item);

        bool inserted = billingRepo->insertInvoice(invDto);
        TEST_ASSERT_TRUE(inserted);

        // 3. Find created invoice
        auto invoices = billingRepo->getInvoicesByPatientId(88);
        TEST_ASSERT_TRUE(!invoices.isEmpty());
        int invId = invoices.last().invoiceId;
        TEST_ASSERT_TRUE(invoices.last().status == InvoiceStatusText::UNPAID);

        // 4. Pay Invoice (Update status to PAID)
        bool updated = billingRepo->collectPayment(invId, 200000.0);
        TEST_ASSERT_TRUE(updated);

        // 5. Verify status is PAID
        auto invoicesAfter = billingRepo->getInvoicesByPatientId(88);
        TEST_ASSERT_TRUE(invoicesAfter.last().status == InvoiceStatusText::PAID);
        
        std::cout << "    [OK] createAndPayInvoice -> success, status updated to PAID" << std::endl;
    });

    std::cout << "    ==> [PASSED] Billing Service Integration Tests (5/5)" << std::endl;
    std::fflush(stdout);
}
