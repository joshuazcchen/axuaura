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

set(BOT_BINARY_NAME "axuaxibot" CACHE STRING "vsn")

add_executable(${BOT_BINARY_NAME} ${SRC_FILES})

target_include_directories(${BOT_BINARY_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${BOT_BINARY_NAME} PRIVATE
    dpp::dpp
    SQLite::SQLite3
    dppp::dppp
    PkgConfig::ImageMagick
)
