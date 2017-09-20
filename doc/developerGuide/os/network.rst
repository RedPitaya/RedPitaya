.. _network:

#######
Network
#######

***********
Quick setup
***********

.. note:: A reboot is required to switch between access point and client modes.

.. note:: 
    
    In order to set wireless or direct Ethernet connection you need to access Red Pitaya STEMlab 
    :ref:`Console interface <console>`.

===========
WiFi client
===========

A list of `Supported USB Wi-Fi adapters`_ is provided at the bottom of the page.

List wireless access pints:

.. code-block:: shell-session

   # iw wlan0 scan | grep SSID

Write a ``wpa_supplicant.conf`` configuration file to the FAT partition.
*ssid* and *passphrase* can/should be inside parentheses.

.. code-block:: shell-session

   # rw
   $ wpa_passphrase <ssid> [passphrase] > /opt/redpitaya/wpa_supplicant.conf

Restart WPA supplicant:

.. code-block:: shell-session

    # systemctl restart wpa_supplicant@wlan0.service

=================
WiFi access point
=================

Write a `hostapd.conf <https://w1.fi/cgit/hostap/plain/hostapd/hostapd.conf>`_ configuration file to the FAT partition,
and remove the ``wpa_supplicant.conf`` client configuration file if exists:

.. code-block:: shell-session

   # rw
   $ nano /opt/redpitaya/hostapd.conf
   $ rm /opt/redpitaya/wpa_supplicant.conf

Restart access point service:

.. code-block:: shell-session

   # systemctl restart hostapd@wlan0.service

*********************
Network configuration
*********************

The current network configuration is using 
`systemd-networkd <https://www.freedesktop.org/software/systemd/man/systemd.network.html>`_ as the base. Almost all
network configuration details are done by the bash script 
`network.sh </OS/debian/network.sh>`_ during the creation of the 
Debian/Ubuntu SD card image. The script installs networking related packages and copies network configuration files 
from the Git repository.

The decision to focus on ``systemd-networkd`` is arbitrary, while at the same time
focusing at a single approach centered around `systemd <https://www.freedesktop.org/wiki/Software/systemd/>`_
should minimize the efforts needed to maintain it.

Most of the WiFi configuration complexity comes from
support for switching between WiFi access point and client mode

====
UDEV
====

``systemd`` provides [predictable network interface names] using `UDEV <https://www.freedesktop.org/software/systemd/man/udev.html>`_ rules.
In our case the kernel names the USB WiFi adapter ``wlan0``, then ``UDEV`` rule ``/lib/udev/rules.d/73-usb-net-by-mac.rules``
renames it into ``enx{MAC}`` using the following rule:

.. code-block:: shell-session

   # Use MAC based names for network interfaces which are directly or indirectly
   # on USB and have an universally administered (stable) MAC address (second bit
   # is 0).
   
   IMPORT{cmdline}="net.ifnames", ENV{net.ifnames}=="0", GOTO="usb_net_by_mac_end"
   PROGRAM="/bin/readlink /etc/udev/rules.d/80-net-setup-link.rules", RESULT=="/dev/null", GOTO="usb_net_by_mac_end"
   
   ACTION=="add", SUBSYSTEM=="net", SUBSYSTEMS=="usb", NAME=="", \
       ATTR{address}=="?[014589cd]:*", \
       IMPORT{builtin}="net_id", NAME="$env{ID_NET_NAME_MAC}"
   
   LABEL="usb_net_by_mac_end"

For a simple generic WiFi configuration it is preferred to have the same
interface name regardless of the used adapter. This is achieved by overriding
``UDEV`` rules with a modified rule file. The overriding is done by placing the
modified rule file into directory ``/etc/udev/rules.d/73-usb-net-by-mac.rules``.
Since the remaining rules in the file are not relevant on Red Pitaya, it is also
possible to deactivate the rule by creating a override file which links to ``/dev/null``.

.. code-block:: shell-session

   # ln -s /dev/null /etc/udev/rules.d/73-usb-net-by-mac.rules

===========
Wired setup
===========

