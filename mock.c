
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "fonts.h"
#include "st7735.h"

/* ============================================================================
 * UI FRAMEWORK
 * ============================================================================ */

#define UI_STATUS_BG COLOR_BLACK
#define UI_STATUS_FG RGB565(196 - 32, 196 - 32, 196 - 32) /* Gray */

#define UI_ABBREV_IDLE    RGB565(128, 128, 128) /* Gray */
#define UI_ABBREV_FLOW    RGB565(0, 200, 0)     /* Green */
#define UI_ABBREV_MEASURE RGB565(50, 100, 255)  /* Blue */
#define UI_ABBREV_ERROR   RGB565(255, 0, 0)     /* Red */
#define UI_ABBREV_FG      COLOR_BLACK

#define UI_MAIN_BG COLOR_BLACK

typedef struct {
    const fontinfo_t *font_status;
    const fontinfo_t *font_abbrev;
    const fontinfo_t *font_text;
    const char **abbreviations;
    int abbrev_padding;
} ui_config_t;

static struct {
    st7735_t *disp;

    const fontinfo_t *font_status;
    const fontinfo_t *font_abbrev;
    const fontinfo_t *font_text;

    int status_x;
    int status_y;
    int status_h;

    int main_y;
    int main_h;

    int abbrev_x;
    int abbrev_y;
    int abbrev_w;
    int abbrev_h;

    int line_x;
    int line1_y;
    int line2_y;
    int line_x_max;

} ui;

static int calc_text_width(const fontinfo_t *font, const char *text) {
    int width = 0;
    for (const char *p = text; *p; p++)
        width += font_char_width(font, *p, false);
    return width;
}

int ui_setup(const ui_config_t *config) {

    /* Initialize display - 270° rotation = 160x80 landscape */
    ui.disp = st7735_init(9, 25, 270);
    if (!ui.disp) {
        fprintf(stderr, "Failed to init display\n");
        return -1;
    }

    /* Enable framebuffer mode */
    st7735_set_buffered(ui.disp, true);

    /* Copy font config */
    ui.font_status = config->font_status;
    ui.font_abbrev = config->font_abbrev;
    ui.font_text = config->font_text;

    const int disp_w = st7735_width(ui.disp);
    const int disp_h = st7735_height(ui.disp);

    /* Status bar at bottom */
    ui.status_h = ui.font_status->height + 4; /* font + 2px padding top/bottom */
    ui.status_y = disp_h - ui.status_h;
    ui.status_x = 4;

    /* Main area above status bar */
    ui.main_y = 0;
    ui.main_h = ui.status_y;

    /* Calculate abbreviation box width from max abbreviation */
    int max_abbrev_w = 0;
    for (const char **abbr = config->abbreviations; *abbr != NULL; abbr++) {
        const int w = calc_text_width(ui.font_abbrev, *abbr);
        if (w > max_abbrev_w)
            max_abbrev_w = w;
    }
    ui.abbrev_w = max_abbrev_w + (config->abbrev_padding * 2);
    ui.abbrev_h = ui.font_abbrev->height + (config->abbrev_padding * 2);
    ui.abbrev_x = 5;
    ui.abbrev_y = (ui.main_h - ui.abbrev_h) / 2; /* Centered vertically */

    /* Text lines - to the right of abbreviation box */
    ui.line_x = ui.abbrev_x + ui.abbrev_w + 8;
    ui.line_x_max = disp_w - ui.line_x - 2;

    /* Vertically center two text lines in main area */
    const int text_total_h = (ui.font_text->height * 2) + 2; /* 2px gap */
    const int text_start_y = (ui.main_h - text_total_h) / 2;
    ui.line1_y = text_start_y;
    ui.line2_y = text_start_y + ui.font_text->height + 2;

    /* Clear display */
    st7735_fill(ui.disp, UI_MAIN_BG);

    ui.status_x = ui.abbrev_x;
    ui.status_y -= 5;

    st7735_flush(ui.disp);

    printf("UI initialized: %dx%d\n", disp_w, disp_h);
    printf("  Status bar: y=%d, h=%d\n", ui.status_y, ui.status_h);
    printf("  Main area: y=%d, h=%d\n", ui.main_y, ui.main_h);
    printf("  Abbrev box: x=%d, y=%d, w=%d, h=%d (max_text_w=%d)\n", ui.abbrev_x, ui.abbrev_y, ui.abbrev_w, ui.abbrev_h, max_abbrev_w);
    printf("  Text lines: x=%d, y1=%d, y2=%d\n", ui.line_x, ui.line1_y, ui.line2_y);

    return 0;
}

