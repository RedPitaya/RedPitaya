chroot $ROOT_DIR <<- EOF_CHROOT
apt-get -y install fake-hwclock \
  python3 python3-numpy build-essential libfftw3-dev python3-scipy \
  xfonts-base tightvncserver xfce4-panel xfce4-session xfwm4 xfdesktop4 \
  xfce4-terminal thunar gnome-icon-theme \
  xserver-xorg xinit xserver-xorg-video-fbdev

X11DIR=$root_dir/usr/share/X11/xorg.conf.d
mkdir -p $X11DIR
cat << EOF_FBDEV > $X11DIR/99-fbdev.conf
Section "Device"  
  Identifier "myfb"
  Driver "fbdev"
  Option "fbdev" "/dev/fb0"
EndSection
EOF_FBDEV

#echo te-audio-codec >> $root_dir/etc/modules

EOF_CHROOT