The wired interface ``eth0`` configuration file `/etc/systemd/network/wired.network
</OS/debian/overlay/etc/systemd/network/wired.network>`_
configures it to use DHCP.

In previous releases, where a `different DHCP client was used <http://linux.die.net/man/8/dhclient>`_,
it was possible to define a fixed lease, which would provide a fallback address
if DHCP fails. Using the ``systemd`` integrated DHCP client this is not possible,
instead a fixed address can be set, or Link Local addressing zeroconf can be
used (described later).

A static IP address can be chosen by modifying the configuration file. It is
also possible to have both a DHCP provided and a static address at the same time,
but this is not a good choice for the release default since it can cause IP address collisions.
A fixed IP address can be configured by adding the next lines to
`systemd.network  <https://www.freedesktop.org/software/systemd/man/systemd.network.html>`_ files.

.. code-block:: none

   [Network]
   Address=192.168.0.15/24
   Gateway=192.168.0.1

==============
Wireless setup
==============

The wireless interface ``wlan0`` configuration file is `/etc/systemd/network/wireless.network </OS/debian/overlay/etc/systemd/network/wireless.network>`_.

To support two modes this file must be linked to either the client mode configuration
`/etc/systemd/network/wireless.network.client </OS/debian/overlay/etc/systemd/network/wireless.network.client>`_
or the access point configuration
`/etc/systemd/network/wireless.network.ap </OS/debian/overlay/etc/systemd/network/wireless.network.ap>`_.
Switching between the two option is implemented by
`/etc/systemd/system/wireless-mode-ap.service </OS/debian/overlay/etc/systemd/system/wireless-mode-ap.service>`_
and
`/etc/systemd/system/wireless-mode-client.service </OS/debian/overlay/etc/systemd/system/wireless-mode-client.service>`_
which must be run early at boot before most other network related services are run.
If no wireless configuration file is available, then a third service
`/etc/systemd/system/wireless_adapter_up@.service </OS/debian/overlay/etc/systemd/system/wireless_adapter_up@.service>`_
will link ``wireless.network`` to client mode, and it will power up the adapter so that ``iwlist`` will work.

The choice of the interface is driven by the availability of access point ``/opt/redpitaya/hostapd.conf``
and client ``/opt/redpitaya/wpa_supplicant.conf`` configuration files.
If ``wpa_supplicant.conf`` is present, client mode configuration will be attempted,
regardless of the presence of ``hostapd.conf``.
If only ``hostapd.conf`` is present access point configuration will be attempted.
If no configuration file is present, WiFi will not be configured.

+-----------------------+------------------------------+
| file                  | comment                      |
+-----------------------+------------------------------+
| `wpa_supplicant.conf` | client configuration         |
+-----------------------+------------------------------+
| `hostapd.conf`        | access point configuration   |
+-----------------------+------------------------------+

---------------------
Wireless client setup
---------------------

Wireless networks almost universally use some king of encryption/authentication scheme for security.
This is handled by the tool `wpa_supplicant <https://w1.fi/wpa_supplicant/>`_.
The default network configuration option on
`Debian NetworkManager <https://wiki.debian.org/NetworkManager>`_ /
`Ubuntu NetworkManager <https://help.ubuntu.com/community/NetworkManager>`_
is `NetworkManager  <https://wiki.gnome.org/Projects/NetworkManager>`_.
Sometimes it conflicts with the default ``systemd-networkd`` install, this seems to be one
of those cases. On `Debian <https://packages.debian.org/jessie/armhf/wpasupplicant/filelist>`_ / Ubuntu
a device `specific @.service <https://w1.fi/cgit/hostap/tree/wpa_supplicant/systemd/wpa_supplicant.service.arg.in>`_
service is missing, so we made a copy `copy of wpa_supplicant@.service </OS/debian/overlay/etc/systemd/system/wpa_supplicant@.service>`_
in our Git repository.

By default the service is installed as a dependency for ``multi-user.target``
which means it would delay ``multi-user.target`` if it could not start properly,
for example due to the USB WiFi adapter not being plugged in. At the same time
the service was not automatically started after the adapter was plugged into
Red Pitaya. The next change fixes both.

