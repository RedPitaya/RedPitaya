INSTALL_DIR ?= .

BINARY_FILES = calibration_script.sh delete_MAC.sh production_env_install.sh production_env_gen1_install.sh production_env_gen2_install.sh
TEXT_FILES = environment_parameters.txt

all:
ifeq ($(MODEL), Z20_250_12)
	make -C production_test_z250_12/apps/uart_tool
	make -C production_test_z250_12/apps/i2c_spi_tool
	make -C production_test_z250_12/apps/analyze_signal
	make -C production_test_z250_12/apps/generate_with_api
	make -C production_test_z250_12/apps/capacitor_calib_tool
endif

ifeq ($(MODEL),$(filter $(MODEL),Z10 Z20_125))
	make -C production_test_z10_125/apps/spi_tool
	make -C production_test_z10_125/apps/generate_with_api
endif

ifeq ($(MODEL), Z20)
	make -C production_test_z20_122/apps/spi_tool
	make -C production_test_z20_122/apps/generate_with_api
endif

clean:
	make -C production_test_z10_125/apps/spi_tool clean
	make -C production_test_z10_125/apps/generate_with_api clean
	make -C production_test_z20_122/apps/spi_tool clean
	make -C production_test_z20_122/apps/generate_with_api clean
	make -C production_test_z250_12/apps/uart_tool clean
	make -C production_test_z250_12/apps/i2c_spi_tool clean
	make -C production_test_z250_12/apps/analyze_signal clean
	make -C production_test_z250_12/apps/generate_with_api clean
	make -C production_test_z250_12/apps/capacitor_calib_tool clean
install:
	mkdir -p $(INSTALL_DIR)/bin
	echo "#!/bin/bash" >> $(INSTALL_DIR)/bin/production_testing_script.sh
	chmod +x $(INSTALL_DIR)/bin/production_testing_script.sh
ifeq ($(MODEL), Z20_250_12)
	make -C production_test_z250_12/apps/uart_tool install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	make -C production_test_z250_12/apps/i2c_spi_tool install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	make -C production_test_z250_12/apps/analyze_signal install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	make -C production_test_z250_12/apps/generate_with_api install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	make -C production_test_z250_12/apps/capacitor_calib_tool install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	cp --parents `find production_test_z250_12 -name \*.sh*` $(INSTALL_DIR)/bin
endif

ifeq ($(MODEL),$(filter $(MODEL),Z10 Z20_125))
	make -C production_test_z10_125/apps/spi_tool install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	make -C production_test_z10_125/apps/generate_with_api install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	cp --parents `find production_test_z10_125 -name \*.sh*` $(INSTALL_DIR)/bin
	echo "cd /opt/redpitaya/bin/production_test_z10_125/" >> $(INSTALL_DIR)/bin/production_testing_script.sh
	echo "./production_testing_script.sh" >> $(INSTALL_DIR)/bin/production_testing_script.sh
endif

ifeq ($(MODEL), Z20)
	make -C production_test_z20_122/apps/spi_tool install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	make -C production_test_z20_122/apps/generate_with_api install INSTALL_DIR=$(abspath $(INSTALL_DIR))
	cp --parents `find production_test_z20_122 -name \*.sh*` $(INSTALL_DIR)/bin
	cp --parents `find production_test_z20_122 -name \*.py*` $(INSTALL_DIR)/bin
	echo "cd /opt/redpitaya/bin/production_test_z20_122/" >> $(INSTALL_DIR)/bin/production_testing_script.sh
	echo "./production_testing_script.sh" >> $(INSTALL_DIR)/bin/production_testing_script.sh
endif
	cp $(BINARY_FILES) $(INSTALL_DIR)/bin
	cp $(TEXT_FILES) $(INSTALL_DIR)

