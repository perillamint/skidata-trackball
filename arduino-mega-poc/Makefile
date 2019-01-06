CC=avr-gcc
CFLAGS=-mmcu=atmega2560 -O3
SRCS = $(wildcard *.c)
OBJ = $(patsubst %.c,%.o,$(SRCS))

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

driver.elf: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
