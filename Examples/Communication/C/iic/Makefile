
#
#  Red Pitaya IIC Makefile
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
OBJECTS = iic.o
#Target file
TARGET = iic_test

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

#Build the executable
all: $(TARGET)

clean:
	$(RM) $(TARGET) *.o ~*

