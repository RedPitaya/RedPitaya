#Cross compiler definition
CC = $(CROSS_COMPILE)gcc

CFLAGS  = -g -std=gnu99 -Wall -Werror
CFLAGS += -lm -lpthread
CFLAGS += -I../src -I../include
CFLAGS += -L ../lib
#CFLAGS += -lrp2

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=)

all: $(OBJS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) *.o
	$(RM) $(OBJS)
