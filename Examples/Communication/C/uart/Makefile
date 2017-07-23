
#
#  Red Pitaya UART makefile
#  Author: Luka Golinar <luka.golinar@gmail.com>
#

# Versioning system
VERSION ?= 0.00-0000

REVISION ?= devbuild

#Cross compiler definition
CC = $(CROSS_COMPILE)gcc
#Flags
CFLAGS = -g -std=gnu99 -Wall -Werror
#Objects
OBJECTS = uart.o
#Target file
TARGET = uart_test

#Build the executable
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(TARGET) *.o ~*
