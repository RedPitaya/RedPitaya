chroot $ROOT_DIR <<- EOF_CHROOT
apt-get -y install fake-hwclock \
  python3 python3-numpy build-essential libfftw3-dev python3-scipy \
  xfonts-base tightvncserver xfce4-panel xfce4-session xfwm4 xfdesktop4 \
  xfce4-terminal thunar gnome-icon-theme \
  xserver-xorg xinit xserver-xorg-video-fbdev
apt-get -y install xinput evtest

#echo te-audio-codec >> $root_dir/etc/modules

EOF_CHROOT

install -v -m 664 -o root -D $OVERLAY/usr/share/X11/xorg.conf.d/99-fbdev.conf $ROOT_DIR/usr/share/X11/xorg.conf.d/99-fbdev.conf
install -v -m 664 -o root -D $OVERLAY/etc/X11/xinit/xinitrc                   $ROOT_DIR/etc/X11/xinit/xinitrc

# creation of UDEV rules for the creation of symlink /dev/input/touchscreen
install -v -m 664 -o root -D $OVERLAY/etc/udev/rules.d/95-ads7846.rules       $ROOT_DIR/etc/udev/rules.d/95-ads7846.rules
install -v -m 664 -o root -D $OVERLAY/etc/udev/rules.d/95-stmpe.rules         $ROOT_DIR/etc/udev/rules.d/95-stmpe.rules

install -v -m 664 -o root -D $OVERLAY/etc/X11/xorg.conf.d/99-calibration.conf $ROOT_DIR/etc/X11/xorg.conf.d/99-calibration.conf
