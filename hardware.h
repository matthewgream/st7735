
#ifndef _HARDWARE_H_
#define _HARDWARE_H_

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#define GPIO_FSEL0 0  /* Function select registers */
#define GPIO_SET0  7  /* Set output high */
#define GPIO_CLR0  10 /* Set output low */
#define GPIO_LEV0  13 /* Read level */

static const char *gpio_dev = "/dev/gpiomem";
static const size_t gpio_mmap_size = 4096;
static volatile uint32_t *gpio_mem = NULL;

static inline int gpio_open(void) {
    const int gpio_fd = open(gpio_dev, O_RDWR | O_SYNC);
    if (gpio_fd < 0) {
        perror("open: gpio_dev");
        return -1;
    }
    gpio_mem = mmap(NULL, gpio_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, gpio_fd, 0);
    close(gpio_fd);
    if (gpio_mem == MAP_FAILED) {
        perror("mmap: gpio_dev");
        gpio_mem = NULL;
        return -1;
    }
    return 0;
}
static inline void gpio_close(void) {
    if (gpio_mem != NULL) {
        munmap((void *)(uintptr_t)gpio_mem, gpio_mmap_size);
        gpio_mem = NULL;
    }
}
static inline void gpio_set_input(int pin) {
    const int reg = pin / 10, shift = (pin % 10) * 3;
    gpio_mem[reg] &= (uint32_t)~(7 << shift); /* 000 = input */
}
static inline void gpio_set_output(int pin) {
    const int reg = pin / 10, shift = (pin % 10) * 3;
    gpio_mem[reg] = (gpio_mem[reg] & (uint32_t)~(7 << shift)) | (1 << shift); /* 001 = output */
}
static inline bool gpio_read(int pin) {
    return (gpio_mem[GPIO_LEV0] & (1 << pin)) != 0;
}
static inline void gpio_write(int pin, bool value) {
    gpio_mem[value ? GPIO_SET0 : GPIO_CLR0] = 1 << pin;
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

static const char *i2c_dev = "/dev/i2c-1";
static int i2c_fd = -1;

static inline int i2c_open(uint8_t addr) {
    i2c_fd = open(i2c_dev, O_RDWR);
    if (i2c_fd < 0) {
        perror("open: i2c_dev");
        return -1;
    }
    if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
        perror("ioctl: i2c_dev");
        close(i2c_fd);
        i2c_fd = -1;
        return -1;
    }
    return 0;
}
static inline void i2c_close(void) {
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
}
static inline int i2c_read_reg16(uint8_t reg, uint16_t *value) {
    uint8_t buf[2];
    if (write(i2c_fd, &reg, 1) != 1)
        return 0;
    if (read(i2c_fd, buf, sizeof(buf)) != sizeof(buf))
        return 0;
    *value = (uint16_t)((buf[0] << 8) | buf[1]);
    return 1;
}
static inline int i2c_write_reg16(uint8_t reg, uint16_t value) {
    const uint8_t buf[3] = { reg, (uint8_t)((value >> 8) & 0xFF), (uint8_t)(value & 0xFF) };
    return write(i2c_fd, buf, sizeof(buf)) == sizeof(buf);
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

static const char *spi_dev = "/dev/spidev0.1";
static const uint32_t spi_speed = 16000000;
static const uint8_t spi_mode = SPI_MODE_0;
static const uint8_t spi_bits = 8;
static int spi_fd = -1;

#define SPI_CHUNK_SIZE 4096

static inline int spi_open(void) {
    spi_fd = open(spi_dev, O_RDWR);
    if (spi_fd < 0) {
        perror("open: spi_dev");
        return -1;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode) < 0 || ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits) < 0 ||
        ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
        perror("ioctl: spi_dev");
        close(spi_fd);
        spi_fd = -1;
        return -1;
    }
    return 0;
}
static inline void spi_close(void) {
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
    }
}
static inline void spi_write(const uint8_t *buf, size_t len) {
    const struct spi_ioc_transfer tr = { .tx_buf = (unsigned long)buf, .len = (unsigned int)len };
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        perror("ioctl: spi_dev");
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#endif // _HARDWARE_H_