.. code-block:: shell-session

    [Install]
   -Alias=multi-user.target.wants/wpa_supplicant@%i.service
   +WantedBy=sys-subsystem-net-devices-%i.device

The encryption/authentication configuration file is linked to the FAT partition
for easier user access. So it is enough to provide a proper ``wpa_supplicant.conf``
file on the FAT partition to enable wireless client mode.

.. code-block:: shell-session

   # ln -s /opt/redpitaya/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant.conf

This configuration file can be created using the `wpa_passphrase` tool can be used:

.. code-block:: shell-session

   $ wpa_passphrase <ssid> [passphrase] > /opt/redpitaya/wpa_supplicant.conf

---------------------------
Wireless access point setup
---------------------------

WiFi access point functionality is provided by the `hostapd <https://w1.fi/hostapd/>`_ application.
Since the upstream version does not support the ``wireless extensions`` API, the application is not
installed as a Debian package, and is instead downloaded, patched, recompiled and installed.

The `hostapd@.service </OS/debian/overlay/etc/systemd/system/hostapd@.service>`_
is handling the start of the daemon. Hotplugging is achieved the same way as with
``wpa_supplicant@.service``.

To enable access point mode a configuration file `hostapd.conf <https://w1.fi/cgit/hostap/plain/hostapd/hostapd.conf>`_
must be placed on the FAT partition on the SD card, and the client mode configuration file ``wpa_supplicant.conf``
must be removed. Inside a shell on Red Pitaya this file is visible as ``/opt/redpitaya/hostapd.conf``.

.. code-block:: none

   interface=wlan0
   ssid=<ssid>
   driver=nl80211
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

This file must be edited to set the chosen ``<ssid>`` and ``<passphrase>``.
Other settings are for the currently most secure personal encryption.

~~~~~~~~~~~~~~~
Wireless router
~~~~~~~~~~~~~~~

In access point mode Red Pitaya behaves as a wireless router,
if the wired interface is connected to the local network.

In the wired network configuration file `/etc/systemd/network/wired.network </OS/debian/overlay/etc/systemd/network/wired.network>`_
there are two lines to enable IP forwarding and masquerading.

.. code-block:: none

   IPForward=yes
   IPMasquerade=yes

An iptables configuration `/etc/iptables/iptables.rules </OS/debian/overlay/etc/iptables/iptables.rules>`_
is enbled by the iptables service `/etc/systemd/system/iptables.service </OS/debian/overlay/etc/systemd/system/iptables.service>`_.

.. note:: This functionality combined with default passwords can be a serious security issue.
   And since it is not needed to provide advertized functionality, we might remove it in the future.

~~~~~~~~~~~~~~~~~~~~~~~~~~~
Supported USB WiFi adapters
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Our main target was a low cost USB adapter which also supports access point mode.
The Edimax EW-7811Un adapter is also commonly used on Raspberry PI.

.. code-block:: shell-session

   $ lsusb
     ID 7392:7811 Edimax Technology Co., Ltd EW-7811Un 802.11n Wireless Adapter [Realtek RTL8188CUS]

The kernel upstream driver for this chip is now working well, so a working
driver was copied from the Raspberry PI repository and applied as a patch.

Other WiFi USB devices might also be supported by upstream kernel drivers,
but there is no comprehensive list for now.

============
DNS Resolver
============

To enable the ``systemd`` integrated resolver, a symlink for ``/etc/resolv.conf`` must be created.

.. code-block:: shell-session

   # ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf

It is also possible to add default DNS servers by adding them to ``*.network`` files.

.. code-block:: none

   nameserver=8.8.8.8
   nameserver=8.8.4.4

===========================
NTP (Network Time Protocol)
===========================

Instead of using the common ``ntpd`` the lightweight ``systemd-timesyncd``
`SNTP  <http://www.ntp.org/ntpfaq/NTP-s-def.htm#AEN1271>`_ client is used.
Since by default NTP servers are provided by DHCP, no additional configuration changes to
`timesyncd.conf <https://www.freedesktop.org/software/systemd/man/timesyncd.conf.html>`_ are needed.

