# Network configuration

The current network configuration is using [`systemd-networkd`](https://www.freedesktop.org/software/systemd/man/systemd.network.html) as the base.
Almost all network configuration details are done by the bash script [`network.sh`](OS/debian/network.sh)
during the creation of the Debian/Ubuntu SD card image. The script installs networking
related packages and copies network configuration files from the Git repository.

The decision to focus on `systemd-networkd` is arbitrary, while at the same time
focusing at a single approach centered around [`systemd`](https://www.freedesktop.org/wiki/Software/systemd/)
should minimize the efforts needed to maintain it.

Most of the WiFi configuration complexity comes from two sources:
1. mixing WiFi drivers based on new [mac80211](https://wireless.wiki.kernel.org/en/developers/documentation/mac80211) API,
   and the old deprecated [wext](https://wireless.wiki.kernel.org/en/developers/documentation/wireless-extensions) API,
   this also requires some duplication of userspace tools
2. support for switching between WiFi access point and client mode

## UDEV

`systemd` provides [predictable network interface names] using [`UDEV`](https://www.freedesktop.org/software/systemd/man/udev.html) rules.
In our case the kernel names the USB WiFi adapeter `wlan0`, then `UDEV` rule `/lib/udev/rules.d/73-special-net-names.rules`
renames it into `wlxMAC` using the following rule:
```
# Use MAC based names for network interfaces which are directly or indirectly
# on USB and have an universally administered (stable) MAC address (second bit
# is 0).
ACTION=="add", SUBSYSTEM=="net", SUBSYSTEMS=="usb", NAME=="", ATTR{address}=="?[014589cd]:*", IMPORT{builtin}="net_id", NAME="$env{ID_NET_NAME_MAC}"
```
For a simple generic WiFi configuration it is preferred to have the same
interface name regardless of the used adapter. This is achieved by overriding
`UDEV` rules with a modified rule file. The overriding is done by placing the
modified rule file into directory `/etc/udev/rules.d/73-special-net-names.rules`.
Since the remaining rules in the file are not relevant on Red Pitaya, it is also
possible to deactivate the rule by creating a override file which links to `/dev/null`.
```bash
ln -s /dev/null /etc/udev/rules.d/73-special-net-names.rules
```

For userspace tools to be able to distinguish between adapters using old and new drivers,
adapeter interfaces using the `rtl8192cu` are renamed into `wlan0wext` while adapter
interfaces using other drivers keep the default name `wlan0`. This is achieved using
[systemd.link](https://www.freedesktop.org/software/systemd/man/systemd.link.html) file
[/etc/systemd/network/10-wireless.link](OS/debian/overlay/etc/systemd/network/10-wireless.link).

## Wired setup

The wired interface `eth0` configuration file [`/etc/systemd/network/wired.network`](OS/debian/overlay/etc/systemd/network/wired.network)
configures it to use DHCP.

In previous releases, where a [different DHCP client was used](http://linux.die.net/man/8/dhclient),
it was possible to define a fixed lease, which would provide a fallback address
if DHCP fails. Using the `systemd` integrated DHCP client this is not possible,
instead a fixed address can be set, or Link Local addressing zeroconf can be
used (described later).

A static IP address can be chosen by modifying the configuration file. It is
also possible to have both a DHCP provided and a static address at the same time,
but this can not appropriate to be set as the release default since it can
cause IP address colissions. A fixed IP address can be confugured by adding the
next lines to [systemd.network](https://www.freedesktop.org/software/systemd/man/systemd.network.html) files.
```
[Network]
Address=192.168.0.15/24
Gateway=192.168.0.1
DNS=8.8.8.8 8.8.4.4
```

## Wireless setup

The wireless interface `wlan0` configuration file is [`/etc/systemd/network/wireless.network`](OS/debian/overlay/etc/systemd/network/wirless.network).

To support two modes this file must be linked to either the client mode configuration
[`/etc/systemd/network/wireless.network.client`](OS/debian/overlay/etc/systemd/network/wirless.network.client)
or the access point configuration
[`/etc/systemd/network/wireless.network.ap`](OS/debian/overlay/etc/systemd/network/wirless.network.ap).
Switching between the two option is implemented
by [/etc/systemd/system/wireless-mode-ap.service](OS/debian/overlay/etc/systemd/system/wireless-mode-ap.service)
and [/etc/systemd/system/wireless-mode-client.service](OS/debian/overlay/etc/systemd/system/wireless-mode-client.service)
which must be run early at boot before most other network related services are run.

The choice of the interface is driven by the availability of access point
`/opt/redpitaya/hostapd.conf` and cliend `/opt/redpitaya/wpa_supplicant.conf`
configuration files. If `wpa_supplicant.conf` is present, client mode configuration
will be attempted, regardles of the presence of `hostapd.conf`. If only `hostapd.conf`
is present access point configuration will be attempted. If no configuration
file is present, WiFi will not be configured.

| file                  | comment
|-----------------------|-------------------------------------------------------
| `wpa_supplicant.conf` | client configuration
| `hostapd.conf`        | access point configuration

### Wireless client setup

Wireless networks almost universally use some king of encryption/authentication
scheme for security. This is handled by the tool [`wpa_supplicant`](https://w1.fi/wpa_supplicant/).
The default network configuration option on [Debian](https://wiki.debian.org/NetworkManager)/[Ubuntu](https://help.ubuntu.com/community/NetworkManager)
is [NetworkManager](https://wiki.gnome.org/Projects/NetworkManager). Sometimes
it conflicts with the default `systemd-networkd` install, this seems to be one
of those cases. On [Debian](https://packages.debian.org/jessie/armhf/wpasupplicant/filelist)/Ubuntu
a device specific [`wpa_supplicant@.service`](https://w1.fi/cgit/hostap/tree/wpa_supplicant/systemd/wpa_supplicant.service.arg.in)
service is missing, so we made a [copy](OS/debian/overlay/etc/systemd/system/wpa_supplicant@.service)
in our Git repository.

By default the service is installed as a dependency for `multi-user.target`
which means it would delay `multi-user.target` if it could not start properly,
for example due to the USB WiFi adapter not being plugged in. At the same time
the service was not automatically started after the adapter was plugged into
Red Pitaya. The next change fixes both.
```
 [Install]
-Alias=multi-user.target.wants/wpa_supplicant@%i.service
+WantedBy=sys-subsystem-net-devices-%i.device
```

The encryption/authentication configuration file is linked to the FAT partition
for easier user access. So it is enough to provide a proper `wpa_supplicant.conf`
file on the FAT partition to enable wireless client mode.
```bash
ln -s /opt/redpitaya/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant.conf
```
This configuration file can be created using the `wpa_passphrase` tool can be used:
```
$ wpa_passphrase <ssid> [passphrase] > /opt/redpitaya/wpa_supplicant.conf
```

TODO add 

### Wireless access point setup

TODO

To enable access point mode a configuration file `hostapd.conf` must be placed on
the FAT partition on the SD card, and the client mode configuration file `wpa_supplicant.conf`
must be removed. Inside a shell on Red Pitaya this file is visible as `/opt/redpitaya/hostapd.conf`.
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
This file must be edited to set the chosen `<ssid>` and `<passphrase>`.
Other seittings are for the currently most secure personal encryption.

### Supported USB WiFi adapters

Our main target was a low cost USB adapter which also supports access point mode.
The Edimax EW-7811Un adapter is also commonly used on Raspberry PI.
```bash
$ lsusb
ID 7392:7811 Edimax Technology Co., Ltd EW-7811Un 802.11n Wireless Adapter [Realtek RTL8188CUS]
```
The kernel upstream driver for this chip is now working well, so a working
driver was copied from the Raspberry PI repository and applied as a patch.

Other WiFi USB devices might also be supported by upstream kernel drivers,
but there is no comprehensive list for now.

## Resolver

To enable the `systemd` integrated resolver, a symlink for `/etc/resolv.conf` must be created.
```bash
ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf
```
It is also possible to add default DNS servers by adding them to `*.network` files.
```
nameserver=8.8.8.8
nameserver=8.8.4.4
```

## NTP

Instead of using the common `ntpd` the lightweight `systemd-timesyncd`
[SNTP](http://www.ntp.org/ntpfaq/NTP-s-def.htm#AEN1271) client is used.
Since by default NTP servers are provided by DHCP, no additional configuration changes to
[`timesyncd.conf`](https://www.freedesktop.org/software/systemd/man/timesyncd.conf.html) are needed.

To observe the status of time synchronization do:
```bash
$ timedatectl status
```
To enable the service do:
```bash
# timedatectl set-ntp true
```

## SSH

The Open SSH server is installed and access to the root user is enabled.
TODO: changed certificate generation, so they will not be generated during SD card image creation. Instead certificates should be created during the first boot.

## Zeroconf

TODO


## `systemd` services

Services handling the described configuration are enabled with:
```bash
systemctl enable systemd-networkd
systemctl enable systemd-resolved
systemctl enable wpa_supplicant@wlan0.service
systemctl enable systemd-timesyncd
```

