#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define LINE_SENSOR_PIN 26  // GPIO 26 connected to the line sensor's digital output

volatile uint64_t start_time, end_time;
volatile bool measuring = false;

void setup() {
    stdio_init_all();
    gpio_init(LINE_SENSOR_PIN);
    gpio_set_dir(LINE_SENSOR_PIN, GPIO_IN); // Set the pin as input
}

void loop() {
    bool sensor_state = gpio_get(LINE_SENSOR_PIN); // Read digital state of the sensor

    if (sensor_state) { // HIGH state (black line detected)
        if (!measuring) {
            start_time = time_us_64(); // Start the timer
            measuring = true; // Indicate that measuring has started
        }
        printf("Black Line Detected (Digital)\n");
        
    } else { // LOW state (white line detected)
        if (measuring) {
            end_time = time_us_64(); // Stop the timer
            uint64_t pulse_width = end_time - start_time; // Calculate pulse width (assuming it is the time)
            printf("Pulse Width (HIGH): %.6f seconds\n", pulse_width / 1000000.0);
            measuring = false; // Indicate that measuring has stopped
        }
        printf("White Line Detected (Digital)\n");
    }

    sleep_ms(100); 
}

int main() 
{
    setup();
    while (1) {
        loop();
    }
    return 0;
}

