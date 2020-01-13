INSTALL_DIR ?= .

CXX=$(CROSS_COMPILE)g++
RM=rm
CP=cp

OUT_DIR=./../bin/

CXXSOURCES=main.cpp

C_OUT_NAME=$(OUT_DIR)capacitor_calib_tool


COMMON_FLAGS+=-Wall -fPIC -Os -s -Wno-sign-compare
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11
CXXFLAGS += -I$(INSTALL_DIR)/include
CXXFLAGS += -I$(INSTALL_DIR)/include/api250-12

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)

LIB =-L$(OUT_DIR) -lstdc++ 
LIB +=-L$(INSTALL_DIR)/lib -static -lrp -lrp-gpio -lrp-i2c -lrp-spi -lpthread

all: $(SOURCES) make_c_app 

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
make_c_app: $(OBJECTS)
	-mkdir -p $(OUT_DIR)
	$(CXX) $(OBJECTS) $(LIB) -o $(C_OUT_NAME) $(LDFLAGS)
clean:
	-$(RM) -rf $(C_OUT_NAME) $(OBJECTS) $(OUT_DIR)

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(C_OUT_NAME) $(INSTALL_DIR)/bin
