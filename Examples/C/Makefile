#Cross compiler definition
CC = $(CROSS_COMPILE)gcc

CFLAGS  = -g -std=gnu99 -Wall -Werror
CFLAGS += -I../../api/rpbase/src/
CFLAGS += -L ../../api/lib -lm -lpthread -lrp

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) *.o
