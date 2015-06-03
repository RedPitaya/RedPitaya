

device=$1

boot_dir=BOOT
root_dir=ROOT

ecosystem_tar=red-pitaya-ecosystem-0.92-20150527.tgz

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

# copy overlay
mkdir -p                    $root_dir/var/log/nginx
cp patches/fw_env.config    $root_dir/etc/
cp -r OS/debian/overlay/*   $root_dir/

chroot $root_dir <<- EOF_CHROOT
# TODO sees sytemd is not running without /proc/cmdline or something
#hostnamectl set-hostname redpitaya
#timedatectl set-timezone Europe/Ljubljana
#localectl set-locale LANG="en_US.UTF-8"

echo redpitaya > /etc/hostname
echo Europe/Ljubljana > /etc/timezone

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

# TODO: /etc/default/hostapd should be copied only here
# TODO: /etc/dhcp/dhcpd.conf should be copied only here

sed -i 's/^PermitRootLogin.*/PermitRootLogin yes/' /etc/ssh/sshd_config

systemctl enable nginx

sed -i '/^#net.ipv4.ip_forward=1$/s/^#//' /etc/sysctl.conf

apt-get clean

echo root:root | chpasswd

service ntp stop

history -c
EOF_CHROOT

rm $root_dir/etc/resolv.conf
rm $root_dir/usr/bin/qemu-arm-static

# Unmount file systems
umount $boot_dir $root_dir
rmdir $boot_dir $root_dir
