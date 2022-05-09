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
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

if (TARGET tinyusb_device)
    add_executable(rp2040_fw
            main.c
            )

    pico_generate_pio_header(rp2040_fw ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
    # pull in common dependencies
    target_link_libraries(rp2040_fw pico_stdlib hardware_pwm hardware_pio hardware_dma)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(rp2040_fw 1)
    pico_enable_stdio_uart(rp2040_fw 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(rp2040_fw)
    # Additionally generate python and hex pioasm outputs for inclusion in the RP2040 datasheet
    add_custom_target(rp2040_fw_datasheet DEPENDS ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
            DEPENDS ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
            COMMAND Pioasm -o python ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
            )
    add_dependencies(rp2040_fw rp2040_fw_datasheet)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building rp2040_fw because TinyUSB submodule is not initialized in the SDK")
endif()
