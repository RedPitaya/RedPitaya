local shell = require("shell")


local args = ngx.req.get_uri_args()

local status, out, err = os.execute("rm -f /opt/redpitaya/wpa_supplicant.conf")
local status, out, err = os.execute("systemctl stop wpa_supplicant_wext@wlan0wext.service")
local status, out, err = os.execute("rm -f /opt/redpitaya/hostapd.conf")

local hostapd = "interface=wlan0wext\n"
hostapd.."ssid="..args.essid.."\n"
hostapd.."driver=rtl871xdrv\n"
hostapd.."hw_mode=g\n"
hostapd.."channel=6\n"
hostapd.."macaddr_acl=0\n"
hostapd.."auth_algs=1\n"
hostapd.."ignore_broadcast_ssid=0\n"
hostapd.."wpa=2\n"
hostapd.."wpa_passphrase="..args.password.."\n"
hostapd.."wpa_key_mgmt=WPA-PSK\n"
hostapd.."wpa_pairwise=TKIP\n"
hostapd.."rsn_pairwise=CCMP\n"

local hostapd_handler = io.open("/opt/redpitaya/hostapd.conf", "w")
hostapd_handler:write(hostapd)
hostapd_handler:close()

local status, out, err = os.execute("systemctl restart hostapd@wlan0wext.service")
