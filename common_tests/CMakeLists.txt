cmake_minimum_required(VERSION 3.13)
project(dns-exfiltration-detector-lib-tests)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
add_definitions(-std=c++17)

SET(SOURCES "main.cpp")

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} gtest pthread)
