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

project(oled-test C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1


add_executable(oled-test main.cpp Adafruit_SSD1306.cpp Adafruit_GFX.cpp translation.cpp Print.cpp)

pico_set_program_name(oled-test "oled-test")
pico_set_program_version(oled-test "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(oled-test 0)
pico_enable_stdio_usb(oled-test 0)

# Add the standard library to the build
target_link_libraries(oled-test pico_stdlib)

# Add the standard include files to the build
target_include_directories(oled-test PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(oled-test hardware_spi)

pico_add_extra_outputs(oled-test)