To observe the status of time synchronization do.

.. code-block:: shell-session

   $ timedatectl status

To enable the service do.

.. code-block:: shell-session

   # timedatectl set-ntp true

==========
SSH server
==========

The Open SSH server is installed and access to the root user is enabled.

At the end of the SD card Debian/Ubuntu image creation encryption certificates are removed.
They are again created on the first boot by `/etc/systemd/system/ssh-reconfigure.service </OS/debian/overlay/etc/systemd/system/ssh-reconfigure.service>`_.
Due to this the first boot takes a bit longer.
This way the SSH encryption certificates are unique on each board.

=============================
Zero-configuration networking
=============================

------------------
Link-local address
------------------

``systemd-networkd`` can provide interfaces with `link-local addresses <https://en.wikipedia.org/wiki/Link-local_address>`_,
if this is enabled inside ``systemd.network`` files with the line ``LinkLocalAddressing=yes``.
All interfaces have this setting enabled, this way each active interface will
acquire an address in the reserved ``169.254.0.0/16`` address block.

--------
Zeroconf
--------

If the computer used to access the device supports zeroconf (Avahi/Bobjour) name resolving is also available.
Since there can be multiple devices on a single network they must be distinguished.
The last three segments of the Ethernet MAC number without semicolons
(as printed on the Ethernet connector on each device) is used
to generate the hostname, which is then used to generate a link name.
For example if the MAC address is ``00:26:32:f0:f1:f2`` then the shortened string ``shortMAC`` is ``f0f1f2``.

Hostname generation is done by `/etc/systemd/system/hostname-mac.service </OS/debian/overlay/etc/systemd/system/hostname-mac.service>`_
which must run early during the boot process.

Each device can now be accessed using the URL ``http://rp-<shortMAC>.local``.

Similarly to get SSH access use.

.. code-block:: shell-session

   $ ssh root@rp-<shortMAC>.local

This service is a good alternative for our *Discovery* service provided on redpitaya.com servers.

`Avahi daemon <http://www.avahi.org>`_ is used to advertise specific services.
Three configuration files are provided.

* HTTP `/etc/avahi/services/bazaar.service </OS/debian/overlay/etc/avahi/services/bazaar.service>`_
* SSH  `/etc/avahi/services/ssh.service    </OS/debian/overlay/etc/avahi/services/ssh.service>`_
* SCPI `/etc/avahi/services/scpi.service   </OS/debian/overlay/etc/avahi/services/scpi.service>`_

.. note:: This services were enabled just recently, so full extent of their usefulness is still unknown.

====================
``systemd`` services
====================

Services handling the described configuration are enabled with.

.. code-block:: shell-session

   # enable systemd network related services
   systemctl enable systemd-networkd
   systemctl enable systemd-resolved
   systemctl enable systemd-timesyncd
   systemctl enable wpa_supplicant@wlan0.service
   systemctl enable hostapd@wlan0.service
   systemctl enable wireless-mode-client.service
   systemctl enable wireless-mode-ap.service
   systemctl enable iptables.service
   #systemctl enable wpa_supplicant@wlan0.path
   #systemctl enable hostapd@wlan0.path
   systemctl enable hostname-mac.service
   systemctl enable avahi-daemon.service
   
   # enable service for creating SSH keys on first boot
   systemctl enable ssh-reconfigure

****************************
Supported USB Wi-Fi adapters
****************************

Support for a specific Wi-Fi adapter usually depends only on the availability
of the driver for the chipset used in the adapter.
Therefore this section focuses on Linux kernel drivers for Wi-Fi adapters.

Before the switch to kernel 4.9 an out of tree driver was used for the **rtl8192cu** chipset.
Support for this patch was removed, due to reliability and maintenance issues.
In practice this means *rtl8192cu* based adapters will only work in client mode.
At the same time support for the deprecated user space tools ``wireless extensions``
was removed, instead the ``nl80211`` framework should be used.
In practice this means ``iw`` should be used instead of ``iwconfig``.

