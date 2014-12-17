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

SHARED=../../shared/
DEPLIBS_DIR=build/
DEPLIBS=$(DEPLIBS_DIR)/lib/libredpitaya.a

all: $(NGINX)

$(NGINX): $(DEPLIBS)
	cd $(NGINX_DIR); ./rp_configure.sh $(VER)-$(BUILD_NUMBER) $(REVISION);
	$(MAKE) -j $(JOBS) -C $(NGINX_DIR)

$(DEPLIBS):
	mkdir -p $(DEPLIBS_DIR)
	$(MAKE) -C $(SHARED)
	$(MAKE) -C $(SHARED) install INSTALL_DIR=$(abspath $(DEPLIBS_DIR))

# Override default nginx install, since Red Pitaya uses only a subset
# of its default install.
# TODO: Configure nginx to exclude client_body_temp.
install:
	$(STRIP) $(NGINX)
	test -d '$(DESTDIR)/sbin'     || mkdir -p '$(DESTDIR)/sbin'
	test -d '$(DESTDIR)/www/conf' || mkdir -p '$(DESTDIR)/www/conf'
	test -d '$(DESTDIR)/www/logs' || mkdir -p '$(DESTDIR)/www/logs'
	test -d '$(DESTDIR)/www/apps' || mkdir -p '$(DESTDIR)/www/apps'
	test -d '$(DESTDIR)/www/client_body_temp' || mkdir -p '$(DESTDIR)/www/client_body_temp'
	cp $(NGINX) '$(DESTDIR)/sbin/'
	cp $(NGINX_DIR)/conf/mime.types '$(DESTDIR)/www/conf/'
	cp $(NGINX_DIR)/conf/nginx.conf '$(DESTDIR)/www/conf/'


clean:
	-$(MAKE) -C $(NGINX_DIR) clean
	rm -rf $(DEPLIBS_DIR)
