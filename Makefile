
CC=gcc
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
CFLAGS=$(CFLAGS_COMMON) $(CFLAGS_STRICT) -O3 -fstack-protector-strong
LDFLAGS = -lm

SRCS = st7735.c fonts.c
OBJS = $(SRCS:.c=.o)

all: test mock

test: test.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
mock: mock.o $(OBJS)
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
	rm -f test *.o

.PHONY: all clean
