CC = $(CROSS_COMPILE)gcc

CFLAGS = -g -std=gnu99 -Wall -Werror
CFLAGS += -I../api/include
LFLAGS = -L../api/lib
LIBS = -lm -lpthread -lrp

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=)

all: $(OBJS)

%: %.c
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@ $(LIBS)

clean:
	$(RM) $(OBJS)
