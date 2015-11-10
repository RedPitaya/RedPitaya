
#
#  Red Pitaya SPI example Makefile
#


# Versioning system
VERSION ?= 0.00-0000

REVISION ?= devbuild

#Cross compiler definition
CC = $(CROSS_COMPILE)gcc
#Flags
CFLAGS = -g -std=gnu99 -Wall -Werror
#Objects
OBJECTS = spi.o
#Target file
TARGET = spi_test

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(RP_LIB_INCLUDE)

%.o: %.c
	$(CC) -c $(CFLAGS) $(RP_PATH_INCLUDE) $< -o $@

#Build the executable
all: $(TARGET)

clean:
	$(RM) $(TARGET) *.o ~*