After plugging an USB Wi-Fi adapter use ``dmesg`` and ``lsusb`` to check
if the adapter was properly recognized by the Linux kernel.

To check what modes (managed, AP, ...) are supported by an adapter use ``iw``.

================
BCM43143 chipset
================

Client (``managed``) and access point (``AP``) modes are supported.

---------------------------------------------
Recommended USB Wi-Fi device for Raspberry PI
---------------------------------------------

https://www.raspberrypi.org/products/usb-wifi-dongle/

https://web.archive.org/web/20161014035710/https://www.raspberrypi.org/products/usb-wifi-dongle/

.. code-block:: shell-session

   # lsusb
   Bus 001 Device 004: ID 0a5c:bd1e Broadcom Corp. 

.. code-block:: shell-session

   # dmesg
   ...
   usb 1-1: new high-speed USB device number 4 using ci_hdrc
   brcmfmac: brcmf_c_preinit_dcmds: Firmware version = wl0: Apr  3 2014 04:43:32 version 6.10.198.66 (r467479) FWID 01-32bd010e
   brcmfmac: brcmf_cfg80211_reg_notifier: not a ISO3166 code (0x30 0x30)
   ...
   usb 1-1: USB disconnect, device number 4
   brcmfmac: brcmf_usb_send_ctl: usb_submit_urb failed -19
   brcmfmac: brcmf_usb_tx_ctlpkt: fail -19 bytes: 45
   brcmfmac: brcmf_fil_cmd_data: bus is down. we have nothing to do.
   brcmfmac: brcmf_fil_cmd_data: bus is down. we have nothing to do.
   brcmfmac: brcmf_fil_cmd_data: bus is down. we have nothing to do.
   brcmfmac: brcmf_cfg80211_get_channel: chanspec failed (-5)

.. code-block:: shell-session

   # iw list
   Wiphy phy3
   	max # scan SSIDs: 10
   	max scan IEs length: 2048 bytes
   	Retry short limit: 7
   	Retry long limit: 4
   	Coverage class: 0 (up to 0m)
   	Device supports roaming.
   	Supported Ciphers:
   		* WEP40 (00-0f-ac:1)
   		* WEP104 (00-0f-ac:5)
   		* TKIP (00-0f-ac:2)
   		* CCMP (00-0f-ac:4)
   	Available Antennas: TX 0 RX 0
   	Supported interface modes:
   		 * IBSS
   		 * managed
   		 * AP
   		 * P2P-client
   		 * P2P-GO
   		 * P2P-device
   	Band 1:
   		Capabilities: 0x1022
   			HT20/HT40
   			Static SM Power Save
   			RX HT20 SGI
   			No RX STBC
   			Max AMSDU length: 3839 bytes
   			DSSS/CCK HT40
   		Maximum RX AMPDU length 65535 bytes (exponent: 0x003)
   		Minimum RX AMPDU time spacing: 16 usec (0x07)
   		HT TX/RX MCS rate indexes supported: 0-7
   		Bitrates (non-HT):
   			* 1.0 Mbps
   			* 2.0 Mbps (short preamble supported)
   			* 5.5 Mbps (short preamble supported)
   			* 11.0 Mbps (short preamble supported)
   			* 6.0 Mbps
   			* 9.0 Mbps
   			* 12.0 Mbps
   			* 18.0 Mbps
   			* 24.0 Mbps
   			* 36.0 Mbps
   			* 48.0 Mbps
   			* 54.0 Mbps
   		Frequencies:
   			* 2412 MHz [1] (20.0 dBm)
   			* 2417 MHz [2] (20.0 dBm)
   			* 2422 MHz [3] (20.0 dBm)
   			* 2427 MHz [4] (20.0 dBm)
   			* 2432 MHz [5] (20.0 dBm)
   			* 2437 MHz [6] (20.0 dBm)
   			* 2442 MHz [7] (20.0 dBm)
   			* 2447 MHz [8] (20.0 dBm)
   			* 2452 MHz [9] (20.0 dBm)
   			* 2457 MHz [10] (20.0 dBm)
   			* 2462 MHz [11] (20.0 dBm)
   			* 2467 MHz [12] (disabled)
   			* 2472 MHz [13] (disabled)
   			* 2484 MHz [14] (disabled)
   	Supported commands:
   		 * new_interface
   		 * set_interface
   		 * new_key
   		 * start_ap
   		 * join_ibss
   		 * set_pmksa
   		 * del_pmksa
   		 * flush_pmksa
   		 * remain_on_channel
   		 * frame
   		 * set_channel
   		 * start_p2p_device
   		 * crit_protocol_start
   		 * crit_protocol_stop
   		 * connect
   		 * disconnect
   	Supported TX frame types:
   		 * managed: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * P2P-client: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * P2P-GO: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * P2P-device: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   	Supported RX frame types:
   		 * managed: 0x40 0xd0
   		 * P2P-client: 0x40 0xd0
   		 * P2P-GO: 0x00 0x20 0x40 0xa0 0xb0 0xc0 0xd0
   		 * P2P-device: 0x40 0xd0
   	software interface modes (can always be added):
   	valid interface combinations:
   		 * #{ managed } <= 1, #{ P2P-device } <= 1, #{ P2P-client, P2P-GO } <= 1,
   		   total <= 3, #channels <= 1
   		 * #{ managed } <= 1, #{ AP } <= 1, #{ P2P-client } <= 1, #{ P2P-device } <= 1,
   		   total <= 4, #channels <= 1
   	Device supports scan flush.

