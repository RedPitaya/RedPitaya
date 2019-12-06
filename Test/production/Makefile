INSTALL_DIR ?= .
BINARY_FILES = calibration_script.sh delete_MAC.sh spectrum_sfdr_test.py production_testing_script_z20_spectrum.sh production_env_install.sh production_env_gen1_install.sh production_env_gen2_install.sh

TEXT_FILES = environment_parameters.txt
PRODUCTION_TESTING_SCRIPT = production_testing_script.sh
FPGA_MODEL ?= Z10

ifeq ($(FPGA_MODEL), Z20)
PRODUCTION_TESTING_SCRIPT = production_testing_script_z20.sh
endif

all:

clean:

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(BINARY_FILES) $(INSTALL_DIR)/bin
	cp $(PRODUCTION_TESTING_SCRIPT) $(INSTALL_DIR)/bin/production_testing_script.sh
	cp $(TEXT_FILES) $(INSTALL_DIR)
