# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS $ENV{PWD}/esp-idf-lib/components $ENV{PWD}/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(spect)
fatfs_create_spiflash_image(storage flash_image FLASH_IN_PROJECT)

if(${IDF_VERSION_MAJOR} EQUAL 4 AND ${IDF_VERSION_MINOR} EQUAL 3)
    # add_library("tinyusb") was used instead of idf_component_register so "__idf_" wasn't added to the name
    set(tinyusb_name "tinyusb")
else()
    set(tinyusb_name "__idf_tinyusb")
endif()

# Workaround for Vendor class. ESP-IDF v4.3 doesn't support it directly through its tinyusb "additions" layer.
# target_compile_definitions(${tinyusb_name} PUBLIC CFG_TUD_VENDOR=1)
# target_compile_definitions(${tinyusb_name} PUBLIC CFG_TUD_VENDOR_RX_BUFSIZE=64)
# target_compile_definitions(${tinyusb_name} PUBLIC CFG_TUD_VENDOR_TX_BUFSIZE=64)