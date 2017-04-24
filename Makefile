# check if download cache directory is available
DL ?= dl

INSTALL_DIR ?= build

################################################################################
# versioning system
################################################################################

VER := $(shell cat apps-tools/ecosystem/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')
BUILD_NUMBER ?= 0
REVISION ?= $(shell git rev-parse --short HEAD)
VERSION = $(VER)-$(BUILD_NUMBER)-$(REVISION)
export BUILD_NUMBER
export REVISION
export VERSION

################################################################################
#
################################################################################

all:  sdr api api1 nginx scpi examples rp_communication apps-tools apps-pro

$(DL):
	mkdir -p $@

$(INSTALL_DIR):
	mkdir -p $@

################################################################################
# API libraries
################################################################################

LIBRP_DIR       = api/rpbase
LIBRP1_DIR      = api1
LIBRP2_DIR      = api2
LIBRPLCR_DIR	= Applications/api/rpApplications/lcr_meter
LIBRPAPP_DIR    = Applications/api/rpApplications
ECOSYSTEM_DIR   = Applications/ecosystem

.PHONY: api api1 api2 librp librp1
.PHONY: librpapp liblcr_meter

librp:
	$(MAKE) -C $(LIBRP_DIR) clean
	$(MAKE) -C $(LIBRP_DIR)
	$(MAKE) -C $(LIBRP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

librp1:
	$(MAKE) -C $(LIBRP1_DIR) clean
	$(MAKE) -C $(LIBRP1_DIR)
	$(MAKE) -C $(LIBRP1_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

librp2:
	$(MAKE) -C $(LIBRP2_DIR) clean
	$(MAKE) -C $(LIBRP2_DIR)
	$(MAKE) -C $(LIBRP2_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))


ifdef ENABLE_LICENSING

api: librp librpapp liblcr_meter

librpapp:
	$(MAKE) -C $(LIBRPAPP_DIR) clean
	$(MAKE) -C $(LIBRPAPP_DIR)
	$(MAKE) -C $(LIBRPAPP_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

liblcr_meter:
	$(MAKE) -C $(LIBRPLCR_DIR) clean
	$(MAKE) -C $(LIBRPLCR_DIR)
	$(MAKE) -C $(LIBRPLCR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

else

api: librp

endif

api1: librp1

api2: librp2

################################################################################
# Red Pitaya ecosystem
################################################################################

# directories
NGINX_DIR       = Bazaar/nginx

# targets
NGINX           = $(INSTALL_DIR)/sbin/nginx
IDGEN           = $(INSTALL_DIR)/sbin/idgen
SOCKPROC        = $(INSTALL_DIR)/sbin/sockproc

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
	curl -L $(WEBSOCKETPP_URL) -o $@

$(WEBSOCKETPP_DIR): $(WEBSOCKETPP_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
	patch -d $@ -p1 < patches/websocketpp-$(WEBSOCKETPP_TAG).patch

$(SOCKPROC_TAR): | $(DL)
	curl -L $(SOCKPROC_URL) -o $@

$(SOCKPROC_DIR): $(SOCKPROC_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

$(LIBJSON_TAR): | $(DL)
	curl -L $(LIBJSON_URL) -o $@

$(LIBJSON_DIR): $(LIBJSON_TAR)
	mkdir -p $@
	unzip $< -d $(@D)
	patch -d $@ -p1 < patches/libjson.patch

$(LUANGINX_TAR): | $(DL)
	curl -L $(LUANGINX_URL) -o $@

$(LUANGINX_DIR): $(LUANGINX_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@

$(NGINX_TAR): | $(DL)
	curl -L $(NGINX_URL) -o $@

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

ifdef ENABLE_LICENSING

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
	curl -L $(SCPI_PARSER_URL) -o $@

$(SCPI_PARSER_DIR): $(SCPI_PARSER_TAR)
	mkdir -p $@
	tar -xzf $< --strip-components=1 --directory=$@
#	patch -d $@ -p1 < patches/scpi-parser-$(SCPI_PARSER_TAG).patch

scpi: api $(INSTALL_DIR) $(SCPI_PARSER_DIR)
	$(MAKE) -C $(SCPI_SERVER_DIR) clean
	$(MAKE) -C $(SCPI_SERVER_DIR)
	$(MAKE) -C $(SCPI_SERVER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# SDR
################################################################################

.PHONY: sdr

SDR_ZIP = sdr_transceiver_hpsdr-0.94-1632.zip
SDR_URL = http://downloads.redpitaya.com/downloads/charly25ab/$(SDR_ZIP)

sdr: | $(DL)
	curl -L $(SDR_URL) -o $(DL)/$(SDR_ZIP)
	mkdir -p $(INSTALL_DIR)/www/apps
	unzip $(DL)/$(SDR_ZIP) -d $(INSTALL_DIR)/www/apps

################################################################################
# Red Pitaya tools
################################################################################

LCR_DIR         = Test/lcr
BODE_DIR        = Test/bode
MONITOR_DIR     = Test/monitor
MONITOR_OLD_DIR = Test/monitor_old
GENERATE_DIR    = Test/generate
ACQUIRE_DIR     = Test/acquire
CALIB_DIR       = Test/calib
CALIBRATE_DIR   = Test/calibrate
GENERATOR_DIR	= Test/generate
COMM_DIR        = Examples/Communication/C
XADC_DIR        = Test/xadc
DISCOVERY_DIR   = Test/discovery
LA_TEST_DIR     = api2/test

.PHONY: examples rp_communication
.PHONY: lcr bode monitor monitor_old generator acquire calib calibrate discovery laboardtest

examples: lcr bode monitor monitor_old generator acquire calib discovery
# calibrate laboardtest

lcr:
	$(MAKE) -C $(LCR_DIR) clean
	$(MAKE) -C $(LCR_DIR)
	$(MAKE) -C $(LCR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

bode:
	$(MAKE) -C $(BODE_DIR) clean
	$(MAKE) -C $(BODE_DIR)
	$(MAKE) -C $(BODE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

monitor:
	$(MAKE) -C $(MONITOR_DIR) clean
	$(MAKE) -C $(MONITOR_DIR)
	$(MAKE) -C $(MONITOR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

monitor_old:
	$(MAKE) -C $(MONITOR_OLD_DIR) clean
	$(MAKE) -C $(MONITOR_OLD_DIR)
	$(MAKE) -C $(MONITOR_OLD_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

generator:
	$(MAKE) -C $(GENERATOR_DIR) clean
	$(MAKE) -C $(GENERATOR_DIR)
	$(MAKE) -C $(GENERATOR_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

acquire:
	$(MAKE) -C $(ACQUIRE_DIR)
	$(MAKE) -C $(ACQUIRE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

calib:
	$(MAKE) -C $(CALIB_DIR) clean
	$(MAKE) -C $(CALIB_DIR)
	$(MAKE) -C $(CALIB_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

discovery:
	$(MAKE) -C $(DISCOVERY_DIR) clean
	$(MAKE) -C $(DISCOVERY_DIR)
	$(MAKE) -C $(DISCOVERY_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

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

################################################################################
# Red Pitaya ecosystem and free applications
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

.PHONY: apps-tools ecosystem updater scpi_manager network_manager jupyter_manager

apps-tools: ecosystem updater scpi_manager network_manager jupyter_manager

ecosystem:
	$(MAKE) -C $(APP_ECOSYSTEM_DIR) clean
	$(MAKE) -C $(APP_ECOSYSTEM_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

updater: ecosystem api $(NGINX)
	$(MAKE) -C $(APP_UPDATER_DIR) clean
	$(MAKE) -C $(APP_UPDATER_DIR)
	$(MAKE) -C $(APP_UPDATER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

scpi_manager: ecosystem api $(NGINX)
	$(MAKE) -C $(APP_SCPIMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

network_manager: ecosystem
	$(MAKE) -C $(APP_NETWORKMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

jupyter_manager:
	$(MAKE) -C $(APP_JUPYTERMANAGER_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

################################################################################
# Red Pitaya ecosystem and free applications
################################################################################

APPS_FREE_DIR = apps-free

.PHONY: apps-free

apps-free: lcr bode
	$(MAKE) -C $(APPS_FREE_DIR) clean
	$(MAKE) -C $(APPS_FREE_DIR) all INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APPS_FREE_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

apps-free-clean:
	$(MAKE) -C $(APPS_FREE_DIR) clean

################################################################################
# Red Pitaya PRO applications
################################################################################

ifdef ENABLE_LICENSING

APP_SCOPEGENPRO_DIR = Applications/scopegenpro
APP_SPECTRUMPRO_DIR = Applications/spectrumpro
APP_LCRMETER_DIR    = Applications/lcr_meter
APP_LA_PRO_DIR 		= Applications/la_pro
APP_BA_PRO_DIR 		= Applications/ba_pro

.PHONY: apps-pro scopegenpro spectrumpro lcr_meter la_pro ba_pro

apps-pro: scopegenpro spectrumpro lcr_meter la_pro ba_pro

scopegenpro: api $(NGINX)
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR) clean
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_SCOPEGENPRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

spectrumpro: api $(NGINX)
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR) clean
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
	$(MAKE) -C $(APP_SPECTRUMPRO_DIR) install INSTALL_DIR=$(abspath $(INSTALL_DIR))

lcr_meter: api $(NGINX)
	$(MAKE) -C $(APP_LCRMETER_DIR) clean
	$(MAKE) -C $(APP_LCRMETER_DIR) INSTALL_DIR=$(abspath $(INSTALL_DIR))
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

clean:
	# todo, remove downloaded libraries and symlinks
	make -C $(NGINX_DIR) clean
	make -C $(MONITOR_DIR) clean
	make -C $(MONITOR_OLD_DIR) clean
	make -C $(GENERATE_DIR) clean
	make -C $(ACQUIRE_DIR) clean
	make -C $(CALIB_DIR) clean
	-make -C $(SCPI_SERVER_DIR) clean
	make -C $(LIBRP2_DIR)    clean
	make -C $(LIBRP_DIR)    clean
	make -C $(LIBRPAPP_DIR) clean
	make -C $(LIBRPLCR_DIR) clean
	make -C $(COMM_DIR) clean
	apps-free-clean
