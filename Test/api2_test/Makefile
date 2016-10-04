TARGET=api2_test

CC=$(CROSS_COMPILE)g++
SRC=main.cpp
CFLAGS=-g -std=c++11 -Wall
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION)
CFLAGS += -I../../api2/src -I../../api2/include

LIBS=-lm -lpthread -L../../api2/lib -lrp2

all:
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o
