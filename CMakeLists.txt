cmake_minimum_required(VERSION 3.16)
project(SmartClinicSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/mingw_64")

# Tìm kiếm các thành phần bộ thư viện cốt lõi bắt buộc trước
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)
# Tìm thành phần phụ tùy chọn (Gỡ bỏ REQUIRED để tránh lỗi tìm không thấy Charts tùy hệ thống)
find_package(Qt6 COMPONENTS Charts)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
    "src/*.hpp"
    "src/*.ui"
    "src/*.qrc"
    "external/Bcrypt.cpp/*.cpp"
    "external/Bcrypt.cpp/*.h"
)

# Loại bỏ các file tạm thời chưa dùng tới để tránh lỗi biên dịch cục bộ
list(FILTER SOURCES EXCLUDE REGEX "src/ui/MainWindow\\.(cpp|h)")

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /w14456 /w14457 /w14458 /w14459)
endif()

# Liên kết các thành phần thư viện bắt buộc cố định
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    Qt6::Sql
)

# Kiểm tra điều kiện nạp thành phần mở rộng an toàn (bcrypt độc lập và Charts tùy chọn)
if(Qt6Charts_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Charts)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_QT_CHARTS)
endif()

# Thiết lập chuẩn hóa tất cả các đường dẫn bao đóng đầu vào cho mã nguồn dự án chính
target_include_directories(${PROJECT_NAME} PRIVATE 
    "external"
    "external/Bcrypt.cpp"
    "external/Bcrypt.cpp/include"
    "src"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}"
)