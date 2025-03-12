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
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(ephys-tester C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(ephys-tester)

pico_generate_pio_header(ephys-tester  ${CMAKE_CURRENT_LIST_DIR}/spi.pio)
pico_generate_pio_header(ephys-tester  ${CMAKE_CURRENT_LIST_DIR}/quadrature.pio)
pico_set_program_name(ephys-tester  "ephys-tester")
pico_set_program_version(ephys-tester  "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(ephys-tester  0)
pico_enable_stdio_usb(ephys-tester mware 0)

# Add the standard library to the build
target_link_libraries(ephys-tester
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(ephys-tester  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(ephys-tester PRIVATE
        main.cpp
        mode.c
        oled.cpp
        channels.c
        sr.c
        tiny-json.c
        ad5683.c
        quadrature.c
        Adafruit_SH110X.cpp
        Adafruit_SH1106G.cpp
        Adafruit_GrayOLED_PicoSPI.cpp
        Adafruit_GrayOLED_PicoSPI.cpp
        Adafruit_GFX.cpp
        Print.cpp
        )

# Add any user requested libraries
target_link_libraries(ephys-tester
        pico_stdlib
        pico_multicore
        pico_bootsel_via_double_reset
        hardware_spi
        hardware_i2c
        hardware_pio
        hardware_dma
        hardware_irq
        )


pico_add_extra_outputs(ephys-tester)

