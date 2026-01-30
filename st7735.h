#ifndef ST7735_H
#define ST7735_H

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

#include "fonts.h"

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_YELLOW 0xFFE0
#define COLOR_CYAN 0x07FF
#define COLOR_MAGENTA 0xF81F

#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

typedef struct st7735 st7735_t;

#define ST7735_ROTATION_0 0
#define ST7735_ROTATION_90 90
#define ST7735_ROTATION_180 180
#define ST7735_ROTATION_270 270

// ------------------------------------------------------------------------------------------------------------------------

/* Initialize display. Returns NULL on failure. */
st7735_t *st7735_init(int dc_pin, int bl_pin, uint32_t spi_speed, int rotation);

/* Close and free resources */
void st7735_close(st7735_t *disp);

/* Backlight control */
void st7735_backlight(st7735_t *disp, bool on);

/* Framebuffer control */
void st7735_set_buffered(st7735_t *disp, bool enabled);
bool st7735_is_buffered(st7735_t *disp);
void st7735_flush(st7735_t *disp);

/* Fill entire screen with color */
void st7735_fill(st7735_t *disp, uint16_t color);

/* Fill rectangle */
void st7735_fill_rect(st7735_t *disp, int x, int y, int w, int h, uint16_t color);

/* Get dimensions */
int st7735_width(st7735_t *disp);
int st7735_height(st7735_t *disp);

/* Draw single pixel */
void st7735_pixel(st7735_t *disp, int x, int y, uint16_t color);

/* Draw line */
void st7735_line(st7735_t *disp, int x0, int y0, int x1, int y1, uint16_t color);

/* Draw rectangle outline */
void st7735_rect(st7735_t *disp, int x, int y, int w, int h, uint16_t color);

/* Draw circle outline */
void st7735_circle(st7735_t *disp, int x, int y, int r, uint16_t color);

/* Draw filled circle */
void st7735_fill_circle(st7735_t *disp, int x, int y, int r, uint16_t color);

/* Draw character (built-in 5x7 font) */
void st7735_char(st7735_t *disp, int x, int y, char c, uint16_t color, uint16_t bg);

/* Draw string (built-in 5x7 font) */
void st7735_text(st7735_t *disp, int x, int y, const char *str, uint16_t color, uint16_t bg);

/* Draw character (external font via fontinfo_t) - returns width drawn */
int st7735_char_font(st7735_t *disp, int x, int y, char c, const fontinfo_t *font, uint16_t fg, uint16_t bg, bool mono);

/* Draw string (external font via fontinfo_t) - returns total width drawn */
int st7735_text_font(st7735_t *disp, int x, int y, const char *str, const fontinfo_t *font, uint16_t fg, uint16_t bg, bool mono, int spacing);

/* Get character width for external font */
int st7735_char_width(const fontinfo_t *font, char c, bool mono);

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#endif /* ST7735_H */
