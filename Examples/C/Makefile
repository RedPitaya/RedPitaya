#
#
#  Red Pitaya Api example Makefile
#
#
# Versioning system
VERSION ?= 0.00-0000

REVISION ?= devbuild

#Define header includes
RP_PATH_INCLUDE = -I../../api/rpbase/src/

#Define library includes
RP_LIB_INCLUDE = -L ../../api/lib -lm -lpthread -lrp

#Cross compiler definition
CC = $(CROSS_COMPILE)gcc
#Flags
CFLAGS = -g -std=gnu99 -Wall -Werror
#Objects
OBJECTS = digital_led_blink.o
#Target file
TARGET = api_test

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(RP_LIB_INCLUDE)

%.o: %.c
	$(CC) -c $(CFLAGS) $(RP_PATH_INCLUDE) $< -o $@

#Build the executable
all: $(TARGET)

clean:
	$(RM) $(TARGET) *.o ~* 
