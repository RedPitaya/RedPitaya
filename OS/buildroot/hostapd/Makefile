#
# $Id: Makefile 1283 2014-03-01 12:01:39Z ales.bardorfer $
#
# Red Pitaya hostapd Makefile
#

# Hostapd name & version
B_NAME=wpa_supplicant_hostapd
B_VER=0.8_rtw_r7475.20130812

B_DIR=$(B_NAME)-$(B_VER)
B_ARCHIVE=$(B_DIR).tar.gz

INSTALL_DIR ?= .
VERSION ?= 0.00-0000
REVISION ?= devbuild

GCC=$(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

HAPD=$(B_DIR)/hostapd/hostapd

all: $(HAPD)

$(HAPD): $(B_DIR)
	$(MAKE) -C $(B_DIR)/hostapd CC=$(GCC)

$(B_DIR):
	tar xfz $(B_ARCHIVE)

install: $(HAPD)
	mkdir -p $(INSTALL_DIR)
	$(STRIP) $(HAPD)
	cp $(HAPD) $(INSTALL_DIR)

clean:
	-rm -rf $(B_DIR)
	-rm *~ -f
