CC = $(CROSS_COMPILE)gcc

CFLAGS  = -g -std=gnu99 -Wall -Werror
CFLAGS += -lm -lpthread

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=)

all: $(OBJS)
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) *.o
	$(RM) $(OBJS)
