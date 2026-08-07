#include <iostream>
#include "../TestHelper.h"
#include <memory>
#include "service/AppointmentService.h"
#include "repository/AppointmentRepository.h"
#include "repository/PatientRepository.h"
#include "repository/StaffRepository.h"
#include <QSqlQuery>

void runAppointmentServiceTestSuite() {
    std::cout << "  [IT 7/9] Running Appointment Service Integration Tests..." << std::endl;
    auto appointmentRepo = std::make_shared<AppointmentRepository>();
    auto patientRepo = std::make_shared<PatientRepository>();
    auto staffRepo = std::make_shared<StaffRepository>();
    AppointmentService appointmentService(appointmentRepo, patientRepo, staffRepo);

    // --- HAPPY: Validate appointment date (static) ---
    RUN_INLINE_TEST("HAPPY: Validate appointment date (static)", {
        // Past date should be invalid
        QString err = AppointmentService::validateAppointmentDate(QDate::currentDate().addDays(-1));
        TEST_ASSERT_TRUE(!err.isEmpty());

        // Future date should be valid
        err = AppointmentService::validateAppointmentDate(QDate::currentDate().addDays(1));
        TEST_ASSERT_TRUE(err.isEmpty());
        std::cout << "    [OK] validateAppointmentDate -> past=invalid, future=valid" << std::endl;
    });

    // --- HAPPY: Validate time slots (static) ---
    RUN_INLINE_TEST("HAPPY: Validate time slots (static)", {
        QTime start(9, 0);
        QTime end(9, 30);
        TEST_ASSERT_TRUE(AppointmentService::validateStartTime(start).isEmpty());
        TEST_ASSERT_TRUE(AppointmentService::validateEndTime(end, start).isEmpty());

        // End before start
        QTime badEnd(8, 30);
        TEST_ASSERT_TRUE(!AppointmentService::validateEndTime(badEnd, start).isEmpty());
        std::cout << "    [OK] validateStartTime/EndTime -> correct" << std::endl;
    });

    // --- HAPPY: Validate appointment status ---
    RUN_INLINE_TEST("HAPPY: Validate appointment status", {
        TEST_ASSERT_TRUE(AppointmentService::validateAppointmentStatus("SCHEDULED").isEmpty());
        TEST_ASSERT_TRUE(!AppointmentService::validateAppointmentStatus("INVALID_STATUS").isEmpty());
        std::cout << "    [OK] validateAppointmentStatus -> correct" << std::endl;
    });

    // --- HAPPY: Get appointments by date (may be empty) ---
    RUN_INLINE_TEST("HAPPY: Get appointments by date (may be empty)", {
        auto appointments = appointmentService.getAppointmentsByDate(QDate::currentDate());
        std::cout << "    [OK] getAppointmentsByDate -> " << appointments.size() << " appointment(s)" << std::endl;
    });

    // --- HAPPY: Schedule Conflict and No-Show Workflow ---
    RUN_INLINE_TEST("HAPPY: Schedule Conflict and No-Show Workflow", {
        // 1. Setup Dummy Data
        QSqlQuery q;
        q.exec("INSERT OR IGNORE INTO patients (patient_id, patient_code, full_name, date_of_birth, gender, citizen_id, phone_number, email, address, emergency_contact_name, emergency_contact_phone) "
               "VALUES (77, 'PAT-77', 'Appointment Test Patient', '1990-01-01', 'MALE', '077777777777', '0900000077', 'appt77@test.com', 'Address', 'Contact', '0900000000')");
        q.exec("INSERT OR IGNORE INTO staff (staff_id, staff_code, password_hash, full_name, role, gender, date_of_birth, citizen_id, phone_number, email, address, department_id, shift) "
               "VALUES (77, 'D7701', 'hash', 'Appointment Test Doctor', 'DOCTOR', 'MALE', '1985-01-01', '079000000077', '0900000077', 'doc77@test.com', 'Address', 1, 'FULL_DAY')");

        // 2. Schedule First Appointment (Future)
        AppointmentInputDTO appt1;
        appt1.patientId = 77;
        appt1.doctorId = 77;
        appt1.date = QDate::currentDate().addDays(2);
        appt1.startTime = QTime(9, 0);
        appt1.endTime = QTime(9, 30);
        
        QString err = appointmentService.createAppointment(appt1);
        TEST_ASSERT_TRUE(err.isEmpty());

        // 3. Test Conflict (Same doctor, same time)
        AppointmentInputDTO apptConflict;
        apptConflict.patientId = 77;
        apptConflict.doctorId = 77;
        apptConflict.date = QDate::currentDate().addDays(2);
        apptConflict.startTime = QTime(9, 0);
        apptConflict.endTime = QTime(9, 30);

        QString errConflict = appointmentService.createAppointment(apptConflict);
        // Expecting an error due to conflict
        TEST_ASSERT_TRUE(!errConflict.isEmpty());
        std::cout << "    [OK] scheduleAppointment -> conflict prevented properly" << std::endl;

        // 4. Test Auto-Mark No-Show
        // Insert a raw past appointment that was PENDING
        QString pastDate = QDate::currentDate().addDays(-2).toString("yyyy-MM-dd");
        q.exec(QString("INSERT INTO appointments (patient_id, doctor_id, appointment_date, appointment_time, status) VALUES (77, 77, '%1', '08:00', 'PENDING')").arg(pastDate));
        
        // Auto mark
        appointmentRepo->autoMarkNoShowAppointments();
        
        // Check if it updated
        auto appts = appointmentRepo->getPatientAppointments(77);
        bool foundNoShow = false;
        for (const auto& a : appts) {
            if (a.appointmentDate == pastDate && a.status == "NO_SHOW") {
                foundNoShow = true;
            }
        }
        TEST_ASSERT_TRUE(foundNoShow);
        std::cout << "    [OK] autoMarkNoShowAppointments -> successfully marked past pending appointments as NO_SHOW" << std::endl;
    });

    std::cout << "    ==> [PASSED] Appointment Service Integration Tests (5/5)" << std::endl;
    std::fflush(stdout);
}
