#
# $ID: Makefile 2015-06-30  15:02:23 luka.golinar
#

#List of applications to build
APPS = scope scope+gen scope+pid spectrum freqanalyzer bode_plotter impedance_analyzer teslameter lti scope+istsensor stemlab_vna

all:
	for APP in $(APPS); do \
		$(MAKE) -C $$APP; \
	done

install:
	for APP in $(APPS); do \
		$(MAKE) -C $$APP install; \
	done

clean:
	for app in $(APPS); do \
		$(MAKE) -C $$app clean; \
	done

	-$(RM) -rf ./build/
