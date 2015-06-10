################################################################################
# TODO: copyright notice and authors should be listed here
################################################################################

# enable chroot access with native execution
cp /etc/resolv.conf         $root_dir/etc/
cp /usr/bin/qemu-arm-static $root_dir/usr/bin/

# Copy files to the boot file system
unzip ecosystem*.zip -d $boot_dir
cp -f $boot_dir/u-boot.scr.debian $boot_dir/u-boot.scr

# Nginx service
install -v -m 664 -o root -d                                                     $root_dir/var/log/redpitaya_nginx
install -v -m 664 -o root -D $OVERLAY/etc/systemd/system/redpitaya_nginx.service $root_dir/etc/systemd/system/redpitaya_nginx.service
install -v -m 664 -o root -D $OVERLAY/etc/sysconfig/redpitaya                    $root_dir/etc/sysconfig/redpitaya

chroot $root_dir <<- EOF_CHROOT
systemctl enable redpitaya_nginx

apt-get -y install libluajit-5.1 lua-cjson unzip
EOF_CHROOT

# final operations and cleanup
install -v -m 664 -o root -D $OVERLAY/etc/profile.d/redpitaya.sh $root_dir/etc/profile.d/redpitaya.sh

# disable chroot access with native execution
rm $root_dir/etc/resolv.conf
rm $root_dir/usr/bin/qemu-arm-static
