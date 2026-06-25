find_package(Qt6 REQUIRED COMPONENTS Core Test Sql)

file(GLOB_RECURSE SRC_SOURCES "../src/*.cpp")
file(GLOB_RECURSE MAIN_SRC "../src/main.cpp")
file(GLOB_RECURSE UI_SOURCES "../src/ui/*.cpp")
if(MAIN_SRC)
    list(REMOVE_ITEM SRC_SOURCES ${MAIN_SRC})
endif()
if(UI_SOURCES)
    list(REMOVE_ITEM SRC_SOURCES ${UI_SOURCES})
endif()

# Bật AUTOMOC cho thư mục test
set(CMAKE_AUTOMOC ON)

add_executable(PatientTest PatientTest.cpp ${SRC_SOURCES})

target_include_directories(PatientTest PRIVATE ../src ../external)
target_compile_definitions(PatientTest PRIVATE PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(PatientTest PRIVATE Qt6::Core Qt6::Test Qt6::Sql)

add_test(NAME PatientTest COMMAND PatientTest)

# Copy sqlite driver cho Test (để DB hoạt động)
add_custom_command(TARGET PatientTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:PatientTest>/sqldrivers"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt6_DIR}/../../../plugins/sqldrivers/qsqlite.dll"
        "$<TARGET_FILE_DIR:PatientTest>/sqldrivers/"
    COMMENT "Deploying qsqlite.dll plugin for Tests..."
)
