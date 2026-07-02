cmake_minimum_required(VERSION 3.13)

project(dns_exfiltration_luckfox_pico_daemon C CXX)

set(SPDLOG_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/../third_party/spdlog/include")
set(RKNN_LIB_DIR "$ENV{HOME}/Projects/rknn-toolkit2/rknpu2/runtime/Linux/librknn_api/armhf-uclibc/")

set(CMAKE_C_COMPILER "$ENV{HOME}/Projects/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc")
set(CMAKE_CXX_COMPILER "$ENV{HOME}/Projects/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-c++")

add_executable(dns_exfiltration_luckfox_pico_daemon src/main.cpp)

target_include_directories(
    dns_exfiltration_luckfox_pico_daemon
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    "${CMAKE_CURRENT_LIST_DIR}/../third_party/spdlog/include"
    "${CMAKE_CURRENT_LIST_DIR}/../common/include"
)

target_compile_options(dns_exfiltration_luckfox_pico_daemon PUBLIC -Wall -O2)

target_link_directories(dns_exfiltration_luckfox_pico_daemon PRIVATE "${RKNN_LIB_DIR}")
target_link_libraries(dns_exfiltration_luckfox_pico_daemon PRIVATE "rknnmrt")