=================
rtl8192cu chipset
=================

The rtl8192cu chipset is supported by the ``rtl8xxxu`` driver.
For now this driver only supports client (``managed``) mode.

----------------
Edimax EW-7811Un
----------------

http://us.edimax.com/edimax/merchandise/merchandise_detail/data/edimax/us/wireless_adapters_n150/ew-7811un/

.. code-block:: shell-session

   # lsusb
   Bus 001 Device 002: ID 7392:7811 Edimax Technology Co., Ltd EW-7811Un 802.11n Wireless Adapter [Realtek RTL8188CUS]

.. code-block:: shell-session

   # dmesg
   ...
   usb 1-1: new high-speed USB device number 2 using ci_hdrc
   usb 1-1: Vendor: Realtek
   usb 1-1: Product: 802.11n WLAN Adapter
   usb 1-1: rtl8192cu_parse_efuse: dumping efuse (0x80 bytes):
   usb 1-1: 00: 29 81 00 74 cd 00 00 00
   usb 1-1: 08: ff 00 92 73 11 78 03 41
   usb 1-1: 10: 32 00 85 62 9e ad 74 da
   usb 1-1: 18: 38 7d d0 48 0a 03 52 65
   usb 1-1: 20: 61 6c 74 65 6b 00 16 03
   usb 1-1: 28: 38 30 32 2e 31 31 6e 20
   usb 1-1: 30: 57 4c 41 4e 20 41 64 61
   usb 1-1: 38: 70 74 65 72 00 00 00 00
   usb 1-1: 40: 00 00 00 00 00 00 00 00
   usb 1-1: 48: 00 00 00 00 00 00 00 00
   usb 1-1: 50: 00 00 00 00 00 00 00 00
   usb 1-1: 58: 06 00 29 29 29 00 00 00
   usb 1-1: 60: 2b 2b 2a 00 00 00 00 00
   usb 1-1: 68: 00 00 00 00 11 11 33 00
   usb 1-1: 70: 00 00 00 00 00 02 00 00
   usb 1-1: 78: 10 00 00 00 36 00 00 00
   usb 1-1: RTL8188CU rev A (TSMC) 1T1R, TX queues 2, WiFi=1, BT=0, GPS=0, HI PA=0
   usb 1-1: RTL8188CU MAC: 74:da:38:7d:d0:48
   usb 1-1: rtl8xxxu: Loading firmware rtlwifi/rtl8192cufw_TMSC.bin
   usb 1-1: Firmware revision 80.0 (signature 0x88c1)
   usb 1-1: rtl8xxxu_iqk_path_a: Path A RX IQK failed!
   usb 1-1: rtl8xxxu_iqk_path_a: Path A RX IQK failed!
   usb 1-1: rtl8xxxu_iqk_path_a: Path A RX IQK failed!
   usb 1-1: rtl8xxxu_iqk_path_a: Path A RX IQK failed!
   ...
   usb 1-1: USB disconnect, device number 2
   usb 1-1: rtl8xxxu_active_to_lps: RX poll timed out (0x05f8)
   usb 1-1: rtl8xxxu_active_to_emu: Disabling MAC timed out
   usb 1-1: disconnecting

