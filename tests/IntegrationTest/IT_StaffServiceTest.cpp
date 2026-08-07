#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "service/StaffService.h"
#include "repository/StaffRepository.h"
#include <QSqlQuery>
#include <QDate>

void runStaffServiceTestSuite() {
    std::cout << "  [IT 1/9] Running Staff Service Integration Tests..." << std::endl;
    auto staffRepo = std::make_shared<StaffRepository>();
    StaffService staffService(staffRepo);

    // --- HAPPY: Hire a Doctor ---
    RUN_INLINE_TEST("HAPPY: Hire a Doctor", {
        DoctorInputDTO dto;
        dto.fullName = "IT Doctor Nguyen";
        dto.gender = "MALE";
        dto.dateOfBirth = QDate(1985, 5, 15);
        dto.citizenId = "079185001001";
        dto.phoneNumber = "0901000001";
        dto.email = "it_doctor@test.com";
        dto.address = "123 Test St";
        dto.departmentId = 1;
        dto.shift = "MORNING";
        dto.specialty = "KHOA_NOI";
        dto.licenseNumber = "CCHNIT001";
        dto.experienceYears = 10;
        dto.consultationFee = 200000;
        dto.bio = "Integration test doctor";

        StaffHireResult result = staffService.hireNewDoctor(dto);
        TEST_ASSERT_TRUE(result.errorMessage.isEmpty()); // No error
        TEST_ASSERT_TRUE(!result.staffCode.isEmpty());   // Staff code generated
        TEST_ASSERT_TRUE(!result.plainPassword.isEmpty()); // Password generated
        std::cout << "    [OK] hireNewDoctor -> staffCode=" << result.staffCode.toStdString() << std::endl;
    });

    // --- HAPPY: Search Staff ---
    RUN_INLINE_TEST("HAPPY: Search Staff", {
        StaffSearchCriteria criteria;
        criteria.searchKey = "IT Doctor";
        criteria.page = 1;
        criteria.pageSize = 10;
        auto results = staffService.searchStaffPaged(criteria);
        TEST_ASSERT_TRUE(results.totalCount >= 1);
        std::cout << "    [OK] searchStaffPaged -> found " << results.totalCount << " result(s)" << std::endl;
    });

    // --- NEGATIVE: Hire with empty name ---
    RUN_INLINE_TEST("NEGATIVE: Hire with empty name", {
        DoctorInputDTO dto;
        dto.fullName = "";  // Empty name should fail validation
        dto.gender = "MALE";
        dto.dateOfBirth = QDate(1985, 5, 15);
        dto.citizenId = "079185001002";
        dto.phoneNumber = "0901000002";
        dto.email = "bad@test.com";
        dto.address = "Test";
        dto.departmentId = 1;
        dto.shift = "MORNING";
        dto.specialty = "KHOA_NOI";
        dto.licenseNumber = "CCHNIT002";
        dto.experienceYears = 5;
        dto.consultationFee = 150000;
        dto.bio = "Bad";

        StaffHireResult result = staffService.hireNewDoctor(dto);
        TEST_ASSERT_TRUE(!result.errorMessage.isEmpty()); // Should have error
        std::cout << "    [OK] hireNewDoctor (empty name) -> rejected" << std::endl;
    });

    // --- HAPPY: Leave Management Workflow ---
    RUN_INLINE_TEST("HAPPY: Leave Management Workflow", {
        // 1. Setup Dummy Staff
        QSqlQuery q;
        q.exec("INSERT OR IGNORE INTO staff (staff_id, staff_code, password_hash, full_name, role, gender, date_of_birth, citizen_id, phone_number, email, address, department_id, shift) "
               "VALUES (66, 'N6601', 'hash', 'Leave Test Staff', 'NURSE', 'FEMALE', '1990-01-01', '079000000066', '0900000066', 'leave66@test.com', 'Address', 1, 'FULL_DAY')");
        // Ensure leave balance exists
        q.exec(QString("INSERT OR IGNORE INTO leave_balances (staff_id, year, total_days, used_days) VALUES (66, %1, 12, 0)").arg(QDate::currentDate().year()));

        // 2. Register Leave (3 days)
        QDate startDate = QDate::currentDate().addDays(10);
        QDate endDate = startDate.addDays(2);
        QString err = staffService.registerLeave(66, startDate, endDate, "Personal trip");
        TEST_ASSERT_TRUE(err.isEmpty());

        // 3. Get pending requests
        auto pendingReqs = staffService.getPendingLeaveRequests();
        TEST_ASSERT_TRUE(pendingReqs.size() > 0);
        
        int leaveRequestId = -1;
        for (const auto& r : pendingReqs) {
            if (r.staffId == 66 && r.reason == "Personal trip") {
                leaveRequestId = r.requestId;
                break;
            }
        }
        TEST_ASSERT_TRUE(leaveRequestId != -1);

        // 4. Process (Approve) Leave Request
        // Pass nullptr for appointmentService since staff is Nurse and we don't need to cancel appointments
        QString processErr = staffService.processLeaveRequest(leaveRequestId, true, nullptr);
        TEST_ASSERT_TRUE(processErr.isEmpty());

        // 5. Verify Leave Balance Deducted
        auto balance = staffService.getLeaveBalance(66, startDate.year());
        TEST_ASSERT_TRUE(balance.usedDays == 3);
        TEST_ASSERT_TRUE((balance.totalDays - balance.usedDays) == 9);

        std::cout << "    [OK] Leave Management Workflow -> success (balance deducted)" << std::endl;
    });

    std::cout << "    ==> [PASSED] Staff Service Integration Tests (4/4)" << std::endl;
    std::fflush(stdout);
}
