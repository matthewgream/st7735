
CC=gcc
CDEFS=-DST7735_EXTERNAL_FONTS -DST7735_IMAGE_SUPPORT_BMP -DST7735_IMAGE_SUPPORT_PNG -DST7735_IMAGE_SUPPORT_JPG -DST7735_IMAGE_SUPPORT_BASE64
CFLAGS_COMMON=-Wall -Wextra -Wpedantic
CFLAGS_STRICT=-Werror -Wcast-align -Wcast-qual \
	-Wstrict-prototypes \
	-Wold-style-definition \
	-Wcast-align -Wcast-qual -Wconversion \
	-Wfloat-equal -Wformat=2 -Wformat-security \
	-Winit-self \
	-Wlogical-op -Wmissing-include-dirs \
	-Wnested-externs -Wpointer-arith \
	-Wredundant-decls -Wshadow \
	-Wstrict-overflow=2 -Wswitch-default \
	-Wswitch-enum -Wundef \
	-Wunreachable-code -Wunused \
	-Wwrite-strings
CFLAGS=$(CDEFS) $(CFLAGS_COMMON) $(CFLAGS_STRICT) -O6 -fstack-protector-strong
LDFLAGS = -lm
ifneq (,$(findstring ST7735_IMAGE_SUPPORT_PNG,$(CFLAGS)))
    LDFLAGS += -lpng
endif
ifneq (,$(findstring ST7735_IMAGE_SUPPORT_JPG,$(CFLAGS)))
    LDFLAGS += -ljpeg
endif

SRCS = st7735.c
ifneq (,$(findstring ST7735_EXTERNAL_FONTS,$(CFLAGS)))
    SRCS += fonts.c
endif

OBJS = $(SRCS:.c=.o)

all: test mock hat

test: test.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
mock: mock.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
hat: hat.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies
test.o: test.c st7735.h fonts.h
mock.o: mock.c st7735.h fonts.h
st7735.o: st7735.c st7735.h
fonts.o: fonts.c fonts.h

clean:
	rm -f test mock *.o

.PHONY: all clean
