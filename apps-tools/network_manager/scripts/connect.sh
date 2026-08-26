#!/bin/bash
#
# Connect wlan0 to an infrastructure network.
#
#   connect.sh <ssid> [passphrase] [keymgmt] [pmf] [hidden]
#
#     keymgmt  auto (WPA-PSK / WPA-PSK-SHA256 / SAE) | wpa2 | open   default auto
#     pmf      0 disabled | 1 optional | 2 required                  default 1
#     hidden   0 | 1  (scan_ssid)                                    default 1
#
# Exit code mirrors the contract the web UI expects (1 = adapter present,
# -1/255 = no adapter) but now only on SUCCESS. On failure it prints a
# diagnostic line and exits 3.

SSID="$1"
PASS="$2"
KEYMGMT="${3:-auto}"
PMF="${4:-1}"
HIDDEN="${5:-1}"

case "$KEYMGMT" in auto|wpa2|open) : ;; *) echo "error: keymgmt must be auto, wpa2 or open"; exit 2 ;; esac
case "$PMF"     in 0|1|2)          : ;; *) echo "error: pmf must be 0, 1 or 2";            exit 2 ;; esac
case "$HIDDEN"  in 0|1)            : ;; *) echo "error: hidden must be 0 or 1";            exit 2 ;; esac

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts
WPA_SUP=/opt/redpitaya/wpa_supplicant.conf
LOG=/tmp/network_manager-connect.log

# How long to wait for association + 4-way handshake. rtw88-based USB dongles
# routinely need 5-8 s, the old code waited 1 s and then kicked networkd.
ASSOC_TIMEOUT=25

: > "$LOG"
log() { echo "$(date '+%H:%M:%S') $*" >> "$LOG"; }

if [ -z "$SSID" ]; then
    echo "error: no SSID given"
    exit 3
fi

$RP_PATH/get_connected_wifi.sh
RES=$?

if [ "$RES" == "0" ]; then
    echo "error: no wlan0 interface (no dongle?)"
    exit -1
fi

log "ssid=[$SSID] pass_len=${#PASS} keymgmt=$KEYMGMT pmf=$PMF hidden=$HIDDEN iface_present=$RES"

# country=00 leaves all 5 GHz channels PASSIVE-SCAN, so a 5 GHz AP can be seen
# but never joined. Prefer what the user picked in the web UI; if nothing is set
# there, fall back to the country the target AP advertises in its beacon.
ip link set dev wlan0 up 2>/dev/null
COUNTRY=$($RP_PATH/get_country.sh)
if [ "$COUNTRY" = "00" ]; then
    BEACON_CC=$(iw dev wlan0 scan 2>/dev/null \
                | awk -v s="$SSID" '/^BSS /{f=0} $0=="\tSSID: "s{f=1} f&&/^\tCountry:/{print $2; exit}')
    case "$BEACON_CC" in
        [A-Z][A-Z]) COUNTRY=$BEACON_CC; log "country not set, taking $COUNTRY from the AP beacon" ;;
    esac
fi
log "regulatory country: $COUNTRY"

$RP_PATH/disconnect.sh >/dev/null 2>&1

rw

# ---------------------------------------------------------------------------
# Build the wpa_supplicant config.
#
# The old code piped `wpa_passphrase` straight into the file. That breaks in
# three separate ways:
#   * empty passphrase (open network) -> wpa_passphrase errors out and its
#     error text lands in the config file
#   * only WPA-PSK is expressed, so WPA3-SAE and WPA2/WPA3-mixed APs (any
#     recent Wi-Fi 6 router) can never be joined
#   * the hashed psk= form is incompatible with SAE, which needs the
#     plaintext passphrase
# ---------------------------------------------------------------------------
{
    # --- global section ---
    echo "ctrl_interface=/run/wpa_supplicant"
    echo "update_config=1"
    echo "country=$COUNTRY"
    # sae_pwe is a GLOBAL option in wpa_supplicant 2.10, not a per-network one.
    # Putting it inside network={} makes the whole block fail to parse and
    # wpa_supplicant refuses to start ("unknown network field 'sae_pwe'").
    if [ -n "$PASS" ] && [ "$KEYMGMT" = "auto" ]; then
        echo "sae_pwe=2"
    fi

    # --- network section ---
    echo "network={"
    printf '\tssid="%s"\n' "$SSID"
    echo "	scan_ssid=$HIDDEN"
    if [ -z "$PASS" ] || [ "$KEYMGMT" = "open" ]; then
        echo "	key_mgmt=NONE"
    else
        printf '\tpsk="%s"\n' "$PASS"
        if [ "$KEYMGMT" = "wpa2" ]; then
            # Escape hatch for adapters whose driver misbehaves with SAE.
            echo "	key_mgmt=WPA-PSK"
        else
            # WPA2-PSK, WPA2-PSK-SHA256 and WPA3-SAE in one block;
            # wpa_supplicant picks whatever the AP actually advertises.
            echo "	key_mgmt=WPA-PSK WPA-PSK-SHA256 SAE"
        fi
        echo "	ieee80211w=$PMF"
    fi
    echo "}"
} > "$WPA_SUP"

chmod 600 "$WPA_SUP"
log "config written:"; sed 's/^/    /' "$WPA_SUP" | sed 's/psk=".*"/psk="<redacted>"/' >> "$LOG"

ip link set dev wlan0 up 2>/dev/null

# nl80211 only. Never -D wext here: wext cannot install CCMP keys through
# mac80211 and the association silently dies.
wpa_supplicant -B -i wlan0 -c "$WPA_SUP" -D nl80211 >> "$LOG" 2>&1
START_RC=$?
if [ $START_RC -ne 0 ]; then
    log "wpa_supplicant failed to start (rc=$START_RC)"
    ro
    echo "error: wpa_supplicant failed to start, see $LOG"
    exit 3
fi

# ---------------------------------------------------------------------------
# Wait for a real association BEFORE touching networkd, instead of sleeping 1 s
# and hoping. Without this DHCP fires while the handshake is still running.
# ---------------------------------------------------------------------------
CONNECTED=0
for i in $(seq 1 $ASSOC_TIMEOUT); do
    iw dev wlan0 link 2>/dev/null | grep -q '^Connected to' && CONNECTED=1
    [ $CONNECTED -eq 1 ] && break
    sleep 1
done

if [ $CONNECTED -eq 0 ]; then
    log "association timed out after ${ASSOC_TIMEOUT}s"
    killall wpa_supplicant 2>/dev/null
    rm -f "$WPA_SUP"
    ro
    echo "error: could not associate with \"$SSID\" within ${ASSOC_TIMEOUT}s (wrong passphrase, or AP requires an unsupported security mode). See $LOG"
    exit 3
fi

log "associated after ${i}s"

systemctl restart wireless-mode-client.service >> "$LOG" 2>&1
systemctl restart systemd-networkd.service    >> "$LOG" 2>&1

ro

# Wait for an address so the UI does not report success on a link with no IP.
for i in $(seq 1 20); do
    ip -4 -o addr show dev wlan0 2>/dev/null | grep -q 'inet ' && break
    sleep 1
done

IPADDR=$(ip -4 -o addr show dev wlan0 | awk '{print $4}' | head -1)
if [ -z "$IPADDR" ]; then
    log "associated but no IPv4 address"
    echo "warning: associated with \"$SSID\" but no IPv4 address was obtained"
    exit 3
fi

log "ok, address $IPADDR"
echo "ok: connected to \"$SSID\", address $IPADDR"
exit $RES
