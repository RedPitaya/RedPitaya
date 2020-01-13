INSTALL_DIR ?= .

CXX=$(CROSS_COMPILE)g++
RM=rm
CP=cp

OUT_DIR=./../bin/

CXXSOURCES=main.cpp
CXXSOURCES_2=main_spi.cpp
C_OUT_NAME=$(OUT_DIR)i2c_spi_test_tool
C_OUT_NAME_2=$(OUT_DIR)spi_ext_test_tool

COMMON_FLAGS+=-Wall -fPIC -Os -s -Wno-sign-compare
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11
CXXFLAGS += -I$(OUT_DIR)include
CXXFLAGS += -I$(INSTALL_DIR)/include/api250-12

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)
CXXOBJECTS_2=$(CXXSOURCES_2:.cpp=.o)
OBJECTS_2=$(CXXOBJECTS_2)
LIB =-L$(OUT_DIR) -lstdc++ 
LIB +=-L$(INSTALL_DIR)/lib -static -lrp-gpio -lrp-i2c -lrp-spi -lpthread

all: $(SOURCES) make_c_app make_c_app_2

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
make_c_app: $(OBJECTS)
	-mkdir -p $(OUT_DIR)
	$(CXX) $(OBJECTS) $(LIB) -o $(C_OUT_NAME) $(LDFLAGS)
make_c_app_2: $(OBJECTS_2)
	-mkdir -p $(OUT_DIR)
	$(CXX) $(OBJECTS_2) $(LIB) -o $(C_OUT_NAME_2) $(LDFLAGS)
clean:
	-$(RM) -rf $(C_OUT_NAME) $(OBJECTS) $(OUT_DIR)
	-$(RM) -rf $(C_OUT_NAME_2) $(OBJECTS_2) $(OUT_DIR)

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(C_OUT_NAME) $(INSTALL_DIR)/bin
	cp $(C_OUT_NAME_2) $(INSTALL_DIR)/bin