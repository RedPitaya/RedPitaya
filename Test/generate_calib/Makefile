
MODEL ?= Z10
INSTALL_DIR ?= .
# Versioning system
VERSION ?= 0.00-0000
REVISION ?= devbuild

APP_NAME=generate_calib
CC = $(CROSS_COMPILE)gcc

CFLAGS =  -Wall -Werror -D$(MODEL) -DVERSION=$(VERSION) -DREVISION=$(REVISION) -Wno-unused-variable -Wno-unused-but-set-variable
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
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $(APP_NAME) $(LIBS)

clean:
	$(RM) $(OBJS)


install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(APP_NAME) $(INSTALL_DIR)/bin