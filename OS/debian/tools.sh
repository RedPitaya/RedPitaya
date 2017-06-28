################################################################################
# miscelaneous tools
################################################################################

chroot $ROOT_DIR <<- EOF_CHROOT
apt-get -y install dbus udev

# development tools
apt-get -y install build-essential less vim nano sudo usbutils psmisc lsof
apt-get -y install parted dosfstools

# install file system tools
apt-get -y install mtd-utils
EOF_CHROOT
