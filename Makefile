# check if download cache directory is available
DL ?= dl

INSTALL_DIR ?= build
STREAMING ?= MASTER

CPU_CORES = $(shell ./get_cpu_ram.sh)

################################################################################
# versioning system
################################################################################

VER := $(shell cat apps-tools/ecosystem/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
BUILD_NUMBER ?= 0
REVISION ?= $(shell git rev-parse --short HEAD)
VERSION = $(VER)-$(BUILD_NUMBER)
LINUX_VER = 2.05
export BUILD_NUMBER
export REVISION
export VERSION
export LINUX_VER
BUILD_MODE ?= Release
VERBOSE = OFF
CMAKEVAR=-DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(BUILD_MODE)  -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DCMAKE_VERBOSE_MAKEFILE:BOOL=$(VERBOSE)
################################################################################
#
################################################################################

CUR_DIR = $(PWD)

all: api nginx examples  apps-tools apps-pro startupsh scpi rp_communication sdr


$(DL):
	test -d $@ || mkdir -p $@

$(INSTALL_DIR):
	test -d $@ || mkdir -p $@

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
LIBRP_SWEEP_DIR  		= rp-api/api-sweep
LIBRPAPP_DIR    		= rp-api/api-app
LIBRP_FORMATTER_DIR   	= rp-api/api-formatter
LIBRP_ARB_DIR		   	= rp-api/api-arb
ECOSYSTEM_DIR   		= Applications/ecosystem

.PHONY: api api2 librp librp250_12 librp_hw librp_dsp librp_hw_profiles librp_hw_calibration librp_hw_can librparb librp_sweep librpapp

api: librp librp_hw librp_hw_can librp_dsp librpapp librp_formatter librparb librp_sweep

api2: librp2

librp: librp250_12 librp_hw_calibration librp_hw_profiles
	cmake -B$(abspath $(LIBRP_DIR)/build) -S$(abspath $(LIBRP_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_DIR)/build install -j$(CPU_CORES)

librp_hw:
	cmake -B$(abspath $(LIBRP_HW_DIR)/build) -S$(abspath $(LIBRP_HW_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_HW_DIR)/build install -j$(CPU_CORES)

librp_hw_can: librp
	cmake -B$(abspath $(LIBRP_HW_CAN_DIR)/build) -S$(abspath $(LIBRP_HW_CAN_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_HW_CAN_DIR)/build install -j$(CPU_CORES)

librp_hw_calibration: librp_hw_profiles
	cmake -B$(abspath $(LIBRP_HW_CALIB_DIR)/build) -S$(abspath $(LIBRP_HW_CALIB_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_HW_CALIB_DIR)/build install -j$(CPU_CORES)

librp_hw_profiles:
	cmake -B$(abspath $(LIBRP_HW_PROFILES_DIR)/build) -S$(abspath $(LIBRP_HW_PROFILES_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_HW_PROFILES_DIR)/build install -j$(CPU_CORES)

librp_dsp: librp
	cmake -B$(abspath $(LIBRP_DSP_DIR)/build) -S$(abspath $(LIBRP_DSP_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_DSP_DIR)/build install -j$(CPU_CORES)

librp_sweep: librp
	cmake -B$(abspath $(LIBRP_SWEEP_DIR)/build) -S$(abspath $(LIBRP_SWEEP_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_SWEEP_DIR)/build install -j$(CPU_CORES)

librp_formatter: librp
	cmake -B$(abspath $(LIBRP_FORMATTER_DIR)/build) -S$(abspath $(LIBRP_FORMATTER_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_FORMATTER_DIR)/build install -j$(CPU_CORES)

librp2:
	cmake -B$(abspath $(LIBRP2_DIR)/build) -S$(abspath $(LIBRP2_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP2_DIR)/build install -j$(CPU_CORES)

librp250_12: librp_hw
	cmake -B$(LIBRP250_12_DIR)/build -S$(LIBRP250_12_DIR) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP250_12_DIR)/build install -j$(CPU_CORES)

librpapp: librp librp_dsp
	cmake -B$(abspath $(LIBRPAPP_DIR)/build) -S$(abspath $(LIBRPAPP_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRPAPP_DIR)/build install -j$(CPU_CORES)

librparb: librp
	cmake -B$(abspath $(LIBRP_ARB_DIR)/build) -S$(abspath $(LIBRP_ARB_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_ARB_DIR)/build install -j$(CPU_CORES)


################################################################################
# Web API libraries
################################################################################

LIBRP_SYSTEM_DIR	= rp-web-api/rp-system
LIBRP_CLIENT_DIR	= rp-web-api/rp-client

.PHONY: librpsystem librpclient

web-api: librpsystem librpclient

librpsystem: api nginx
	cmake -B$(abspath $(LIBRP_SYSTEM_DIR)/build) -S$(abspath $(LIBRP_SYSTEM_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_SYSTEM_DIR)/build install -j$(CPU_CORES)

librpclient: api nginx
	cmake -B$(abspath $(LIBRP_CLIENT_DIR)/build) -S$(abspath $(LIBRP_CLIENT_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LIBRP_CLIENT_DIR)/build install -j$(CPU_CORES)

################################################################################
# Red Pitaya ecosystem
################################################################################

# directories
NGINX_DIR       = Bazaar/nginx

# targets
NGINX           = $(INSTALL_DIR)/sbin/nginx
IDGEN           = $(INSTALL_DIR)/sbin/idgen
SOCKPROC        = $(INSTALL_DIR)/sbin/sockproc

WEBSOCKETPP_TAG = 0.8.2
LUANGINX_TAG    = v0.10.21
LUARESTY_TAG    = v0.1.23
LUARESTY_L_TAG  = v0.13
NGINX_TAG       = 1.19.10
SOCKPROC_TAG    = master

WEBSOCKETPP_URL = https://github.com/zaphoyd/websocketpp/archive/$(WEBSOCKETPP_TAG).tar.gz
LIBJSON_URL     = https://github.com/RedPitaya/libjson/archive/refs/tags/7.6.3.tar.gz
LUANGINX_URL    = https://codeload.github.com/openresty/lua-nginx-module/tar.gz/$(LUANGINX_TAG)
LUARESTY_URL    = https://github.com/openresty/lua-resty-core/archive/refs/tags/$(LUARESTY_TAG).tar.gz
LUARESTY_L_URL  = https://github.com/openresty/lua-resty-lrucache/archive/refs/tags/$(LUARESTY_L_TAG).tar.gz
NGINX_URL       = http://nginx.org/download/nginx-$(NGINX_TAG).tar.gz
SOCKPROC_URL	= https://github.com/juce/sockproc/archive/$(SOCKPROC_TAG).tar.gz

WEBSOCKETPP_TAR = $(DL)/websocketpp-$(WEBSOCKETPP_TAG).tar.gz
LIBJSON_TAR     = $(DL)/libjson_7.6.3.zip
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

$(WEBSOCKETPP_TAR): $(DL)
	wget $(WEBSOCKETPP_URL) -O $@ --show-progress

$(WEBSOCKETPP_DIR): $(WEBSOCKETPP_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

$(SOCKPROC_TAR): $(DL)
	wget $(SOCKPROC_URL) -O $@ --show-progress

$(SOCKPROC_DIR): $(SOCKPROC_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

$(LIBJSON_TAR): $(DL)
	wget $(LIBJSON_URL) -O $@ --show-progress

$(LIBJSON_DIR): $(LIBJSON_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	patch -d $@ -p1 < patches/libjson.patch

$(LUANGINX_TAR): $(DL)
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
	wget -c $(NGINX_URL) -O $@ --show-progress

$(NGINX_SRC_DIR): $(NGINX_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	cp -f apps-tools/nginx.conf $@/conf/
	mkdir -p $@/conf/lua/
	cp -fr patches/lua/* $@/conf/lua/

$(NGINX):  $(CRYPTOPP_DIR) $(WEBSOCKETPP_DIR) $(LIBJSON_DIR) $(LUARESTY_DIR) $(LUARESTY_L_DIR) $(LUANGINX_DIR) $(NGINX_SRC_DIR)
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

nginx_clean:
	$(MAKE) -C $(NGINX_DIR) clean
	rm -rf $(WEBSOCKETPP_TAR)
	rm -rf $(LIBJSON_TAR)
	rm -rf $(LUANGINX_TAR)
	rm -rf $(LUARESTY_TAR)
	rm -rf $(LUARESTY_L_TAR)
	rm -rf $(NGINX_TAR)
	rm -rf $(SOCKPROC_TAR)


################################################################################
# SCPI server
################################################################################

SCPI_PARSER_TAG = redpitaya

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
	$(MAKE) -i -C $(SCPI_SERVER_DIR) clean
	$(MAKE) -C $(SCPI_SERVER_DIR) MODEL=$(MODEL) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(SCPI_SERVER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

scpi_clean:
	rm -rf $(SCPI_PARSER_TAR)
	$(MAKE) -i -C $(SCPI_SERVER_DIR) clean

################################################################################
# SDR
################################################################################

.PHONY: sdr

SDR_ZIP = SDR-bundle-59-1ba76002.zip
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
ACQUIRE_P_DIR      = Test/acquire_p
CALIB_DIR          = Test/calib
GENERATOR_DIR	   = Test/generate
SPECTRUM_DIR       = Test/spectrum
LED_CONTROL_DIR    = Test/led_control
COMM_DIR           = Examples/Communication/C
XADC_DIR           = Test/xadc
LA_TEST_DIR        = rp-api/api2/test
DAISY_TOOL_DIR     = Test/daisy_tool
FPGA_TESTS_DIR     = Examples/Tests
STARTUPSH          = $(INSTALL_DIR)/sbin/startup.sh

.PHONY: examples rp_communication fpgautils
.PHONY: lcr bode monitor generator acquire acquire_p calib laboardtest spectrum led_control daisy_tool fpga_tests

examples: lcr bode monitor calib spectrum acquire acquire_p generator led_control fpgautils daisy_tool fpga_tests


lcr: api
	cmake -B$(abspath $(LCR_DIR)/build) -S$(abspath $(LCR_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LCR_DIR)/build install -j$(CPU_CORES)

bode: api
	cmake -B$(abspath $(BODE_DIR)/build) -S$(abspath $(BODE_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(BODE_DIR)/build install -j$(CPU_CORES)

monitor: api
	cmake -B$(abspath $(MONITOR_DIR)/build) -S$(abspath $(MONITOR_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(MONITOR_DIR)/build install -j$(CPU_CORES)

generator: api
	cmake -B$(abspath $(GENERATOR_DIR)/build) -S$(abspath $(GENERATOR_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(GENERATOR_DIR)/build install -j$(CPU_CORES)

acquire: api
	cmake -B$(abspath $(ACQUIRE_DIR)/build) -S$(abspath $(ACQUIRE_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(ACQUIRE_DIR)/build install -j$(CPU_CORES)

acquire_p: api
	cmake -B$(abspath $(ACQUIRE_P_DIR)/build) -S$(abspath $(ACQUIRE_P_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(ACQUIRE_P_DIR)/build install -j$(CPU_CORES)

calib: api
	cmake -B$(abspath $(CALIB_DIR)/build) -S$(abspath $(CALIB_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(CALIB_DIR)/build install -j$(CPU_CORES)

daisy_tool: api
	cmake -B$(abspath $(DAISY_TOOL_DIR)/build) -S$(abspath $(DAISY_TOOL_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(DAISY_TOOL_DIR)/build install -j$(CPU_CORES)

spectrum: api
	cmake -B$(abspath $(SPECTRUM_DIR)/build) -S$(abspath $(SPECTRUM_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(SPECTRUM_DIR)/build install -j$(CPU_CORES)

led_control: api
	cmake -B$(abspath $(LED_CONTROL_DIR)/build) -S$(abspath $(LED_CONTROL_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(LED_CONTROL_DIR)/build install -j$(CPU_CORES)

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

startupsh:
	cp -f patches/startup/startup.sh $(STARTUPSH)

################################################################################
# Red Pitaya ecosystem and tools
################################################################################


APP_ECOSYSTEM_DIR        = apps-tools/ecosystem
APP_SCPIMANAGER_DIR      = apps-tools/scpi_manager
APP_NETWORKMANAGER_DIR   = apps-tools/network_manager
APP_UPDATER_DIR          = apps-tools/updater
APP_JUPYTERMANAGER_DIR   = apps-tools/jupyter_manager
APP_STREAMINGMANAGER_DIR = apps-tools/streaming_manager
APP_CALIB_DIR			 = apps-tools/calib_app
APP_MAIN_MENU_DIR        = apps-tools/main_menu
APP_ARB_MANAGER_DIR      = apps-tools/arb_manager

.PHONY: apps-tools ecosystem updater scpi_manager network_manager jupyter_manager streaming_manager calib_app main_menu arb_manager $(NGINX)


apps-tools: ecosystem updater network_manager scpi_manager streaming_manager jupyter_manager calib_app main_menu arb_manager


ecosystem:
	$(MAKE) -C $(APP_ECOSYSTEM_DIR) clean
	$(MAKE) -C $(APP_ECOSYSTEM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

updater: ecosystem web-api $(NGINX)
	cmake -B$(abspath $(APP_UPDATER_DIR)/build) -S$(abspath $(APP_UPDATER_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_UPDATER_DIR)/build install -j$(CPU_CORES)

main_menu: ecosystem web-api api $(NGINX)
	cmake -B$(abspath $(APP_MAIN_MENU_DIR)/build) -S$(abspath $(APP_MAIN_MENU_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_MAIN_MENU_DIR)/build install -j$(CPU_CORES)

arb_manager: ecosystem web-api api $(NGINX)
	cmake -B$(abspath $(APP_ARB_MANAGER_DIR)/build) -S$(abspath $(APP_ARB_MANAGER_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_ARB_MANAGER_DIR)/build install -j$(CPU_CORES)

scpi_manager: ecosystem web-api api $(NGINX)
	cmake -B$(abspath $(APP_SCPIMANAGER_DIR)/build) -S$(abspath $(APP_SCPIMANAGER_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_SCPIMANAGER_DIR)/build install -j$(CPU_CORES)

streaming_manager: web-api api $(NGINX)
	$(MAKE) -i -C $(APP_STREAMINGMANAGER_DIR) clean
	$(MAKE) -C $(APP_STREAMINGMANAGER_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_STREAMINGMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

calib_app: web-api api  $(NGINX)
	cmake -B$(abspath $(APP_CALIB_DIR)/build) -S$(abspath $(APP_CALIB_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_CALIB_DIR)/build install -j$(CPU_CORES)

network_manager: ecosystem
	cmake -B$(abspath $(APP_NETWORKMANAGER_DIR)/build) -S$(abspath $(APP_NETWORKMANAGER_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_NETWORKMANAGER_DIR)/build install -j$(CPU_CORES)

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
# Red Pitaya applications
################################################################################

APP_SCOPEGENPRO_DIR = apps-tools/scopegenpro
APP_SPECTRUMPRO_DIR = apps-tools/spectrumpro
APP_LCRMETER_DIR    = apps-tools/lcr_meter
APP_LA_PRO_DIR 		= apps-tools/la_pro
APP_BA_PRO_DIR 		= apps-tools/ba_pro
APP_IMP_ANAL_DIR 	= apps-tools/impedance_analyzer

.PHONY: apps-pro scopegenpro spectrumpro lcr_meter la_pro ba_pro lcr_meter impedance_analyzer

apps-tools: scopegenpro spectrumpro la_pro ba_pro lcr_meter impedance_analyzer

scopegenpro: web-api api $(NGINX)
	cmake -B$(abspath $(APP_SCOPEGENPRO_DIR)/build) -S$(abspath $(APP_SCOPEGENPRO_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR)/build install -j$(CPU_CORES)

spectrumpro: web-api api $(NGINX)
	cmake -B$(abspath $(APP_SPECTRUMPRO_DIR)/build) -S$(abspath $(APP_SPECTRUMPRO_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR)/build install -j$(CPU_CORES)

lcr_meter: web-api api $(NGINX)
	cmake -B$(abspath $(APP_LCRMETER_DIR)/build) -S$(abspath $(APP_LCRMETER_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_LCRMETER_DIR)/build install -j$(CPU_CORES)

la_pro: web-api api api2 $(NGINX)
	cmake -B$(abspath $(APP_LA_PRO_DIR)/build) -S$(abspath $(APP_LA_PRO_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_LA_PRO_DIR)/build install -j$(CPU_CORES)

ba_pro: web-api api $(NGINX)
	cmake -B$(abspath $(APP_BA_PRO_DIR)/build) -S$(abspath $(APP_BA_PRO_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_BA_PRO_DIR)/build install -j$(CPU_CORES)

impedance_analyzer: web-api api $(NGINX)
	cmake -B$(abspath $(APP_IMP_ANAL_DIR)/build) -S$(abspath $(APP_IMP_ANAL_DIR)) $(CMAKEVAR)
	$(MAKE) -C $(APP_IMP_ANAL_DIR)/build install -j$(CPU_CORES)


################################################################################
#
################################################################################


clean: nginx_clean scpi_clean

	rm -rf $(abspath $(LIBRP_DIR)/build)
	rm -rf $(abspath $(LIBRP2_DIR)/build)
	rm -rf $(abspath $(LIBRP_HW_DIR)/build)
	rm -rf $(abspath $(LIBRP_HW_CAN_DIR)/build)
	rm -rf $(abspath $(LIBRP_HW_PROFILES_DIR)/build)
	rm -rf $(abspath $(LIBRP250_12_DIR)/build)
	rm -rf $(abspath $(LIBRP_DSP_DIR)/build)
	rm -rf $(abspath $(LIBRP_SWEEP_DIR)/build)
	rm -rf $(abspath $(LIBRP_FORMATTER_DIR)/build)
	rm -rf $(abspath $(LIBRP_HW_CALIB_DIR)/build)
	rm -rf $(abspath $(LIBRP_ARB_DIR)/build)

	rm -rf $(abspath $(LIBRP_SYSTEM_DIR)/build)
	rm -rf $(abspath $(LIBRP_CLIENT_DIR)/build)

	rm -rf $(abspath $(LCR_DIR)/build)
	rm -rf $(abspath $(CALIB_DIR)/build)
	rm -rf $(abspath $(BODE_DIR)/build)
	rm -rf $(abspath $(ACQUIRE_DIR)/build)
	rm -rf $(abspath $(ACQUIRE_P_DIR)/build)
	rm -rf $(abspath $(DAISY_TOOL_DIR)/build)
	rm -rf $(abspath $(GENERATOR_DIR)/build)
	rm -rf $(abspath $(LED_CONTROL_DIR)/build)
	rm -rf $(abspath $(MONITOR_DIR)/build)
	rm -rf $(abspath $(SPECTRUM_DIR)/build)
	rm -rf $(abspath $(LIBRPAPP_DIR)/build)

	rm -rf $(abspath $(APP_ARB_MANAGER_DIR)/build)
	rm -rf $(abspath $(APP_BA_PRO_DIR)/build)
	rm -rf $(abspath $(APP_CALIB_DIR)/build)
	rm -rf $(abspath $(APP_IMP_ANAL_DIR)/build)
	rm -rf $(abspath $(APP_LA_PRO_DIR)/build)
	rm -rf $(abspath $(APP_LCRMETER_DIR)/build)
	rm -rf $(abspath $(APP_MAIN_MENU_DIR)/build)
	rm -rf $(abspath $(APP_NETWORKMANAGER_DIR)/build)
	rm -rf $(abspath $(APP_SCOPEGENPRO_DIR)/build)
	rm -rf $(abspath $(APP_SCPIMANAGER_DIR)/build)
	rm -rf $(abspath $(APP_SPECTRUMPRO_DIR)/build)
	rm -rf $(abspath $(APP_UPDATER_DIR)/build)

	$(MAKE) -C $(NGINX_DIR) clean
	$(MAKE) -C $(SCPI_SERVER_DIR) clean
	$(MAKE) -C $(COMM_DIR) clean
	$(MAKE) -C $(APP_STREAMINGMANAGER_DIR) clean
	$(MAKE) -C $(IDGEN_DIR) clean
	$(MAKE) -C $(FPGA_TESTS_DIR) clean

	rm -rf $(DL)









