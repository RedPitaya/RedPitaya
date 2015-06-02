device=$1

boot_dir=/tmp/BOOT
root_dir=/tmp/ROOT

ecosystem_tar=red-pitaya-ecosystem-0.92-20150527.tgz
ecosystem_url=https://googledrive.com/host/0B-t5klOOymMNfmJ0bFQzTVNXQ3RtWm5SQ2NGTE1hRUlTd3V2emdSNzN6d0pYamNILW83Wmc/red-pitaya-ecosystem/$ecosystem_tar

mirror=http://ftp.heanet.ie/pub/debian
distro=jessie
arch=armel

passwd=root
timezone=Europe/Ljubljana

# Create partitions
parted -s $device mklabel msdos
parted -s $device mkpart primary fat16  4MB 32MB
parted -s $device mkpart primary ext4  32MB 100%

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

cp boot.bin devicetree.dtb uImage uEnv.txt $boot_dir

# Install Debian base system to the root file system

debootstrap --foreign --arch $arch $distro $root_dir $mirror

# Add missing configuration files and packages

cp /etc/resolv.conf $root_dir/etc/
cp /usr/bin/qemu-arm-static $root_dir/usr/bin/

cp patches/fw_env.config $root_dir/etc/

test -f $ecosystem_tar || curl -L $ecosystem_url -o $ecosystem_tar

mkdir -p $root_dir/var/log/nginx
tar -zxf $ecosystem_tar --directory=$root_dir

chroot $root_dir <<- EOF_CHROOT
export LANG=C

/debootstrap/debootstrap --second-stage

cat <<- EOF_CAT > /etc/apt/sources.list
deb $mirror $distro main contrib non-free
deb-src $mirror $distro main contrib non-free
deb $mirror $distro-updates main contrib non-free
deb-src $mirror $distro-updates main contrib non-free
deb http://security.debian.org/debian-security $distro/updates main contrib non-free
deb-src http://security.debian.org/debian-security $distro/updates main contrib non-free
EOF_CAT

cat <<- EOF_CAT > etc/apt/apt.conf.d/99norecommends
APT::Install-Recommends "0";
APT::Install-Suggests "0";
EOF_CAT

cat <<- EOF_CAT > etc/fstab
# /etc/fstab: static file system information.
# <file system> <mount point>   <type>  <options>           <dump>  <pass>
/dev/mmcblk0p2  /               ext4    errors=remount-ro   0       1
/dev/mmcblk0p1  /boot           vfat    defaults            0       2
EOF_CAT

cat <<- EOF_CAT >> etc/securetty

# Serial Console for Xilinx Zynq-7000
ttyPS0
EOF_CAT

echo red-pitaya > etc/hostname

apt-get update
apt-get -y upgrade

apt-get -y install locales

sed -i "/^# en_US.UTF-8 UTF-8$/s/^# //" etc/locale.gen
locale-gen
update-locale LANG=en_US.UTF-8

echo $timezone > etc/timezone
dpkg-reconfigure --frontend=noninteractive tzdata

apt-get -y install openssh-server ca-certificates ntp ntpdate fake-hwclock \
  usbutils psmisc lsof parted curl vim wpasupplicant hostapd isc-dhcp-server \
  iw firmware-realtek firmware-ralink build-essential libluajit-5.1 lua-cjson \
  unzip ifplugd

sed -i 's/^PermitRootLogin.*/PermitRootLogin yes/' etc/ssh/sshd_config

cat <<- EOF_CAT > etc/systemd/system/nginx.service
[Unit]
Description=A high performance web server and a reverse proxy server
After=network.target

[Service]
Type=forking
PIDFile=/run/nginx.pid
ExecStart=/opt/sbin/nginx -p /opt/www
ExecReload=/opt/sbin/nginx -p /opt/www -s reload
ExecStop=/opt/sbin/nginx -p /opt/www -s quit

[Install]
WantedBy=multi-user.target
EOF_CAT

systemctl enable nginx

cat <<- EOF_CAT > etc/profile.d/red-pitaya.sh
export PATH=\\\$PATH:/opt/bin
export LD_LIBRARY_PATH=\\\$LD_LIBRARY_PATH:/opt/lib
EOF_CAT