.. code-block:: shell-session

   # iw list
   Wiphy phy0
   	max # scan SSIDs: 4
   	max scan IEs length: 2257 bytes
   	RTS threshold: 2347
   	Retry short limit: 7
   	Retry long limit: 4
   	Coverage class: 0 (up to 0m)
   	Supported Ciphers:
   		* WEP40 (00-0f-ac:1)
   		* WEP104 (00-0f-ac:5)
   		* TKIP (00-0f-ac:2)
   		* CCMP (00-0f-ac:4)
   		* 00-0f-ac:10
   		* GCMP (00-0f-ac:8)
   		* 00-0f-ac:9
   	Available Antennas: TX 0 RX 0
   	Supported interface modes:
   		 * managed
   		 * monitor
   	Band 1:
   		Capabilities: 0x60
   			HT20
   			Static SM Power Save
   			RX HT20 SGI
   			RX HT40 SGI
   			No RX STBC
   			Max AMSDU length: 3839 bytes
   			No DSSS/CCK HT40
   		Maximum RX AMPDU length 65535 bytes (exponent: 0x003)
   		Minimum RX AMPDU time spacing: 16 usec (0x07)
   		HT TX/RX MCS rate indexes supported: 0-7, 32
   		Bitrates (non-HT):
   			* 1.0 Mbps
   			* 2.0 Mbps
   			* 5.5 Mbps
   			* 11.0 Mbps
   			* 6.0 Mbps
   			* 9.0 Mbps
   			* 12.0 Mbps
   			* 18.0 Mbps
   			* 24.0 Mbps
   			* 36.0 Mbps
   			* 48.0 Mbps
   			* 54.0 Mbps
   		Frequencies:
   			* 2412 MHz [1] (20.0 dBm)
   			* 2417 MHz [2] (20.0 dBm)
   			* 2422 MHz [3] (20.0 dBm)
   			* 2427 MHz [4] (20.0 dBm)
   			* 2432 MHz [5] (20.0 dBm)
   			* 2437 MHz [6] (20.0 dBm)
   			* 2442 MHz [7] (20.0 dBm)
   			* 2447 MHz [8] (20.0 dBm)
   			* 2452 MHz [9] (20.0 dBm)
   			* 2457 MHz [10] (20.0 dBm)
   			* 2462 MHz [11] (20.0 dBm)
   			* 2467 MHz [12] (20.0 dBm) (no IR)
   			* 2472 MHz [13] (20.0 dBm) (no IR)
   			* 2484 MHz [14] (20.0 dBm) (no IR)
   	Supported commands:
   		 * new_interface
   		 * set_interface
   		 * new_key
   		 * start_ap
   		 * new_station
   		 * set_bss
   		 * authenticate
   		 * associate
   		 * deauthenticate
   		 * disassociate
   		 * join_ibss
   		 * set_tx_bitrate_mask
   		 * frame
   		 * frame_wait_cancel
   		 * set_wiphy_netns
   		 * set_channel
   		 * set_wds_peer
   		 * probe_client
   		 * set_noack_map
   		 * register_beacons
   		 * start_p2p_device
   		 * set_mcast_rate
   		 * Unknown command (104)
   		 * connect
   		 * disconnect
   	Supported TX frame types:
   		 * IBSS: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * managed: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * AP: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * AP/VLAN: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * mesh point: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * P2P-client: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * P2P-GO: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   		 * P2P-device: 0x00 0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xa0 0xb0 0xc0 0xd0 0xe0 0xf0
   	Supported RX frame types:
   		 * IBSS: 0x40 0xb0 0xc0 0xd0
   		 * managed: 0x40 0xd0
   		 * AP: 0x00 0x20 0x40 0xa0 0xb0 0xc0 0xd0
   		 * AP/VLAN: 0x00 0x20 0x40 0xa0 0xb0 0xc0 0xd0
   		 * mesh point: 0xb0 0xc0 0xd0
   		 * P2P-client: 0x40 0xd0
   		 * P2P-GO: 0x00 0x20 0x40 0xa0 0xb0 0xc0 0xd0
   		 * P2P-device: 0x40 0xd0
   	software interface modes (can always be added):
   		 * monitor
   	interface combinations are not supported
   	HT Capability overrides:
   		 * MCS: ff ff ff ff ff ff ff ff ff ff
   		 * maximum A-MSDU length
   		 * supported channel width
   		 * short GI for 40 MHz
   		 * max A-MPDU length exponent
   		 * min MPDU start spacing
   	Device supports TX status socket option.
   	Device supports HT-IBSS.
   	Device supports SAE with AUTHENTICATE command
   	Device supports low priority scan.
   	Device supports scan flush.
   	Device supports AP scan.
   	Device supports per-vif TX power setting
   	Driver supports full state transitions for AP/GO clients
   	Driver supports a userspace MPM

