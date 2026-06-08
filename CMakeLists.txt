cmake_minimum_required(VERSION 3.15)

project("AxuaxiServerBot")

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(dpp REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(dppp CONFIG REQUIRED)
pkg_check_modules(ImageMagick REQUIRED IMPORTED_TARGET Magick++-7.Q16HDRI)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(axuaxibot ${SRC_FILES})

target_include_directories(axuaxibot PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(axuaxibot PRIVATE
    dpp::dpp
    SQLite::SQLite3
    dppp::dppp
    PkgConfig::ImageMagick
)

file(GLOB AXUCTL_FILES "tools/axuctl/*.cpp")

add_executable(axuctl ${AXUCTL_FILES})

target_include_directories(axuctl PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tools/axuctl
)

target_link_libraries(axuctl PRIVATE
    SQLite::SQLite3
)

enable_testing()

add_executable(axuaura_tests tests/test_main.cpp)
target_include_directories(axuaura_tests PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(axuaura_tests PRIVATE SQLite::SQLite3)
add_test(NAME unit_tests COMMAND axuaura_tests)
