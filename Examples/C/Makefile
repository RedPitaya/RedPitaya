#Cross compiler definition

CFLAGS  = -g -std=gnu99 -Wall -Werror
CFLAGS += -Wl,--no-as-needed
CFLAGS += -I/opt/redpitaya/include
CFLAGS += -L/opt/redpitaya/lib -lm -lpthread -lrp

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=)

all: $(OBJS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) *.o
	$(RM) $(OBJS)
