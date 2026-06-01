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

add_executable(axuctl tools/axuctl.cpp)

target_include_directories(axuctl PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(axuctl PRIVATE 
    SQLite::SQLite3
)
