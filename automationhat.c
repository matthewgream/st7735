
/*
 * hat.c - Automation HAT Mini driver for Raspberry Pi
 *   - 3x Analog inputs (via ADS1015 I2C ADC)
 *   - 3x Digital inputs (GPIO)
 *   - 3x Digital outputs (GPIO)
 *   - 1x Relay (GPIO)
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "automationhat.h"
#include "hardware.h"

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

#define INPUT_1 26
#define INPUT_2 20
#define INPUT_3 21

#define OUTPUT_1 5
#define OUTPUT_2 12
#define OUTPUT_3 6

#define RELAY_1 16

#define ADS1015_ADDR     0x48
#define ADS1015_REG_CONV 0x00
#define ADS1015_REG_CONF 0x01

#define ADS1015_OS_SINGLE   0x8000 /* Start single conversion */
#define ADS1015_MUX_AIN0    0x4000 /* AIN0 vs GND */
#define ADS1015_MUX_AIN1    0x5000 /* AIN1 vs GND */
#define ADS1015_MUX_AIN2    0x6000 /* AIN2 vs GND */
#define ADS1015_MUX_AIN3    0x7000 /* AIN3 vs GND */
#define ADS1015_PGA_4V      0x0200 /* +/- 4.096V range */
#define ADS1015_MODE_SINGLE 0x0100 /* Single-shot mode */
#define ADS1015_DR_1600     0x0080 /* 1600 samples/sec */
#define ADS1015_COMP_DIS    0x0003 /* Disable comparator */
#define ADS1015_CONF_BASE   (ADS1015_OS_SINGLE | ADS1015_PGA_4V | ADS1015_MODE_SINGLE | ADS1015_DR_1600 | ADS1015_COMP_DIS)

/* Voltage scaling: ADC reads 0-3.3V, inputs are 0-25.85V */
#define ANALOG_MAX_VOLTAGE 25.85f
#define ADC_REF_VOLTAGE    3.3f
#define ADC_PGA_VOLTAGE    4.096f

/* ============================================================================
 * ADS1015
 * ============================================================================ */

static float ads1015_read_channel(int channel) {
    if (channel < 0 || channel > 3)
        return -1.0f;
    static uint16_t mux_conf_ain[4] = { ADS1015_MUX_AIN0, ADS1015_MUX_AIN1, ADS1015_MUX_AIN2, ADS1015_MUX_AIN3 };
    if (!i2c_write_reg16(ADS1015_REG_CONF, ADS1015_CONF_BASE | mux_conf_ain[channel]))
        return -1.0f;
    /* Wait for conversion (1600 SPS = ~0.625ms per sample) */
    usleep(1000);
    /* ADS1015 is 12-bit, left-aligned in 16-bit register */
    uint16_t raw;
    if (!i2c_read_reg16(ADS1015_REG_CONV, &raw))
        return -1.0f;
    /* Convert to voltage (with PGA = 4.096V) */
    return (float)(raw >> 4) * ADC_PGA_VOLTAGE / 2048.0f;
}

/* ============================================================================
 * PUBLIC API - SETUP/CLEANUP
 * ============================================================================ */

int automationhat_init(void) {

    if (gpio_open() < 0)
        return -1;
    if (i2c_open(ADS1015_ADDR) < 0) {
        gpio_close();
        return -1;
    }

    gpio_set_input(INPUT_1);
    gpio_set_input(INPUT_2);
    gpio_set_input(INPUT_3);

    gpio_set_output(OUTPUT_1);
    gpio_set_output(OUTPUT_2);
    gpio_set_output(OUTPUT_3);

    gpio_set_output(RELAY_1);

    gpio_write(OUTPUT_1, false);
    gpio_write(OUTPUT_2, false);
    gpio_write(OUTPUT_3, false);

    gpio_write(RELAY_1, false);

    return 0;
}

void automationhat_close(void) {

    gpio_write(OUTPUT_1, false);
    gpio_write(OUTPUT_2, false);
    gpio_write(OUTPUT_3, false);

    gpio_write(RELAY_1, false);

    i2c_close();
    gpio_close();
}

/* ============================================================================
 * PUBLIC API - ANALOG INPUTS
 * ============================================================================ */

float analog_read(int channel) {
    if (channel < 0 || channel > 2)
        return -1.0f;
    /* Scale from ADC voltage (0-3.3V) to input voltage (0-25.85V) */
    return (ads1015_read_channel(channel) / ADC_REF_VOLTAGE) * ANALOG_MAX_VOLTAGE;
}

/* ============================================================================
 * PUBLIC API - DIGITAL INPUTS
 * ============================================================================ */

static const int input_pins[] = { INPUT_1, INPUT_2, INPUT_3 };
bool input_read(int channel) {
    if (channel < 0 || channel > 2)
        return false;
    return gpio_read(input_pins[channel]);
}
bool input_is_on(int channel) {
    return input_read(channel);
}
bool input_is_off(int channel) {
    return !input_read(channel);
}

/* ============================================================================
 * PUBLIC API - DIGITAL OUTPUTS
 * ============================================================================ */

static const int output_pins[] = { OUTPUT_1, OUTPUT_2, OUTPUT_3 };
void output_write(int channel, bool value) {
    if (channel < 0 || channel > 2)
        return;
    gpio_write(output_pins[channel], value);
}
void output_on(int channel) {
    output_write(channel, true);
}
void output_off(int channel) {
    output_write(channel, false);
}
void output_toggle(int channel) {
    if (channel < 0 || channel > 2)
        return;
    gpio_write(output_pins[channel], !gpio_read(output_pins[channel]));
}
bool output_read(int channel) {
    if (channel < 0 || channel > 2)
        return false;
    return gpio_read(output_pins[channel]);
}
bool output_is_on(int channel) {
    return output_read(channel);
}
bool output_is_off(int channel) {
    return !output_read(channel);
}

/* ============================================================================
 * PUBLIC API - RELAY
 * ============================================================================ */

void relay_write(bool value) {
    gpio_write(RELAY_1, value);
}
void relay_on(void) {
    relay_write(true);
}
void relay_off(void) {
    relay_write(false);
}
void relay_toggle(void) {
    gpio_write(RELAY_1, !gpio_read(RELAY_1));
}
bool relay_read(void) {
    return gpio_read(RELAY_1);
}
bool relay_is_on(void) {
    return relay_read();
}
bool relay_is_off(void) {
    return !relay_read();
}
