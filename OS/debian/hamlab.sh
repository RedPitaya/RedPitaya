################################################################################
# Authors:
# - Iztok Jeras <iztok.jeras@redpitaya.com>
# - Uroš Golob <uros.golob@redpitaya.com>
# License:
# https://raw.githubusercontent.com/RedPitaya/RedPitaya/master/COPYING
################################################################################

# Overlay loading service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/hamlab_i2cmux.service       $ROOT_DIR/etc/systemd/system/hamlab_i2cmux.service

# Install lm-sensors, fancontrol
apt-get install lm-sensors fancontrol

# enable pwm control service


# add fancontrol config file, generated using pwmconfig
install -v -m 664 -o root -D $OVERLAY/etc/fancontrol $ROOT_DIR/etc/fancontrol

chroot $ROOT_DIR <<- EOF_CHROOT
systemctl enable fancontrol.service
EOF_CHROOT
