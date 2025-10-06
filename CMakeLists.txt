# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico_sega_led C CXX ASM)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(CMAKE_C_STANDARD 11)

pico_sdk_init()
include_directories(${CMAKE_CURRENT_LIST_DIR})
add_compile_options(-Wall -Wfatal-errors -fexceptions -O3)
link_libraries(pico_multicore pico_stdlib
        hardware_pio hardware_adc hardware_flash hardware_watchdog
        tinyusb_device tinyusb_board)


function(make_firmware board board_def)
    add_executable(${board}
        main.cpp usb_descriptors.cpp jvs.cpp led_strip.cpp)

    add_subdirectory(pico_led)

    target_link_libraries(${board} PicoLed)

    target_compile_definitions(${board} PUBLIC ${board_def})

    pico_add_extra_outputs(${board})

    add_custom_command(TARGET ${board} POST_BUILD
        COMMAND arm-none-eabi-size ${board}.elf)
endfunction()

make_firmware(pico_sega_led PICO_SEGA_LED)

