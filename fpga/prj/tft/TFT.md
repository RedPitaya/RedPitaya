Instructions for starting XFCE on the TFT display.

1. download OS image:
http://downloads.redpitaya.com/downloads/red_pitaya_OS-beta.img.zip

2. copy the image onto the SD card

3. remove all files from FAT partition on the SD card

4. extract ecosystem onto the SD card:

5. power up the board

6. SSH into the board and run the next code to install the graphical interface
```bash
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
```

7. over SSH start the X server
```bash
startx
```
