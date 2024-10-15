#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

#define EchoPin 0
#define TrigPin 1
#define EncoderPin 2
#define NumofSamples 10
const int timeout = 26100; 
volatile static absolute_time_t rise_time;
volatile static absolute_time_t fall_time;
volatile static uint64_t pulse_width_us = 0;

uint16_t sample_buf[NumofSamples];

//Things to do filtering (to do validated stuff)
//measuring temperature to record the room stuff
//
void setupPins()
{
    adc_init();
    adc_select_input(4);
    adc_set_temperature_sensor_enabled(true);

    gpio_init(TrigPin);
    gpio_init(EchoPin);

    gpio_set_dir(TrigPin, GPIO_OUT);
    gpio_set_dir(EchoPin, GPIO_IN);
    gpio_put(TrigPin, 0);

    gpio_init(EncoderPin);
    gpio_set_dir(EncoderPin, GPIO_IN);
}

void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count) {
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);
    for (size_t i = 0; i < count; i++) {
        buf[i] = adc_fifo_get_blocking();  // Capture ADC sample
    }
    adc_run(false);
    adc_fifo_drain();
}

float getSoundOfSpeed(){
    float total_voltage = 0, volt = 0, adc_value = 0;
    float conversion_value = (3.3 / (1 << 12));

    adc_capture(sample_buf, NumofSamples);

    for (int i = 0 ; i < NumofSamples; i ++){
        volt = (conversion_value) * sample_buf[i];
        total_voltage += volt;
    }

    float temperature = 27 - ((total_voltage / NumofSamples) - 0.706) / 0.001721;
    
    float soundofSpeed = 331 + ( 0.61 * temperature );

    return soundofSpeed;
}

uint64_t getPulse()
{
    gpio_put(TrigPin, 1);
    sleep_us(10); // Send a 10us pulse to trigger
    gpio_put(TrigPin, 0);

    // Wait for the EchoPin to go HIGH (pulse start)
    absolute_time_t startTime = get_absolute_time();
    while (gpio_get(EchoPin) == 0) 
    {
        if (absolute_time_diff_us(startTime, get_absolute_time()) > timeout) 
        {
            return 0; // Timeout, no pulse detected
        }
    }

    // Measure the pulse duration while EchoPin is HIGH (pulse end)
    absolute_time_t pulseStart = get_absolute_time();
    while (gpio_get(EchoPin) == 1) 
    {
        if (absolute_time_diff_us(pulseStart, get_absolute_time()) > timeout) 
        {
            return 0; // Timeout, pulse too long
        }
    }
    absolute_time_t pulseEnd = get_absolute_time();
    
    return absolute_time_diff_us(pulseStart, pulseEnd); // Pulse width in microseconds
}

uint64_t getCm()
{
    uint64_t pulseLength = getPulse();
    float soundofSpeed = getSoundOfSpeed();

    if (pulseLength == 0 || soundofSpeed < 331 ) return 0; // No valid pulse detected or temperature
    return (pulseLength * soundofSpeed) / 29 / 2; // Convert microseconds to cm
}

void IRQcallback(uint gpio, uint32_t events){

    if(events & GPIO_IRQ_EDGE_RISE){
        rise_time = get_absolute_time();
    }

    if(events & GPIO_IRQ_EDGE_FALL){
        fall_time = get_absolute_time();
        pulse_width_us = absolute_time_diff_us(rise_time,fall_time);
        printf("Pulse width: %llu us\n", pulse_width_us);
    }
}

void setupIRQInterrupt(){
    gpio_set_irq_enabled_with_callback(EncoderPin, GPIO_IRQ_EDGE_RISE 
    | GPIO_IRQ_EDGE_FALL, true, &IRQcallback);
}

int main()
{
    stdio_init_all();
    setupPins();
    gpio_set_irq_enabled_with_callback(EncoderPin, GPIO_IRQ_EDGE_RISE 
    | GPIO_IRQ_EDGE_FALL, true, &IRQcallback);

    while (1)
    {
        uint64_t cm = getCm();
        
        printf("Distance: %llu cm\n", cm);
        sleep_ms(1000); // Delay 1 second between measurements

    }

    return 0;
}
