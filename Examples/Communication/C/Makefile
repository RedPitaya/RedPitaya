
#
#  Red Pitaya GPIO communication example
#  Author: Luka Golinar <luka.golinar@gmail.com>
#


# Versioning system
VERSION ?= 0.00-0000
REVISION ?= devbuild

TARGET = ./executables

#variable definition
UART_DIR = ./uart
SPI_DIR = ./spi
IIC_DIR = ./iic

UART = uart/uart_test
SPI = spi/spi_test
IIC = iic/iic_test

#Flags
CFLAGS = -g -std=gnu99 -Wall -Werror

#Build the executable
all: $(TARGET)

$(TARGET): $(UART) $(IIC) copy

$(UART):
	make -C $(UART_DIR) CROSS_COMPILE=$(CROSS_COMPILE)

#$(SPI):
#	make -C $(SPI_DIR) CROSS_COMPILE=$(CROSS_COMPILE)

$(IIC):
	make -C $(IIC_DIR) CROSS_COMPILE=$(CROSS_COMPILE)

copy:
	mkdir -p $(TARGET)
	cp $(UART_DIR)/uart_test $(TARGET)
	#cp $(SPI_DIR/spi_test) $(TARGET)
	cp $(IIC_DIR)/iic_test $(TARGET)
	@echo SUCCESS!

clean:
	make -C $(UART_DIR) clean
	#make -C $(SPI_DIR) clean
	make -C $(IIC_DIR) clean
	rm -rf $(TARGET)
