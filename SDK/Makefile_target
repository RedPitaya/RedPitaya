#
# Red Pitaya SDK Makefile
#

#Includes
RP_PATH_INCLUDE = -I./include

#Library
RP_LIB_INCLUDE = -L./include -lm -lpthread -lrp

#Cross compiler 
CROSS_COMPILE ?= arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc

#Flags
CLFAGS = -g -std=gnu99 -Wall -Werror

#Objects
OBJECT ?= template.o
TARGET ?= template

#Build the executable
all: $(TARGET)

$(TARGET): $(OBJECT)
	$(CC) -o $@ $^ $(CFLAGS) $(RP_LIB_INCLUDE)

%.o: %.c
	$(CC) -c $(CFLAGS) $(RP_PATH_INCLUDE) $< -o $@

clean:
	-$(RM) $(TARGET) *.o ~*
