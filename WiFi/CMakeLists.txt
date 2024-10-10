# Add the executable
add_executable(wifi
    wifi.c
)

# Link with libraries for standard functionality
target_link_libraries(wifi
    pico_stdlib              # Core standard library
)

# Create map/bin/hex files, etc.
pico_add_extra_outputs(wifi)

# Enable USB for input/output
pico_enable_stdio_usb(wifi 1)
