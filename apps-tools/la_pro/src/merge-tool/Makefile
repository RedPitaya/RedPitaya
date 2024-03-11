TARGET=merge-tool
#CC=g++
CC=$(CROSS_COMPILE)g++
CXXFLAGS=-std=c++11 -Wall -O3

SRC=merge-tool.cpp

all:
	$(CC) $(CXXFLAGS) $(SRC) -o $(TARGET)
clean:
	rm -f $(TARGET)
