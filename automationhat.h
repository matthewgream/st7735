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
float analog_read(int channel);

/* ============================================================================
 * DIGITAL INPUTS (3 channels: 0, 1, 2)
 * ============================================================================ */

/* Read digital input state */
bool input_read(int channel);
bool input_is_on(int channel);
bool input_is_off(int channel);

/* ============================================================================
 * DIGITAL OUTPUTS (3 channels: 0, 1, 2)
 * ============================================================================ */

/* Write digital output */
void output_write(int channel, bool value);
void output_on(int channel);
void output_off(int channel);
void output_toggle(int channel);

/* Read current output state */
bool output_read(int channel);
bool output_is_on(int channel);
bool output_is_off(int channel);

/* ============================================================================
 * RELAY (1 channel)
 * ============================================================================ */

/* Control relay */
void relay_write(bool value);
void relay_on(void);
void relay_off(void);
void relay_toggle(void);

/* Read relay state */
bool relay_read(void);
bool relay_is_on(void);
bool relay_is_off(void);

#endif /* AUTOMATIONHAT_H */
