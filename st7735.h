#ifndef ST7735_H
#define ST7735_H

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

#ifdef ST7735_EXTERNAL_FONTS
#include "fonts.h"
#endif

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F

#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

// ------------------------------------------------------------------------------------------------------------------------

typedef struct st7735 st7735_t;

#define ST7735_ROTATION_0   0
#define ST7735_ROTATION_90  90
#define ST7735_ROTATION_180 180
#define ST7735_ROTATION_270 270

// ------------------------------------------------------------------------------------------------------------------------

/* Initialize display. Returns NULL on failure. */
st7735_t *st7735_init(int dc_pin, int bl_pin, uint32_t spi_speed, int rotation);

/* Close and free resources */
void st7735_close(st7735_t *disp);

/* Get dimensions */
int st7735_width(const st7735_t *disp);
int st7735_height(const st7735_t *disp);

/* Backlight control */
void st7735_backlight(st7735_t *disp, bool on);

/* Framebuffer control */
void st7735_set_buffered(st7735_t *disp, bool enabled);
bool st7735_is_buffered(const st7735_t *disp);
void st7735_flush(st7735_t *disp);

/* Draw single pixel */
void st7735_pixel(st7735_t *disp, int x, int y, uint16_t color);

/* Fill entire screen with color */
void st7735_fill(st7735_t *disp, uint16_t color);

/* Draw line */
void st7735_line(st7735_t *disp, int x0, int y0, int x1, int y1, uint16_t color);

/* Draw rectangle outline */
void st7735_rect(st7735_t *disp, int x, int y, int w, int h, uint16_t color);

/* Fill rectangle */
void st7735_fill_rect(st7735_t *disp, int x, int y, int w, int h, uint16_t color);

/* Draw circle outline */
void st7735_circle(st7735_t *disp, int x, int y, int r, uint16_t color);

/* Draw filled circle */
void st7735_fill_circle(st7735_t *disp, int x, int y, int r, uint16_t color);

/* Draw character (built-in 5x7 font) - returns width drawn */
int st7735_char(st7735_t *disp, int x, int y, uint16_t fg, uint16_t bg, char c);

/* Draw string (built-in 5x7 font) - returns total width drawn */
int st7735_text(st7735_t *disp, int x, int y, uint16_t fg, uint16_t bg, int spacing, const char *str);

#ifdef ST7735_EXTERNAL_FONTS

/* Draw character (external font via fontinfo_t) - returns width drawn */
int st7735_char_font(st7735_t *disp, int x, int y, uint16_t fg, uint16_t bg, const fontinfo_t *font, bool mono, char c);

/* Draw string (external font via fontinfo_t) - returns total width drawn */
int st7735_text_font(st7735_t *disp, int x, int y, uint16_t fg, uint16_t bg, const fontinfo_t *font, bool mono, int spacing, const char *str);

#endif

/* Hardware scrolling (works best with rotation=0) */
void st7735_scroll_setup(st7735_t *disp, int top_fixed, int scroll_area, int bottom_fixed);
void st7735_scroll(st7735_t *disp, int line);

#ifdef ST7735_IMAGE_SUPPORT

/* Image format constants */
#define ST7735_IMAGE_FORMAT_BMP 0
/* Image encoding constants */
#define ST7735_IMAGE_ENCODING_RAW    0
#define ST7735_IMAGE_ENCODING_BASE64 1

/* Draw image - returns 0 on success, -1 on error */
int st7735_image(st7735_t *disp, int x, int y, const char *data, int format, int encoding);

#endif

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#endif /* ST7735_H */
