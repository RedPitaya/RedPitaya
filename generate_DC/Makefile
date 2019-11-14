
MODEL ?= Z10
INSTALL_DIR ?= .

CC = $(CROSS_COMPILE)gcc

CFLAGS = -g -std=gnu99 -Wall -Werror
CFLAGS += -I../api/include
LFLAGS = -L../api/lib
LIBS = -lm -lpthread -lrp

ifeq ($(MODEL),Z20_250_12)
LIBS += -L$(INSTALL_DIR)/lib 
LIBS += -lrp-i2c
endif 

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=)

all: $(OBJS)

%: %.c
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@ $(LIBS)

clean:
	$(RM) $(OBJS)
