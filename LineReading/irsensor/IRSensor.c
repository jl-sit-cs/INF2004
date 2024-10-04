#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define IR_SENSOR_PIN 26  //Need change based on IR sensor

void setup() {
    stdio_init_all();
    
    // Initialize ADC
    adc_init();
    adc_gpio_init(IR_SENSOR_PIN); // Initialize the GPIO for ADC
    adc_select_input(0); // ADC channel 0 (pin 26)
}

void loop() {
    // Read analog value from the IR sensor
    uint16_t adc_value = adc_read();

    //Based on this test, black surface have > 2000, while white has < 1000
    //printf("%d\n", adc_value);
    
    if (adc_value < 1000) {  
        printf("White Surface Detected\n");
    } else {
        printf("Black Surface Detected\n");
    }

    sleep_ms(1000);  
}

int main() {
    setup();
    while (1) {
        loop();
    }
    return 0;
}
