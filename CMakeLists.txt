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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

set(TINYUSB_MCU "OPT_MCU_RP2040" CACHE STRING "TinyUSB MCU name")
add_definitions(-DMBEDTLS_CONFIG_FILE="${CMAKE_SOURCE_DIR}/include/mbedtls-config.h")
add_definitions(-DCFG_TUSB_CONFIG_FILE="${CMAKE_CURRENT_LIST_DIR}/include/tusb_config.h")
add_definitions(-DCFG_TUSB_MCU=${TINYUSB_MCU})

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(canokey-pico2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(canokey-pico2 canokey-pico2.c lfs_init.c nfc_dummy.c device.c)

pico_set_program_name(canokey-pico2 "canokey-pico2")
pico_set_program_version(canokey-pico2 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(canokey-pico2 0)
pico_enable_stdio_usb(canokey-pico2 0)

add_subdirectory(canokey-core)

# Add the standard library to the build
target_link_libraries(canokey-pico2
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(canokey-pico2 PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(canokey-pico2 
        canokey-core
        hardware_timer
        hardware_clocks
        hardware_sync
        pico_unique_id
        pico_rand
        pico_sync
        pico_aon_timer
        tinyusb_device
        tinyusb_board
        )

pico_add_extra_outputs(canokey-pico2)

