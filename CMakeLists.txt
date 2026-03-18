cmake_minimum_required(VERSION 3.10)
project(axuaxibot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(dpp REQUIRED)

add_executable(axuaxibot main.cpp)

target_link_libraries(axuaxibot dpp::dpp)
