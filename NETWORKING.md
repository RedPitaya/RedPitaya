# WiFi configuretion

Red Pitaya OS allows the use of USB WiFi adapters to achieve wireless consctivity. It is possible to setup WiFi in client mode, or to set up an access point.

This document explains how to setup WiFi by manually editing configuration files. For ease of use, expecially for Windows OS users, the files are stored on the FAT partition on the SD card.

| file                  | comment
|-----------------------|-------------------------------------------------------
| `wpa_supplicant.conf` | client configuration
| `hostapd.conf`        | access point congiguration

It is possible to create WiFi configuration files using our WEB services, but this will not be discussed here.

## Supported devices

Our main target was a low cost usb addapter which also supports access point mode. The Edimax EW-7811Un adapter is also commonly used on Raspberry PI.
```bash
$ lsusb
ID 7392:7811 Edimax Technology Co., Ltd EW-7811Un 802.11n Wireless Adapter [Realtek RTL8188CUS]
```
The kernel upstream driver for this chip is now working well, so a working driver was copied from the Raspberry PI repository and applied as a patch.

Other WiFi USB devices might also be supported by upstream kernel drivers, but there is no comprehensive list for now.

## Client mode



## Access point mode

```
interface=wlan0
ssid=Red Pitaya AP
driver=rtl871xdrv
hw_mode=g
channel=6
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=redpitaya
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP

```