void ui_cleanup(void) {
    if (ui.disp) {
        st7735_close(ui.disp);
        ui.disp = NULL;
    }
}

void ui_flush(void) {
    st7735_flush(ui.disp);
}

void ui_set_status(const char *fmt, ...) {
    char text[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);
    st7735_fill_rect(ui.disp, 0, ui.status_y, st7735_width(ui.disp), ui.status_h, UI_STATUS_BG);
    const int text_y = ui.status_y + 2;
    st7735_text_font(ui.disp, ui.status_x, text_y, UI_STATUS_FG, UI_STATUS_BG, ui.font_status, false, 1, text);
}
void ui_set_abbrev(uint16_t color_bg, const char *fmt, ...) {
    char text[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);
    st7735_fill_rect(ui.disp, ui.abbrev_x, ui.abbrev_y, ui.abbrev_w, ui.abbrev_h, color_bg);
    const int line_x = ui.abbrev_x + (ui.abbrev_w - calc_text_width(ui.font_abbrev, text)) / 2;
    const int text_y = ui.abbrev_y + (ui.abbrev_h - ui.font_abbrev->height) / 2;
    st7735_text_font(ui.disp, line_x, text_y, UI_ABBREV_FG, color_bg, ui.font_abbrev, false, 0, text);
}
void ui_set_line(int num, uint16_t color, const char *fmt, ...) {
    char text[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);
    st7735_fill_rect(ui.disp, ui.line_x, num == 1 ? ui.line1_y : ui.line2_y, ui.line_x_max, ui.font_text->height, UI_MAIN_BG);
    st7735_text_font(ui.disp, ui.line_x, num == 1 ? ui.line1_y : ui.line2_y, color, UI_MAIN_BG, ui.font_text, false, 1, text);
}

/* ============================================================================
 * MOCK DRIVER - Simulates water quality testing sequence
 * ============================================================================ */

typedef const char *(*render_fn)(float *values, char *buf, size_t buflen);

static char render_buf[48];
static const char *render_ph(float *v, char *buf, size_t len) {
    snprintf(buf, len, "%.2f pH", v[0]);
    return buf;
}
static const char *render_orp(float *v, char *buf, size_t len) {
    snprintf(buf, len, "%.0f mV", v[0]);
    return buf;
}
static const char *render_do(float *v, char *buf, size_t len) {
    snprintf(buf, len, "%.1f mg/L", v[0]);
    return buf;
}
static const char *render_ec(float *v, char *buf, size_t len) {
    snprintf(buf, len, "%.0f uS/cm", v[0]);
    return buf;
}
static const char *render_temp(float *v, char *buf, size_t len) {
    snprintf(buf, len, "%.1f C", v[0]);
    return buf;
}
static const char *render_turbidity(float *v, char *buf, size_t len) {
    snprintf(buf, len, "%.1f NTU", v[0]);
    return buf;
}
static const char *render_rgb(float *v, char *buf, size_t len) {
    snprintf(buf, len, "%02X/%02X/%02X", (int)v[0] & 0xFF, (int)v[1] & 0xFF, (int)v[2] & 0xFF);
    return buf;
}

typedef struct {
    const char *abbrev;
    const char *name;
    int num_samples;
    float values[3];
    float stddev;
    render_fn render;
} measurement_t;

static measurement_t measurements[] = {
    { "t",   "Temperature",  3, { 23.5f },            0.1f,  render_temp      },
    { "ph",  "pH Level",     5, { 7.2f },             0.05f, render_ph        },
    { "orp", "ORP",          5, { 245.0f },           3.2f,  render_orp       },
    { "do",  "Dissolved O2", 5, { 8.1f },             0.15f, render_do        },
    { "ec",  "Conductivity", 5, { 520.0f },           8.0f,  render_ec        },
    { "tur", "Turbidity",    5, { 3.2f },             0.4f,  render_turbidity },
    { "rgb", "Color",        5, { 0x8A, 0xC4, 0x7E }, 5.0f,  render_rgb       },
};
#define NUM_MEASUREMENTS (sizeof(measurements) / sizeof(measurements[0]))

