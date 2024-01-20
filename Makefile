# check if download cache directory is available
DL ?= dl

INSTALL_DIR ?= build
STREAMING ?= MASTER
CPU_CORES=$($(shell grep '^processor' /proc/cpuinfo | sort -u | wc -l) + 1)

################################################################################
# versioning system
################################################################################

VER := $(shell cat apps-tools/ecosystem/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
BUILD_NUMBER ?= 0
REVISION ?= $(shell git rev-parse --short HEAD)
VERSION = $(VER)-$(BUILD_NUMBER)
LINUX_VER = 2.03
export BUILD_NUMBER
export REVISION
export VERSION
export LINUX_VER
BUILD_MODE ?= Release
################################################################################
#
################################################################################

CUR_DIR = $(PWD)

all: api nginx examples  apps-tools apps-pro startupsh scpi rp_communication sdr


$(DL):
	mkdir -p $@

$(INSTALL_DIR):
	mkdir -p $@

################################################################################
# API libraries
################################################################################

LIBRP_DIR       		= rp-api/api
LIBRP_HW_DIR    		= rp-api/api-hw
LIBRP_HW_CAN_DIR  		= rp-api/api-hw-can
LIBRP_HW_PROFILES_DIR	= rp-api/api-hw-profiles
LIBRP_HW_CALIB_DIR		= rp-api/api-hw-calib
LIBRP2_DIR      		= rp-api/api2
LIBRP250_12_DIR 		= rp-api/api-250-12
LIBRP_DSP_DIR   		= rp-api/api-dsp
LIBRPAPP_DIR    		= rp-api/api-app
LIBRP_FORMATTER_DIR   	= rp-api/api-formatter
LIBRP_ARB_DIR		   	= rp-api/api-arb
ECOSYSTEM_DIR   		= Applications/ecosystem

.PHONY: api api2 librp librp250_12 librp_hw librp_dsp librp_hw_profiles librp_hw_calibration librp_hw_can librparb
.PHONY: librpapp liblcr_meter

api: librp librp_hw librp_hw_can librp_dsp librpapp librp_formatter librparb

api2: librp2

librp: librp250_12 librp_hw_calibration librp_hw_profiles
	cmake -B$(abspath $(LIBRP_DIR)/build) -S$(abspath $(LIBRP_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)  -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP_DIR)/build install

librp_hw:
	cmake -B$(abspath $(LIBRP_HW_DIR)/build) -S$(abspath $(LIBRP_HW_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP_HW_DIR)/build install

librp_hw_can: librp
	cmake -B$(abspath $(LIBRP_HW_CAN_DIR)/build) -S$(abspath $(LIBRP_HW_CAN_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP_HW_CAN_DIR)/build install

librp_hw_calibration: librp_hw_profiles
	cmake -B$(abspath $(LIBRP_HW_CALIB_DIR)/build) -S$(abspath $(LIBRP_HW_CALIB_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP_HW_CALIB_DIR)/build install

librp_hw_profiles:
	cmake -B$(abspath $(LIBRP_HW_PROFILES_DIR)/build) -S$(abspath $(LIBRP_HW_PROFILES_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP_HW_PROFILES_DIR)/build install

librp_dsp:
	cmake -B$(abspath $(LIBRP_DSP_DIR)/build) -S$(abspath $(LIBRP_DSP_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP_DSP_DIR)/build install

librp_formatter:
	cmake -B$(abspath $(LIBRP_FORMATTER_DIR)/build) -S$(abspath $(LIBRP_FORMATTER_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP_FORMATTER_DIR)/build install

librp2:
	cmake -B$(abspath $(LIBRP2_DIR)/build) -S$(abspath $(LIBRP2_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP2_DIR)/build install

librp250_12: librp_hw
	cmake -B$(LIBRP250_12_DIR)/build -S$(LIBRP250_12_DIR) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP250_12_DIR)/build install

librpapp: librp
	cmake -B$(abspath $(LIBRPAPP_DIR)/build) -S$(abspath $(LIBRPAPP_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRPAPP_DIR)/build install

librparb: librp
	cmake -B$(abspath $(LIBRP_ARB_DIR)/build) -S$(abspath $(LIBRP_ARB_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LIBRP_ARB_DIR)/build install


################################################################################
# Red Pitaya ecosystem
################################################################################

# directories
NGINX_DIR       = Bazaar/nginx

# targets
NGINX           = $(INSTALL_DIR)/sbin/nginx
IDGEN           = $(INSTALL_DIR)/sbin/idgen
SOCKPROC        = $(INSTALL_DIR)/sbin/sockproc
STARTUPSH       = $(INSTALL_DIR)/sbin/startup.sh
GETSYSINFOSH    = $(INSTALL_DIR)/sbin/getsysinfo.sh

WEBSOCKETPP_TAG = 0.8.2
LUANGINX_TAG    = v0.10.21
LUARESTY_TAG    = v0.1.23
LUARESTY_L_TAG  = v0.13
NGINX_TAG       = 1.19.10
SOCKPROC_TAG    = master

WEBSOCKETPP_URL = https://github.com/zaphoyd/websocketpp/archive/$(WEBSOCKETPP_TAG).tar.gz
LIBJSON_URL     = https://github.com/RedPitaya/libjson/archive/refs/tags/7.6.1.tar.gz
LUANGINX_URL    = https://codeload.github.com/openresty/lua-nginx-module/tar.gz/$(LUANGINX_TAG)
LUARESTY_URL    = https://github.com/openresty/lua-resty-core/archive/refs/tags/$(LUARESTY_TAG).tar.gz
LUARESTY_L_URL  = https://github.com/openresty/lua-resty-lrucache/archive/refs/tags/$(LUARESTY_L_TAG).tar.gz
NGINX_URL       = http://nginx.org/download/nginx-$(NGINX_TAG).tar.gz
SOCKPROC_URL	= https://github.com/juce/sockproc/archive/$(SOCKPROC_TAG).tar.gz

WEBSOCKETPP_TAR = $(DL)/websocketpp-$(WEBSOCKETPP_TAG).tar.gz
LIBJSON_TAR     = $(DL)/libjson_7.6.1.zip
LUANGINX_TAR    = $(DL)/lua-nginx-module-$(LUANGINX_TAG).tar.gz
LUARESTY_TAR    = $(DL)/lua-resty-module-$(LUARESTY_TAG).tar.gz
LUARESTY_L_TAR  = $(DL)/lua-resty-lrucache-module-$(LUARESTY_L_TAG).tar.gz
NGINX_TAR       = $(DL)/nginx-$(NGINX_TAG).tar.gz
SOCKPROC_TAR	= $(DL)/sockproc-$(SOCKPROC_TAG).tar.gz

WEBSOCKETPP_DIR = Bazaar/nginx/ngx_ext_modules/ws_server/websocketpp
LIBJSON_DIR     = Bazaar/tools/libjson
LUANGINX_DIR    = Bazaar/nginx/ngx_ext_modules/lua-nginx-module
LUARESTY_DIR    = Bazaar/nginx/ngx_ext_modules/lua-resty-module
LUARESTY_L_DIR  = Bazaar/nginx/ngx_ext_modules/lua-resty-lrucache-module
NGINX_SRC_DIR   = Bazaar/nginx/nginx
SOCKPROC_DIR    = Bazaar/tools/sockproc

.PHONY: ecosystem nginx $(LIBJSON_DIR) $(LIBJSON_TAR) $(DL) $(SOCKPROC_TAR) $(WEBSOCKETPP_DIR) $(WEBSOCKETPP_TAR) $(LUANGINX_TAR) $(NGINX_TAR) $(NGINX_SRC_DIR) $(LUANGINX_DIR)

$(WEBSOCKETPP_TAR): | $(DL)
	wget $(WEBSOCKETPP_URL) -O $@ --show-progress

$(WEBSOCKETPP_DIR): $(WEBSOCKETPP_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
#	patch -d $@ -p1 < patches/websocketpp-$(WEBSOCKETPP_TAG).patch

$(SOCKPROC_TAR): | $(DL)
	wget $(SOCKPROC_URL) -O $@ --show-progress

$(SOCKPROC_DIR): $(SOCKPROC_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

$(LIBJSON_TAR): | $(DL)
	wget $(LIBJSON_URL) -O $@ --show-progress

$(LIBJSON_DIR): $(LIBJSON_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	patch -d $@ -p1 < patches/libjson.patch

$(LUANGINX_TAR): | $(DL)
	wget $(LUANGINX_URL) -O $@ --show-progress

$(LUANGINX_DIR): $(LUANGINX_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

$(LUARESTY_TAR): | $(DL)
	wget $(LUARESTY_URL) -O $@ --show-progress

$(LUARESTY_DIR): $(LUARESTY_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	$(MAKE) -C $(LUARESTY_DIR) install PREFIX=$(abspath $(INSTALL_DIR)/www/conf)

$(LUARESTY_L_TAR): | $(DL)
	wget $(LUARESTY_L_URL) -O $@ --show-progress

$(LUARESTY_L_DIR): $(LUARESTY_L_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	$(MAKE) -C $(LUARESTY_L_DIR) install PREFIX=$(abspath $(INSTALL_DIR)/www/conf)

$(NGINX_TAR): | $(DL)
	wget $(NGINX_URL) -O $@ --show-progress

$(NGINX_SRC_DIR): $(NGINX_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	cp -f apps-tools/nginx.conf $@/conf/
	mkdir -p $@/conf/lua/
	cp -fr patches/lua/* $@/conf/lua/

$(NGINX): $(CRYPTOPP_DIR) $(WEBSOCKETPP_DIR) $(LIBJSON_DIR) $(LUARESTY_DIR) $(LUARESTY_L_DIR) $(LUANGINX_DIR) $(NGINX_SRC_DIR)
# $(MAKE) -C $(NGINX_DIR) clean
	$(MAKE) -C $(NGINX_DIR)
	$(MAKE) -C $(NGINX_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))
	mkdir -p $(INSTALL_DIR)/www/conf/lua
	cp -fr $(NGINX_DIR)/nginx/conf/lua/* $(abspath $(INSTALL_DIR))/www/conf/lua


IDGEN_DIR = apps-tools/idgen

$(IDGEN):
	$(MAKE) -C $(IDGEN_DIR) clean
	$(MAKE) -C $(IDGEN_DIR)
	$(MAKE) -C $(IDGEN_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))

$(SOCKPROC): $(SOCKPROC_DIR)
	$(MAKE) -C $<
	test -d $(INSTALL_DIR)/sbin || mkdir -p $(INSTALL_DIR)/sbin
	cp $</sockproc $@

nginx: $(NGINX) $(SOCKPROC)

startupsh:
	cp -f patches/startup/startup.sh $(STARTUPSH)

streaming_slave:
	test -d $(INSTALL_DIR)/bin || mkdir -p $(INSTALL_DIR)/bin
	echo "slave mode" > $(abspath $(INSTALL_DIR))/bin/.streaming_mode


################################################################################
# SCPI server
################################################################################

#SCPI_PARSER_TAG = 26aaabc20ef93754efe3ed43674e94c7cc444373
SCPI_PARSER_TAG = redpitaya

#SCPI_PARSER_URL = https://github.com/j123b567/scpi-parser/archive/$(SCPI_PARSER_TAG).tar.gz
SCPI_PARSER_URL = https://github.com/RedPitaya/scpi-parser/archive/$(SCPI_PARSER_TAG).tar.gz
SCPI_PARSER_TAR = $(DL)/scpi-parser-$(SCPI_PARSER_TAG).tar.gz
SCPI_SERVER_DIR = scpi-server
SCPI_PARSER_DIR = $(SCPI_SERVER_DIR)/scpi-parser

.PHONY: scpi

$(SCPI_PARSER_TAR): | $(DL)
	wget $(SCPI_PARSER_URL) -O $@ --show-progress

$(SCPI_PARSER_DIR): $(SCPI_PARSER_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

scpi: api $(INSTALL_DIR) $(SCPI_PARSER_DIR)
	$(MAKE) -C $(SCPI_SERVER_DIR) clean
	$(MAKE) -C $(SCPI_SERVER_DIR) MODEL=$(MODEL) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(SCPI_SERVER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# SDR
################################################################################

.PHONY: sdr

# git clone https://github.com/RedPitaya/red-pitaya-notes.git -b charly25ab
# ZIP file name should be updated for each new build
SDR_ZIP = SDR-bundle-26-cc97bf9a.zip
SDR_URL = https://downloads.redpitaya.com/hamlab/sdr-bundle/$(SDR_ZIP)

sdr: | $(DL)
ifeq ($(MODEL),$(filter $(MODEL),Z10))
	wget $(SDR_URL) -O $(DL)/$(SDR_ZIP) --show-progress
	mkdir -p $(INSTALL_DIR)/www/apps
	unzip -o $(DL)/$(SDR_ZIP) -d $(INSTALL_DIR)/www/apps
endif

################################################################################
# Red Pitaya tools
################################################################################

LCR_DIR            = Test/lcr
BODE_DIR           = Test/bode
MONITOR_DIR        = Test/monitor
ACQUIRE_DIR        = Test/acquire
CALIB_DIR          = Test/calib
#CALIBRATE_DIR      = Test/calibrate
GENERATOR_DIR	   = Test/generate
SPECTRUM_DIR       = Test/spectrum
LED_CONTROL_DIR    = Test/led_control
COMM_DIR           = Examples/Communication/C
XADC_DIR           = Test/xadc
LA_TEST_DIR        = rp-api/api2/test
DAISY_TOOL_DIR     = Test/daisy_tool
FPGA_TESTS_DIR     = Examples/Tests

.PHONY: examples rp_communication fpgautils
.PHONY: lcr bode monitor generator acquire calib spectrum laboardtest led_control daisy_tool fpga_tests

examples: lcr bode monitor calib spectrum acquire generator led_control fpgautils daisy_tool fpga_tests

# calibrate laboardtest

# lcr:
# 	$(MAKE) -C $(LCR_DIR) clean
# 	$(MAKE) -C $(LCR_DIR) MODEL=$(MODEL)
#	$(MAKE) -C $(LCR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

bode: api
	rm -rf $(abspath $(BODE_DIR)/build)
	cmake -B$(abspath $(BODE_DIR)/build) -S$(abspath $(BODE_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(BODE_DIR)/build install

monitor: api
	rm -rf $(abspath $(MONITOR_DIR)/build)
	cmake -B$(abspath $(MONITOR_DIR)/build) -S$(abspath $(MONITOR_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(MONITOR_DIR)/build install

generator: api
	rm -rf $(abspath $(GENERATOR_DIR)/build)
	cmake -B$(abspath $(GENERATOR_DIR)/build) -S$(abspath $(GENERATOR_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(GENERATOR_DIR)/build install

acquire: api
	rm -rf $(abspath $(ACQUIRE_DIR)/build)
	cmake -B$(abspath $(ACQUIRE_DIR)/build) -S$(abspath $(ACQUIRE_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(ACQUIRE_DIR)/build install

calib: api
	rm -rf $(abspath $(CALIB_DIR)/build)
	cmake -B$(abspath $(CALIB_DIR)/build) -S$(abspath $(CALIB_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(CALIB_DIR)/build install

daisy_tool: api
	rm -rf $(abspath $(DAISY_TOOL_DIR)/build)
	cmake -B$(abspath $(DAISY_TOOL_DIR)/build) -S$(abspath $(DAISY_TOOL_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(DAISY_TOOL_DIR)/build install

spectrum: api
	rm -rf $(abspath $(SPECTRUM_DIR)/build)
	cmake -B$(abspath $(SPECTRUM_DIR)/build) -S$(abspath $(SPECTRUM_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(SPECTRUM_DIR)/build install

# calibrate: api
# 	$(MAKE) -C $(CALIBRATE_DIR) clean
# 	$(MAKE) -C $(CALIBRATE_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
# 	$(MAKE) -C $(CALIBRATE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

led_control: api
	rm -rf $(abspath $(LED_CONTROL_DIR)/build)
	cmake -B$(abspath $(LED_CONTROL_DIR)/build) -S$(abspath $(LED_CONTROL_DIR)) -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)   -DVERSION=$(VERSION) -DREVISION=$(REVISION)
	$(MAKE) -C $(LED_CONTROL_DIR)/build install

laboardtest: api2
	$(MAKE) -C $(LA_TEST_DIR) clean
	$(MAKE) -C $(LA_TEST_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	mkdir -p $(abspath $(INSTALL_DIR))/bin
	cp rp-api/api2/test/laboardtest $(abspath $(INSTALL_DIR))/bin/laboardtest
	cp rp-api/api2/test/install.sh $(abspath $(INSTALL_DIR))/install.sh

rp_communication:
	make -C $(COMM_DIR)

fpgautils:
	mkdir -p $(abspath $(INSTALL_DIR))/bin
	$(CC) tools/fpgautils/fpgautil.c -o $(abspath $(INSTALL_DIR))/bin/fpgautil

fpga_tests: api
	$(MAKE) -C $(FPGA_TESTS_DIR) clean
	$(MAKE) -C $(FPGA_TESTS_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(FPGA_TESTS_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# Red Pitaya ecosystem and tools
################################################################################

#LIB_BOOTSTRAP_TAG = 3.3.6
#LIB_BOOTSTRAP_URL = https://github.com/twbs/bootstrap/releases/download/v$(LIB_BOOTSTRAP_TAG)/bootstrap-$(LIB_BOOTSTRAP_TAG)-dist.zip
#LIB_BOOTSTRAP_TAR = $(DL)/bootstrap-$(LIB_BOOTSTRAP_TAG)-dist.zip
#LIB_BOOTSTRAP_DIR = apps-tools/assets/bootstrap
#
#$(LIB_BOOTSTRAP_TAR): | $(DL)
#	curl -L $(LIB_BOOTSTRAP_URL) -o $@
#
#$(LIB_BOOTSTRAP_DIR): $(LIB_BOOTSTRAP_TAR)
#	unzip $< -d $(@D)
#	mv $(@D)/bootstrap-$(LIB_BOOTSTRAP_TAG)-dist $@

#LIB_JQUERY_TAG = 3.0.0
#LIB_JQUERY_URL = https://code.jquery.com/jquery-$(LIB_JQUERY_TAG).min.js
#LIB_JQUERY_TAR = $(DL)/jquery-$(LIB_JQUERY_TAG).min.js
#LIB_JQUERY_FIL = apps-tools/assets/jquery-$(LIB_JQUERY_TAG).min.js
#
#$(LIB_JQUERY_TAR): | $(DL)
#	curl -L $(LIB_JQUERY_URL) -o $@
#
#$(LIB_JQUERY_FIL): $(LIB_JQUERY_TAR)
#	mkdir -p $@
#	cp $< $(@D)

APP_ECOSYSTEM_DIR        = apps-tools/ecosystem
APP_SCPIMANAGER_DIR      = apps-tools/scpi_manager
APP_NETWORKMANAGER_DIR   = apps-tools/network_manager
APP_UPDATER_DIR          = apps-tools/updater
APP_JUPYTERMANAGER_DIR   = apps-tools/jupyter_manager
APP_STREAMINGMANAGER_DIR = apps-tools/streaming_manager
APP_CALIB_DIR			 = apps-tools/calib_app
APP_MAIN_MENU_DIR        = apps-tools/main_menu
APP_ARB_MANAGER_DIR      = apps-tools/arb_manager

.PHONY: apps-tools ecosystem updater scpi_manager network_manager jupyter_manager streaming_manager calib_app main_menu arb_manager

apps-tools: ecosystem updater network_manager scpi_manager streaming_manager jupyter_manager calib_app main_menu arb_manager


ecosystem:
	$(MAKE) -C $(APP_ECOSYSTEM_DIR) clean
	$(MAKE) -C $(APP_ECOSYSTEM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

updater: ecosystem $(NGINX)
	$(MAKE) -C $(APP_UPDATER_DIR) clean
	$(MAKE) -C $(APP_UPDATER_DIR)
	$(MAKE) -C $(APP_UPDATER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

main_menu: ecosystem api $(NGINX)
	$(MAKE) -C $(APP_MAIN_MENU_DIR) clean
	$(MAKE) -C $(APP_MAIN_MENU_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_MAIN_MENU_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

arb_manager: ecosystem api $(NGINX)
	$(MAKE) -C $(APP_ARB_MANAGER_DIR) clean
	$(MAKE) -C $(APP_ARB_MANAGER_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_ARB_MANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

scpi_manager: ecosystem api $(NGINX)
	$(MAKE) -C $(APP_SCPIMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

streaming_manager: api $(NGINX)
	$(MAKE) -i -C $(APP_STREAMINGMANAGER_DIR) clean
	$(MAKE) -C $(APP_STREAMINGMANAGER_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_STREAMINGMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

calib_app: api  $(NGINX)
	$(MAKE) -i -C $(APP_CALIB_DIR) clean
	$(MAKE) -C $(APP_CALIB_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR)) MODEL=$(MODEL)
	$(MAKE) -C $(APP_CALIB_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR)) MODEL=$(MODEL)


network_manager: ecosystem
	$(MAKE) -C $(APP_NETWORKMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

jupyter_manager:
	$(MAKE) -C $(APP_JUPYTERMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# Red Pitaya ecosystem and free applications
################################################################################

APPS_FREE_DIR = apps-free
VNA_DIR = $(APPS_FREE_DIR)/stemlab_vna

.PHONY: apps-free

apps-free: lcr bode
	$(MAKE) -C $(APPS_FREE_DIR) clean
	$(MAKE) -C $(APPS_FREE_DIR) all INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APPS_FREE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

apps-free-vna: api2
ifeq ($(MODEL),$(filter $(MODEL),Z10))
	$(MAKE) -C $(VNA_DIR) clean
	$(MAKE) -C $(VNA_DIR) all INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(VNA_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))
endif

apps-free-clean:
	$(MAKE) -i -C $(APPS_FREE_DIR) clean
	$(MAKE) -i -C $(VNA_DIR) clean

################################################################################
# Red Pitaya PRO applications
################################################################################

APP_SCOPEGENPRO_DIR = apps-tools/scopegenpro
APP_SPECTRUMPRO_DIR = apps-tools/spectrumpro
APP_LCRMETER_DIR    = apps-tools/lcr_meter
APP_LA_PRO_DIR 		= apps-tools/la_pro
APP_BA_PRO_DIR 		= apps-tools/ba_pro

.PHONY: apps-pro scopegenpro spectrumpro lcr_meter la_pro ba_pro lcr_meter

apps-tools: scopegenpro spectrumpro la_pro ba_pro lcr_meter

scopegenpro: api $(NGINX)
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR) clean
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR)) MODEL=$(MODEL)
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

spectrumpro: api $(NGINX)
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR) clean
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR)) MODEL=$(MODEL)
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

lcr_meter: api $(NGINX)
	$(MAKE) -C $(APP_LCRMETER_DIR) clean
	$(MAKE) -C $(APP_LCRMETER_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR)) MODEL=$(MODEL)
	$(MAKE) -C $(APP_LCRMETER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

la_pro: api api2
	$(MAKE) -C $(APP_LA_PRO_DIR) clean
	$(MAKE) -C $(APP_LA_PRO_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_LA_PRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

ba_pro: api $(NGINX)
	$(MAKE) -C $(APP_BA_PRO_DIR) clean
	$(MAKE) -C $(APP_BA_PRO_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_BA_PRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))



################################################################################
#
################################################################################


clean:
	# todo, remove downloaded libraries and symlinks
	rm -rf $(abspath $(LIBRP_DIR)/build)
	rm -rf $(abspath $(LIBRP2_DIR)/build)
	rm -rf $(abspath $(LIBRP_HW_DIR)/build)
	rm -rf $(abspath $(LIBRP_HW_CAN_DIR)/build)
	rm -rf $(abspath $(LIBRP_HW_PROFILES_DIR)/build)
	rm -rf $(abspath $(LIBRP250_12_DIR)/build)
	rm -rf $(abspath $(LIBRP_DSP_DIR)/build)
	rm -rf $(abspath $(LIBRP_FORMATTER_DIR)/build)
	rm -rf $(abspath $(LIBRP_HW_CALIB_DIR)/build)
	rm -rf $(abspath $(LIBRP_ARB_DIR)/build)


	rm -rf $(abspath $(CALIB_DIR)/build)
	rm -rf $(abspath $(BODE_DIR)/build)
	rm -rf $(abspath $(ACQUIRE_DIR)/build)
	rm -rf $(abspath $(DAISY_TOOL_DIR)/build)
	rm -rf $(abspath $(GENERATOR_DIR)/build)
	rm -rf $(abspath $(LED_CONTROL_DIR)/build)
	rm -rf $(abspath $(MONITOR_DIR)/build)
	rm -rf $(abspath $(SPECTRUM_DIR)/build)
	rm -rf $(abspath $(LIBRPAPP_DIR)/build)


	$(MAKE) -C $(NGINX_DIR) clean
	$(MAKE) -C $(SCPI_SERVER_DIR) clean
	$(MAKE) -C $(COMM_DIR) clean
	$(MAKE) -C $(APP_STREAMINGMANAGER_DIR) clean
	$(MAKE) -C $(APP_MAIN_MENU_DIR) clean

	$(MAKE) -C $(APP_ARB_MANAGER_DIR) clean
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR) clean
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR) clean
	$(MAKE) -C $(APP_LCRMETER_DIR) clean
	$(MAKE) -C $(APP_LA_PRO_DIR) clean
	$(MAKE) -C $(APP_BA_PRO_DIR) clean
	$(MAKE) -C $(IDGEN_DIR) clean


