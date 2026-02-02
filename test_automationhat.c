
#include <stdio.h>
#include <unistd.h>

#include "automationhat.h"

/* ============================================================================
 * MAIN - TEST/DEMO
 * ============================================================================ */

int main(void) {

    printf("Automation HAT Mini - C Driver Test\n");
    printf("====================================\n\n");

    if (automationhat_init() < 0) {
        fprintf(stderr, "Failed to initialize Automation HAT\n");
        return 1;
    }

    printf("Initialized. Press Ctrl+C to exit.\n\n");

    int cycle = 0;

    while (1) {

        printf("ANALOG:  ");
        for (int ch = 0; ch < 3; ch++)
            printf("CH%d=%.2fV  ", ch + 1, analog_read(ch));
        printf("\n");

        printf("INPUT:   ");
        for (int ch = 0; ch < 3; ch++)
            printf("CH%d=%s   ", ch + 1, input_is_on(ch) ? "ON " : "OFF");
        printf("\n");

        const int active_output = cycle % 6; /* 0-2 = on, 3-5 = off cycle */
        if (active_output < 3)
            output_on(active_output);
        else
            output_off(active_output - 3);

        printf("OUTPUT:  ");
        for (int ch = 0; ch < 3; ch++)
            printf("CH%d=%s   ", ch + 1, output_is_on(ch) ? "ON " : "OFF");
        printf("\n");

        /* Toggle relay every 2 cycles */
        if (cycle % 2 == 0)
            relay_toggle();

        printf("RELAY:   %s\n", relay_is_on() ? "ON" : "OFF");

        printf("---\n");

        cycle++;
        sleep(1);
    }

    automationhat_close();

    return 0;
}
