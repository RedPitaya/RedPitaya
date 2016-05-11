#!/bin/sh

echo
echo "Upgrading current RedPitaya image to support additional RadioBox features"
echo "========================================================================="
echo

echo
echo "Step 1: preparing kernel modules"
echo "------"
ln -s /opt/redpitaya/lib/modules /lib/modules
depmod -a
echo "... done."

echo
echo "Step 2: updating the dpkg catalog"
echo "------"
rw
apt-get update -y
echo "... done."

echo
echo "Step 3:"
echo "------"
apt-get upgrade -y
echo "... done."

echo
echo "Step 4: installing additionally packages"
echo "------"
apt-get -y install alsaplayer-alsa alsa-tools alsa-utils dbus dbus-x11 dosfstools esound-common flac icecast2 ices2 jack-tools locate multicat pavucontrol pulseaudio pulseaudio-esound-compat pulseaudio-module-jack rsync speex tcpdump vorbis-tools x11-common x11-xkb-utils x11-xserver-utils xauth xfonts-100dpi xfonts-75dpi xfonts-base xfonts-encodings xfonts-scalable xfonts-utils xinetd xkb-data xserver-common  xserver-xorg-core
echo "... done."

echo
echo "Step 5: adding dpkg selections and upgrading to the current Ubuntu release."
echo "------"
dpkg --get-selections > RadioBox-Upgrade_dpkg-selections-current.dat
LC_ALL=C cat RadioBox-Upgrade_dpkg-selections-needed.dat RadioBox-Upgrade_dpkg-selections-current.dat | grep -v deinstall | sort | uniq > RadioBox-Upgrade_dpkg-selections-new.dat
dpkg --set-selections < RadioBox-Upgrade_dpkg-selections-new.dat
echo "... done."

echo
echo "Step 6:"
echo "------"
apt-get upgrade -y
echo "... done."

echo
echo "Step 7: clean-up not more needed automatic packages"
echo "------"
apt-get autoremove -y
echo "... done."

echo
echo "Step 8: clean-up packages not more needed"
echo "------"
apt-get autoclean -y
echo "... done."

echo
echo "Step 9: setting up new file links"
echo "------"
mv /etc/xinetd.conf /etc/xinetd.conf_old
mv /etc/xinetd.d    /etc/xinetd.d_old
cp -r /opt/redpitaya/etc/xinetd.conf /etc/xinetd.conf
cp -r /opt/redpitaya/etc/xinetd.d    /etc/xinetd.d
rm -rf /etc/xinetd.conf_old /etc/xinetd.d_old
echo "... done."

echo
echo "Step 10: setting up audio streaming"
echo "-------"
addgroup --quiet --gid 115 icecast
adduser  --quiet --home /usr/share/icecast2 --disabled-password --disabled-login --uid 115 --gid 115 icecast2
adduser  --quiet icecast2 icecast
mv /etc/icecast2 /etc/icecast2_old
mv /etc/ices2    /etc/ices2_old 
cp -r /opt/redpitaya/etc/icecast2 /etc/icecast2
cp -r /opt/redpitaya/etc/ices2    /etc/ices2
chown -R icecast2:icecast /etc/icecast2 /etc/ices2
rm -rf /etc/icecast2_old
rm -rf /etc/ices2_old
cp /opt/redpitaya/www/apps/radiobox/bin/RadioBox-Upgrade_etc_default_icecast2 /etc/default/icecast2

echo
echo "Step 11: setting up sound system"
echo "-------"
redpitaya-ac97_stop
cp RadioBox-Upgrade_asound.state /var/lib/alsa/asound.state
redpitaya-ac97_start
alsactl restore
# amixer -D pulse sset Master 100% on
# amixer -D pulse sset Capture 100% on
# amixer -D hw:CARD=RedPitayaAC97 sset Master 100% on
# amixer -D hw:CARD=RedPitayaAC97 sset PCM 100% on
# amixer -D hw:CARD=RedPitayaAC97 sset Line 100% off
# amixer -D hw:CARD=RedPitayaAC97 sset Capture 100% on
echo "... done."

echo
echo "Step 12: update locate database"
echo "-------"
updatedb
sync
ro
echo "... done."

echo
echo ">>> FINISH <<<  Congrats, the system is ready for RadioBox additional features"
echo

