################################################################################
#
################################################################################

INSTALL_DIR=build
TARGET=target

ZIPFILE=ecosystem-$(VERSION).zip *

################################################################################
# tarball
################################################################################

all: zip

$(INSTALL_DIR):
	mkdir -p $@

: $(BOOT_UBOOT) u-boot $(DEVICETREE) $(LINUX) $(IDGEN) $(NGINX) \
	   examples $(DISCOVERY) $(HEARTBEAT) ecosystem \
	   scpi api apps_pro rp_communication
	mkdir -p               $(TARGET)
	# copy boot images and select FSBL as default
	cp $(BOOT_UBOOT)       $(TARGET)/boot.bin
	# copy device tree and Linux kernel
	cp $(DEVICETREE)       $(TARGET)
	cp $(LINUX)            $(TARGET)
	# copy FPGA bitstream images and decompress them
	mkdir -p               $(TARGET)/fpga
	cp $(FPGA)             $(TARGET)/fpga/fpga.bit
	cp fpga/archive/*.xz   $(TARGET)/fpga
	cd $(TARGET)/fpga; xz -df *.xz
	#
	cp -r $(INSTALL_DIR)/* $(TARGET)
	cp -r OS/filesystem/*  $(TARGET)
	@echo "$$GREET_MSG" >  $(TARGET)/version.txt
	# copy configuration file for WiFi access point
	cp OS/debian/overlay/etc/hostapd/hostapd.conf $(TARGET)/hostapd.conf
	# build zip file
	cd $(TARGET); zip -r ../$(NAME)-$(VERSION).zip *

################################################################################
# local (on RP board) install process
# TODO: generalized install should be provided instead
################################################################################

localinstall:
	systemctl stop redpitaya_nginx
	rw
	/bin/cp build/sbin/* /opt/redpitaya/sbin/
	/bin/cp build/lib/* /opt/redpitaya/lib/
	/bin/cp -r build/www/apps/* /opt/redpitaya/www/apps/
	ro
	systemctl start redpitaya_nginx

imagemount:
	DEVICE=`sudo losetup -f`
	sudo losetup -v $DEVICE -o 127926272 ../red_pitaya_OS_23-Mar-2016.img
	#sudo losetup -P -f ../red_pitaya_OS_23-Mar-2016.img
	sudo mount -o ro ${DEVICE} ../debian
	sudo losetup -d ${DEVICE}

################################################################################
#
################################################################################

clean:
	$(RM) $(INSTALL_DIR) -rf
	$(RM) $(ZIPFILE)
