
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

LIB_AUTOMATIONHAT = libautomationhat.a

OBJS = $(SRCS:.c=.o)

all: test_st7735 mock_st7735 test_automationhat $(LIB_AUTOMATIONHAT)

test_st7735: test_st7735.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
mock_st7735: mock_st7735.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
test_automationhat: test_automationhat.o automationhat.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(LIB_AUTOMATIONHAT): fonts.o st7735.o automationhat.o
	ar rcs $@ $^

%.o: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies
fonts.o: fonts.c fonts.h
st7735.o: st7735.c st7735.h hardware.h
test_st7735.o: test_st7735.c st7735.h fonts.h
mock_st7735.o: mock_st7735.c st7735.h fonts.h
automationhat.o: automationhat.h hardware.h
test_automationhat.o: test_automationhat.c automationhat.h hardware.h

clean:
	rm -f test_st7735 mock_st7735 test_automationhat *.o

.PHONY: all clean
