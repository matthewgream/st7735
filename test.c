
#include "fonts.h"
#include "st7735.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    bool use_buffer = false;

    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--buffered") == 0 || strcmp(argv[i], "-b") == 0)
            use_buffer = true;

    printf("=== ST7735 Library Test ===\n");
    printf("Mode: %s\n\n", use_buffer ? "BUFFERED" : "DIRECT");

    /* 270 degree rotation = landscape 160x80 */
    st7735_t *disp = st7735_init(9, 25, 16000000, 270);
    if (!disp) {
        fprintf(stderr, "Failed to init display\n");
        return 1;
    }
    printf("Display: %dx%d\n\n", st7735_width(disp), st7735_height(disp));

    if (use_buffer) {
        st7735_set_buffered(disp, true);
        printf("Buffering: %s\n\n", st7735_is_buffered(disp) ? "enabled" : "failed");
    }

    clock_t start, end;

    /* Test 1: Solid fill */
    printf("[1] st7735_fill - solid colors\n");
    start = clock();
    st7735_fill(disp, COLOR_RED);
    if (use_buffer)
        st7735_flush(disp);
    end = clock();
    printf("    Fill time: %.3f ms\n", (double)(end - start) * 1000 / CLOCKS_PER_SEC);
    sleep(1);
    st7735_fill(disp, COLOR_GREEN);
    if (use_buffer)
        st7735_flush(disp);
    sleep(1);
    st7735_fill(disp, COLOR_BLUE);
    if (use_buffer)
        st7735_flush(disp);
    sleep(1);

    /* Test 2: Filled rectangles */
    printf("[2] st7735_fill_rect - filled rectangles\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_fill_rect(disp, 5, 5, 30, 30, COLOR_RED);
    st7735_fill_rect(disp, 45, 5, 30, 30, COLOR_GREEN);
    st7735_fill_rect(disp, 85, 5, 30, 30, COLOR_BLUE);
    st7735_fill_rect(disp, 125, 5, 30, 30, COLOR_YELLOW);
    st7735_fill_rect(disp, 5, 45, 30, 30, COLOR_CYAN);
    st7735_fill_rect(disp, 45, 45, 30, 30, COLOR_MAGENTA);
    if (use_buffer)
        st7735_flush(disp);
    sleep(2);

    /* Test 3: Pixels */
    printf("[3] st7735_pixel - random pixels\n");
    st7735_fill(disp, COLOR_BLACK);
    start = clock();
    for (int i = 0; i < 500; i++) {
        int x = rand() % st7735_width(disp), y = rand() % st7735_height(disp);
        uint16_t c = (uint16_t)RGB565(rand() % 256, rand() % 256, rand() % 256);
        st7735_pixel(disp, x, y, c);
    }
    if (use_buffer)
        st7735_flush(disp);
    end = clock();
    printf("    500 pixels time: %.3f ms\n", (double)(end - start) * 1000 / CLOCKS_PER_SEC);
    sleep(2);

    /* Test 4: Lines */
    printf("[4] st7735_line - lines\n");
    st7735_fill(disp, COLOR_BLACK);
    start = clock();
    st7735_line(disp, 0, 0, 159, 79, COLOR_RED);
    st7735_line(disp, 159, 0, 0, 79, COLOR_GREEN);
    st7735_line(disp, 0, 40, 159, 40, COLOR_BLUE);
    st7735_line(disp, 80, 0, 80, 79, COLOR_YELLOW);
    int cx = 80, cy = 40;
    for (int angle = 0; angle < 360; angle += 30) {
        int x = cx + (int)(30 * cos(angle * 3.14159 / 180)), y = cy + (int)(30 * sin(angle * 3.14159 / 180));
        st7735_line(disp, cx, cy, x, y, COLOR_WHITE);
    }
    if (use_buffer)
        st7735_flush(disp);
    end = clock();
    printf("    Lines time: %.3f ms\n", (double)(end - start) * 1000 / CLOCKS_PER_SEC);
    sleep(2);

    /* Test 5: Rectangle outlines */
    printf("[5] st7735_rect - rectangle outlines\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_rect(disp, 5, 5, 150, 70, COLOR_WHITE);
    st7735_rect(disp, 20, 15, 50, 50, COLOR_RED);
    st7735_rect(disp, 90, 15, 50, 50, COLOR_GREEN);
    if (use_buffer)
        st7735_flush(disp);
    sleep(2);

    /* Test 6: Circle outlines */
    printf("[6] st7735_circle - circle outlines\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_circle(disp, 40, 40, 30, COLOR_RED);
    st7735_circle(disp, 120, 40, 30, COLOR_GREEN);
    st7735_circle(disp, 80, 40, 15, COLOR_BLUE);
    if (use_buffer)
        st7735_flush(disp);
    sleep(2);

    /* Test 7: Filled circles */
    printf("[7] st7735_fill_circle - filled circles\n");
    st7735_fill(disp, COLOR_BLACK);
    start = clock();
    st7735_fill_circle(disp, 30, 40, 25, COLOR_RED);
    st7735_fill_circle(disp, 80, 40, 25, COLOR_GREEN);
    st7735_fill_circle(disp, 130, 40, 25, COLOR_BLUE);
    if (use_buffer)
        st7735_flush(disp);
    end = clock();
    printf("    3 filled circles time: %.3f ms\n", (double)(end - start) * 1000 / CLOCKS_PER_SEC);
    sleep(2);

    /* Test 8: Built-in 5x7 text */
    printf("[8] st7735_text - built-in 5x7 font\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_text(disp, 5, 5, "Built-in 5x7 font", COLOR_WHITE, COLOR_BLACK);
    st7735_text(disp, 5, 20, "ABCDEFGHIJKLMNOP", COLOR_GREEN, COLOR_BLACK);
    st7735_text(disp, 5, 35, "abcdefghijklmnop", COLOR_RED, COLOR_BLACK);
    st7735_text(disp, 5, 50, "0123456789!@#$%", COLOR_YELLOW, COLOR_BLACK);
    st7735_text(disp, 5, 65, use_buffer ? "BUFFERED" : "DIRECT", COLOR_CYAN, COLOR_BLACK);
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

    /* Test 9: External font - 6x10 small */
    printf("[9] External font - Noto Mono 6x10\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_text_font(disp, 5, 5, "External 6x10 font", &font_noto_mono6x10, COLOR_WHITE, COLOR_BLACK, true, 1);
    st7735_text_font(disp, 5, 20, "ABCDEFGHIJKLMNOPQRSTUVWX", &font_noto_mono6x10, COLOR_GREEN, COLOR_BLACK, true, 1);
    st7735_text_font(disp, 5, 35, "abcdefghijklmnopqrstuvwx", &font_noto_mono6x10, COLOR_RED, COLOR_BLACK, true, 1);
    st7735_text_font(disp, 5, 50, "0123456789 !@#$%^&*()", &font_noto_mono6x10, COLOR_YELLOW, COLOR_BLACK, true, 1);
    st7735_text_font(disp, 5, 65, "Compact text rendering", &font_noto_mono6x10, COLOR_CYAN, COLOR_BLACK, true, 1);
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

    /* Test 10: External font - 23x37 large */
    printf("[10] External font - Noto Mono 23x37\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_text_font(disp, 5, 5, "23.5", &font_noto_mono23x37, COLOR_WHITE, COLOR_BLACK, true, 1);
    st7735_text_font(disp, 5, 45, "1013", &font_noto_mono23x37, COLOR_GREEN, COLOR_BLACK, true, 1);
    /* Add units with small font */
    st7735_text_font(disp, 105, 15, "C", &font_noto_mono6x10, COLOR_WHITE, COLOR_BLACK, true, 0);
    st7735_text_font(disp, 105, 55, "mb", &font_noto_mono6x10, COLOR_GREEN, COLOR_BLACK, true, 0);
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

    /* Test 11: Mixed fonts - status display */
    printf("[11] Mixed fonts - status display\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_rect(disp, 0, 0, 160, 80, COLOR_WHITE);
    st7735_line(disp, 0, 18, 160, 18, COLOR_WHITE);
    /* Title with small font */
    st7735_text_font(disp, 50, 4, "WEATHER", &font_noto_mono6x10, COLOR_CYAN, COLOR_BLACK, true, 1);
    /* Large temperature */
    st7735_text_font(disp, 5, 25, "23.5", &font_noto_mono23x37, COLOR_WHITE, COLOR_BLACK, true, 1);
    st7735_text_font(disp, 100, 30, "C", &font_noto_mono6x10, COLOR_WHITE, COLOR_BLACK, true, 0);
    /* Humidity with small font */
    st7735_text_font(disp, 5, 65, "Humidity: 45%", &font_noto_mono6x10, COLOR_GREEN, COLOR_BLACK, true, 1);
    /* Status indicator */
    st7735_fill_circle(disp, 145, 55, 8, COLOR_GREEN);
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

    /* Test 12: Variable width font test */
    printf("[12] Variable width font - Noto Vari 20x28\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_text_font(disp, 5, 5, "Variable", &font_noto_vari20x28, COLOR_WHITE, COLOR_BLACK, false, 1);
    st7735_text_font(disp, 5, 40, "Width", &font_noto_vari20x28, COLOR_YELLOW, COLOR_BLACK, false, 1);
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

    printf("\n=== Test Complete ===\n");
    st7735_close(disp);
    return 0;
}
