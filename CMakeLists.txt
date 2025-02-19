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

# Enable USB support in SDK
set(PICO_SDK_USB_ENABLED 1)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Enable USB support in SDK
set(PICO_SDK_USB_ENABLED 1)

project(canokey-pico2 C CXX ASM)

# Initialize the Raspberry Pi Pico SDK first
pico_sdk_init()

# Add USB definitions after SDK init
add_definitions(
    -DPICO_SDK_USE_USB=1
    -DPICO_USB=1
    -DPICO_USB_ENABLE_HOST=0
    -DPICO_USB_ENABLE_DEVICE=1
)

set(TINYUSB_MCU "OPT_MCU_RP2040" CACHE STRING "TinyUSB MCU name")
add_definitions(-DMBEDTLS_CONFIG_FILE="${CMAKE_SOURCE_DIR}/include/mbedtls-config.h")
add_definitions(-DCFG_TUSB_CONFIG_FILE="${CMAKE_CURRENT_LIST_DIR}/include/tusb_config.h")
add_definitions(-DCFG_TUSB_MCU=${TINYUSB_MCU})
add_definitions(-DCFG_TUSB_OS=OPT_OS_PICO)

file(GLOB SRCS
    "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
)

# Add executable. Default name is the project name, version 0.1
add_executable(canokey-pico2 ${SRCS})

pico_set_program_name(canokey-pico2 "canokey-pico2")
pico_set_program_version(canokey-pico2 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(canokey-pico2 0)
pico_enable_stdio_usb(canokey-pico2 0)

add_dependencies(canokey-pico2 gitrev)
set(gitrev_in include/git-rev.h.in)
set(gitrev include/git-rev.h)
add_custom_target(gitrev
        ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev_in} ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev}
        COMMAND git describe --always --tags --long --abbrev=8 --dirty >> ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev}
        )

set(PICO_SDK_PATH "${PICO_SDK_PATH}" CACHE PATH "Path to Pico SDK")
add_subdirectory(canokey-core)

# Add the standard include files to the build
target_include_directories(canokey-pico2 PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/canokey-core/interfaces/USB/device
  ${CMAKE_CURRENT_LIST_DIR}/canokey-core/include
)

# Add any user requested libraries
target_link_libraries(canokey-pico2 
        pico_stdlib
        canokey-core
        hardware_flash
        hardware_timer
        hardware_clocks
        hardware_sync
        hardware_watchdog
        pico_unique_id
        pico_rand
        pico_sync
        pico_time
        pico_aon_timer
        )

pico_add_extra_outputs(canokey-pico2)
