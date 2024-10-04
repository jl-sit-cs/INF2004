add_executable(IRSensor
        IRSensor.c
        )

# pull in common dependencies
target_link_libraries(IRSensor pico_stdlib hardware_adc)

pico_enable_stdio_usb(IRSensor 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(IRSensor)

# add url via pico_set_program_url
example_auto_set_url(IRSensor)
