## network configuration
#install -v -m 664 -o root -D $OVERLAY/etc/udev/rules.d/75-persistent-net-generator.rules $ROOT_DIR/etc/udev/rules.d/75-persistent-net-generator.rules
#install -v -m 664 -o root -D $OVERLAY/etc/default/ifplugd                                $ROOT_DIR/etc/default/ifplugd
## NOTE: the next line is now preformed elsewhere, while preparing the ecosystem ZIP for the FAT partition
##install -v -m 664 -o root -D $OVERLAY/etc/hostapd/hostapd.conf                           $BOOT_DIR/hostapd.conf
#install -v -m 664 -o root -D $OVERLAY/etc/default/hostapd                                $ROOT_DIR/etc/default/hostapd
#install -v -m 664 -o root -D $OVERLAY/etc/dhcp/dhcpd.conf                                $ROOT_DIR/etc/dhcp/dhcpd.conf
#install -v -m 664 -o root -D $OVERLAY/etc/dhcp/dhclient.conf                             $ROOT_DIR/etc/dhcp/dhclient.conf
#install -v -m 664 -o root -D $OVERLAY/etc/iptables.ipv4.nat                              $ROOT_DIR/etc/iptables.ipv4.nat
#install -v -m 664 -o root -D $OVERLAY/etc/iptables.ipv4.nonat                            $ROOT_DIR/etc/iptables.ipv4.nonat
#install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces                             $ROOT_DIR/etc/network/interfaces
#install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/eth0                      $ROOT_DIR/etc/network/interfaces.d/eth0
## TODO: the next three files are not handled cleanly, netwoking should be documented and cleaned 
#install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/wlan0.ap                  $ROOT_DIR/etc/network/interfaces.d/wlan0.ap
#install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/wlan0.client              $ROOT_DIR/etc/network/interfaces.d/wlan0.client
#ln -s                                                          wlan0.ap                  $ROOT_DIR/etc/network/interfaces.d/wlan0

# systemd-networkd wired/wireless network configuration (DHCP and WPA supplicant for WiFi)
install -v -m 664 -o root -D $OVERLAY/etc/systemd/network/wired.network                  $ROOT_DIR/etc/systemd/network/wired.network
install -v -m 664 -o root -D $OVERLAY/etc/systemd/network/wireless.network.client        $ROOT_DIR/etc/systemd/network/wireless.network.client
install -v -m 664 -o root -D $OVERLAY/etc/systemd/network/wireless.network.ap            $ROOT_DIR/etc/systemd/network/wireless.network.ap
install -v -m 664 -o root -D $OVERLAY/etc/systemd/network/wireless.network               $ROOT_DIR/etc/systemd/network/wireless.network
install -v -m 664 -o root -D $OVERLAY/etc/systemd/network/wireless.link                  $ROOT_DIR/etc/systemd/network/wireless.link
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/wpa_supplicant@.service         $ROOT_DIR/etc/systemd/system/wpa_supplicant@.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/wpa_supplicant_wext@.service    $ROOT_DIR/etc/systemd/system/wpa_supplicant_wext@.service

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
ln -sf /opt/redpitaya/hostapd.conf /etc/hostapd/hostapd.conf

# compile hostapd
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

# this enables placing the WiFi WPA configuration into the FAT partition
ln -s /opt/redpitaya/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant-wlan0.conf

# this is a fix for persistent naming rules for USB network adapters
# otherwise WiFi adapters are named "wlx[MACAddress]"
ln -s /dev/null /etc/udev/rules.d/73-special-net-names.rules

# use systemd-reloslver
# TODO: this link is currently created at the end of the install process, just before unmounting the image (ubuntu.sh)
#ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf

# there is a Systemd approach to this, might be used later
#sed -i '/^#net.ipv4.ip_forward=1$/s/^#//' /etc/sysctl.conf

# enable systemd network related services
systemctl enable systemd-networkd
systemctl enable systemd-resolved
systemctl enable systemd-timesyncd
systemctl enable wpa_supplicant@wlan0.service
systemctl enable wpa_supplicant_wext@wlan0wext.service
systemctl enable hostapd@wlan0.service 
systemctl enable hostapd@wlan0wext.service 

# enable service for creating SSH keys on first boot
systemctl enable ssh-reconfigure
EOF_CHROOT
