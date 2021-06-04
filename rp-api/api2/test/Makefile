#Cross compiler definition
CC = $(CROSS_COMPILE)gcc

TARGET=laboardtest

# List of compiled object files (not yet linked to executable)
OBJS = test_la.o
# List of raw source files (all object files, renamed from .o to .c)
SRCS = $(subst .o,.c, $(OBJS)))

CFLAGS  = -g -std=c11 -Wall -Werror
CFLAGS += -I$(INSTALL_DIR)/include
CFLAGS += -I$(INSTALL_DIR)/include/api2
CFLAGS += -I$(INSTALL_DIR)/include/redpitaya

LIBS  = -L$(INSTALL_DIR)/lib 
LIBS +=-static -lrp2 -lm -lpthread


all: $(OBJS)

all: $(TARGET)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	$(RM) *.o
	$(RM) $(OBJS)
