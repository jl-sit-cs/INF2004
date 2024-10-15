set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")

if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
    if (NOT TARGET pico_cyw43_arch)
        message("Skipping Pico W examples as support is not available")
    else()

        if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
            set(WIFI_SSID $ENV{WIFI_SSID})
            message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
        endif()

        if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
            set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
            message("Using WIFI_PASSWORD from environment")
        endif()

        set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
        set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")
    endif()
endif()

if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Skipping Pico W FreeRTOS examples as FREERTOS_KERNEL_PATH not defined")
else()
    include(FreeRTOS_Kernel_import.cmake)
endif()

# Add the executable
add_executable(wifi
    wifi.c
    ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c
)

target_compile_definitions(wifi PRIVATE
WIFI_SSID=\"${WIFI_SSID}\"
WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
PING_USE_SOCKETS=1
)
target_include_directories(wifi PRIVATE
${CMAKE_CURRENT_LIST_DIR}
${PICO_LWIP_CONTRIB_PATH}/apps/ping
)

target_link_libraries(wifi
pico_stdlib
FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
hardware_adc
pico_cyw43_arch_lwip_sys_freertos
pico_lwip_iperf
)

# Create map/bin/hex files, etc.
pico_add_extra_outputs(wifi)

# Enable USB for input/output
pico_enable_stdio_usb(wifi 1)
