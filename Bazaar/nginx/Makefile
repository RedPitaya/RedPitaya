#
# $Id: Makefile 1234 2014-02-21 15:54:39Z tomaz.beltram $
#
# Red Pitaya Nginx WEB server Makefile
#

NAME=redpitaya-nginx

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat ../../Applications/ecosystem/info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

NGINX_DIR=nginx-1.5.3
NGINX=$(NGINX_DIR)/objs/nginx

STRIP=$(CROSS_COMPILE)strip
NUM_CPU := $(shell grep -c ^processor /proc/cpuinfo)
JOBS=$$(($(NUM_CPU)+1))

WS_SERVER_DIR=ngx_ext_modules/ws_server
WS_SERVER_LIB=$(WS_SERVER_DIR)/libws_server.a

# Remove everything we can.
# We will add things one-by-one accoording to our needs in add_modules.
without:=`cat configure_withouts.txt`
add_modules=--add-module=../ngx_ext_modules/lua-nginx-module --add-module=../ngx_ext_modules/ngx_http_rp_module
add_conf_params=--with-zlib=../../../OS/buildroot/buildroot-2014.02/output/build/zlib-1.2.8 --with-pcre=../../../OS/buildroot/buildroot-2014.02/output/build/pcre-8.34
VERSION=$(VER)-$(BUILD_NUMBER)

all: $(NGINX)

$(NGINX): $(RP_MANAGER_LIB) $(WS_SERVER_LIB)
	cd $(NGINX_DIR); ./configure `cat ../configure_withouts.txt` $(add_conf_params) $(add_modules);
	$(MAKE) -j $(JOBS) -C $(NGINX_DIR) CC=$(CROSS_COMPILE)gcc LINK=$(CROSS_COMPILE)g++

$(WS_SERVER_LIB): 
	$(MAKE) -C $(WS_SERVER_DIR)

$(DEPLIBS):
	mkdir -p $(DEPLIBS_DIR)
	$(MAKE) -C $(SHARED)
	$(MAKE) -C $(SHARED) install INSTALL_DIR=$(abspath $(DEPLIBS_DIR))

# Override default nginx install, since Red Pitaya uses only a subset
# of its default install.
# TODO: Configure nginx to exclude client_body_temp.
install:
	$(STRIP) $(NGINX)
	test -d '$(DESTDIR)/sbin'                 || mkdir -p '$(DESTDIR)/sbin'
	test -d '$(DESTDIR)/lib'                  || mkdir -p '$(DESTDIR)/lib'
	test -d '$(DESTDIR)/www/conf'             || mkdir -p '$(DESTDIR)/www/conf'
	test -d '$(DESTDIR)/www/logs'             || mkdir -p '$(DESTDIR)/www/logs'
	test -d '$(DESTDIR)/www/apps'             || mkdir -p '$(DESTDIR)/www/apps'
	test -d '$(DESTDIR)/www/client_body_temp' || mkdir -p '$(DESTDIR)/www/client_body_temp'
	cp $(NGINX)                     '$(DESTDIR)/sbin/'
	cp $(NGINX_DIR)/conf/mime.types '$(DESTDIR)/www/conf/'
	cp $(NGINX_DIR)/conf/nginx.conf '$(DESTDIR)/www/conf/'


clean:
	-$(MAKE) -C $(NGINX_DIR) clean
	-$(MAKE) -C $(WS_SERVER_DIR) clean
