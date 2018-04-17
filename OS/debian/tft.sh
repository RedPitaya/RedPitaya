# Added by DM; 2017/10/17 to check ROOT_DIR setting
if [ $ROOT_DIR ]; then 
    echo ROOT_DIR is "$ROOT_DIR"
else
    echo Error: ROOT_DIR is not set
    echo exit with error
    exit
fi

chroot $ROOT_DIR <<- EOF_CHROOT
# install sshfs
apt-get -y install sshfs

# development tools
apt-get -y install build-essential libfftw3-dev

# Python
apt-get -y install python3 python3-numpy python3-scipy

# X server and xfce
apt-get -y install \
  build-essential libfftw3-dev \
  xfonts-base tightvncserver xfce4-panel xfce4-session xfwm4 xfdesktop4 \
  xfce4-terminal thunar gnome-icon-theme \
  xserver-xorg xinit xserver-xorg-video-fbdev

# touch screen debug tools
apt-get -y install xinput evtest

# This is just a placeholder, audio is not available
#echo te-audio-codec >> $root_dir/etc/modules

# install QT5
apt-get -y install qt5-default libqt5script5 libqt5scripttools5 libqt5serialport5

# install X2Go
sudo add-apt-repository ppa:x2go/stable
sudo apt-get update
apt-get -y install x2goserver
EOF_CHROOT

install -v -m 664 -o root -D $OVERLAY/usr/share/X11/xorg.conf.d/99-fbdev.conf $ROOT_DIR/usr/share/X11/xorg.conf.d/99-fbdev.conf
install -v -m 664 -o root -D $OVERLAY/etc/X11/xinit/xinitrc                   $ROOT_DIR/etc/X11/xinit/xinitrc

# creation of UDEV rules for the creation of symlink /dev/input/touchscreen
install -v -m 664 -o root -D $OVERLAY/etc/udev/rules.d/95-ads7846.rules       $ROOT_DIR/etc/udev/rules.d/95-ads7846.rules
install -v -m 664 -o root -D $OVERLAY/etc/udev/rules.d/95-stmpe.rules         $ROOT_DIR/etc/udev/rules.d/95-stmpe.rules

install -v -m 664 -o root -D $OVERLAY/etc/X11/xorg.conf.d/99-calibration.conf $ROOT_DIR/etc/X11/xorg.conf.d/99-calibration.conf
