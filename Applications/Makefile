#
# $Id: Makefile 1200 2014-02-20 12:52:27Z ales.bardorfer $
#
# Red Pitaya Applications main Makefile
#

# List of applications to build
ifeq ($(ENABLE_LICENSING),true)
APPS=scopegenpro spectrumpro
else
APPS=
endif

all: zips

zips:
	for app in $(APPS); do \
		$(MAKE) -C $$app zip INSTALL_DIR=$(abspath .); \
	done

clean:
	for app in $(APPS); do \
		$(MAKE) -C $$app clean; \
	done
	$(RM) *.zip
