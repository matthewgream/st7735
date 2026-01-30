
// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#include "st7735.h"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT 0x11
#define ST7735_NORON 0x13
#define ST7735_INVON 0x21
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_MADCTL 0x36
#define ST7735_COLMOD 0x3A
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST7735_COLS 132
#define ST7735_ROWS 162
#define ST7735_WIDTH 80
#define ST7735_HEIGHT 160

#define GPIO_FSEL0 0
#define GPIO_SET0 7
#define GPIO_CLR0 10

struct st7735 {
    int spi_fd;
    volatile uint32_t *gpio;

    uint8_t dc_pin;
    uint8_t bl_pin;

    uint16_t width;
    uint16_t height;
    uint8_t offset_left;
    uint8_t offset_top;
    uint8_t madctl;

    uint16_t *buffer;
    bool buffered;
};

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

static void gpio_set_output(st7735_t *disp, int pin) {
    int reg = pin / 10, shift = (pin % 10) * 3;
    disp->gpio[reg] = (disp->gpio[reg] & (uint32_t)~(7 << shift)) | (1 << shift);
}
static void gpio_set(st7735_t *disp, int pin) { disp->gpio[GPIO_SET0] = 1 << pin; }
static void gpio_clr(st7735_t *disp, int pin) { disp->gpio[GPIO_CLR0] = 1 << pin; }

// ------------------------------------------------------------------------------------------------------------------------