touch etc/udev/rules.d/75-persistent-net-generator.rules

cat <<- EOF_CAT > etc/network/interfaces.d/eth0
iface eth0 inet dhcp
EOF_CAT

cat <<- EOF_CAT > etc/default/ifplugd
INTERFACES="eth0"
HOTPLUG_INTERFACES=""
ARGS="-q -f -u0 -d10 -w -I"
SUSPEND_ACTION="stop"
EOF_CAT

cat <<- EOF_CAT > etc/network/interfaces.d/wlan0
allow-hotplug wlan0
iface wlan0 inet static
  address 192.168.42.1
  netmask 255.255.255.0
  post-up service hostapd restart
  post-up service isc-dhcp-server restart
  post-up iptables-restore < /etc/iptables.ipv4.nat
  pre-down iptables-restore < /etc/iptables.ipv4.nonat
  pre-down service isc-dhcp-server stop
  pre-down service hostapd stop
EOF_CAT

cat <<- EOF_CAT > etc/hostapd/hostapd.conf
interface=wlan0
ssid=RedPitaya
driver=nl80211
hw_mode=g
channel=6
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=RedPitaya
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF_CAT

cat <<- EOF_CAT > etc/default/hostapd
DAEMON_CONF=/etc/hostapd/hostapd.conf

if [ "\\\$1" = "start" ]
then
  iw wlan0 info > /dev/null 2>&1
  if [ \\\$? -eq 0 ]
  then
    sed -i '/^driver/s/=.*/=nl80211/' /etc/hostapd/hostapd.conf
    DAEMON_SBIN=/usr/sbin/hostapd
  else
    sed -i '/^driver/s/=.*/=rtl871xdrv/' /etc/hostapd/hostapd.conf
    DAEMON_SBIN=/opt/sbin/hostapd
  fi
  echo \\\$DAEMON_SBIN > /run/hostapd.which
elif [ "\\\$1" = "stop" ]
then
  DAEMON_SBIN=\\\$(cat /run/hostapd.which)
fi
EOF_CAT

cat <<- EOF_CAT > etc/dhcp/dhcpd.conf
ddns-update-style none;
default-lease-time 600;
max-lease-time 7200;
authoritative;
log-facility local7;
subnet 192.168.42.0 netmask 255.255.255.0 {
  range 192.168.42.10 192.168.42.50;
  option broadcast-address 192.168.42.255;
  option routers 192.168.42.1;
  default-lease-time 600;
  max-lease-time 7200;
  option domain-name "local";
  option domain-name-servers 8.8.8.8, 8.8.4.4;
}
EOF_CAT

sed -i '/^#net.ipv4.ip_forward=1$/s/^#//' etc/sysctl.conf

cat <<- EOF_CAT > etc/iptables.ipv4.nat
*nat
:PREROUTING ACCEPT [0:0]
:INPUT ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
:POSTROUTING ACCEPT [0:0]
-A POSTROUTING -o eth0 -j MASQUERADE
COMMIT
*mangle
:PREROUTING ACCEPT [0:0]
:INPUT ACCEPT [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
:POSTROUTING ACCEPT [0:0]
COMMIT
*filter
:INPUT ACCEPT [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
-A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
-A FORWARD -i wlan0 -o eth0 -j ACCEPT
COMMIT
EOF_CAT

cat <<- EOF_CAT > etc/iptables.ipv4.nonat
*nat
:PREROUTING ACCEPT [0:0]
:INPUT ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
:POSTROUTING ACCEPT [0:0]
COMMIT
*mangle
:PREROUTING ACCEPT [0:0]
:INPUT ACCEPT [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
:POSTROUTING ACCEPT [0:0]
COMMIT
*filter
:INPUT ACCEPT [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
COMMIT
EOF_CAT

apt-get clean

echo root:$passwd | chpasswd

service ntp stop

history -c
EOF_CHROOT

rm $root_dir/etc/resolv.conf
rm $root_dir/usr/bin/qemu-arm-static

# Unmount file systems

umount $boot_dir $root_dir

rmdir $boot_dir $root_dir
