add_executable(UnitTestsRunner
    UnitTestMain.cpp
    UT_ValidationTest.cpp
    UT_PatientModelTest.cpp
    UT_InvoiceModelTest.cpp
    UT_MedicationModelTest.cpp
    UT_MedicalRecordModelTest.cpp
    UT_AuthAndStaffModelTest.cpp
    UT_CommonEnumsTest.cpp
    UT_StaffRolesTest.cpp
    UT_InvoiceFactoryTest.cpp
    UT_PaginationTest.cpp
    UT_UserSessionTest.cpp
    UT_StringSanitizeTest.cpp
    ../../src/utils/StringSanitize.cpp
    ../../src/service/Validation.cpp
    ../../src/model/Patient.cpp
    ../../src/model/OutPatient.cpp
    ../../src/model/InPatient.cpp
    ../../src/model/EmergencyPatient.cpp
    ../../src/model/Invoice.cpp
    ../../src/model/OutPatientInvoice.cpp
    ../../src/model/InPatientInvoice.cpp
    ../../src/model/EmergencyInvoice.cpp
    ../../src/model/Medication.cpp
    ../../src/model/MedicalRecord.cpp
    ../../src/model/SystemUser.cpp
    ../../src/model/doctor.cpp
    ../../src/model/Pharmacist.cpp
    ../../src/factory/OutPatientInvoiceFactory.cpp
    ../../src/factory/InPatientInvoiceFactory.cpp
    ../../src/factory/EmergencyInvoiceFactory.cpp
    ../../src/service/UserSession.h
)

set_target_properties(UnitTestsRunner PROPERTIES WIN32_EXECUTABLE FALSE)

target_include_directories(UnitTestsRunner PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/external"
    "${CMAKE_SOURCE_DIR}/tests"
)

target_link_libraries(UnitTestsRunner PRIVATE
    bcrypt
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET UnitTestsRunner POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:UnitTestsRunner>"
        COMMENT "Deploying Qt platform libraries for UnitTestsRunner..."
    )
endif()

add_test(NAME AllUnitTests COMMAND UnitTestsRunner)
