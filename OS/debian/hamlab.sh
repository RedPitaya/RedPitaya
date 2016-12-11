################################################################################
# Authors:
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# - Uroš Golob <uros.golob@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################


# Overlay loading service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/hamlab.service $ROOT_DIR/etc/systemd/system/hamlab.service

# add fancontrol config file, generated using pwmconfig
install -v -m 664 -o root -D $OVERLAY/etc/fancontrol $ROOT_DIR/etc/fancontrol

# enable services
chroot $ROOT_DIR <<- EOF_CHROOT
# Install lm-sensors, fancontrol
apt-get install lm-sensors fancontrol

# enable services
systemctl enable fancontrol.service
systemctl enable hamlab.service
EOF_CHROOT
