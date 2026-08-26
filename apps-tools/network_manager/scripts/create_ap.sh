#!/bin/bash
#
# Start the access point.
#
#   create_ap.sh <ssid> <passphrase> [channel] [hw_mode] \
#                [address] [netmask] [pool_offset] [pool_size] [lease] [dns]
#
#     passphrase   empty for an open network
#     channel      1..13, default 6
#     hw_mode      n | g, default n
#     address      the board's own address on the AP segment, default 192.168.128.1
#     netmask      default 255.255.255.0
#     pool_offset  first address handed out, default 10
#     pool_size    how many, default 40
#     lease        seconds, default 600
#     dns          resolver given to clients; empty means this board
#
# Two files are written. hostapd.conf is the radio: SSID, passphrase, channel,
# regulatory domain. /etc/systemd/network/wireless.network.ap is the IP side:
# the board's address on the segment and the DHCP pool it serves. AP mode is
# selected by wireless-mode-ap.service, which symlinks wireless.network to that
# second file, so writing it is how the addressing becomes configurable at all -
# it shipped as a fixed 192.168.128.1/24, which is fine right up until it
# collides with the network on eth0.
#
# Prints "ok: ..." or an "error: ..." line and exits non-zero on failure, so
# the web UI can say what went wrong instead of spinning.

SSID=$1
PASS=$2
CHAN=${3:-6}
HWMODE=${4:-n}
ADDR=${5:-192.168.128.1}
MASK=${6:-255.255.255.0}
POOL_OFF=${7:-10}
POOL_SIZE=${8:-40}
LEASE=${9:-600}
DNS=${10:-}

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts
CONF_F=/opt/redpitaya/hostapd.conf
NET_F=/etc/systemd/network/wireless.network.ap

if [ -z "$SSID" ]; then
    echo "error: no SSID given"
    exit 2
fi

# hostapd rejects a 1..7 character passphrase outright, and an empty
# wpa_passphrase together with wpa=2 is two config errors rather than an open
# network. Decide here instead of writing a file that cannot start.
if [ -n "$PASS" ] && { [ ${#PASS} -lt 8 ] || [ ${#PASS} -gt 63 ]; }; then
    echo "error: passphrase must be 8 to 63 characters, or empty for an open network"
    exit 2
fi

case "$CHAN" in
    [1-9]|1[0-3]) : ;;
    *) echo "error: channel must be 1 to 13"; exit 2 ;;
esac

case "$HWMODE" in
    n|g) : ;;
    *) echo "error: radio mode must be n or g"; exit 2 ;;
esac

# ------------------------------------------------------------- IP arguments --
ip4() {
    case "$1" in
        *[!0-9.]*|"") return 1 ;;
    esac
    local IFS=. o n=0
    set -- $1
    [ $# -eq 4 ] || return 1
    for o in "$@"; do
        [ -n "$o" ] || return 1
        [ "$o" -le 255 ] 2>/dev/null || return 1
        n=$((n + 1))
    done
    [ $n -eq 4 ]
}

ip4 "$ADDR" || { echo "error: address must be an IPv4 address"; exit 2; }

# systemd wants a prefix length. Accept a dotted mask, and reject one whose bits
# are not contiguous - 255.0.255.0 is not a netmask, and a config file is the
# wrong place to find that out.
mask_to_prefix() {
    local IFS=. bits=0 seen0=0 o b
    set -- $1
    [ $# -eq 4 ] || return 1
    for o in "$@"; do
        [ "$o" -le 255 ] 2>/dev/null || return 1
        for b in 128 64 32 16 8 4 2 1; do
            if [ $((o & b)) -ne 0 ]; then
                [ $seen0 -eq 1 ] && return 1
                bits=$((bits + 1))
            else
                seen0=1
            fi
        done
    done
    printf '%s' "$bits"
}

PREFIX=$(mask_to_prefix "$MASK") || { echo "error: invalid netmask \"$MASK\""; exit 2; }
[ -n "$PREFIX" ] || { echo "error: invalid netmask \"$MASK\""; exit 2; }

num() { case "$1" in ''|*[!0-9]*) return 1 ;; esac; return 0; }

num "$POOL_OFF" && [ "$POOL_OFF" -ge 1 ] && [ "$POOL_OFF" -le 254 ] \
    || { echo "error: pool first address must be 1 to 254"; exit 2; }
num "$POOL_SIZE" && [ "$POOL_SIZE" -ge 1 ] && [ "$POOL_SIZE" -le 254 ] \
    || { echo "error: pool size must be 1 to 254"; exit 2; }
[ $((POOL_OFF + POOL_SIZE - 1)) -le 254 ] \
    || { echo "error: pool of $POOL_SIZE starting at $POOL_OFF runs past the end of the subnet"; exit 2; }

