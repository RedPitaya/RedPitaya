rw
rm -fr /opt/redpitaya/wpa_supplicant.conf
echo '# Red Pitaya WiFi configuration file' >> /opt/redpitaya/wpa_supplicant.conf
echo 'ctrl_interface=/var/run/wpa_supplicant' >> /opt/redpitaya/wpa_supplicant.conf
echo 'network={' >> /opt/redpitaya/wpa_supplicant.conf
echo '		ssid="'$1'"' >> /opt/redpitaya/wpa_supplicant.conf
echo '#		proto=RSN' >> /opt/redpitaya/wpa_supplicant.conf
echo '		key_mgmt=WPA-PSK' >> /opt/redpitaya/wpa_supplicant.conf
echo '#		pairwise=CCMP TKIP' >> /opt/redpitaya/wpa_supplicant.conf
echo '#		group=CCMP TKIP' >> /opt/redpitaya/wpa_supplicant.conf
echo '		psk="'$2'"' >> /opt/redpitaya/wpa_supplicant.conf
echo '}' >> /opt/redpitaya/wpa_supplicant.conf
echo '' >> /opt/redpitaya/wpa_supplicant.conf
reboot