static void spi_write(st7735_t *disp, const uint8_t *data, size_t len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)data,
        .len = (unsigned int)len,
    };
    ioctl(disp->spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

static void cmd(st7735_t *disp, uint8_t c) {
    gpio_clr(disp, disp->dc_pin);
    spi_write(disp, &c, 1);
}
static void dat(st7735_t *disp, uint8_t d) {
    gpio_set(disp, disp->dc_pin);
    spi_write(disp, &d, 1);
}
static void dat_buf(st7735_t *disp, const uint8_t *buf, size_t len) {
    gpio_set(disp, disp->dc_pin);
    const size_t CHUNK = 4096;
    while (len > 0) {
        size_t n = (len < CHUNK) ? len : CHUNK;
        spi_write(disp, buf, n);
        buf += n;
        len -= n;
    }
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

static void set_window(st7735_t *disp, int x0, int y0, int x1, int y1) {
    cmd(disp, ST7735_CASET);
    dat(disp, 0x00);
    dat(disp, (uint8_t)(x0 + disp->offset_left));
    dat(disp, 0x00);
    dat(disp, (uint8_t)(x1 + disp->offset_left));
    cmd(disp, ST7735_RASET);
    dat(disp, 0x00);
    dat(disp, (uint8_t)(y0 + disp->offset_top));
    dat(disp, 0x00);
    dat(disp, (uint8_t)(y1 + disp->offset_top));
    cmd(disp, ST7735_RAMWR);
}

// ------------------------------------------------------------------------------------------------------------------------

static void init_seq(st7735_t *disp) {
    cmd(disp, ST7735_SWRESET);
    usleep(150000);

    cmd(disp, ST7735_SLPOUT);
    usleep(500000);

    cmd(disp, ST7735_FRMCTR1);
    dat(disp, 0x01);
    dat(disp, 0x2C);
    dat(disp, 0x2D);

    cmd(disp, ST7735_FRMCTR2);
    dat(disp, 0x01);
    dat(disp, 0x2C);
    dat(disp, 0x2D);

    cmd(disp, ST7735_FRMCTR3);
    dat(disp, 0x01);
    dat(disp, 0x2C);
    dat(disp, 0x2D);
    dat(disp, 0x01);
    dat(disp, 0x2C);
    dat(disp, 0x2D);

    cmd(disp, ST7735_INVCTR);
    dat(disp, 0x07);

    cmd(disp, ST7735_PWCTR1);
    dat(disp, 0xA2);
    dat(disp, 0x02);
    dat(disp, 0x84);

    cmd(disp, ST7735_PWCTR2);
    dat(disp, 0x0A);
    dat(disp, 0x00);

    cmd(disp, ST7735_PWCTR4);
    dat(disp, 0x8A);
    dat(disp, 0x2A);

    cmd(disp, ST7735_PWCTR5);
    dat(disp, 0x8A);
    dat(disp, 0xEE);

    cmd(disp, ST7735_VMCTR1);
    dat(disp, 0x0E);

    cmd(disp, ST7735_INVON);

    cmd(disp, ST7735_MADCTL);
    dat(disp, disp->madctl);

    cmd(disp, ST7735_COLMOD);
    dat(disp, 0x05);

    cmd(disp, ST7735_CASET);
    dat(disp, 0x00);
    dat(disp, disp->offset_left);
    dat(disp, 0x00);
    dat(disp, (uint8_t)(disp->width + disp->offset_left - 1));

    cmd(disp, ST7735_RASET);
    dat(disp, 0x00);
    dat(disp, disp->offset_top);
    dat(disp, 0x00);
    dat(disp, (uint8_t)(disp->height + disp->offset_top - 1));

    cmd(disp, ST7735_GMCTRP1);
    uint8_t gp[] = {0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10};
    for (int i = 0; i < 16; i++)
        dat(disp, gp[i]);

    cmd(disp, ST7735_GMCTRN1);
    uint8_t gn[] = {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10};
    for (int i = 0; i < 16; i++)
        dat(disp, gn[i]);

    cmd(disp, ST7735_NORON);
    usleep(10000);

    cmd(disp, ST7735_DISPON);
    usleep(100000);
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

st7735_t *st7735_init(int dc_pin, int bl_pin, uint32_t spi_speed, int rotation) {
    st7735_t *disp = calloc(1, sizeof(st7735_t));
    if (!disp)
        return NULL;

    disp->dc_pin = (uint8_t)dc_pin;
    disp->bl_pin = (uint8_t)bl_pin;
    disp->buffered = false;
    disp->buffer = NULL;

    switch (rotation) {
    case 0:
        disp->width = ST7735_WIDTH;
        disp->height = ST7735_HEIGHT;
        disp->offset_left = (ST7735_COLS - ST7735_WIDTH) / 2;
        disp->offset_top = (ST7735_ROWS - ST7735_HEIGHT) / 2;
        disp->madctl = 0x00;
        break;
    case 90:
        disp->width = ST7735_HEIGHT;
        disp->height = ST7735_WIDTH;
        disp->offset_left = (ST7735_ROWS - ST7735_HEIGHT) / 2;
        disp->offset_top = (ST7735_COLS - ST7735_WIDTH) / 2;
        disp->madctl = 0x60;
        break;
    case 180:
        disp->width = ST7735_WIDTH;
        disp->height = ST7735_HEIGHT;
        disp->offset_left = (ST7735_COLS - ST7735_WIDTH) / 2;
        disp->offset_top = (ST7735_ROWS - ST7735_HEIGHT) / 2;
        disp->madctl = 0xC0;
        break;
    case 270:
    default:
        disp->width = ST7735_HEIGHT;
        disp->height = ST7735_WIDTH;
        disp->offset_left = (ST7735_ROWS - ST7735_HEIGHT) / 2;
        disp->offset_top = (ST7735_COLS - ST7735_WIDTH) / 2;
        disp->madctl = 0xA0;
        break;
    }
    disp->madctl |= 0x08;

    disp->spi_fd = open("/dev/spidev0.1", O_RDWR);
    if (disp->spi_fd < 0) {
        free(disp);
        return NULL;
    }

    uint8_t spi_mode = SPI_MODE_0, spi_bits = 8;
    ioctl(disp->spi_fd, SPI_IOC_WR_MODE, &spi_mode);
    ioctl(disp->spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits);
    ioctl(disp->spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);

    int mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        close(disp->spi_fd);
        free(disp);
        return NULL;
    }

    disp->gpio = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
    close(mem_fd);

    if (disp->gpio == MAP_FAILED) {
        close(disp->spi_fd);
        free(disp);
        return NULL;
    }

    gpio_set_output(disp, dc_pin);
    gpio_set_output(disp, bl_pin);
    gpio_set(disp, bl_pin); /* Backlight on */

    init_seq(disp);

    return disp;
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_close(st7735_t *disp) {
    if (!disp)
        return;

    gpio_clr(disp, disp->bl_pin); /* Backlight off */

    if (disp->buffer)
        free(disp->buffer);
    if (disp->gpio)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        munmap((void *)disp->gpio, 4096);
#pragma GCC diagnostic pop
    if (disp->spi_fd >= 0)
        close(disp->spi_fd);
    free(disp);
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

int st7735_width(st7735_t *disp) { return disp->width; }
int st7735_height(st7735_t *disp) { return disp->height; }

// ------------------------------------------------------------------------------------------------------------------------

void st7735_backlight(st7735_t *disp, bool on) {
    if (on)
        gpio_set(disp, disp->bl_pin);
    else
        gpio_clr(disp, disp->bl_pin);
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_set_buffered(st7735_t *disp, bool enabled) {
    if (enabled && !disp->buffer) {
        disp->buffer = malloc(disp->width * disp->height * sizeof(uint16_t));
        if (disp->buffer)
            memset(disp->buffer, 0, disp->width * disp->height * sizeof(uint16_t));
    }
    disp->buffered = enabled && (disp->buffer != NULL);
}

bool st7735_is_buffered(st7735_t *disp) { return disp->buffered; }

// ------------------------------------------------------------------------------------------------------------------------

void st7735_flush(st7735_t *disp) {
    if (!disp->buffer)
        return;
    set_window(disp, 0, 0, disp->width - 1, disp->height - 1);
    size_t pixels = disp->width * disp->height;
    uint8_t *buf = malloc(pixels * 2);
    if (!buf)
        return;
    for (size_t i = 0; i < pixels; i++) {
        buf[i * 2] = (uint8_t)(disp->buffer[i] >> 8);
        buf[i * 2 + 1] = (uint8_t)(disp->buffer[i] & 0xFF);
    }
    dat_buf(disp, buf, pixels * 2);
    free(buf);
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_pixel(st7735_t *disp, int x, int y, uint16_t color) {
    if (x < 0 || x >= disp->width || y < 0 || y >= disp->height)
        return;
    if (disp->buffered && disp->buffer) {
        disp->buffer[y * disp->width + x] = color;
    } else {
        set_window(disp, x, y, x, y);
        uint8_t buf[2] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};
        dat_buf(disp, buf, 2);
    }
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_fill(st7735_t *disp, uint16_t color) {
    if (disp->buffered && disp->buffer) {
        size_t pixels = disp->width * disp->height;
        for (size_t i = 0; i < pixels; i++)
            disp->buffer[i] = color;
    } else {
        set_window(disp, 0, 0, disp->width - 1, disp->height - 1);
        uint8_t hi = (uint8_t)(color >> 8), lo = (uint8_t)(color & 0xFF);
        size_t pixels = disp->width * disp->height;
        uint8_t *buf = malloc(pixels * 2);
        for (size_t i = 0; i < pixels; i++) {
            buf[i * 2] = hi;
            buf[i * 2 + 1] = lo;
        }
        dat_buf(disp, buf, pixels * 2);
        free(buf);
    }
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_line(st7735_t *disp, int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    while (1) {
        st7735_pixel(disp, x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_rect(st7735_t *disp, int x, int y, int w, int h, uint16_t color) {
    st7735_line(disp, x, y, x + w - 1, y, color);                 /* top */
    st7735_line(disp, x, y + h - 1, x + w - 1, y + h - 1, color); /* bottom */
    st7735_line(disp, x, y, x, y + h - 1, color);                 /* left */
    st7735_line(disp, x + w - 1, y, x + w - 1, y + h - 1, color); /* right */
}

void st7735_fill_rect(st7735_t *disp, int x, int y, int w, int h, uint16_t color) {
    if (disp->buffered && disp->buffer) {
        for (int py = y; py < y + h; py++)
            for (int px = x; px < x + w; px++)
                if (px >= 0 && px < disp->width && py >= 0 && py < disp->height)
                    disp->buffer[py * disp->width + px] = color;
    } else {
        set_window(disp, x, y, x + w - 1, y + h - 1);
        uint8_t hi = (uint8_t)(color >> 8), lo = (uint8_t)(color & 0xFF);
        size_t pixels = (size_t)(w * h);
        uint8_t *buf = malloc(pixels * 2);
        for (size_t i = 0; i < pixels; i++) {
            buf[i * 2] = hi;
            buf[i * 2 + 1] = lo;
        }
        dat_buf(disp, buf, pixels * 2);
        free(buf);
    }
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_circle(st7735_t *disp, int x0, int y0, int r, uint16_t color) {
    int x = r, y = 0;
    int err = 0;
    while (x >= y) {
        st7735_pixel(disp, x0 + x, y0 + y, color);
        st7735_pixel(disp, x0 + y, y0 + x, color);
        st7735_pixel(disp, x0 - y, y0 + x, color);
        st7735_pixel(disp, x0 - x, y0 + y, color);
        st7735_pixel(disp, x0 - x, y0 - y, color);
        st7735_pixel(disp, x0 - y, y0 - x, color);
        st7735_pixel(disp, x0 + y, y0 - x, color);
        st7735_pixel(disp, x0 + x, y0 - y, color);
        err += 1 + 2 * (++y);
        if (2 * (err - x) + 1 > 0)
            err += 1 - 2 * (--x);
    }
}

void st7735_fill_circle(st7735_t *disp, int x0, int y0, int r, uint16_t color) {
    int x = r, y = 0;
    int err = 0;
    while (x >= y) {
        st7735_line(disp, x0 - x, y0 + y, x0 + x, y0 + y, color);
        st7735_line(disp, x0 - y, y0 + x, x0 + y, y0 + x, color);
        st7735_line(disp, x0 - x, y0 - y, x0 + x, y0 - y, color);
        st7735_line(disp, x0 - y, y0 - x, x0 + y, y0 - x, color);
        err += 1 + 2 * (++y);
        if (2 * (err - x) + 1 > 0)
            err += 1 - 2 * (--x);
    }
}

// ------------------------------------------------------------------------------------------------------------------------

/* 5x7 font - ASCII 32-126 */
static const uint8_t font5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, /* 32 space */
    0x00, 0x00, 0x5F, 0x00, 0x00, /* 33 ! */
    0x00, 0x07, 0x00, 0x07, 0x00, /* 34 " */
    0x14, 0x7F, 0x14, 0x7F, 0x14, /* 35 # */
    0x24, 0x2A, 0x7F, 0x2A, 0x12, /* 36 $ */
    0x23, 0x13, 0x08, 0x64, 0x62, /* 37 % */
    0x36, 0x49, 0x55, 0x22, 0x50, /* 38 & */
    0x00, 0x05, 0x03, 0x00, 0x00, /* 39 ' */
    0x00, 0x1C, 0x22, 0x41, 0x00, /* 40 ( */
    0x00, 0x41, 0x22, 0x1C, 0x00, /* 41 ) */
    0x08, 0x2A, 0x1C, 0x2A, 0x08, /* 42 * */
    0x08, 0x08, 0x3E, 0x08, 0x08, /* 43 + */
    0x00, 0x50, 0x30, 0x00, 0x00, /* 44 , */
    0x08, 0x08, 0x08, 0x08, 0x08, /* 45 - */
    0x00, 0x60, 0x60, 0x00, 0x00, /* 46 . */
    0x20, 0x10, 0x08, 0x04, 0x02, /* 47 / */
    0x3E, 0x51, 0x49, 0x45, 0x3E, /* 48 0 */
    0x00, 0x42, 0x7F, 0x40, 0x00, /* 49 1 */
    0x42, 0x61, 0x51, 0x49, 0x46, /* 50 2 */
    0x21, 0x41, 0x45, 0x4B, 0x31, /* 51 3 */
    0x18, 0x14, 0x12, 0x7F, 0x10, /* 52 4 */
    0x27, 0x45, 0x45, 0x45, 0x39, /* 53 5 */
    0x3C, 0x4A, 0x49, 0x49, 0x30, /* 54 6 */
    0x01, 0x71, 0x09, 0x05, 0x03, /* 55 7 */
    0x36, 0x49, 0x49, 0x49, 0x36, /* 56 8 */
    0x06, 0x49, 0x49, 0x29, 0x1E, /* 57 9 */
    0x00, 0x36, 0x36, 0x00, 0x00, /* 58 : */
    0x00, 0x56, 0x36, 0x00, 0x00, /* 59 ; */
    0x00, 0x08, 0x14, 0x22, 0x41, /* 60 < */
    0x14, 0x14, 0x14, 0x14, 0x14, /* 61 = */
    0x41, 0x22, 0x14, 0x08, 0x00, /* 62 > */
    0x02, 0x01, 0x51, 0x09, 0x06, /* 63 ? */
    0x32, 0x49, 0x79, 0x41, 0x3E, /* 64 @ */
    0x7E, 0x11, 0x11, 0x11, 0x7E, /* 65 A */
    0x7F, 0x49, 0x49, 0x49, 0x36, /* 66 B */
    0x3E, 0x41, 0x41, 0x41, 0x22, /* 67 C */
    0x7F, 0x41, 0x41, 0x22, 0x1C, /* 68 D */
    0x7F, 0x49, 0x49, 0x49, 0x41, /* 69 E */
    0x7F, 0x09, 0x09, 0x01, 0x01, /* 70 F */
    0x3E, 0x41, 0x41, 0x51, 0x32, /* 71 G */
    0x7F, 0x08, 0x08, 0x08, 0x7F, /* 72 H */
    0x00, 0x41, 0x7F, 0x41, 0x00, /* 73 I */
    0x20, 0x40, 0x41, 0x3F, 0x01, /* 74 J */
    0x7F, 0x08, 0x14, 0x22, 0x41, /* 75 K */
    0x7F, 0x40, 0x40, 0x40, 0x40, /* 76 L */
    0x7F, 0x02, 0x04, 0x02, 0x7F, /* 77 M */
    0x7F, 0x04, 0x08, 0x10, 0x7F, /* 78 N */
    0x3E, 0x41, 0x41, 0x41, 0x3E, /* 79 O */
    0x7F, 0x09, 0x09, 0x09, 0x06, /* 80 P */
    0x3E, 0x41, 0x51, 0x21, 0x5E, /* 81 Q */
    0x7F, 0x09, 0x19, 0x29, 0x46, /* 82 R */
    0x46, 0x49, 0x49, 0x49, 0x31, /* 83 S */
    0x01, 0x01, 0x7F, 0x01, 0x01, /* 84 T */
    0x3F, 0x40, 0x40, 0x40, 0x3F, /* 85 U */
    0x1F, 0x20, 0x40, 0x20, 0x1F, /* 86 V */
    0x7F, 0x20, 0x18, 0x20, 0x7F, /* 87 W */
    0x63, 0x14, 0x08, 0x14, 0x63, /* 88 X */
    0x03, 0x04, 0x78, 0x04, 0x03, /* 89 Y */
    0x61, 0x51, 0x49, 0x45, 0x43, /* 90 Z */
    0x00, 0x00, 0x7F, 0x41, 0x41, /* 91 [ */
    0x02, 0x04, 0x08, 0x10, 0x20, /* 92 \ */
    0x41, 0x41, 0x7F, 0x00, 0x00, /* 93 ] */
    0x04, 0x02, 0x01, 0x02, 0x04, /* 94 ^ */
    0x40, 0x40, 0x40, 0x40, 0x40, /* 95 _ */
    0x00, 0x01, 0x02, 0x04, 0x00, /* 96 ` */
    0x20, 0x54, 0x54, 0x54, 0x78, /* 97 a */
    0x7F, 0x48, 0x44, 0x44, 0x38, /* 98 b */
    0x38, 0x44, 0x44, 0x44, 0x20, /* 99 c */
    0x38, 0x44, 0x44, 0x48, 0x7F, /* 100 d */
    0x38, 0x54, 0x54, 0x54, 0x18, /* 101 e */
    0x08, 0x7E, 0x09, 0x01, 0x02, /* 102 f */
    0x08, 0x14, 0x54, 0x54, 0x3C, /* 103 g */
    0x7F, 0x08, 0x04, 0x04, 0x78, /* 104 h */
    0x00, 0x44, 0x7D, 0x40, 0x00, /* 105 i */
    0x20, 0x40, 0x44, 0x3D, 0x00, /* 106 j */
    0x00, 0x7F, 0x10, 0x28, 0x44, /* 107 k */
    0x00, 0x41, 0x7F, 0x40, 0x00, /* 108 l */
    0x7C, 0x04, 0x18, 0x04, 0x78, /* 109 m */
    0x7C, 0x08, 0x04, 0x04, 0x78, /* 110 n */
    0x38, 0x44, 0x44, 0x44, 0x38, /* 111 o */
    0x7C, 0x14, 0x14, 0x14, 0x08, /* 112 p */
    0x08, 0x14, 0x14, 0x18, 0x7C, /* 113 q */
    0x7C, 0x08, 0x04, 0x04, 0x08, /* 114 r */
    0x48, 0x54, 0x54, 0x54, 0x20, /* 115 s */
    0x04, 0x3F, 0x44, 0x40, 0x20, /* 116 t */
    0x3C, 0x40, 0x40, 0x20, 0x7C, /* 117 u */
    0x1C, 0x20, 0x40, 0x20, 0x1C, /* 118 v */
    0x3C, 0x40, 0x30, 0x40, 0x3C, /* 119 w */
    0x44, 0x28, 0x10, 0x28, 0x44, /* 120 x */
    0x0C, 0x50, 0x50, 0x50, 0x3C, /* 121 y */
    0x44, 0x64, 0x54, 0x4C, 0x44, /* 122 z */
    0x00, 0x08, 0x36, 0x41, 0x00, /* 123 { */
    0x00, 0x00, 0x7F, 0x00, 0x00, /* 124 | */
    0x00, 0x41, 0x36, 0x08, 0x00, /* 125 } */
    0x08, 0x08, 0x2A, 0x1C, 0x08, /* 126 ~ */
};

void st7735_char(st7735_t *disp, int x, int y, char c, uint16_t color, uint16_t bg) {
    if (c < 32 || c > 126)
        c = '?';
    const uint8_t *glyph = &font5x7[(c - 32) * 5];
    for (int col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (int row = 0; row < 7; row++)
            if (line & (1 << row)) {
                st7735_pixel(disp, x + col, y + row, color);
            } else if (bg != color) {
                st7735_pixel(disp, x + col, y + row, bg);
            }
    }
    /* 1 pixel gap after char */
    if (bg != color)
        for (int row = 0; row < 7; row++)
            st7735_pixel(disp, x + 5, y + row, bg);
}

void st7735_text(st7735_t *disp, int x, int y, const char *str, uint16_t color, uint16_t bg) {
    while (*str) {
        st7735_char(disp, x, y, *str++, color, bg);
        x += 6; /* 5 pixels + 1 gap */
    }
}

// ------------------------------------------------------------------------------------------------------------------------

int st7735_char_width(const fontinfo_t *font, char c, bool mono) {
    if (!font || !font->data)
        return 0;
    if (c < font->base)
        return 0;
    int font_rows = (font->height + 7) / 8;
    int char_offs = ((font->width * font_rows) + 1) * (c - font->base);
    return mono ? font->width : (int)font->data[char_offs];
}

int st7735_char_font(st7735_t *disp, int x, int y, char c, const fontinfo_t *font, uint16_t fg, uint16_t bg, bool mono) {
    if (!font || !font->data)
        return 0;
    if (c < font->base)
        return 0;
    int font_rows = (font->height + 7) / 8;
    int char_offs = ((font->width * font_rows) + 1) * (c - font->base);
    int char_width = mono ? font->width : (int)font->data[char_offs];
    for (int i = 0; i < char_width; i++) {
        for (int j = 0; j < font_rows; j++) {
            uint8_t char_byte = font->data[(char_offs + 1) + (i * font_rows) + j];
            for (int k = 0; k < 8; k++) {
                int py = y + (j * 8) + k;
                if (py >= y + font->height)
                    break;
                if (char_byte & (1 << k)) {
                    st7735_pixel(disp, x + i, py, fg);
                } else {
                    st7735_pixel(disp, x + i, py, bg);
                }
            }
        }
    }
    return char_width;
}

int st7735_text_font(st7735_t *disp, int x, int y, const char *str, const fontinfo_t *font, uint16_t fg, uint16_t bg, bool mono, int spacing) {
    if (!font || !font->data || !str)
        return 0;
    int start_x = x;
    while (*str)
        x += st7735_char_font(disp, x, y, *str++, font, fg, bg, mono) + spacing;
    return x - start_x;
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------
