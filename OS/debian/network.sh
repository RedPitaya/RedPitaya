# systemd-networkd wired/wireless network configuration (DHCP and WPA supplicant for WiFi)
mkdir                                                                                    $ROOT_DIR/etc/iptables
install -v -m 664 -o root -D $OVERLAY/etc/iptables/iptables.rules                        $ROOT_DIR/etc/iptables/iptables.rules
install -V -m 755 -o root -D $OVERLAY/etc/systemd/system/iptables-flush                  $ROOT_DIR/etc/systemd/system/iptables-flush
install -v -m 664 -o root -D $OVERLAY/etc/systemd/network/wired.network                  $ROOT_DIR/etc/systemd/network/wired.network
install -v -m 664 -o root -D $OVERLAY/etc/systemd/network/wireless.network.client        $ROOT_DIR/etc/systemd/network/wireless.network.client
install -v -m 664 -o root -D $OVERLAY/etc/systemd/network/wireless.network.ap            $ROOT_DIR/etc/systemd/network/wireless.network.ap
install -v -m 664 -o root -D $OVERLAY/etc/systemd/network/10-wireless.link               $ROOT_DIR/etc/systemd/network/10-wireless.link
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/ssh-reconfigure.service         $ROOT_DIR/etc/systemd/system/ssh-reconfigure.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/wireless_adapter_up@.service    $ROOT_DIR/etc/systemd/system/wireless_adapter_up@.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/wireless-mode-client.service    $ROOT_DIR/etc/systemd/system/wireless-mode-client.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/wireless-mode-ap.service        $ROOT_DIR/etc/systemd/system/wireless-mode-ap.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/wpa_supplicant@.service         $ROOT_DIR/etc/systemd/system/wpa_supplicant@.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/wpa_supplicant_wext@.service    $ROOT_DIR/etc/systemd/system/wpa_supplicant_wext@.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/hostapd@.service                $ROOT_DIR/etc/systemd/system/hostapd@.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/iptables.service                $ROOT_DIR/etc/systemd/system/iptables.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/wpa_supplicant@.path            $ROOT_DIR/etc/systemd/system/wpa_supplicant@.path
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/wpa_supplicant_wext@.path       $ROOT_DIR/etc/systemd/system/wpa_supplicant_wext@.path
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/hostapd@.path                   $ROOT_DIR/etc/systemd/system/hostapd@.path

# Avahi daemon configuration files
install -v -m 664 -o root -D $OVERLAY/etc/avahi/services/ssh.service                     $ROOT_DIR/etc/avahi/services/ssh.service
install -v -m 664 -o root -D $OVERLAY/etc/avahi/services/bazaar.service                  $ROOT_DIR/etc/avahi/services/bazaar.service
install -v -m 664 -o root -D $OVERLAY/etc/avahi/services/scpi.service                    $ROOT_DIR/etc/avahi/services/scpi.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/hostname-mac.service            $ROOT_DIR/etc/systemd/system/hostname-mac.service

chroot $ROOT_DIR <<- EOF_CHROOT
# network tools
apt-get -y install iproute2 iputils-ping curl

# WiFi AP configuration and DHCP server
#apt-get -y install isc-dhcp-server

# SSH access
# TODO: check cert generation, should it be moved to first boot?
apt-get -y install openssh-server ca-certificates
# enable SSH access to the root user
sed -i 's/^PermitRootLogin.*/PermitRootLogin yes/' /etc/ssh/sshd_config
# remove SSH keys, so they can be created at boot by ssh-reconfigure.service
/bin/rm -v /etc/ssh/ssh_host_*

# WiFi tools
# TODO: install was asking about /etc/{protocols,services}
apt-get -y install linux-firmware
apt-get -y install wpasupplicant iw

# WiFi tools (wext)
apt-get -y install wireless-tools

# WiFi tools (AP)
apt-get -y install libnl-3-dev libnl-genl-3-dev pkg-config libssl-dev
mkdir -p /etc/hostapd/
ln -sf /opt/redpitaya/hostapd.conf /etc/hostapd/hostapd.conf

# compile hostapd
apt-get -y install iptables
apt-get -y install build-essential gcc 
#git clone https://github.com/pritambaral/hostapd-rtl871xdrv.git
curl -L https://github.com/pritambaral/hostapd-rtl871xdrv/archive/master.tar.gz -o hostapd-rtl871xdrv-master.tar.gz
tar zxvf hostapd-rtl871xdrv-master.tar.gz
curl -L http://w1.fi/releases/hostapd-2.5.tar.gz -o hostapd-2.5.tar.gz
tar zxvf hostapd-2.5.tar.gz
cd hostapd-2.5
patch -p1 -i ../hostapd-rtl871xdrv-master/rtlxdrv.patch
cd hostapd
cp defconfig .config
echo CONFIG_DRIVER_RTW=y >> .config
echo CONFIG_LIBNL32=y    >> .config
make
make install
cd ../../
rm -rf hostapd*

# this enables placing the WiFi WPA configuration into the FAT partition
ln -s /opt/redpitaya/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant.conf

# this is a fix for persistent naming rules for USB network adapters
# otherwise WiFi adapters are named "wlx[MACAddress]"
ln -s /dev/null /etc/udev/rules.d/73-special-net-names.rules

# use systemd-reloslver
# TODO: this link is currently created at the end of the install process, just before unmounting the image (ubuntu.sh)
#ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf

# Avahi daemon
apt-get -y install avahi-daemon libnss-mdns

# enable systemd network related services
systemctl enable systemd-networkd.service
systemctl enable systemd-resolved.service
systemctl enable systemd-timesyncd.service
systemctl enable systemd-networkd-wait-online.service

# wireless related services
systemctl enable wireless_adapter_up@wlan0.service
systemctl enable wireless_adapter_up@wlan0wext.service
systemctl enable wpa_supplicant@wlan0.service
systemctl enable wpa_supplicant_wext@wlan0wext.service
systemctl enable hostapd@wlan0.service
systemctl enable hostapd@wlan0wext.service
systemctl enable wireless-mode-client.service
systemctl enable wireless-mode-ap.service
systemctl enable iptables.service
#systemctl enable wpa_supplicant@wlan0.path
#systemctl enable wpa_supplicant_wext@wlan0wext.path
#systemctl enable hostapd@wlan0.path
#systemctl enable hostapd@wlan0wext.path

# zeroconf/avahi
systemctl enable hostname-mac.service
systemctl enable avahi-daemon.service
mkdir -p /etc/systemd/system/avahi-daemon.service.d
printf "[Unit]\nAfter = systemd-resolved.service\n" > /etc/systemd/system/avahi-daemon.service.d/ad.conf

# enable service for creating SSH keys on first boot
systemctl enable ssh-reconfigure.service
EOF_CHROOT
