if (TARGET tinyusb_device)
    add_executable(ephys-tester
            main.c
            tiny-json.c
            )

    # pull in common dependencies
    target_link_libraries(ephys-tester pico_stdlib hardware_spi hardware_gpio)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(ephys-tester 1)
    pico_enable_stdio_uart(ephys-tester 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(ephys-tester)

    # add url via pico_set_program_url
    example_auto_set_url(ephys-tester)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building ephys-tester because TinyUSB submodule is not initialized in the SDK")
endif()
