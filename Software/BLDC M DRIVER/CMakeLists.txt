# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

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
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(BLDC-M-driver-with-RP2354A C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add include directory for project headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add executable with all source files
add_executable(BLDC-M-driver-with-RP2354A 
    src/main.cpp
    src/encoder_driver.c
    src/current_sense.c
    src/motor_control.c
    src/drv8323_driver.c
    src/foc_controller.c
    src/usb_hid_device.c
    src/usb_descriptors.c
)

pico_set_program_name(BLDC-M-driver-with-RP2354A "BLDC-M-driver-with-RP2354A")
pico_set_program_version(BLDC-M-driver-with-RP2354A "0.1")

# Enable stdio over USB (for debugging and USB HID communication)
pico_enable_stdio_uart(BLDC-M-driver-with-RP2354A 0)
pico_enable_stdio_usb(BLDC-M-driver-with-RP2354A 1)

# Link required libraries
target_link_libraries(BLDC-M-driver-with-RP2354A
    pico_stdlib
    pico_multicore
    hardware_pwm
    hardware_spi
    hardware_adc
    hardware_gpio
    hardware_irq
    hardware_timer
    tinyusb_device
)

# Add the standard include files to the build
target_include_directories(BLDC-M-driver-with-RP2354A PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(BLDC-M-driver-with-RP2354A 
        hardware_spi
        )

pico_add_extra_outputs(BLDC-M-driver-with-RP2354A)

