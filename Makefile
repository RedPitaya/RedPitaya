# check if download cache directory is available
DL ?= dl

INSTALL_DIR ?= build
ENABLE_LICENSING ?= 1
################################################################################
# versioning system
################################################################################

VER := $(shell cat apps-tools/ecosystem/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
BUILD_NUMBER ?= 0
REVISION ?= $(shell git rev-parse --short HEAD)
VERSION = $(VER)-$(BUILD_NUMBER)-$(REVISION)
LINUX_VER = 1.03
export BUILD_NUMBER
export REVISION
export VERSION
export LINUX_VER
################################################################################
#
################################################################################

# MODEL USE FOR determine kind of assembly
# USED parameters:
# Z10 - for Redpitaya 125-14
# Z20 - for Redpitaya 122-16
# Z20_125 - for Redpitaya Z20 125-14
# Z20_250_12 - for RepPitaya 250-12
# Production test script
MODEL ?= Z10
ENABLE_PRODUCTION_TEST ?= 0

all: api nginx examples  apps-tools apps-pro  production_test startupsh scpi

ifeq ($(MODEL),Z20_250_12)
all: 
else
all: sdr apps-free-vna rp_communication
endif

$(DL):
	mkdir -p $@

$(INSTALL_DIR):
	mkdir -p $@

################################################################################
# API libraries
################################################################################

LIBRP_DIR       = api
LIBRP2_DIR      = api2
LIBRP250_12_DIR = api-250-12
LIBRPLCR_DIR	= Applications/api/rpApplications/lcr_meter
LIBRPAPP_DIR    = Applications/api/rpApplications
ECOSYSTEM_DIR   = Applications/ecosystem

.PHONY: api api2 librp librp1 librp250_12
.PHONY: librpapp liblcr_meter

api: librp

api2: librp2

ifeq ($(MODEL),Z20_250_12)
librp: librp250_12
else
librp:
endif
	$(MAKE) -C $(LIBRP_DIR) clean
	$(MAKE) -C $(LIBRP_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(LIBRP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

librp1:
	$(MAKE) -C $(LIBRP1_DIR) clean
	$(MAKE) -C $(LIBRP1_DIR)
	$(MAKE) -C $(LIBRP1_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

librp2:
	$(MAKE) -C $(LIBRP2_DIR) clean
	$(MAKE) -C $(LIBRP2_DIR)
	$(MAKE) -C $(LIBRP2_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

librp250_12:
	$(MAKE) -C $(LIBRP250_12_DIR) clean
	$(MAKE) -C $(LIBRP250_12_DIR)
	$(MAKE) -C $(LIBRP250_12_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

ifeq ($(ENABLE_LICENSING),1)

api: librpapp liblcr_meter

librpapp: librp
	$(MAKE) -C $(LIBRPAPP_DIR) clean
	$(MAKE) -C $(LIBRPAPP_DIR)
	$(MAKE) -C $(LIBRPAPP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

liblcr_meter: librp
	$(MAKE) -C $(LIBRPLCR_DIR) clean
	$(MAKE) -C $(LIBRPLCR_DIR)
	$(MAKE) -C $(LIBRPLCR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))
endif


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

WEBSOCKETPP_TAG = 0.7.0
LUANGINX_TAG    = v0.10.7
NGINX_TAG       = 1.11.4
SOCKPROC_TAG    = master

WEBSOCKETPP_URL = https://github.com/zaphoyd/websocketpp/archive/$(WEBSOCKETPP_TAG).tar.gz
LIBJSON_URL     = http://sourceforge.net/projects/libjson/files/libjson_7.6.1.zip
LUANGINX_URL    = https://codeload.github.com/openresty/lua-nginx-module/tar.gz/$(LUANGINX_TAG)
NGINX_URL       = http://nginx.org/download/nginx-$(NGINX_TAG).tar.gz
SOCKPROC_URL	= https://github.com/juce/sockproc/archive/$(SOCKPROC_TAG).tar.gz

WEBSOCKETPP_TAR = $(DL)/websocketpp-$(WEBSOCKETPP_TAG).tar.gz
LIBJSON_TAR     = $(DL)/libjson_7.6.1.zip
LUANGINX_TAR    = $(DL)/lua-nginx-module-$(LUANGINX_TAG).tr.gz
NGINX_TAR       = $(DL)/nginx-$(NGINX_TAG).tar.gz
SOCKPROC_TAR	= $(DL)/sockproc-$(SOCKPROC_TAG).tar.gz

WEBSOCKETPP_DIR = Bazaar/nginx/ngx_ext_modules/ws_server/websocketpp
LIBJSON_DIR     = Bazaar/tools/libjson
LUANGINX_DIR    = Bazaar/nginx/ngx_ext_modules/lua-nginx-module
NGINX_SRC_DIR   = Bazaar/nginx/nginx
SOCKPROC_DIR    = Bazaar/tools/sockproc

.PHONY: ecosystem nginx

$(WEBSOCKETPP_TAR): | $(DL)
	wget $(WEBSOCKETPP_URL) -O $@ --show-progress

$(WEBSOCKETPP_DIR): $(WEBSOCKETPP_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	patch -d $@ -p1 < patches/websocketpp-$(WEBSOCKETPP_TAG).patch

$(SOCKPROC_TAR): | $(DL)
	wget $(SOCKPROC_URL) -O $@ --show-progress

$(SOCKPROC_DIR): $(SOCKPROC_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

$(LIBJSON_TAR): | $(DL)
	wget $(LIBJSON_URL) -O $@ --show-progress

$(LIBJSON_DIR): $(LIBJSON_TAR)
	mkdir -p $@
	unzip $< -d $(@D)
	patch -d $@ -p1 < patches/libjson.patch

$(LUANGINX_TAR): | $(DL)
	wget $(LUANGINX_URL) -O $@ --show-progress

$(LUANGINX_DIR): $(LUANGINX_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

$(NGINX_TAR): | $(DL)
	wget $(NGINX_URL) -O $@ --show-progress

$(NGINX_SRC_DIR): $(NGINX_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	cp -f apps-tools/nginx.conf $@/conf/
	mkdir $@/conf/lua/
	cp -fr patches/lua/* $@/conf/lua/

$(NGINX): $(CRYPTOPP_DIR) $(WEBSOCKETPP_DIR) $(LIBJSON_DIR) $(LUANGINX_DIR) $(NGINX_SRC_DIR)
	$(MAKE) -C $(NGINX_DIR) clean
	$(MAKE) -C $(NGINX_DIR)
	$(MAKE) -C $(NGINX_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))
	mkdir -p $(INSTALL_DIR)/www/conf/lua
	cp -fr $(NGINX_DIR)/nginx/conf/lua/* $(abspath $(INSTALL_DIR))/www/conf/lua

ifeq ($(ENABLE_LICENSING),1)

IDGEN_DIR = Applications/idgen

$(IDGEN):
	$(MAKE) -C $(IDGEN_DIR) clean
	$(MAKE) -C $(IDGEN_DIR)
	$(MAKE) -C $(IDGEN_DIR) install DESTDIR=$(abspath $(INSTALL_DIR))

else

$(IDGEN):
	touch $(IDGEN)

endif

$(SOCKPROC): $(SOCKPROC_DIR)
	$(MAKE) -C $<
	test -d $(INSTALL_DIR)/sbin || mkdir -p $(INSTALL_DIR)/sbin
	cp $</sockproc $@

nginx: $(NGINX) $(IDGEN) $(SOCKPROC)

startupsh:
ifeq ($(MODEL),Z20_250_12)
	cp -f patches/startup/startup.sh.Z250_12 $(STARTUPSH)
else
	cp -f patches/startup/startup.sh $(STARTUPSH)
endif


################################################################################
# SCPI server
################################################################################

SCPI_PARSER_TAG = fb6979d1926bb6813898012de934eca366d93ff8
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
SDR_ZIP = stemlab_sdr_transceiver_hpsdr-0.94-1656.zip
SDR_URL = http://downloads.redpitaya.com/hamlab/charly25ab/$(SDR_ZIP)

sdr: | $(DL)
ifeq ($(MODEL),$(filter $(MODEL),Z10 Z20_125))  
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
MONITOR_OLD_DIR    = Test/monitor_old
ACQUIRE_DIR        = Test/acquire
ACQUIRE2_DIR       = Test/acquire2
CALIB_DIR          = Test/calib
CALIBRATE_DIR      = Test/calibrate
GENERATOR_DIR	   = Test/generate
SPECTRUM_DIR       = Test/spectrum
COMM_DIR           = Examples/Communication/C
XADC_DIR           = Test/xadc
LA_TEST_DIR        = api2/test
GENERATE_DC_DIR    = Test/generate_DC

.PHONY: examples rp_communication
.PHONY: lcr bode monitor generator acquire calib calibrate spectrum laboardtest
.PHONY: acquire2

examples: lcr bode monitor calib generate_DC spectrum acquire2 generator

ifeq ($(MODEL),Z20_250_12)
examples: rp_i2c_tool
endif
# calibrate laboardtest


rp_i2c_tool:
	$(MAKE) -C $(LIBRP250_12_DIR) clean
	$(MAKE) -C $(LIBRP250_12_DIR) tool
	$(MAKE) -C $(LIBRP250_12_DIR) install_tool INSTALL_DIR=$(abspath $(INSTALL_DIR))

# lcr:
# 	$(MAKE) -C $(LCR_DIR) clean
# 	$(MAKE) -C $(LCR_DIR) MODEL=$(MODEL)
#	$(MAKE) -C $(LCR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

bode: api
	$(MAKE) -C $(BODE_DIR) clean
	$(MAKE) -C $(BODE_DIR) MODEL=$(MODEL) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(BODE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

monitor:
	$(MAKE) -C $(MONITOR_DIR) clean
	$(MAKE) -C $(MONITOR_DIR)
	$(MAKE) -C $(MONITOR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

monitor_old:
	$(MAKE) -C $(MONITOR_OLD_DIR) clean
	$(MAKE) -C $(MONITOR_OLD_DIR)
	$(MAKE) -C $(MONITOR_OLD_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

generator: api
	$(MAKE) -C $(GENERATOR_DIR) clean 
	$(MAKE) -C $(GENERATOR_DIR) MODEL=$(MODEL) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(GENERATOR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

# remove old tool
#acquire: api
#	$(MAKE) -C $(ACQUIRE_DIR) MODEL=$(MODEL)
#	$(MAKE) -C $(ACQUIRE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

acquire2: api
	$(MAKE) -C $(ACQUIRE2_DIR) MODEL=$(MODEL) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(ACQUIRE2_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

calib:
	$(MAKE) -C $(CALIB_DIR) clean
	$(MAKE) -C $(CALIB_DIR) MODEL=$(MODEL)
	$(MAKE) -C $(CALIB_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

spectrum: api
	$(MAKE) -C $(SPECTRUM_DIR) clean
	$(MAKE) -C $(SPECTRUM_DIR) MODEL=$(MODEL) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(SPECTRUM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

calibrate: api
	$(MAKE) -C $(CALIBRATE_DIR) clean
	$(MAKE) -C $(CALIBRATE_DIR)
	$(MAKE) -C $(CALIBRATE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

laboardtest: api2
	$(MAKE) -C $(LA_TEST_DIR) clean
	$(MAKE) -C $(LA_TEST_DIR)
	cp api2/test/laboardtest build/bin/laboardtest
	cp api2/test/install.sh build/install.sh
rp_communication:
	make -C $(COMM_DIR)

generate_DC: api
ifeq ($(ENABLE_PRODUCTION_TEST), 1)
	$(MAKE) -C $(GENERATE_DC_DIR) MODEL=$(MODEL) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	cp $(GENERATE_DC_DIR)/generate_DC $(INSTALL_DIR)/bin/
	cp $(GENERATE_DC_DIR)/generate_DC_LO $(INSTALL_DIR)/bin/
endif

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

.PHONY: apps-tools ecosystem updater scpi_manager network_manager jupyter_manager streaming_manager calib_app

apps-tools: ecosystem updater network_manager scpi_manager streaming_manager

ifeq ($(MODEL),Z20_250_12)
apps-tools: calib_app
endif

ifeq ($(MODEL),$(filter $(MODEL),Z10 Z20_125))
apps-tools: jupyter_manager calib_app
endif

ifeq ($(MODEL),Z20)
apps-tools: jupyter_manager
endif

ecosystem:
	$(MAKE) -C $(APP_ECOSYSTEM_DIR) clean
	$(MAKE) -C $(APP_ECOSYSTEM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

updater: ecosystem api $(NGINX)
	$(MAKE) -C $(APP_UPDATER_DIR) clean
	$(MAKE) -C $(APP_UPDATER_DIR)
	$(MAKE) -C $(APP_UPDATER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

scpi_manager: ecosystem api $(NGINX)
	$(MAKE) -C $(APP_SCPIMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

streaming_manager: api $(NGINX)
	$(MAKE) -i -C $(APP_STREAMINGMANAGER_DIR) clean
	$(MAKE) -C $(APP_STREAMINGMANAGER_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR)) MODEL=$(MODEL)
	$(MAKE) -C $(APP_STREAMINGMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR)) MODEL=$(MODEL)

calib_app: api $(NGINX)
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
ifeq ($(MODEL),$(filter $(MODEL),Z10 Z20_125))
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

ifeq ($(ENABLE_LICENSING),1)

APP_SCOPEGENPRO_DIR = Applications/scopegenpro
APP_SPECTRUMPRO_DIR = Applications/spectrumpro
APP_LCRMETER_DIR    = Applications/lcr_meter
APP_LA_PRO_DIR 		= Applications/la_pro
APP_BA_PRO_DIR 		= Applications/ba_pro

.PHONY: apps-pro scopegenpro spectrumpro lcr_meter la_pro ba_pro lcr_meter

apps-pro: scopegenpro spectrumpro 
ifeq ($(MODEL),Z20_250_12)
apps-pro: ba_pro lcr_meter
else
ifeq ($(MODEL),Z20)
apps-pro:
else
apps-pro: la_pro ba_pro lcr_meter
endif
endif

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

la_pro: api api2 $(NGINX)
	$(MAKE) -C $(APP_LA_PRO_DIR) clean
	$(MAKE) -C $(APP_LA_PRO_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_LA_PRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

ba_pro: api $(NGINX)
	$(MAKE) -C $(APP_BA_PRO_DIR) clean
	$(MAKE) -C $(APP_BA_PRO_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_BA_PRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

else

apps-pro:

endif


################################################################################
#
################################################################################

PRODUCTION_TEST_DIR = Test/production

.PHONY: production_test

production_test:
ifeq ($(ENABLE_PRODUCTION_TEST), 1)
	$(MAKE) -C $(PRODUCTION_TEST_DIR) clean
	$(MAKE) -C $(PRODUCTION_TEST_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR)) MODEL=$(MODEL)
	$(MAKE) -C $(PRODUCTION_TEST_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR)) MODEL=$(MODEL)
endif

clean:
	# todo, remove downloaded libraries and symlinks
	make -C $(NGINX_DIR) clean
	make -C $(MONITOR_DIR) clean
	make -C $(MONITOR_OLD_DIR) clean
	make -C $(GENERATOR_DIR) clean
	make -C $(ACQUIRE_DIR) clean
	make -C $(GENERATOR250_DIR) clean
	make -C $(ACQUIRE250_DIR) clean
	make -C $(CALIB_DIR) clean
	make -C $(SCPI_SERVER_DIR) clean
	make -C $(LIBRP250_12_DIR)    clean
	make -C $(LIBRP2_DIR)    clean
	make -C $(LIBRP_DIR)    clean
	make -C $(LIBRPAPP_DIR) clean
	make -C $(LIBRPLCR_DIR) clean
	make -C $(COMM_DIR) clean
	make -C $(GENERATE_DC_DIR) clean
	make -C $(PRODUCTION_TEST_DIR) clean
	apps-free-clean
