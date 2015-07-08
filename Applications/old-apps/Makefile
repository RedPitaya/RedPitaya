#
# $ID: Makefile 2015-06-30  15:02:23 luka.golinar
#

#List of applications to build
APPS = scope scope+gen scope+pid spectrum freqanalyzer
NAME = praeteritum-apps
O_INSTALL_DIR ?= .

all: zips

zips:
	for APP in $(APPS); do \
		$(MAKE) -C $$APP; \
	done

install:
	mkdir target
	for APP in $(APPS); do \
		$(MAKE) -C $$APP zip; \
	done

	cd target; zip $(NAME).zip *.zip; cd -
	cp ./target/$(NAME).zip $(O_INSTALL_DIR)
	rm -rf *.zip

clean:
	for app in $(APPS); do \
		$(MAKE) -C $$app clean; \
	done

	$(RM) -rf $(NAME).zip
	$(RM) -rf ./target
