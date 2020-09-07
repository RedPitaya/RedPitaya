INSTALL_DIR ?= .

CXX=$(CROSS_COMPILE)g++
RM=rm
CP=cp

OUT_DIR=./../bin/

CXXSOURCES_2=main_spi.cpp
C_OUT_NAME_2=$(OUT_DIR)spi_ext_test_tool

COMMON_FLAGS+=-Wall -fPIC -Os -s -Wno-sign-compare
CXXFLAGS+=$(COMMON_FLAGS) -std=c++11
CXXFLAGS += -I$(OUT_DIR)include

CXXOBJECTS=$(CXXSOURCES:.cpp=.o)
OBJECTS=$(CXXOBJECTS)
CXXOBJECTS_2=$(CXXSOURCES_2:.cpp=.o)
OBJECTS_2=$(CXXOBJECTS_2)
LIB =-L$(OUT_DIR) -lstdc++

all: $(SOURCES) make_c_app_2

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

make_c_app_2: $(OBJECTS_2)
	-mkdir -p $(OUT_DIR)
	$(CXX) $(OBJECTS_2) $(LIB) -o $(C_OUT_NAME_2) $(LDFLAGS)
clean:
	-$(RM) -rf $(C_OUT_NAME_2) $(OBJECTS_2) $(OUT_DIR)

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(C_OUT_NAME_2) $(INSTALL_DIR)/bin
