TARGET=decoder-cli
CXX=$(CROSS_COMPILE)g++
CXXFLAGS=-std=c++11 -Wall -pedantic -DCLI -Ofast -funroll-loops

INCLUDE += -I$(INSTALL_DIR)/rp_sdk
INCLUDE += -I$(INSTALL_DIR)/rp_sdk/libjson

SRC=decoder-cli.cpp -I$(INCLUDE) ../i2c_decoder.cpp ../spi_decoder.cpp ../uart_decoder.cpp ../can_decoder.cpp

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)
clean:
	rm -f $(TARGET)
