#
# $ID: Makefile 2015-06-30  15:02:23 luka.golinar
#

#List of applications to build

all: zips

zips:
	for APP in $(APPS); do \
		$(MAKE) -C $$APP; \
	done

install:
	mkdir build
	for APP in $(APPS); do \
		$(MAKE) -C $$APP INSTALL_DIR=../../build zip; \
	done

clean:
	for app in $(APPS); do \
		$(MAKE) -C $$app clean; \
	done

	-$(RM) -rf ./build/
