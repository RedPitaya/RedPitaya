
MODEL ?= Z10
INSTALL_DIR ?= .

CC = $(CROSS_COMPILE)gcc

CFLAGS =  -Wall -Werror -D$(MODEL)
CFLAGS += -I$(INSTALL_DIR)/include
CFLAGS += -I$(INSTALL_DIR)/include/api250-12
LFLAGS = -L$(INSTALL_DIR)/lib 
LIBS += -static -lrp
ifeq ($(MODEL),Z20_250_12)
LIBS += -lrp-i2c -lrp-gpio -lrp-spi
endif 
LIBS += -lm -lstdc++ -lpthread


SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=)

all: $(OBJS)

%: %.cpp
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@ $(LIBS)

clean:
	$(RM) $(OBJS)
