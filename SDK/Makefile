#
# Red Pitaya SDK Makefile
#

#Includes
RP_PATH_INCLUDE = -I./include

#Library
RP_LIB_INCLUDE = -L ./include -lm -lpthread -lrp

#Cross compiler definition
CC = $(CROSS_COMPILE)gcc

#Flags
CFLAGS = -g -std=gnu99 -Wall -Werror

OBJECT ?= frame_work.o
TARGET ?= executable

$(TARGET): $(OBJECT)
	$(CC) -o $@ $^ $(CFLAGS) $(RP_LIB_INCLUDE)

%.o: %.c
	$(CC) -c $(CFLAGS) $(RP_PATH_INCLUDE) $< -o $@

#Build the executable
all: $(TARGET)

clean:
	$(RM) $(TARGET) *.o ~*
