# ─────────────────────────────────────────────────────────────
# IntegrationTestsRunner: Service + Repository + Database (SQLite)
# ─────────────────────────────────────────────────────────────

# Collect ALL src .cpp files EXCEPT main.cpp and UI files
file(GLOB_RECURSE INTEGRATION_SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/model/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/service/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/repository/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/factory/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/service/UserSession.h"
)

add_executable(IntegrationTestsRunner
    IntegrationTestMain.cpp
    IT_PatientServiceTest.cpp
    IT_MedicalRecordServiceTest.cpp
    IT_StaffServiceTest.cpp
    IT_AuthServiceTest.cpp
    IT_PharmacyServiceTest.cpp
    IT_BillingServiceTest.cpp
    IT_AppointmentServiceTest.cpp
    IT_ServiceRequestServiceTest.cpp
    IT_AnalyticServiceTest.cpp
    IT_RepositoryCrudTest.cpp
    ${INTEGRATION_SRC_FILES}
)

set_target_properties(IntegrationTestsRunner PROPERTIES WIN32_EXECUTABLE FALSE)

target_include_directories(IntegrationTestsRunner PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/external"
    "${CMAKE_SOURCE_DIR}/tests"
)

# Override PROJECT_ROOT_DIR to point to a test-specific temp directory
# This ensures DatabaseManager creates a SEPARATE test database
target_compile_definitions(IntegrationTestsRunner PRIVATE
    PROJECT_ROOT_DIR="${CMAKE_CURRENT_BINARY_DIR}/test_env"
)

target_link_libraries(IntegrationTestsRunner PRIVATE
    bcrypt
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET IntegrationTestsRunner POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:IntegrationTestsRunner>"
        COMMENT "Deploying Qt platform libraries for IntegrationTestsRunner..."
    )
endif()

add_test(NAME AllIntegrationTests COMMAND IntegrationTestsRunner)
