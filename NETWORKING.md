# WiFi configuration

Red Pitaya OS allows the use of USB WiFi adapters to achieve wireless connectivity. It is possible to setup WiFi in client mode, or to set up an access point.

This document explains how to setup WiFi by manually editing configuration files. For ease of use, especially for Windows OS users, the files are stored on the FAT partition on the SD card.

| file                  | comment
|-----------------------|-------------------------------------------------------
| `wpa_supplicant.conf` | client configuration
| `hostapd.conf`        | access point configuration

It is possible to create WiFi configuration files using our WEB services, but this will not be discussed here.

## Supported devices

Our main target was a low cost USB adapter which also supports access point mode. The Edimax EW-7811Un adapter is also commonly used on Raspberry PI.
```bash
$ lsusb
ID 7392:7811 Edimax Technology Co., Ltd EW-7811Un 802.11n Wireless Adapter [Realtek RTL8188CUS]
```
The kernel upstream driver for this chip is now working well, so a working driver was copied from the Raspberry PI repository and applied as a patch.

Other WiFi USB devices might also be supported by upstream kernel drivers, but there is no comprehensive list for now.

## Client mode

To enable client mode a configuration file `wpa_suplicant.conf` must be placed on the FAT partition on the SD card. Inside a shell on Red Pitaya this file is visible as `/opt/redpitaya/wpa_suplicant.conf`. This file must be edited to set the correct `<ssid>` and `<passphrase>` values to be able to connect to the WiFi access point (wireless router) of your choice.

### Password protected network

The currently most secure personal encryption should be used, consult the Internet for other encryption options.

```
# Red Pitaya WiFi configuration file
ctrl_interface=/var/run/wpa_supplicant
network={
		ssid="<ssid>"
#		proto=RSN
		key_mgmt=WPA-PSK
#		pairwise=CCMP TKIP
#		group=CCMP TKIP
		psk="<passphrase>"
}
```

### Open network

We strongly discourage using open networks. This is nothing specific to Red Pitaya, it is just a common security practice to use the strongest WiFi encryption available.

```
# Red Pitaya WiFi configuration file
ctrl_interface=/var/run/wpa_supplicant
network={
        ssid="<ssid>"
        key_mgmt=NONE
}
```

## Access point mode

To enable access point mode a configuration file `hostapd.conf` must be placed on the FAT partition on the SD card, and the client mode configuration file `wpa_suplicant.conf` must be removed. Inside a shell on Red Pitaya this file is visible as `/opt/redpitaya/hostapd.conf`.
```
interface=wlan0
ssid=<ssid>
driver=rtl871xdrv
hw_mode=g
channel=6
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=<passphrase>
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
```
This file must be edited to set the chosen `<ssid>` and `<passphrase>`. Other seittings are for the currently most secure personal encryption.
