#ifndef AUTOMATIONHAT_H
#define AUTOMATIONHAT_H

#include <stdbool.h>

/* ============================================================================
 * SETUP / CLEANUP
 * ============================================================================ */

/* Initialize the Automation HAT. Returns 0 on success, -1 on failure. */
int automationhat_init(void);
void automationhat_term(void);

/* ============================================================================
 * ANALOG INPUTS (3 channels: 0, 1, 2)
 * ============================================================================ */

/* Read analog voltage (0-25.85V range). Returns -1 on error. */
float automationhat_analog_read(int channel);

/* ============================================================================
 * DIGITAL INPUTS (3 channels: 0, 1, 2)
 * ============================================================================ */

bool automationhat_input_read(int channel);
inline bool automationhat_input_is_on(int channel) {
    return automationhat_input_read(channel);
}
inline bool automationhat_input_is_off(int channel) {
    return !automationhat_input_read(channel);
}

/* ============================================================================
 * DIGITAL OUTPUTS (3 channels: 0, 1, 2)
 * ============================================================================ */

void automationhat_output_write(int channel, bool value);
inline void automationhat_output_on(int channel) {
    automationhat_output_write(channel, true);
}
inline void automationhat_output_off(int channel) {
    automationhat_output_write(channel, false);
}
void automationhat_output_toggle(int channel);
bool automationhat_output_read(int channel);
inline bool automationhat_output_is_on(int channel) {
    return automationhat_output_read(channel);
}
inline bool automationhat_output_is_off(int channel) {
    return !automationhat_output_read(channel);
}

/* ============================================================================
 * RELAY (1 channel)
 * ============================================================================ */

void automationhat_relay_write(bool value);
inline void automationhat_relay_on(void) {
    automationhat_relay_write(true);
}
inline void automationhat_relay_off(void) {
    automationhat_relay_write(false);
}
void automationhat_relay_toggle(void);
bool automationhat_relay_read(void);
inline bool automationhat_relay_is_on(void) {
    return automationhat_relay_read();
}
inline bool automationhat_relay_is_off(void) {
    return !automationhat_relay_read();
}

#endif /* AUTOMATIONHAT_H */
