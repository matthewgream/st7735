
// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#include "st7735.h"

#include <errno.h>
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
#define ST7735_SLPOUT  0x11
#define ST7735_NORON   0x13
#define ST7735_INVON   0x21
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_MADCTL  0x36
#define ST7735_COLMOD  0x3A
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST7735_COLS   132
#define ST7735_ROWS   162
#define ST7735_WIDTH  80
#define ST7735_HEIGHT 160

#define GPIO_FSEL0 0
#define GPIO_SET0  7
#define GPIO_CLR0  10

#define GPIO_MMAP_SIZE 4096

#define SPI_CHUNK_SIZE 4096

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

    size_t pixels;
    uint8_t *tmpbuf;

    uint16_t *buffer;

    int dirty_x1, dirty_y1;
    int dirty_x2, dirty_y2;
    bool dirty;
};

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

static inline void gpio_set_output(const st7735_t *disp, int pin) {
    int reg = pin / 10, shift = (pin % 10) * 3;
    disp->gpio[reg] = (disp->gpio[reg] & (uint32_t)~(7 << shift)) | (1 << shift);
}
static inline void gpio_set(const st7735_t *disp, int pin) {
    disp->gpio[GPIO_SET0] = 1 << pin;
}
static inline void gpio_clr(const st7735_t *disp, int pin) {
    disp->gpio[GPIO_CLR0] = 1 << pin;
}

// ------------------------------------------------------------------------------------------------------------------------

