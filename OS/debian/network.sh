# network configuration
install -v -m 664 -o root -D $OVERLAY/etc/udev/rules.d/75-persistent-net-generator.rules $ROOT_DIR/etc/udev/rules.d/75-persistent-net-generator.rules
install -v -m 664 -o root -D $OVERLAY/etc/default/ifplugd                                $ROOT_DIR/etc/default/ifplugd
# NOTE: the next line is now preformed elsewhere, while preparing the ecosystem ZIP for the FAT partition
#install -v -m 664 -o root -D $OVERLAY/etc/hostapd/hostapd.conf                           $BOOT_DIR/hostapd.conf
install -v -m 664 -o root -D $OVERLAY/etc/default/hostapd                                $ROOT_DIR/etc/default/hostapd
install -v -m 664 -o root -D $OVERLAY/etc/dhcp/dhcpd.conf                                $ROOT_DIR/etc/dhcp/dhcpd.conf
install -v -m 664 -o root -D $OVERLAY/etc/dhcp/dhclient.conf                             $ROOT_DIR/etc/dhcp/dhclient.conf
install -v -m 664 -o root -D $OVERLAY/etc/iptables.ipv4.nat                              $ROOT_DIR/etc/iptables.ipv4.nat
install -v -m 664 -o root -D $OVERLAY/etc/iptables.ipv4.nonat                            $ROOT_DIR/etc/iptables.ipv4.nonat
install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces                             $ROOT_DIR/etc/network/interfaces
install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/eth0                      $ROOT_DIR/etc/network/interfaces.d/eth0
# TODO: the next three files are not handled cleanly, netwoking should be documented and cleaned 
install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/wlan0.ap                  $ROOT_DIR/etc/network/interfaces.d/wlan0.ap
install -v -m 664 -o root -D $OVERLAY/etc/network/interfaces.d/wlan0.client              $ROOT_DIR/etc/network/interfaces.d/wlan0.client
ln -s                                                          wlan0.ap                  $ROOT_DIR/etc/network/interfaces.d/wlan0

chroot $ROOT_DIR <<- EOF_CHROOT
sed -i '/^#net.ipv4.ip_forward=1$/s/^#//' /etc/sysctl.conf
EOF_CHROOT


