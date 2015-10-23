#
#  Red Pitaya Api example Makefile
#

CROSS_COMPILE ?= arm-linux-gnueabi-

#Define header includes
RP_PATH_INCLUDE ?= -I../../../rpbase/ -I../../../include

#Define library includes
RP_LIB_INCLUDE ?= -L ../../../lib -lm -lpthread -lrp

#Cross compiler definition
CC = $(CROSS_COMPILE)gcc

#Flags
CFLAGS = -g -std=gnu99 -Wall -Werror

#Objects
OBJECTS = common.o impedance_analyzer.o impApp.o
#Target file
TARGET = imp_test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(RP_LIB_INCLUDE)

%.o: %.c
	$(CC) -c $(CFLAGS) $(RP_PATH_INCLUDE) $< -o $@

clean:
	-$(RM) $(TARGET) *.o ~* 
