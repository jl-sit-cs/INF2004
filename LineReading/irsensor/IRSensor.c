#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

#define LINE_SENSOR_PIN 26  // GPIO 26 connected to the line sensor's digital output
#define NUM_SAMPLES 10       // Number of samples for averaging
#define THRESHOLD 1500         // Threshold for ADC readings

volatile uint64_t black_start_time, black_end_time, white_start_time, white_end_time;
volatile bool measuring_black = false;
volatile bool measuring_white = false;
volatile bool last_color_black = false; // Track the last detected color
uint16_t moving_avg_values[NUM_SAMPLES] = {0}; // Store moving average values
uint8_t moving_avg_index = 0; // Index for moving average values
uint32_t moving_avg_total = 0; // Total for moving average

void setup() {
    stdio_init_all();
    gpio_init(LINE_SENSOR_PIN);
    gpio_set_dir(LINE_SENSOR_PIN, GPIO_IN); // Set the pin as input

    // Initialize ADC for reading analog values
    adc_init();
    adc_gpio_init(27); // Assuming the analog output is connected to GPIO 27
    adc_select_input(0); // Select ADC input 0 (GP27)
}

uint16_t moving_average(uint16_t new_value) {
    // Check if color has changed
    bool is_black = (new_value > THRESHOLD); // Black line detected

    // If color changes, reset the moving average
    if (is_black != last_color_black) {
        moving_avg_total = 0; // Reset total
        for (int i = 0; i < NUM_SAMPLES; i++) {
            moving_avg_values[i] = 0; // Reset all values
        }
        moving_avg_index = 0; // Reset index
        last_color_black = is_black; // Update last color
    }

    // Subtract the oldest value and add the new one
    moving_avg_total -= moving_avg_values[moving_avg_index];
    moving_avg_total += new_value;

    // Store the new value
    moving_avg_values[moving_avg_index] = new_value;
    moving_avg_index = (moving_avg_index + 1) % NUM_SAMPLES; // Circular index

    return moving_avg_total / NUM_SAMPLES; // Return the average
}

void loop() {
    // Read the analog value from the sensor
    uint16_t analog_value = adc_read(); // Read ADC value (0-4095)
    uint16_t filtered_value = moving_average(analog_value); //Used to filter the ADC using moving average

    //Used for testing
    //printf("Filtered Analog Value: %d\n", filtered_value); // Print the filtered value
    //printf("Analog Value: %d\n", analog_value); // Print the Analog value


    if (filtered_value > THRESHOLD) { // ADC > 400  (black line detected)
        if (!measuring_black) {
            black_start_time = time_us_64(); // Start the timer for black line
            measuring_black = true; // Indicate that measuring has started for black line
            printf("Black Line Detected\n");
        }

        // If we are measuring a white surface, calculate the pulse width
        if (measuring_white) {
            white_end_time = time_us_64(); // Stop the timer for white surface
            uint64_t pulse_width_white = white_end_time - white_start_time; // Calculate pulse width for white surface
            printf("Pulse Width (White Surface): %.6f seconds\n", pulse_width_white / 1000000.0);
            measuring_white = false; // Indicate that measuring has stopped for white surface
        }

    }  
    else // ADC < 400> state (White line detected) 
    {
         if (measuring_black) {
            black_end_time = time_us_64(); // Stop the timer for black line
            uint64_t pulse_width_black = black_end_time - black_start_time; // Calculate pulse width for black line
            printf("Pulse Width (Black Line): %.6f seconds\n", pulse_width_black / 1000000.0);
            measuring_black = false; // Indicate that measuring has stopped for black line
        }
        
        // Handle white line detection
        if (!measuring_white) {
            white_start_time = time_us_64(); // Start the timer for white surface
            measuring_white = true; // Indicate that measuring has started for white surface
            printf("White Line Detected\n");
        }
       
    }

    sleep_ms(20); 
}

int main() 
{
    setup();
    while (1) {
        loop();
    }
    return 0;
}

