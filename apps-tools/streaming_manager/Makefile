#
# $Id: Makefile 1235 2014-02-21 16:44:10Z ales.bardorfer $
#
# Red Pitaya specific application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))

QT_DIR ?= /srv/Qt5.15.2
# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
LINUX_VER ?= dev
INSTALL_DIR ?= .
STREAMING_MODE ?= MASTER
CLIENT_SUFFIX ?=
MODE ?= Release
JENKINS_BUILD ?=

JOBS=$(shell ./get_cpu_ram.sh)


CONTROLLER=controllerhf.so
ZIP=../../$(APP)-$(LINUX_VER)-$(BUILD_NUMBER)-$(REVISION).zip


all: servers

servers:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(MODE) -DVERSION=$(VERSION) -DBUILD_NUMBER=$(BUILD_NUMBER) -DREVISION=$(REVISION) -DBUILD_RPSA_CLIENT=OFF -DBUILD_CONVERT_TOOL=OFF; \
	make -C build -j $(JOBS)
# VERBOSE=1

client:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(MODE) -DVERSION=$(VERSION) -DBUILD_NUMBER=$(BUILD_NUMBER) -DREVISION=$(REVISION) -DJENKINS_BUILD=$(JENKINS_BUILD) -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF -DBUILD_PYTHON_LIB=ON; \
	make -C build -j $(JOBS)

client-win:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(MODE)  -DVERSION=$(VERSION) -DBUILD_NUMBER=$(BUILD_NUMBER) -DREVISION=$(REVISION) -DJENKINS_BUILD=$(JENKINS_BUILD) -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF -DBUILD_PYTHON_LIB=ON -DCMAKE_TOOLCHAIN_FILE=./toolchains/toolchain-x86_64-mingw32-gcc.cmake; \
	make -C build -j $(JOBS)

install_servers:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(MODE)  -DVERSION=$(VERSION) -DBUILD_NUMBER=$(BUILD_NUMBER) -DREVISION=$(REVISION) -DBUILD_RPSA_CLIENT=OFF -DBUILD_CONVERT_TOOL=OFF; \
	make -C build -j $(JOBS) install

install_client:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(MODE)  -DVERSION=$(VERSION) -DBUILD_NUMBER=$(BUILD_NUMBER) -DREVISION=$(REVISION) -DJENKINS_BUILD=$(JENKINS_BUILD) -DCLIENT_SUFFIX=$(CLIENT_SUFFIX) -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF -DBUILD_PYTHON_LIB=ON; \
	make -C build -j $(JOBS) install

install_client_win:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(MODE) -DVERSION=$(VERSION) -DBUILD_NUMBER=$(BUILD_NUMBER) -DREVISION=$(REVISION) -DJENKINS_BUILD=$(JENKINS_BUILD) -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF -DBUILD_PYTHON_LIB=ON -DCMAKE_TOOLCHAIN_FILE=./toolchains/toolchain-x86_64-mingw32-gcc.cmake; \
	make -C build -j $(JOBS) install

install_client_qt:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(MODE)  -DVERSION=$(VERSION) -DBUILD_NUMBER=$(BUILD_NUMBER) -DREVISION=$(REVISION)  -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF -DBUILD_RPSA_CLIENT_QT=ON -DBUILD_RPSA_CLIENT=OFF -DBUILD_CONVERT_TOOL=OFF -DCMAKE_PREFIX_PATH:PATH=$(QT_DIR) -DQT_DIR=$(QT_DIR) -DQt5_DIR:PATH=$(QT_DIR)/lib/cmake/Qt5 -DQT=$(QT_DIR); \
	make -C build -j $(JOBS) install
	cd ${INSTALL_DIR}/streaming/rpsa_client-qt_${VERSION}-${REVISION}; \
	zip -r ${INSTALL_DIR}/streaming/rpsa_client-desktop_${VERSION}-${REVISION}.zip *
	rm -rf ${INSTALL_DIR}/streaming/rpsa_client-qt_${VERSION}-${REVISION}

install_client_qt_win:
	cd src; \
	cmake -B build -DINSTALL_DIR=$(abspath $(INSTALL_DIR)) -DCMAKE_BUILD_TYPE=$(MODE)  -DVERSION=$(VERSION) -DBUILD_NUMBER=$(BUILD_NUMBER) -DREVISION=$(REVISION)  -DBUILD_WEB_APP_SERVER=OFF -DBUILD_CONSOLE_SERVER=OFF -DBUILD_RPSA_CLIENT_QT=ON -DBUILD_RPSA_CLIENT=OFF -DBUILD_CONVERT_TOOL=OFF -DCMAKE_PREFIX_PATH:PATH=$(QT_DIR) -DQt5_DIR:PATH=$(QT_DIR)/lib/cmake/Qt5 -DQT=$(QT_DIR) -DQT_DIR=$(QT_DIR) -DCMAKE_TOOLCHAIN_FILE=./toolchains/toolchain-x86_64-mingw32-gcc.cmake; \
	make -C build -j $(JOBS) install
	cd ${INSTALL_DIR}/streaming/rpsa_client-qt_${VERSION}-${REVISION}-win; \
	zip -r ${INSTALL_DIR}/streaming/rpsa_client-desktop_${VERSION}-${REVISION}-win.zip *
	rm -rf ${INSTALL_DIR}/streaming/rpsa_client-qt_${VERSION}-${REVISION}-win

$(ZIP): install_servers
	rm target -rf
	mkdir -p target/$(APP)
	mv $(INSTALL_DIR)/bin/libweb_server.so target/$(APP)/$(CONTROLLER)
	cp -r fpga.conf nginx.conf css img *.html fpga.sh info js target/$(APP)
	sed -i target/$(APP)/info/info.json -e 's/REVISION/$(REVISION)/'
	sed -i target/$(APP)/info/info.json -e 's/VERSION/$(VERSION)/'
	cd target; zip -r ../$(ZIP) *
	$(RM) target -rf

install: $(ZIP)
	unzip -o $(ZIP) -d $(INSTALL_DIR)/www/apps

clean:
	rm -rf src/build
