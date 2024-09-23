cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0057 NEW)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(FAMILY rp2040)
set(BOARD pico_w)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")

# # Pull in SDK (must be before project)
include("$ENV{PICO_SDK_PATH}/pico_sdk_init.cmake")
# # include(pico_extras_import_optional.cmake)

project(pico_hid)

# # Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(pico_hid)

pico_enable_stdio_usb(pico_hid 0)
pico_enable_stdio_uart(pico_hid 1)

target_sources(pico_hid PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/pico_hid.c
        
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(pico_hid PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(pico_hid PUBLIC 
        pico_stdlib
        tinyusb_device
        tinyusb_board
        hardware_adc
)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
# target_compile_definitions(pico_hid PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs(pico_hid)

# add url via pico_set_program_url
# example_auto_set_url(pico_hid)