void mock_measure(measurement_t *m, int m_current, int m_number) {
    int sample_time = 1; /* seconds per sample */
    printf("  measuring: %s\n", m->name);
    ui_set_abbrev(UI_ABBREV_MEASURE, m->abbrev);
    for (int s = 1; s <= m->num_samples; s++) {
        ui_set_line(1, COLOR_WHITE, "sampling %d/%d", s, m->num_samples);
        for (int t = sample_time; t > 0; t--) {
            ui_set_status("measuring %d/%d ... %ds", m_current, m_number, (m->num_samples - s) * sample_time + t);
            float simulated[3];
            for (int i = 0; i < 3; i++)
                simulated[i] = m->values[i] + (((float)(rand() % 100) / 100.0f - 0.5f) * m->stddev * 2);
            ui_set_line(2, COLOR_GREEN, "%s", m->render(simulated, render_buf, sizeof(render_buf)));
            ui_flush();
            sleep(1);
        }
    }
}

void mock_idle(const char *time_ago) {
    printf("[IDLE]\n");
    ui_set_abbrev(UI_ABBREV_IDLE, "Zz");
    ui_set_status("idle: X=run Y=view");
    ui_set_line(1, COLOR_WHITE, "last measured");
    ui_set_line(2, COLOR_YELLOW, "%s ago", time_ago);
    ui_flush();
    sleep(3);
}

void mock_water(const char *type, const char *abbrev, const char *status, int level_start, int level_end) {
    printf("[%s]\n", type);
    ui_set_abbrev(UI_ABBREV_FLOW, abbrev);
    ui_set_status(status);
    for (int level = level_start; level != level_end + (level_end > level_start ? 5 : -5); level += (level_end > level_start ? 5 : -5)) {
        const float temp = 22.0f + (float)(rand() % 20) / 10.0f;
        ui_set_line(1, COLOR_CYAN, "level %d mm", level);
        ui_set_line(2, COLOR_GREEN, "temp %.1f C", temp);
        ui_flush();
        usleep(500000); /* 0.5 sec per step */
    }
    sleep(1);
}

void mock_error(void) {
    printf("[ERROR]\n");
    ui_set_abbrev(UI_ABBREV_ERROR, "<!>");
    ui_set_status("error: sensor fault");
    ui_set_line(1, COLOR_RED, "check");
    ui_set_line(2, COLOR_RED, "sensors");
    ui_flush();
    sleep(3);
}

void mock_run(void) {

    printf("\n=== Starting Mock Sequence ===\n\n");

    mock_idle("3d14h20m");

    mock_water("FILLING", ">>>", "filling from inlet ...", 0, 50);

    printf("[MEASURING]\n");
    for (int i = 0; i < (int)NUM_MEASUREMENTS; i++)
        mock_measure(&measurements[i], i + 1, NUM_MEASUREMENTS);

    mock_water("DRAINING", "<<<", "draining to outlet ...", 50, 0);

    printf("[COMPLETE]\n");
    ui_set_abbrev(UI_ABBREV_IDLE, "-");
    ui_set_status("complete: tests done");
    ui_set_line(1, COLOR_GREEN, "passes %d/%d", (int)NUM_MEASUREMENTS, (int)NUM_MEASUREMENTS);
    ui_set_line(2, COLOR_YELLOW, "errors 0/%d", (int)NUM_MEASUREMENTS);
    ui_flush();
    sleep(3);

    printf("[RESULTS]\n");
    ui_set_status("view: X=next Y=exit");
    for (size_t i = 0; i < NUM_MEASUREMENTS; i++) {
        ui_set_abbrev(UI_ABBREV_MEASURE, measurements[i].abbrev);
        ui_set_line(1, COLOR_WHITE, "%s", measurements[i].render(measurements[i].values, render_buf, sizeof(render_buf)));
        ui_set_line(2, COLOR_YELLOW, "+/-%.2f", measurements[i].stddev);
        ui_flush();
        sleep(5);
    }

    mock_error();

    mock_idle("5m");

    printf("\n=== Mock Sequence Complete ===\n");
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

static const char *ui_abbreviations[] = { "Zz", ">>>", "<<<", "<!>", "ph", "orp", "do", "ec", "t", "tur", "rgb", NULL };

int main(void) {
    srand((unsigned int)time(NULL));

    ui_config_t config = {
        .font_status = &font_noto_mono6x10,
        .font_abbrev = &font_noto_vari20x28,
        .font_text = &font_noto_mono6x10,
        .abbreviations = ui_abbreviations,
        .abbrev_padding = 4,
    };

    if (ui_setup(&config) < 0)
        return 1;

    mock_run();

    printf("\nPress Ctrl+C to exit...\n");
    while (1)
        sleep(1);

    ui_cleanup();
    return 0;
}
