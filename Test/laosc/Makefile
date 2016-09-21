INSTALL_DIR ?= .
CC = $(CROSS_COMPILE)gcc

CFLAGS  = -g -std=gnu99 -Wall -Werror
CFLAGS += -I../../patches/redpitaya
CFLAGS += -I../include
CFLAGS += -I../src -I../include
CFLAGS += -L ../lib -lm -lpthread
#CFLAGS += -lrp2

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=)

all: $(OBJS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(OBJS) $(INSTALL_DIR)/bin

clean:
	$(RM) *.o
	$(RM) $(OBJS)
