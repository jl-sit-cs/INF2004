#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

#define IR_SENSOR_PIN 26  // ADC pin connected to IR sensor's analog output

volatile uint64_t start_time, end_time;
volatile bool measuring = false;

void setup() {
    stdio_init_all();
    
    // Initialize ADC
    adc_init();
    adc_gpio_init(IR_SENSOR_PIN);  // Initialize the GPIO for ADC
    adc_select_input(0);           // ADC channel 0 (pin 26)
}

void loop() {
    uint16_t adc_value = adc_read();
    
    if (adc_value < 1000) {
        //Assuming the high of the PWM will be the black surface
        if (measuring) {
            end_time = time_us_64();    
            uint64_t pulse_width = end_time - start_time; //This is the time that the pulse is high
            //Not sure if that is what they want
            printf("Pulse Width (HIGH): %.6f second\n", pulse_width / 1000000.0);
            measuring = false;
        }
        printf("White Surface Detected (Analog)\n");

        
    } else if (adc_value > 2000) {
        //Start timer for high pulse?W
           if (!measuring) {
            start_time = time_us_64();  
            measuring = true;
        }

        printf("Black Surface Detected (Analog)\n");

    }
    //The detection is every one second
    sleep_ms(1000);  
}

int main() {
    setup();
    while (1) {
        loop();
    }
    return 0;
}
