

# Added by DM; 2017/10/17 to check ROOT_DIR setting
if [ $ROOT_DIR ]; then 
    echo ROOT_DIR is "$ROOT_DIR"
else
    echo Error: ROOT_DIR is not set
    echo exit with error
    exit
fi

install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/r8188eu.network.services $ROOT_DIR/etc/systemd/system/r8188eu.network.service
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/netstart.service $ROOT_DIR/etc/systemd/system/netstart.service

chroot $ROOT_DIR <<- EOF_CHROOT

export DEBIAN_FRONTEND=noninteractive
apt -y install wireless-tools

systemctl enable r8188eu.network
systemctl enable netstart.service
systemctl disable wpa_supplicant@wlan0.service
rm -f /etc/systemd/system/wpa_supplicant@.service

echo 1.07 > /root/.version

EOF_CHROOT
