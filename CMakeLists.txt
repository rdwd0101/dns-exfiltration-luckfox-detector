cmake_minimum_required(VERSION 3.13)

project(dns_exfiltration)

option(BUILD_RPI_PICO2W "Enable building Raspberry Pi Pico 2 W binaries" OFF)
option(BUILD_LUCKFOX_PICO "Enable building Luckfox Pico Pro/Max binaries" ON)

if(BUILD_RPI_PICO2W)
    add_subdirectory(raspberry_pico2w)
endif()
if(BUILD_LUCKFOX_PICO)
    add_subdirectory(luckfox_pico)
endif()