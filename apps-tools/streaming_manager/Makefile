#
# $Id: Makefile 1235 2014-02-21 16:44:10Z ales.bardorfer $
#
# Red Pitaya specific application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))

MODEL ?= Z10

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

INSTALL_DIR ?= .
STREAMING_MODE ?= MASTER

UNAME := $(shell uname -m)
ifeq ($(UNAME), x86_64)
NUM_CPU := $(shell grep -c ^processor /proc/cpuinfo)
endif

JOBS=$$(($(NUM_CPU)+1))

CONTROLLER=controllerhf.so
ZIP=../../$(APP)-$(VER)-$(BUILD_NUMBER)-$(REVISION).zip


all: servers

servers:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=Release -DMODEL=$(MODEL) -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DSTREAMING_MODE=$(STREAMING_MODE) -DBUILD_RPSA_CLIENT=OFF -DBUILD_CONVERT_TOOL=OFF; \
	make -C build -j $(JOBS)
# VERBOSE=1
	
client:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=Release -DMODEL=$(MODEL) -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF; \
	make -C build -j $(JOBS)

client-win:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=Release -DMODEL=$(MODEL) -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF -DCMAKE_TOOLCHAIN_FILE=./toolchains/toolchain-i686-w64-mingw32.cmake; \
	make -C build -j $(JOBS)

install_servers:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=Release -DMODEL=$(MODEL) -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DSTREAMING_MODE=$(STREAMING_MODE) -DBUILD_RPSA_CLIENT=OFF -DBUILD_CONVERT_TOOL=OFF; \
	make -C build -j $(JOBS) install 
	
install_client:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=Release -DMODEL=$(MODEL) -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DSTREAMING_MODE=$(STREAMING_MODE) -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF; \
	make -C build -j $(JOBS) install

install_client_win:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=Release -DMODEL=$(MODEL) -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF -DCMAKE_TOOLCHAIN_FILE=./toolchains/toolchain-i686-w64-mingw32.cmake; \
	make -C build -j $(JOBS) install

install_client_qt:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=Release -DMODEL=$(MODEL) -DVERSION=$(VERSION) -DREVISION=$(REVISION) -DSTREAMING_MODE=$(STREAMING_MODE) -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF -DBUILD_RPSA_CLIENT_QT=ON -DBUILD_RPSA_CLIENT=OFF -DBUILD_CONVERT_TOOL=OFF -DCMAKE_PREFIX_PATH:PATH=/srv/Qt5.15.2 -DQt5_DIR:PATH=/srv/Qt5.15.2/lib/cmake/Qt5; \
	make -C build -j $(JOBS) install
	
$(ZIP): install_servers
	rm target -rf
	mkdir -p target/$(APP)
	mv $(INSTALL_DIR)/bin/libweb_server.so target/$(APP)/$(CONTROLLER)
	cp -r fpga.conf nginx.conf css img *.html fpga.sh info js target/$(APP)
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/BUILD_NUMBER/$(BUILD_NUMBER)/'
	cd target; zip -r ../$(ZIP) *
	$(RM) target -rf

install: $(ZIP)
	unzip -o $(ZIP) -d $(INSTALL_DIR)/www/apps

clean:
	rm -rf src/build