#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

#define EchoPin 0
#define TrigPin 1
#define EncoderPin 2
#define NumofSamples 10
const int timeout = 50000; // Increased timeout for pulse detection
volatile static absolute_time_t rise_time;
volatile static absolute_time_t fall_time;
volatile static uint64_t pulse_width_us = 0;

void setupPins() {
    // Initialize ADC
    adc_init();
    adc_set_temp_sensor_enabled(true); // Enable temperature sensor
    adc_select_input(4); // Select ADC input 4 (temperature sensor)

    // Initialize TrigPin and EchoPin for ultrasonic sensor
    gpio_init(TrigPin);
    gpio_init(EchoPin);
    gpio_set_dir(TrigPin, GPIO_OUT);
    gpio_set_dir(EchoPin, GPIO_IN);
    gpio_put(TrigPin, 0); // Ensure TrigPin is low initially

    // Initialize EncoderPin for encoder interrupts
    gpio_init(EncoderPin);
    gpio_set_dir(EncoderPin, GPIO_IN);
}

void __not_in_flash_func(adc_capture)(float *buf) {
    adc_select_input(4); // Ensure selecting input 4 for temperature sensor

    // Configure ADC FIFO
    adc_fifo_setup(
        true,  // FIFO enabled
        false, // DMA disabled
        1,     // Threshold (minimum number of samples before a read request)
        false, // Shift unused bits to LSB
        false  // Disable continuous read
    );

    adc_run(true); // Start ADC conversions

    float conversion_factor = 3.3f / (1 << 12); // Conversion factor for 12-bit ADC

    // Capture NumofSamples samples using FIFO
    for (size_t i = 0; i < NumofSamples; i++) {
        uint16_t adc_raw = adc_fifo_get_blocking(); // Blocking read from FIFO
        buf[i] = adc_raw * conversion_factor;       // Store voltage value
}

    adc_run(false);   // Stop ADC conversions
    adc_fifo_drain(); // Clear the FIFO
}

float movingAvgofSpeed(float value, float *buffer, int *index, float *sum) {
    *sum -= buffer[*index];       // Subtract the oldest value
    buffer[*index] = value;       // Store the new value
    *sum += value;                // Add the new value to the sum
    *index = (*index + 1) % NumofSamples; // Move to the next index
    return *sum / NumofSamples;   // Return the average
}

bool has_Zeros(float *buf) {
    for (size_t i = 0; i < NumofSamples; i++) {
        if (buf[i] == 0.0f) {
            return true;
        }
    }
    return false;
}

float computeTotalVolt(float *buf) {
    float totalVolt = 0.0f;
    for (size_t i = 0; i < NumofSamples; i++) {
        totalVolt += buf[i];
    }
    return totalVolt;
}

float getSoundOfSpeed() {
    static float sample_buf[NumofSamples] = {0.0f};
    static int index = 0;
    static float sum = 0.0f;
    float total_voltage = 0.0f;
    float conversion_factor = 3.3f / (1 << 12); // Correct conversion factor for 12-bit ADC

    // If buffer contains zeros, perform ADC capture
    if (has_Zeros(sample_buf)) {
        adc_capture(sample_buf);
        sum = computeTotalVolt(sample_buf);
    }

    // Read a single ADC value and update moving average
    uint16_t raw_value = adc_read();
    float value = raw_value * conversion_factor;

    total_voltage = movingAvgofSpeed(value, sample_buf, &index, &sum);
 
    // Calculate temperature based on voltage
    float temperature = 27 - ((total_voltage - 0.706) / 0.001721);

    // Calculate speed of sound based on temperature
    float soundSpeed = 331 + (0.61 * temperature);

    return soundSpeed;
}

uint64_t getPulse() {
    // Trigger ultrasonic pulse
    gpio_put(TrigPin, 1);
    sleep_us(10); // Send a 10us pulse to trigger
    gpio_put(TrigPin, 0);

    // Wait for the EchoPin to go HIGH (pulse start)
    absolute_time_t startTime = get_absolute_time();
    while (gpio_get(EchoPin) == 0) {
        if (absolute_time_diff_us(startTime, get_absolute_time()) > timeout) {
            printf("EchoPin HIGH timeout\n"); // Debugging statement
            return 0; // Timeout, no pulse detected
        }
    }

    // Measure the pulse duration while EchoPin is HIGH (pulse end)
    absolute_time_t pulseStart = get_absolute_time();
    while (gpio_get(EchoPin) == 1) {
        if (absolute_time_diff_us(pulseStart, get_absolute_time()) > timeout) {
            printf("EchoPin LOW timeout\n"); // Debugging statement
            return 0; // Timeout, pulse too long
        }
    }
    absolute_time_t pulseEnd = get_absolute_time();

    uint64_t pulse_duration = absolute_time_diff_us(pulseStart, pulseEnd);
    printf("Pulse duration: %llu us\n", pulse_duration); // Debugging statement
    return pulse_duration; // Pulse width in microseconds
}

uint64_t getCm() {
    uint64_t pulseLength = getPulse();
    float soundSpeed = getSoundOfSpeed();

    if (pulseLength == 0 || soundSpeed < 331) {
        printf("Invalid pulse length or sound speed\n"); // Debugging statement
        return 0; // No valid pulse detected or temperature too low
    }

    // Calculate distance in centimeters
    // Distance = (Time * Speed of Sound) / 2
    // Convert pulseLength from microseconds to seconds
    float distance = ((float)pulseLength / 1e6) * soundSpeed / 2.0f * 100.0f; // Convert to cm
    return (uint64_t)distance;
}

void IRQcallback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE) {
        rise_time = get_absolute_time();
    }

    if (events & GPIO_IRQ_EDGE_FALL) {
        fall_time = get_absolute_time();
        pulse_width_us = absolute_time_diff_us(rise_time, fall_time);
        printf("Pulse width: %llu us\n", pulse_width_us); // Debugging statement
    }
}

void setupIRQInterrupt() {
    gpio_set_irq_enabled_with_callback(
        EncoderPin, 
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
        true, 
        &IRQcallback
    );
}

int main() {
    stdio_init_all();
    setupPins();
    setupIRQInterrupt();

    printf("System initialized. Starting measurements...\n"); // Debugging statement

    while (1) {
        uint64_t cm = getCm();
        printf("Distance: %llu cm\n", cm);
        sleep_ms(1000); // Delay 1 second between measurements
    }

    return 0;
}
