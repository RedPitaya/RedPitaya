CXX=$(CROSS_COMPILE)g++
RM=rm
CP=cp

OUT_DIR=./bin/
OUT_DIR_INC=$(OUT_DIR)include/
XML_DIR=./src/xml
TEST_DIR=./test
TOOL_DIR=./src/tool
POWER_ON_DIR=./src/power_on
CONFIG_DIR=./configs

CXXSOURCES_I2C=src/i2c/i2c.cpp src/rp-i2c.cpp src/rp-i2c-mcp47x6.cpp src/rp-i2c-max7311.cpp src/rp-i2c-mcp47x6-c.cpp src/rp-i2c-max7311-c.cpp 
C_OUT_NAME_I2C=$(OUT_DIR)librp-i2c.so
C_OUT_NAME_I2C_A=$(OUT_DIR)librp-i2c.a

CXXSOURCES_SPI=src/spi/spi.cpp src/rp-spi.cpp
C_OUT_NAME_SPI=$(OUT_DIR)librp-spi.so
C_OUT_NAME_SPI_A=$(OUT_DIR)librp-spi.a

CXXSOURCES_GPIO=src/gpio/gpio.cpp src/rp-gpio-power.cpp
C_OUT_NAME_GPIO=$(OUT_DIR)librp-gpio.so
C_OUT_NAME_GPIO_A=$(OUT_DIR)librp-gpio.a

COMMON_FLAGS+=-Wall -fPIC -Os -s -Wno-sign-compare
CXXFLAGS += $(COMMON_FLAGS) -std=c++11
CXXFLAGS += -I$(OUT_DIR)lib/include


CXXOBJECTS_I2C=$(CXXSOURCES_I2C:.cpp=.o)
OBJECTS_I2C=$(CXXOBJECTS_I2C)
CXXOBJECTS_SPI=$(CXXSOURCES_SPI:.cpp=.o)
OBJECTS_SPI=$(CXXOBJECTS_SPI)
CXXOBJECTS_GPIO=$(CXXSOURCES_GPIO:.cpp=.o)
OBJECTS_GPIO=$(CXXOBJECTS_GPIO)
LIB=-L$(OUT_DIR)lib -lxml -lstdc++ -lpthread


all: $(SOURCES) xml make_i2c make_spi make_gpio

test: all
	$(MAKE) -C $(TEST_DIR)

xml:
	$(MAKE) -C $(XML_DIR) -j4

tool: all 
	$(MAKE) -C $(TOOL_DIR)
	$(MAKE) -C $(POWER_ON_DIR)


.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

make_i2c: xml $(OBJECTS_I2C) 
	-mkdir -p $(OUT_DIR_INC)
	rm -f $(OUT_DIR)lib/*.o
	( cd $(OUT_DIR)lib ; ar -x libxml.a )
	ar rcs $(C_OUT_NAME_I2C_A)  $(OBJECTS_I2C) $(OUT_DIR)lib/*.o
	ranlib $(C_OUT_NAME_I2C_A)
	rm -f $(OUT_DIR)lib/*.o
	$(CXX) -shared $(CXXFLAGS) $(OBJECTS_I2C) $(LIB) -o $(C_OUT_NAME_I2C) $(LDFLAGS)
	-$(CP) src/*.h $(OUT_DIR_INC)

make_spi: xml $(OBJECTS_SPI)
	-mkdir -p $(OUT_DIR_INC)
	rm -f $(OUT_DIR)lib/*.o
	( cd $(OUT_DIR)lib ; ar -x libxml.a )
	ar rcs $(C_OUT_NAME_SPI_A) $(OBJECTS_SPI) $(OUT_DIR)lib/*.o
	ranlib $(C_OUT_NAME_SPI_A)
	rm -f $(OUT_DIR)lib/*.o
	$(CXX) -shared $(CXXFLAGS) $(OBJECTS_SPI) $(LIB) -o $(C_OUT_NAME_SPI) $(LDFLAGS)
	-$(CP) src/*.h $(OUT_DIR_INC)

make_gpio: $(OBJECTS_GPIO)
	-mkdir -p $(OUT_DIR_INC)
	ar rcs $(C_OUT_NAME_GPIO_A) $(OBJECTS_GPIO)
	ranlib $(C_OUT_NAME_GPIO_A)
	$(CXX) -shared $(CXXFLAGS) $(OBJECTS_GPIO) $(LIB) -o $(C_OUT_NAME_GPIO) $(LDFLAGS)
	-$(CP) src/*.h $(OUT_DIR_INC)

clean:
	$(MAKE) -C $(XML_DIR) clean
	$(MAKE) -C $(TEST_DIR) clean
	$(MAKE) -C $(TOOL_DIR) clean
	-$(RM) -rf $(C_OUT_NAME) $(OBJECTS_SPI) $(OBJECTS_I2C) $(OUT_DIR)

install:
	mkdir -p $(INSTALL_DIR)/lib
	mkdir -p $(INSTALL_DIR)/include/api250-12
	cp  $(C_OUT_NAME_I2C) $(INSTALL_DIR)/lib
	cp  $(C_OUT_NAME_SPI) $(INSTALL_DIR)/lib
	cp  $(C_OUT_NAME_GPIO) $(INSTALL_DIR)/lib
	cp  $(C_OUT_NAME_I2C_A) $(INSTALL_DIR)/lib
	cp  $(C_OUT_NAME_SPI_A) $(INSTALL_DIR)/lib
	cp  $(C_OUT_NAME_GPIO_A) $(INSTALL_DIR)/lib
	cp -r $(OUT_DIR_INC)/*.h $(INSTALL_DIR)/include/api250-12
	cp -r $(CONFIG_DIR) $(INSTALL_DIR)/lib
	
install_tool:
	$(MAKE) -C $(TOOL_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(POWER_ON_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))
