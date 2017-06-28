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

# DSP library for C language
# TODO: the package does not exist yet in Ubuntu 16.04
#apt-get -y install libliquid-dev
EOF_CHROOT
