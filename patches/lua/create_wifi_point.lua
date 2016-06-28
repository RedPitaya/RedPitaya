local shell = require("shell")


local args = ngx.req.get_uri_args()

local status, out, err = shell.execute("rm -f /opt/redpitaya/wpa_supplicant.conf")
local status, out, err = shell.execute("systemctl stop wpa_supplicant_wext@wlan0wext.service")
local status, out, err = shell.execute("rm -f /opt/redpitaya/hostapd.conf")

local status, out, err = shell.execute("echo interface=wlan0wext >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo ssid="..args.essid.." >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo driver=rtl871xdrv >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo hw_mode=g >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo channel=6 >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo macaddr_acl=0 >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo auth_algs=1 >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo ignore_broadcast_ssid=0 >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo wpa=2 >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo wpa_passphrase="..args.password.." >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo wpa_key_mgmt=WPA-PSK >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo wpa_pairwise=TKIP >> /opt/redpitaya/hostapd.conf")
local status, out, err = shell.execute("echo rsn_pairwise=CCMP >> /opt/redpitaya/hostapd.conf")

local status, out, err = shell.execute("systemctl restart hostapd@wlan0wext.service")
