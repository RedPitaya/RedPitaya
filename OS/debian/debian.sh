
# TODO: copyright notice and authors shold be listed here

device=$1

boot_dir=BOOT
root_dir=ROOT
OVERLAY=OS/debian/overlay

# Create partitions
parted -s $device mklabel msdos
parted -s $device mkpart primary fat16   4MB 128MB
parted -s $device mkpart primary ext4  128MB 100%

boot_dev=/dev/`lsblk -lno NAME $device | sed '2!d'`
root_dev=/dev/`lsblk -lno NAME $device | sed '3!d'`

# Create file systems
mkfs.vfat -v    $boot_dev
mkfs.ext4 -F -j $root_dev

# Mount file systems
mkdir -p $boot_dir $root_dir
mount $boot_dev $boot_dir
mount $root_dev $root_dir

# Copy files to the boot file system
unzip ecosystem*.zip -d $boot_dir

# Install Debian base system to the root file system
mirror=http://ftp.heanet.ie/pub/debian
distro=jessie
arch=armel
debootstrap --foreign --arch $arch $distro $root_dir $mirror

# Add missing configuration files and packages
cp /etc/resolv.conf         $root_dir/etc/
cp /usr/bin/qemu-arm-static $root_dir/usr/bin/

chroot $root_dir <<- EOF_CHROOT
export LANG=C
/debootstrap/debootstrap --second-stage
EOF_CHROOT

# copy U-Boot environment tools
install -v -m 664 -o root -D patches/fw_env.config                      $root_dir/etc/fw_env.config
# TODO missing executables

install -v -m 664 -o root -D $OVERLAY/etc/apt/apt.conf.d/99norecommends $root_dir/etc/apt/apt.conf.d/99norecommends
install -v -m 664 -o root -D $OVERLAY/etc/apt/sources.list              $root_dir/etc/apt/sources.list
install -v -m 664 -o root -D $OVERLAY/etc/fstab                         $root_dir/etc/fstab
install -v -m 664 -o root -D $OVERLAY/etc/hostname                      $root_dir/etc/hostname
install -v -m 664 -o root -D $OVERLAY/etc/timezone                      $root_dir/etc/timezone
install -v -m 664 -o root -D $OVERLAY/etc/securetty                     $root_dir/etc/securetty

# setup locale and timezune, install packages
chroot $root_dir <<- EOF_CHROOT
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
  iw firmware-realtek firmware-ralink build-essential libluajit-5.1 lua-cjson \
  unzip ifplugd

sed -i 's/^PermitRootLogin.*/PermitRootLogin yes/' /etc/ssh/sshd_config
EOF_CHROOT

# Nginx service
install -v -m 664 -o root -d                                                     $root_dir/var/log/nginx
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_nginx.service $root_dir/etc/systemd/system/redpitaya_nginx.service
install -v -m 664 -o root -D $OVERLAY/etc/sysconfig/redpitaya                    $root_dir/etc/sysconfig/redpitaya

chroot $root_dir <<- EOF_CHROOT
systemctl enable redpitaya_nginx
EOF_CHROOT

# network configuration
install -v -m 664 -o root -D $OVERLAY/etc/udev/rules.d/75-persistent-net-generator.rules $root_dir/etc/udev/rules.d/75-persistent-net-generator.rules
install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/eth0                      $root_dir/etc/network/interfaces.d/eth0
install -v -m 664 -o root -D $OVERLAY/etc/default/ifplugd                                $root_dir/etc/default/ifplugd
install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/wlan0                     $root_dir/etc/network/interfaces.d/wlan0
install -v -m 664 -o root -D $OVERLAY/etc/hostapd/hostapd.conf                           $root_dir/etc/hostapd/hostapd.conf
install -v -m 664 -o root -D $OVERLAY/etc/default/hostapd                                $root_dir/etc/default/hostapd
install -v -m 664 -o root -D $OVERLAY/etc/dhcp/dhcpd.conf                                $root_dir/etc/dhcp/dhcpd.conf
install -v -m 664 -o root -D $OVERLAY/etc/iptables.ipv4.nat                              $root_dir/etc/iptables.ipv4.nat
install -v -m 664 -o root -D $OVERLAY/etc/iptables.ipv4.nonat                            $root_dir/etc/iptables.ipv4.nonat

chroot $root_dir <<- EOF_CHROOT
sed -i '/^#net.ipv4.ip_forward=1$/s/^#//' /etc/sysctl.conf
EOF_CHROOT

# final operations and cleanup
install -v -m 664 -o root -D $OVERLAY/etc/profile.d/redpitaya.sh $root_dir/etc/profile.d/redpitaya.sh

chroot $root_dir <<- EOF_CHROOT
echo root:root | chpasswd
apt-get clean
service ntp stop
history -c
EOF_CHROOT


rm $root_dir/etc/resolv.conf
rm $root_dir/usr/bin/qemu-arm-static

# Unmount file systems
umount $boot_dir $root_dir
rmdir $boot_dir $root_dir
