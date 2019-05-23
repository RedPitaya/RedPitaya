#
# $Id: Makefile 1234 2014-02-21 15:54:39Z tomaz.beltram $
#
# Red Pitaya Nginx WEB server Makefile
#

NAME=redpitaya-nginx

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat ../../apps-tools/ecosystem/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

NGINX_DIR=nginx
NGINX=$(NGINX_DIR)/objs/nginx

STRIP=$(CROSS_COMPILE)strip
NUM_CPU := $(shell grep -c ^processor /proc/cpuinfo)
JOBS=$$(($(NUM_CPU)+1))

LIBJSON_DIR = ../tools/libjson

WS_SERVER_DIR=ngx_ext_modules/ws_server
WS_SERVER_LIB=$(WS_SERVER_DIR)/libws_server.a

# Remove everything we can.
# We will add things one-by-one accoording to our needs.
configuration  = --add-module=../ngx_ext_modules/ngx_http_rp_module
configuration += --add-module=../ngx_ext_modules/lua-nginx-module
configuration += --error-log-path=/var/log/redpitaya_nginx/error.log
configuration += --with-ipv6
configuration += --with-http_dav_module

VERSION=$(VER)-$(BUILD_NUMBER)

all: $(NGINX)

$(NGINX): $(RP_MANAGER_LIB) $(WS_SERVER_LIB)
	cd $(NGINX_DIR); ./configure `cat ../configure_withouts.txt` $(configuration);
	$(MAKE) -j $(JOBS) -C $(NGINX_DIR) CC=$(CROSS_COMPILE)gcc LINK=$(CROSS_COMPILE)g++

$(WS_SERVER_LIB):
	$(MAKE) -C $(WS_SERVER_DIR)

# Override default nginx install, since Red Pitaya uses only a subset
# of its default install.
# TODO: Configure nginx to exclude client_body_temp.
install:
	$(STRIP) $(NGINX)
	test -d '$(DESTDIR)/sbin'                 || mkdir -p '$(DESTDIR)/sbin'
	test -d '$(DESTDIR)/lib'                  || mkdir -p '$(DESTDIR)/lib'
	test -d '$(DESTDIR)/www/conf'             || mkdir -p '$(DESTDIR)/www/conf'
	test -d '$(DESTDIR)/www/apps'             || mkdir -p '$(DESTDIR)/www/apps'
	test -d '$(DESTDIR)/www/client_body_temp' || mkdir -p '$(DESTDIR)/www/client_body_temp'
	test -d '$(DESTDIR)/rp_sdk' 		 	  || mkdir -p '$(DESTDIR)/rp_sdk'
	test -d '$(DESTDIR)/rp_sdk/libjson' 	  || mkdir -p '$(DESTDIR)/rp_sdk/libjson'
	cp $(NGINX)                     '$(DESTDIR)/sbin/'
	cp $(NGINX_DIR)/conf/mime.types '$(DESTDIR)/www/conf/'
	cp $(NGINX_DIR)/conf/nginx.conf '$(DESTDIR)/www/conf/'
	cp -r $(WS_SERVER_DIR)/rp_sdk/*.h '$(DESTDIR)/rp_sdk/'
	cp -r $(WS_SERVER_DIR)/rp_sdk/*.a '$(DESTDIR)/rp_sdk/'
	cp -r $(LIBJSON_DIR)/*.h '$(DESTDIR)/rp_sdk/libjson/'
	cp -r $(LIBJSON_DIR)/_internal '$(DESTDIR)/rp_sdk/libjson/'


clean:
	-$(MAKE) -C $(NGINX_DIR) clean
	-$(MAKE) -C $(WS_SERVER_DIR) clean
