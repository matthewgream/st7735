
#include "fonts.h"
#include "st7735.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    bool use_buffer = true;

    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--unbuffered") == 0 || strcmp(argv[i], "-u") == 0)
            use_buffer = false;

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
    st7735_text(disp, 5, 5, COLOR_WHITE, COLOR_BLACK, 1, "Built-in 5x7 font");
    st7735_text(disp, 5, 20, COLOR_GREEN, COLOR_BLACK, 1, "ABCDEFGHIJKLMNOP");
    st7735_text(disp, 5, 35, COLOR_RED, COLOR_BLACK, 1, "abcdefghijklmnop");
    st7735_text(disp, 5, 50, COLOR_YELLOW, COLOR_BLACK, 1, "0123456789!@#$%");
    st7735_text(disp, 5, 65, COLOR_CYAN, COLOR_BLACK, 1, use_buffer ? "BUFFERED" : "DIRECT");
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

#ifdef ST7735_EXTERNAL_FONTS

    /* Test 9: External font - 6x10 small */
    printf("[9] External font - Noto Mono 6x10\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_text_font(disp, 5, 5, COLOR_WHITE, COLOR_BLACK, &font_noto_mono6x10, true, 1, "External 6x10 font");
    st7735_text_font(disp, 5, 20, COLOR_GREEN, COLOR_BLACK, &font_noto_mono6x10, true, 1, "ABCDEFGHIJKLMNOPQRSTUVWX");
    st7735_text_font(disp, 5, 35, COLOR_RED, COLOR_BLACK, &font_noto_mono6x10, true, 1, "abcdefghijklmnopqrstuvwx");
    st7735_text_font(disp, 5, 50, COLOR_YELLOW, COLOR_BLACK, &font_noto_mono6x10, true, 1, "0123456789 !@#$%^&*()");
    st7735_text_font(disp, 5, 65, COLOR_CYAN, COLOR_BLACK, &font_noto_mono6x10, true, 1, "Compact text rendering");
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

    /* Test 10: External font - 23x37 large */
    printf("[10] External font - Noto Mono 23x37\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_text_font(disp, 5, 5, COLOR_WHITE, COLOR_BLACK, &font_noto_mono23x37, true, 1, "23.5");
    st7735_text_font(disp, 5, 45, COLOR_GREEN, COLOR_BLACK, &font_noto_mono23x37, true, 1, "1013");
    /* Add units with small font */
    st7735_text_font(disp, 105, 15, COLOR_WHITE, COLOR_BLACK, &font_noto_mono6x10, true, 0, "C");
    st7735_text_font(disp, 105, 55, COLOR_GREEN, COLOR_BLACK, &font_noto_mono6x10, true, 0, "mb");
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

    /* Test 11: Mixed fonts - status display */
    printf("[11] Mixed fonts - status display\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_rect(disp, 0, 0, 160, 80, COLOR_WHITE);
    st7735_line(disp, 0, 18, 160, 18, COLOR_WHITE);
    /* Title with small font */
    st7735_text_font(disp, 50, 4, COLOR_CYAN, COLOR_BLACK, &font_noto_mono6x10, true, 1, "WEATHER");
    /* Large temperature */
    st7735_text_font(disp, 5, 25, COLOR_WHITE, COLOR_BLACK, &font_noto_mono23x37, true, 1, "23.5");
    st7735_text_font(disp, 100, 30, COLOR_WHITE, COLOR_BLACK, &font_noto_mono6x10, true, 0, "C");
    /* Humidity with small font */
    st7735_text_font(disp, 5, 65, COLOR_GREEN, COLOR_BLACK, &font_noto_mono6x10, true, 1, "Humidity: 45%");
    /* Status indicator */
    st7735_fill_circle(disp, 145, 55, 8, COLOR_GREEN);
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

    /* Test 12: Variable width font test */
    printf("[12] Variable width font - Noto Vari 20x28\n");
    st7735_fill(disp, COLOR_BLACK);
    st7735_text_font(disp, 5, 5, COLOR_WHITE, COLOR_BLACK, &font_noto_vari20x28, false, 1, "Variable");
    st7735_text_font(disp, 5, 40, COLOR_YELLOW, COLOR_BLACK, &font_noto_vari20x28, false, 1, "Width");
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

#endif

    /* Test 13: Dirty rectangle - bouncing circle */
    printf("[13] Dirty rect - bouncing circle\n");
    st7735_fill(disp, COLOR_BLACK);
    if (use_buffer)
        st7735_flush(disp);
    int ball_x = 40, ball_y = 40;
    int ball_dx = 3, ball_dy = 2;
    int ball_r = 8;
    for (int frame = 0; frame < 100; frame++) {
        /* Erase old position */
        st7735_fill_circle(disp, ball_x, ball_y, ball_r, COLOR_BLACK);
        /* Move */
        ball_x += ball_dx;
        ball_y += ball_dy;
        /* Bounce off walls */
        if (ball_x <= ball_r || ball_x >= 160 - ball_r)
            ball_dx = -ball_dx;
        if (ball_y <= ball_r || ball_y >= 80 - ball_r)
            ball_dy = -ball_dy;
        /* Draw new position */
        st7735_fill_circle(disp, ball_x, ball_y, ball_r, COLOR_YELLOW);
        if (use_buffer)
            st7735_flush(disp);
        usleep(30000); /* ~30fps */
    }
    sleep(1);

#ifdef ST7735_EXTERNAL_FONTS

    /* Test 14: Font spacing and backgrounds */
    printf("[14] Font spacing and background colors\n");
    st7735_fill(disp, COLOR_BLACK);
    /* Spacing 0 - tight */
    st7735_text_font(disp, 5, 5, COLOR_WHITE, COLOR_BLACK, &font_noto_mono6x10, true, 0, "Spacing=0");
    /* Spacing 1 - normal */
    st7735_text_font(disp, 5, 18, COLOR_WHITE, COLOR_BLACK, &font_noto_mono6x10, true, 1, "Spacing=1");
    /* Spacing 3 - wide */
    st7735_text_font(disp, 5, 31, COLOR_WHITE, COLOR_BLACK, &font_noto_mono6x10, true, 3, "Spacing=3");
    /* Different backgrounds */
    st7735_text_font(disp, 5, 48, COLOR_BLACK, COLOR_RED, &font_noto_mono6x10, true, 1, "Red BG");
    st7735_text_font(disp, 60, 48, COLOR_BLACK, COLOR_GREEN, &font_noto_mono6x10, true, 1, "Green BG");
    st7735_text_font(disp, 120, 48, COLOR_BLACK, COLOR_CYAN, &font_noto_mono6x10, true, 1, "Cyan");
    /* Inverted text styles */
    st7735_text_font(disp, 5, 64, COLOR_YELLOW, COLOR_BLUE, &font_noto_mono6x10, true, 1, "Yellow/Blue");
    st7735_text_font(disp, 90, 64, COLOR_MAGENTA, COLOR_WHITE, &font_noto_mono6x10, true, 1, "Mag/Wht");
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);

#endif

    /* Test 15: Performance stress test */
    printf("[15] Performance stress test\n");
    int frames;
    double elapsed;
    /* Test A: Full screen fills */
    printf("    A) Full screen fills: ");
    fflush(stdout);
    start = clock();
    for (frames = 0; frames < 50; frames++) {
        st7735_fill(disp, frames & 1 ? COLOR_RED : COLOR_BLUE);
        if (use_buffer)
            st7735_flush(disp);
    }
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("%.1f FPS\n", frames / elapsed);
    /* Test B: Small rect updates (simulates UI updates) */
    printf("    B) Small rect (40x20): ");
    fflush(stdout);
    st7735_fill(disp, COLOR_BLACK);
    if (use_buffer)
        st7735_flush(disp);
    start = clock();
    for (frames = 0; frames < 200; frames++) {
        st7735_fill_rect(disp, 60, 30, 40, 20, (uint16_t)RGB565(frames * 5, frames * 3, frames * 7));
        if (use_buffer)
            st7735_flush(disp);
    }
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("%.1f FPS\n", frames / elapsed);
    /* Test C: Text updates (typical status display) */
    printf("    C) Text line update: ");
    fflush(stdout);
    st7735_fill(disp, COLOR_BLACK);
    if (use_buffer)
        st7735_flush(disp);
    char fps_buf[32];
    start = clock();
    for (frames = 0; frames < 200; frames++) {
        st7735_fill_rect(disp, 0, 36, 160, 12, COLOR_BLACK);
        snprintf(fps_buf, sizeof(fps_buf), "Frame: %d", frames);
        st7735_text(disp, 5, 36, COLOR_GREEN, COLOR_BLACK, 1, fps_buf);
        if (use_buffer)
            st7735_flush(disp);
    }
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("%.1f FPS\n", frames / elapsed);
    /* Test D: Multiple moving sprites */
    printf("    D) 5 moving circles: ");
    fflush(stdout);
    int sx[5] = { 20, 60, 100, 40, 120 };
    int sy[5] = { 20, 50, 30, 60, 45 };
    int sdx[5] = { 2, -3, 2, -2, 3 };
    int sdy[5] = { 3, 2, -2, 3, -2 };
    uint16_t sc[5] = { COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN };
    start = clock();
    for (frames = 0; frames < 100; frames++) {
        /* Erase all */
        for (int i = 0; i < 5; i++)
            st7735_fill_circle(disp, sx[i], sy[i], 6, COLOR_BLACK);
        /* Move all */
        for (int i = 0; i < 5; i++) {
            sx[i] += sdx[i];
            sy[i] += sdy[i];
            if (sx[i] <= 6 || sx[i] >= 154)
                sdx[i] = -sdx[i];
            if (sy[i] <= 6 || sy[i] >= 74)
                sdy[i] = -sdy[i];
        }
        /* Draw all */
        for (int i = 0; i < 5; i++)
            st7735_fill_circle(disp, sx[i], sy[i], 6, sc[i]);
        if (use_buffer)
            st7735_flush(disp);
    }
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("%.1f FPS\n", frames / elapsed);
    sleep(1);

#ifdef ST7735_IMAGE_SUPPORT
    /* Test 16: Image/icon rendering */
    printf("[16] Image rendering (BMP icons)\n");
    st7735_fill(disp, COLOR_BLACK);
    /* 16x16 green checkmark */
    static const char icon_check_b64[] =
        "Qk02AwAAAAAAADYAAAAoAAAAEAAAABAAAAABABgAAAAAAAADAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAMgAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAMgAAAAAAMgAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AMgAAMgAAMgAAAAAAAAAAAAAAMgAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM"
        "gAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAMgAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMgAAMgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    /* 16x16 red X */
    static const char icon_x_b64[] =
        "Qk02AwAAAAAAADYAAAAoAAAAEAAAABAAAAABABgAAAAAAAADAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAMjLcMjLcMjLcMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcMjLcMjLcMj"
        "LcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAMjLcMjLcMjLc"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    /* 16x16 yellow star */
    static const char icon_star_b64[] =
        "Qk02AwAAAAAAADYAAAAoAAAAEAAAABAAAAABABgAAAAAAAADAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMj/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMj/AAAAAAAAAMj/AMj/"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AAAAAAAAAAAAAMj/AMj/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AAAAAAAAAAAAAAAAAMj/AMj/AMj/AAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AMj/AAAAAAAAAAAAAAAAAAAAAMj/AMj/AMj/AAAAAAAAAAAAAAAAAMj/AMj/"
        "AMj/AAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AAAAAAAAAAAAAAAAAAAAAMj/AMj/AMj/"
        "AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AAAAAAAAAAAAAMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AAAAAMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AMj/AAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AMj/AMj/"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AMj/AMj/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMj/AMj/AAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    /* 16x16 red heart */
    static const char icon_heart_b64[] =
        "Qk02AwAAAAAAADYAAAAoAAAAEAAAABAAAAABABgAAAAAAAADAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMj"
        "LcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcMjLcMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAA"
        "MjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcAAAAAAAAAAAAAAAAMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcAAAAAAAAAAAAMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcAAAAAAAAMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMj"
        "LcMjLcMjLcMjLcAAAAMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcAAAAMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcAAAAMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcMjLcAAAAAAAAMjLcMjLcMjLcMjLc"
        "MjLcAAAAAAAAMjLcMjLcMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAMjLcMjLcMjLcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    /* Draw icons in a grid */
    st7735_text(disp, 5, 5, COLOR_WHITE, COLOR_BLACK, 1, "Icons:");
    st7735_image(disp, 10, 20, icon_check_b64, ST7735_IMAGE_FORMAT_BMP, ST7735_IMAGE_ENCODING_BASE64);
    st7735_image(disp, 35, 20, icon_x_b64, ST7735_IMAGE_FORMAT_BMP, ST7735_IMAGE_ENCODING_BASE64);
    st7735_image(disp, 60, 20, icon_star_b64, ST7735_IMAGE_FORMAT_BMP, ST7735_IMAGE_ENCODING_BASE64);
    st7735_image(disp, 85, 20, icon_heart_b64, ST7735_IMAGE_FORMAT_BMP, ST7735_IMAGE_ENCODING_BASE64);
    st7735_image(disp, 10, 45, icon_heart_b64, ST7735_IMAGE_FORMAT_BMP, ST7735_IMAGE_ENCODING_BASE64);
    st7735_image(disp, 35, 45, icon_star_b64, ST7735_IMAGE_FORMAT_BMP, ST7735_IMAGE_ENCODING_BASE64);
    st7735_image(disp, 60, 45, icon_x_b64, ST7735_IMAGE_FORMAT_BMP, ST7735_IMAGE_ENCODING_BASE64);
    st7735_image(disp, 85, 45, icon_check_b64, ST7735_IMAGE_FORMAT_BMP, ST7735_IMAGE_ENCODING_BASE64);
    /* Labels */
    st7735_text(disp, 110, 25, COLOR_GREEN, COLOR_BLACK, 1, "OK");
    st7735_text(disp, 110, 40, COLOR_RED, COLOR_BLACK, 1, "ERR");
    st7735_text(disp, 110, 55, COLOR_YELLOW, COLOR_BLACK, 1, "FAV");
    if (use_buffer)
        st7735_flush(disp);
    sleep(3);
#endif

    printf("\n=== Test Complete ===\n");
    st7735_close(disp);
    return 0;
}
