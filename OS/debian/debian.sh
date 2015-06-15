################################################################################
# TODO: copyright notice and authors should be listed here
################################################################################

# Install Debian base system to the root file system
MIRROR=http://ftp.heanet.ie/pub/debian
DISTRO=jessie
ARCH=armel
debootstrap --foreign --arch $ARCH $DISTRO $ROOT_DIR $MIRROR

chroot $ROOT_DIR <<- EOF_CHROOT
export LANG=C
/debootstrap/debootstrap --second-stage
EOF_CHROOT

# copy U-Boot environment tools
install -v -m 664 -o root -D patches/fw_env.config                      $ROOT_DIR/etc/fw_env.config

install -v -m 664 -o root -D $OVERLAY/etc/apt/apt.conf.d/99norecommends $ROOT_DIR/etc/apt/apt.conf.d/99norecommends
install -v -m 664 -o root -D $OVERLAY/etc/apt/sources.list              $ROOT_DIR/etc/apt/sources.list
install -v -m 664 -o root -D $OVERLAY/etc/fstab                         $ROOT_DIR/etc/fstab
install -v -m 664 -o root -D $OVERLAY/etc/hostname                      $ROOT_DIR/etc/hostname
install -v -m 664 -o root -D $OVERLAY/etc/timezone                      $ROOT_DIR/etc/timezone
install -v -m 664 -o root -D $OVERLAY/etc/securetty                     $ROOT_DIR/etc/securetty

# setup locale and timezune, install packages
chroot $ROOT_DIR <<- EOF_CHROOT
# TODO sees sytemd is not running without /proc/cmdline or something
#hostnamectl set-hostname redpitaya
#timedatectl set-timezone Europe/Ljubljana
#localectl set-locale LANG="en_US.UTF-8"

apt-get update
apt-get -y upgrade
apt-get -y install locales

sed -i "/^# en_US.UTF-8 UTF-8$/s/^# //" /etc/locale.gen
locale-gen
update-locale LANG=en_US.UTF-8

dpkg-reconfigure --frontend=noninteractive tzdata

apt-get -y install openssh-server ca-certificates ntp ntpdate fake-hwclock \
  usbutils psmisc lsof parted curl vim wpasupplicant hostapd isc-dhcp-server \
  iw firmware-realtek firmware-ralink build-essential ifplugd sudo u-boot-tools

sed -i 's/^PermitRootLogin.*/PermitRootLogin yes/' /etc/ssh/sshd_config
EOF_CHROOT

# network configuration
install -v -m 664 -o root -D $OVERLAY/etc/udev/rules.d/75-persistent-net-generator.rules $ROOT_DIR/etc/udev/rules.d/75-persistent-net-generator.rules
install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/eth0                      $ROOT_DIR/etc/network/interfaces.d/eth0
install -v -m 664 -o root -D $OVERLAY/etc/default/ifplugd                                $ROOT_DIR/etc/default/ifplugd
install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/wlan0                     $ROOT_DIR/etc/network/interfaces.d/wlan0
install -v -m 664 -o root -D $OVERLAY/etc/hostapd/hostapd.conf                           $ROOT_DIR/etc/hostapd/hostapd.conf
install -v -m 664 -o root -D $OVERLAY/etc/default/hostapd                                $ROOT_DIR/etc/default/hostapd
install -v -m 664 -o root -D $OVERLAY/etc/dhcp/dhcpd.conf                                $ROOT_DIR/etc/dhcp/dhcpd.conf
install -v -m 664 -o root -D $OVERLAY/etc/iptables.ipv4.nat                              $ROOT_DIR/etc/iptables.ipv4.nat
install -v -m 664 -o root -D $OVERLAY/etc/iptables.ipv4.nonat                            $ROOT_DIR/etc/iptables.ipv4.nonat

chroot $ROOT_DIR <<- EOF_CHROOT
sed -i '/^#net.ipv4.ip_forward=1$/s/^#//' /etc/sysctl.conf
EOF_CHROOT

chroot $ROOT_DIR <<- EOF_CHROOT
echo root:root | chpasswd
apt-get clean
service ntp stop
history -c
EOF_CHROOT