------------------------------------------------------
Generic Realtek Semiconductor Corp. RTL8188CUS 802.11n
------------------------------------------------------

.. code-block:: shell-session

   # dmesg
   ...
   usb 1-1: new high-speed USB device number 3 using ci_hdrc
   usb 1-1: Vendor: Realtek
   usb 1-1: Product: 802.11n WLAN Adapter
   usb 1-1: rtl8192cu_parse_efuse: dumping efuse (0x80 bytes):
   usb 1-1: 00: 29 81 00 74 cd 00 00 00
   usb 1-1: 08: ff 00 da 0b 76 81 01 41
   usb 1-1: 10: 32 00 85 62 9e ad 00 13
   usb 1-1: 18: ef 60 22 15 0a 03 52 65
   usb 1-1: 20: 61 6c 74 65 6b 00 16 03
   usb 1-1: 28: 38 30 32 2e 31 31 6e 20
   usb 1-1: 30: 57 4c 41 4e 20 41 64 61
   usb 1-1: 38: 70 74 65 72 00 00 00 00
   usb 1-1: 40: 00 00 00 00 00 00 00 00
   usb 1-1: 48: 00 00 00 00 00 00 00 00
   usb 1-1: 50: 00 00 00 00 00 00 00 00
   usb 1-1: 58: 06 00 28 28 28 00 00 00
   usb 1-1: 60: 28 28 28 00 00 00 00 00
   usb 1-1: 68: 00 00 00 00 02 02 02 00
   usb 1-1: 70: 00 00 00 00 00 02 00 00
   usb 1-1: 78: 10 00 00 00 36 00 00 00
   usb 1-1: RTL8188CU rev A (TSMC) 1T1R, TX queues 2, WiFi=1, BT=0, GPS=0, HI PA=0
   usb 1-1: RTL8188CU MAC: 00:13:ef:60:22:15
   usb 1-1: rtl8xxxu: Loading firmware rtlwifi/rtl8192cufw_TMSC.bin
   usb 1-1: Firmware revision 80.0 (signature 0x88c1)
   ...
   usb 1-1: USB disconnect, device number 3
   usb 1-1: rtl8xxxu_active_to_lps: RX poll timed out (0x05f8)
   usb 1-1: rtl8xxxu_active_to_emu: Disabling MAC timed out
   usb 1-1: disconnecting

.. code-block:: shell-session

   # lsusb
   Bus 001 Device 003: ID 0bda:8176 Realtek Semiconductor Corp. RTL8188CUS 802.11n WLAN Adapter
