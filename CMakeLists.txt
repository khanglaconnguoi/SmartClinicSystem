cmake_minimum_required(VERSION 3.16)
project(SmartClinicSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

enable_testing()
add_subdirectory(tests)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /w14456 /w14457 /w14458 /w14459)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)

target_include_directories(${PROJECT_NAME} PRIVATE external src)

# Automatically deploy all required Qt DLLs after every build
find_program(WINDEPLOYQT windeployqt6 HINTS "C:/Qt/6.11.1/mingw_64/bin")

if(WINDEPLOYQT)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${WINDEPLOYQT}
            --no-translations
            "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt DLLs with windeployqt6..."
    )
else()
    message(WARNING "windeployqt6 not found — Qt DLLs will not be auto-deployed.")
endif()
