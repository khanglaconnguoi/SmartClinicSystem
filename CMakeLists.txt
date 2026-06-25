cmake_minimum_required(VERSION 3.16)
project(SmartClinicSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Charts)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
    "src/*.hpp"
    "src/*.ui"
    "src/*.qrc"
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /w14456 /w14457 /w14458 /w14459)
endif()

add_subdirectory(external/Bcrypt.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Charts)

target_include_directories(${PROJECT_NAME} PRIVATE external/Bcrypt.cpp/include src)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}"
)
