#
# $ID: Makefile 2015-06-30  15:02:23 luka.golinar
#

#List of applications to build
APPS = spectrum scope scope+gen freqanalyzer scope+pid
NAME = praeteritum-apps

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
	cp ./target/$(NAME).zip $(INSTALL_DIR)
	rm -rf *.zip

clean:
	for app in $(APPS); do \
		$(MAKE) -C $$app clean; \
	done

	$(RM) -rf $(NAME).zip
	$(RM) -rf ./target