static inline void spi_write(const st7735_t *disp, const uint8_t *buf, size_t len) {
    const struct spi_ioc_transfer tr = { .tx_buf = (unsigned long)buf, .len = (unsigned int)len };
    if (ioctl(disp->spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        perror("ioctl");
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

static void cmd(const st7735_t *disp, uint8_t c) {
    gpio_clr(disp, disp->dc_pin);
    spi_write(disp, &c, 1);
}
static void dat(const st7735_t *disp, uint8_t d) {
    gpio_set(disp, disp->dc_pin);
    spi_write(disp, &d, 1);
}
static void dat_buf(const st7735_t *disp, const uint8_t *buf, size_t len) {
    gpio_set(disp, disp->dc_pin);
    while (len > 0) {
        const size_t n = (len < SPI_CHUNK_SIZE) ? len : SPI_CHUNK_SIZE;
        spi_write(disp, buf, n);
        buf += n;
        len -= n;
    }
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

static void set_window(const st7735_t *disp, int x0, int y0, int x1, int y1) {
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

static void init_seq(const st7735_t *disp) {
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
    const uint8_t gp[] = { 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10 };
    for (int i = 0; i < 16; i++)
        dat(disp, gp[i]);

    cmd(disp, ST7735_GMCTRN1);
    const uint8_t gn[] = { 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10 };
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
    if (!disp) {
        perror("calloc");
        return NULL;
    }

    disp->dc_pin = (uint8_t)dc_pin;
    disp->bl_pin = (uint8_t)bl_pin;
    disp->buffer = NULL;
    disp->dirty = false;

    if (rotation == 0 || rotation == 180) {
        disp->width = ST7735_WIDTH;
        disp->height = ST7735_HEIGHT;
        disp->offset_left = (ST7735_COLS - ST7735_WIDTH) / 2;
        disp->offset_top = (ST7735_ROWS - ST7735_HEIGHT) / 2;
    } else {
        disp->width = ST7735_HEIGHT;
        disp->height = ST7735_WIDTH;
        disp->offset_left = (ST7735_ROWS - ST7735_HEIGHT) / 2;
        disp->offset_top = (ST7735_COLS - ST7735_WIDTH) / 2;
    }

    if (rotation == 0)
        disp->madctl = 0x00 | 0x08;
    else if (rotation == 90)
        disp->madctl = 0x60 | 0x08;
    else if (rotation == 180)
        disp->madctl = 0xC0 | 0x08;
    else
        disp->madctl = 0xA0 | 0x08;

    disp->pixels = disp->width * disp->height;
    disp->tmpbuf = malloc(disp->pixels * sizeof(uint16_t));
    if (!disp->tmpbuf) {
        perror("malloc");
        goto failed;
    }

    disp->spi_fd = open("/dev/spidev0.1", O_RDWR);
    if (disp->spi_fd < 0) {
        perror("open: /dev/spidev0.1");
        goto failed;
    }
    const uint8_t spi_mode = SPI_MODE_0, spi_bits = 8;
    if (ioctl(disp->spi_fd, SPI_IOC_WR_MODE, &spi_mode) < 0 || ioctl(disp->spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits) < 0 || ioctl(disp->spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
        perror("ioctl: /dev/spidev0.1");
        goto failed;
    }

    const int mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("open: /dev/gpiomem");
        goto failed_spi;
    }
    disp->gpio = mmap(NULL, GPIO_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
    close(mem_fd);
    if (disp->gpio == MAP_FAILED) {
        perror("mmap: /dev/gpiomem");
        goto failed_spi;
    }

    gpio_set_output(disp, dc_pin);
    gpio_set_output(disp, bl_pin);
    gpio_set(disp, bl_pin); /* Backlight on */

    init_seq(disp);

    return disp;

failed_spi:
    close(disp->spi_fd);
failed:
    free(disp);
    return NULL;
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_close(st7735_t *disp) {
    if (!disp)
        return;

    gpio_clr(disp, disp->bl_pin); /* Backlight off */

    if (disp->buffer)
        free(disp->buffer);
    if (disp->gpio)
        munmap((void *)(uintptr_t)disp->gpio, GPIO_MMAP_SIZE);
    if (disp->spi_fd >= 0)
        close(disp->spi_fd);
    if (disp->tmpbuf)
        free(disp->tmpbuf);
    free(disp);
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

int st7735_width(const st7735_t *disp) {
    return disp->width;
}
int st7735_height(const st7735_t *disp) {
    return disp->height;
}

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
        if (!disp->buffer) {
            perror("malloc");
            return;
        }
        memset(disp->buffer, 0, disp->width * disp->height * sizeof(uint16_t));
    } else if (!enabled && disp->buffer) {
        free(disp->buffer);
        disp->buffer = NULL;
    }
}

bool st7735_is_buffered(const st7735_t *disp) {
    return disp->buffer != NULL;
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_flush(st7735_t *disp) {
    if (!disp->buffer || !disp->dirty)
        return;
    const int x1 = disp->dirty_x1, y1 = disp->dirty_y1;
    const int x2 = disp->dirty_x2, y2 = disp->dirty_y2;
    const int w = x2 - x1 + 1, h = y2 - y1 + 1;
    set_window(disp, x1, y1, x2, y2);
    uint8_t *tmp = disp->tmpbuf;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    for (int y = y1; y <= y2; y++) {
        memcpy(tmp, &disp->buffer[y * disp->width + x1], w * 2);
        tmp += w * 2;
    }
    dat_buf(disp, disp->tmpbuf, (size_t)(w * h) * 2);
#else
    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++) {
            const uint16_t px = disp->buffer[y * disp->width + x];
            *tmp++ = (uint8_t)(px >> 8);
            *tmp++ = (uint8_t)(px & 0xFF);
        }
    dat_buf(disp, disp->tmpbuf, (size_t)(w * h) * 2);
#endif
    disp->dirty = false;
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_pixel(st7735_t *disp, int x, int y, uint16_t color) {
    if (x < 0 || x >= disp->width || y < 0 || y >= disp->height)
        return;
    if (disp->buffer) {
        disp->buffer[y * disp->width + x] = color;
        if (!disp->dirty) {
            disp->dirty_x1 = disp->dirty_x2 = x;
            disp->dirty_y1 = disp->dirty_y2 = y;
            disp->dirty = true;
        } else {
            if (x < disp->dirty_x1)
                disp->dirty_x1 = x;
            if (x > disp->dirty_x2)
                disp->dirty_x2 = x;
            if (y < disp->dirty_y1)
                disp->dirty_y1 = y;
            if (y > disp->dirty_y2)
                disp->dirty_y2 = y;
        }
    } else {
        const uint8_t buf[2] = { (uint8_t)(color >> 8), (uint8_t)(color & 0xFF) };
        set_window(disp, x, y, x, y);
        dat_buf(disp, buf, 2);
    }
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_fill(st7735_t *disp, uint16_t color) {
    if (disp->buffer) {
        for (int y = 0; y < disp->height; y++)
            for (int x = 0; x < disp->width; x++)
                st7735_pixel(disp, x, y, color);
    } else {
        for (size_t i = 0; i < disp->pixels; i++) {
            disp->tmpbuf[i * 2] = (uint8_t)(color >> 8);
            disp->tmpbuf[i * 2 + 1] = (uint8_t)(color & 0xFF);
        }
        set_window(disp, 0, 0, disp->width - 1, disp->height - 1);
        dat_buf(disp, disp->tmpbuf, disp->pixels * 2);
    }
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_line(st7735_t *disp, int x0, int y0, int x1, int y1, uint16_t color) {
    const int dx = abs(x1 - x0), dy = abs(y1 - y0);
    const int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
    int e = dx - dy;
    while (1) {
        st7735_pixel(disp, x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        const int e2 = 2 * e;
        if (e2 > -dy) {
            e -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            e += dx;
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
    if (disp->buffer) {
        for (int py = y; py < y + h; py++)
            for (int px = x; px < x + w; px++)
                st7735_pixel(disp, px, py, color);
    } else {
        for (size_t i = 0; i < (size_t)(w * h); i++) {
            disp->tmpbuf[i * 2] = (uint8_t)(color >> 8);
            disp->tmpbuf[i * 2 + 1] = (uint8_t)(color & 0xFF);
        }
        set_window(disp, x, y, x + w - 1, y + h - 1);
        dat_buf(disp, disp->tmpbuf, (size_t)(w * h) * 2);
    }
}

// ------------------------------------------------------------------------------------------------------------------------

void st7735_circle(st7735_t *disp, int x0, int y0, int r, uint16_t color) {
    int x = r, y = 0;
    int e = 0;
    while (x >= y) {
        st7735_pixel(disp, x0 + x, y0 + y, color);
        st7735_pixel(disp, x0 + y, y0 + x, color);
        st7735_pixel(disp, x0 - y, y0 + x, color);
        st7735_pixel(disp, x0 - x, y0 + y, color);
        st7735_pixel(disp, x0 - x, y0 - y, color);
        st7735_pixel(disp, x0 - y, y0 - x, color);
        st7735_pixel(disp, x0 + y, y0 - x, color);
        st7735_pixel(disp, x0 + x, y0 - y, color);
        e += 1 + 2 * (++y);
        if (2 * (e - x) + 1 > 0)
            e += 1 - 2 * (--x);
    }
}

void st7735_fill_circle(st7735_t *disp, int x0, int y0, int r, uint16_t color) {
    int x = r, y = 0;
    int e = 0;
    while (x >= y) {
        st7735_line(disp, x0 - x, y0 + y, x0 + x, y0 + y, color);
        st7735_line(disp, x0 - y, y0 + x, x0 + y, y0 + x, color);
        st7735_line(disp, x0 - x, y0 - y, x0 + x, y0 - y, color);
        st7735_line(disp, x0 - y, y0 - x, x0 + y, y0 - x, color);
        e += 1 + 2 * (++y);
        if (2 * (e - x) + 1 > 0)
            e += 1 - 2 * (--x);
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

static inline int st7735_char_space(st7735_t *disp, int x, int y, uint16_t bg, int char_height, int spacing) {
    for (int i = 0; i < spacing; i++, x++)
        for (int j = 0; j < char_height; j++)
            st7735_pixel(disp, x, y + j, bg);
    return spacing;
}

int st7735_char(st7735_t *disp, int x, int y, uint16_t fg, uint16_t bg, char c) {
    if (c < 32 || c > 126)
        c = '?';
    const int char_height = 7, char_offs = c - 32, char_width = 5;
    for (int i = 0; i < char_width; i++) {
        const uint8_t char_byte = font5x7[char_offs * char_width + i];
        for (int j = 0; j < char_height; j++)
            st7735_pixel(disp, x + i, y + j, (char_byte & (1 << j)) ? fg : bg);
    }
    return char_width;
}

int st7735_text(st7735_t *disp, int x, int y, uint16_t fg, uint16_t bg, int spacing, const char *str) {
    if (!str)
        return 0;
    const int x_start = x;
    const int char_height = 7;
    while (*str) {
        x += st7735_char(disp, x, y, fg, bg, *str++);
        x += st7735_char_space(disp, x, y, bg, char_height, spacing);
    }
    return x - x_start;
}

// ------------------------------------------------------------------------------------------------------------------------

#ifdef ST7735_EXTERNAL_FONTS

int st7735_char_font(st7735_t *disp, int x, int y, uint16_t fg, uint16_t bg, const fontinfo_t *font, bool mono, char c) {
    if (!font || !font->data)
        return 0;
    if (c < font->base || c > font->limit)
        c = '?';
    const int char_height = (font->height + 7) / 8;
    const int char_offs = ((font->width * char_height) + 1) * (c - font->base);
    const int char_width = mono ? font->width : (int)font->data[char_offs];
    for (int i = 0; i < char_width; i++) {
        for (int j = 0; j < char_height; j++) {
            const uint8_t char_byte = font->data[(char_offs + 1) + (i * char_height) + j];
            for (int k = 0; k < 8; k++) {
                if ((j * 8) + k >= font->height)
                    break;
                st7735_pixel(disp, x + i, y + (j * 8) + k, (char_byte & (1 << k)) ? fg : bg);
            }
        }
    }
    return char_width;
}

int st7735_text_font(st7735_t *disp, int x, int y, uint16_t fg, uint16_t bg, const fontinfo_t *font, bool mono, int spacing, const char *str) {
    if (!font || !font->data || !str)
        return 0;
    const int x_start = x;
    const int char_height = (font->height + 7) / 8;
    while (*str) {
        x += st7735_char_font(disp, x, y, fg, bg, font, mono, *str++);
        x += st7735_char_space(disp, x, y, bg, char_height, spacing);
    }
    return x - x_start;
}

#endif

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------
