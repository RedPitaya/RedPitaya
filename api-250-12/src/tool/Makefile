CXX=$(CROSS_COMPILE)gcc
RM=rm
CP=cp

OUT_DIR=./../../bin/

CXXSOURCES=main.cpp
C_OUT_NAME=$(OUT_DIR)rp_i2c_tool

COMMON_FLAGS+=-Wall -fPIC -Os -s -Wno-sign-compare
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11
CXXFLAGS += -I$(OUT_DIR)include


CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS) 
LIB=-L$(OUT_DIR) -static -lrp-i2c -lstdc++ -lpthread


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
