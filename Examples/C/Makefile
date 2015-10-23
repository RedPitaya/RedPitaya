#Cross compiler definition
CC = $(CROSS_COMPILE)gcc

CFLAGS  = -g -std=gnu99 -Wall -Werror
CFLAGS += -I../../api/include
CFLAGS += -L ../../api/lib -lm -lpthread -lrp

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=)

all: $(OBJS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) *.o
	$(RM) $(OBJS)
