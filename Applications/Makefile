#
# $Id: Makefile 1200 2014-02-20 12:52:27Z ales.bardorfer $
#
# Red Pitaya Applications main Makefile
#

# List of applications to build
APPS=scope scope+gen spectrum


all: zips

zips:
	for app in $(APPS); do \
		$(MAKE) -C $$app zip INSTALL_DIR=$(abspath .); \
	done

clean:
	for app in $(APPS); do \
		$(MAKE) -C $$app clean; \
	done
	$(RM) redpitaya-*.zip
