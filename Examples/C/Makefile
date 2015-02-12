#
#
#  Red Pitaya Api example Makefile
#
#
# Versioning system
VERSION ?= 0.00-0000

REVISION ?= devbuild

CROSS_COMPILE = arm-linux-gnueabi-

#Define header includes
RP_PATH_INCLUDE = -I../../api-mockup/rpbase/src/

#Define library includes
RP_LIB_INCLUDE = -L ../../api-mockup/rpbase -lm -lpthread -lrp

#Cross compiler definition
CC = $(CROSS_COMPILE)gcc
#Flags
CFLAGS = -g -std=gnu99 -Wall -Werror
#Objects
OBJECTS = blink_diode.o
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


#  -----Error----- I have no idea what was wrong. 
#  ../../api-mockup/rpbase/librp.so: file not recognized: File format not recognized
#
# 