# The board's own address must not be inside the pool it hands out, or it and a
# client both claim it.
HOST_OCTET=${ADDR##*.}
if [ "$HOST_OCTET" -ge "$POOL_OFF" ] && [ "$HOST_OCTET" -lt $((POOL_OFF + POOL_SIZE)) ]; then
    echo "error: board address $ADDR is inside the lease pool $POOL_OFF-$((POOL_OFF + POOL_SIZE - 1)); move the pool or the address"
    exit 2
fi

num "$LEASE" && [ "$LEASE" -ge 10 ] && [ "$LEASE" -le 604800 ] \
    || { echo "error: lease time must be 10 to 604800 seconds"; exit 2; }

if [ -n "$DNS" ]; then
    ip4 "$DNS" || { echo "error: DNS must be an IPv4 address, or empty for this board"; exit 2; }
fi

$RP_PATH/get_connected_wifi.sh
RES=$?

if [ "$RES" != "1" ]; then
    echo "error: no wlan0 interface (no adapter?)"
    exit 3
fi

# An adapter that cannot do AP mode leaves hostapd.conf on disk after hostapd
# fails, and that file then blocks wpa_supplicant@ and wireless_adapter_up@
# through their ConditionPathExists - the board ends up with no wireless at
# all. Check before writing anything.
$RP_PATH/check_ap_mode.sh
if [ $? != 1 ]; then
    echo "error: this adapter does not support AP mode"
    exit 3
fi

$RP_PATH/disconnect.sh >/dev/null 2>&1

rw || { echo "error: rw remount failed"; exit 4; }

{
    echo "interface=wlan0"
    printf 'ssid=%s\n' "$SSID"
    echo "driver=nl80211"
    echo "hw_mode=g"
    echo "channel=$CHAN"
    echo "macaddr_acl=0"
    echo "auth_algs=1"
    echo "ignore_broadcast_ssid=0"

    if [ -n "$PASS" ]; then
        echo "wpa=2"
        printf 'wpa_passphrase=%s\n' "$PASS"
        echo "wpa_key_mgmt=WPA-PSK"
        echo "wpa_pairwise=CCMP"
        echo "rsn_pairwise=CCMP"
    fi

    # 802.11n roughly quadruples the ceiling over plain g. It wants the
    # regulatory domain advertised, which the country block below does.
    if [ "$HWMODE" = "n" ]; then
        echo "ieee80211n=1"
        echo "wmm_enabled=1"
        echo "ht_capab=[SHORT-GI-20]"
    fi

    # Advertise the regulatory domain so clients learn which channels and
    # powers this AP may use. There is nothing meaningful to advertise for the
    # world domain, and hostapd rejects country_code=00.
    COUNTRY=$($RP_PATH/get_country.sh)
    if [ "$COUNTRY" != "00" ]; then
        echo "country_code=$COUNTRY"
        echo "ieee80211d=1"
    fi
} > "$CONF_F"

chmod 600 "$CONF_F"

# The IP side of the access point. wireless-mode-ap.service symlinks
# wireless.network to this file, so this is the one networkd reads.
#
# LinkLocalAddressing=ipv6, not ipv4: the address here is stated explicitly, and
# asking for IPv4LL as well put a second address (169.254.0.0/16) and a
# metric-2048 route on wlan0 next to it. IPv4LL is a fallback for having no
# address at all and rescues nothing on an interface that serves DHCP.
{
    echo "[Match]"
    echo "Name=wlan0*"
    echo
    echo "[Network]"
    echo "Description = Wireless IP interface"
    echo "LinkLocalAddressing=ipv6"
    echo "DHCPServer=yes"
    echo
    echo "[Address]"
    echo "Address   = $ADDR/$PREFIX"
    echo
    echo "[DHCPServer]"
    echo "PoolOffset          = $POOL_OFF"
    echo "PoolSize            = $POOL_SIZE"
    echo "DefaultLeaseTimeSec = $LEASE"
    echo "MaxLeaseTimeSec     = $LEASE"
    echo "EmitDNS             = yes"
    if [ -n "$DNS" ]; then
        echo "DNS                 = $DNS"
    fi
    echo "EmitRouter          = yes"
} > "$NET_F"

chmod 644 "$NET_F"

systemctl restart hostapd@wlan0.service    >/dev/null 2>&1
sleep 1
systemctl restart wireless-mode-ap.service >/dev/null 2>&1
sleep 1
systemctl restart systemd-networkd.service >/dev/null 2>&1
sleep 2

ro

# Verify rather than assume. If hostapd did not come up, take the config back
# off disk: leaving it there is exactly what strands the board with no
# wireless at all.
if ! systemctl is-active --quiet hostapd@wlan0.service; then
    rw
    rm -f "$CONF_F"
    ro
    systemctl restart systemd-networkd.service >/dev/null 2>&1
    echo "error: hostapd did not start, configuration rolled back. See journalctl -u hostapd@wlan0"
    exit 5
fi

$RP_PATH/apmode_def_route.sh a >/dev/null 2>&1

NET_PREFIX=${ADDR%.*}
echo "ok: access point \"$SSID\" running on channel $CHAN (802.11$HWMODE), $ADDR/$PREFIX, clients get $NET_PREFIX.$POOL_OFF to $NET_PREFIX.$((POOL_OFF + POOL_SIZE - 1))"
exit $RES
