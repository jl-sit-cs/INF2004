# Add the executable
add_executable(Ultrasonic
    Ultrasonic.c
)

# Link with libraries for standard functionality
target_link_libraries(Ultrasonic
    pico_stdlib              # Core standard library
)

# Create map/bin/hex files, etc.
pico_add_extra_outputs(Ultrasonic)

# Enable USB for input/output
pico_enable_stdio_usb(Ultrasonic 1)
