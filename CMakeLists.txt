cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(dpp REQUIRED)
find_package(SQLite3 REQUIRED)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

set(BOT_BINARY_NAME "axuaxibot" CACHE STRING "vsn")

add_executable(${BOT_BINARY_NAME} ${SRC_FILES})
target_include_directories(${BOT_BINARY_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${BOT_BINARY_NAME} PRIVATE dpp::dpp SQLite::SQLite3)