cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
set(PICO_XOSC_STARTUP_DELAY_MULTIPLIER, yes)
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

if (TARGET tinyusb_device)
    add_executable(rp2040_fw)

    #pico_generate_pio_header(rp2040_fw ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)
    #pico_generate_pio_header(rp2040_fw ${CMAKE_CURRENT_LIST_DIR}/spi.pio)
    target_sources(rp2040_fw PRIVATE
        main.c
    )
    target_link_libraries(rp2040_fw PRIVATE pico_stdlib pico_stdio_usb hardware_pio hardware_pwm hardware_dma hardware_uart hardware_spi)
    pico_add_extra_outputs(rp2040_fw)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building rp2040_fw because TinyUSB submodule is not initialized in the SDK")
endif()
