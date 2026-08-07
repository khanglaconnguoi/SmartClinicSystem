# ─────────────────────────────────────────────────────────────
# SystemTestsRunner: GUI / System-level Tests with QTestLib
find_package(Qt6 REQUIRED COMPONENTS Test)

# Collect ALL src .cpp files (including UI) for full system tests
file(GLOB_RECURSE SYSTEM_SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# Remove main.cpp from the list (we have our own main)
list(FILTER SYSTEM_SRC_FILES EXCLUDE REGEX ".*src/main\\.cpp$")

add_executable(SystemTestsRunner
    SystemTestMain.cpp
    GUI_LoginSystemTest.cpp
    GUI_AdminStaffRegistrationTest.cpp
    GUI_ReceptionPatientRegistrationTest.cpp
    GUI_NurseVitalSignsTest.cpp
    GUI_DoctorExamTest.cpp
    GUI_PharmacyInventoryTest.cpp
    GUI_CashierBillingTest.cpp
    GUI_ExecutiveAnalyticsTest.cpp
    GUI_FullClinicWorkflowTest.cpp
    GuiTestHelper.h
    ${SYSTEM_SRC_FILES}
)

set_target_properties(SystemTestsRunner PROPERTIES WIN32_EXECUTABLE FALSE)

target_include_directories(SystemTestsRunner PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/external"
    "${CMAKE_SOURCE_DIR}/tests"
)

# Override PROJECT_ROOT_DIR to isolate test database
target_compile_definitions(SystemTestsRunner PRIVATE
    PROJECT_ROOT_DIR="${CMAKE_CURRENT_BINARY_DIR}/gui_test_env"
)

target_link_libraries(SystemTestsRunner PRIVATE
    bcrypt
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    Qt6::Test
)

if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET SystemTestsRunner POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:SystemTestsRunner>"
        COMMENT "Deploying Qt platform libraries for SystemTestsRunner..."
    )
endif()

add_test(NAME AllSystemTests COMMAND SystemTestsRunner)
