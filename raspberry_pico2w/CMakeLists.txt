# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_dns_server C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_dns_server main.cpp model.c)
target_include_directories(pico_dns_server PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(pico_dns_server 
    pico_stdlib 
    pico_cyw43_arch_lwip_threadsafe_background
)

pico_enable_stdio_usb(pico_dns_server 1)
pico_enable_stdio_uart(pico_dns_server 0)
pico_add_extra_outputs(pico_dns_server)
