INSTALL_DIR ?= .
BINARY_FILES = calibration_script.sh delete_MAC.sh
TEXT_FILES = environment_parameters.txt
PRODUCTION_TESTING_SCRIPT = production_testing_script.sh
MODEL ?= Z10

ifeq ($(MODEL), Z20)
PRODUCTION_TESTING_SCRIPT = production_testing_script_z20.sh
endif

all:

clean:

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(BINARY_FILES) $(INSTALL_DIR)/bin
	cp $(PRODUCTION_TESTING_SCRIPT) $(INSTALL_DIR)/bin/production_testing_script.sh
	cp $(TEXT_FILES) $(INSTALL_DIR)
