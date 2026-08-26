#!/bin/bash
#
# Everything the Status section shows about the wireless side, as key=value
# lines. One request instead of the six the page would otherwise need, and one
# place to change when a field moves.
#
# Absent values are printed as an empty string rather than omitted, so the
# front end can tell "not applicable" from "endpoint is broken".

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts
IF=wlan0

emit() { printf '%s=%s\n' "$1" "$2"; }

$RP_PATH/get_connected_wifi.sh
PRESENT=$?

if [ "$PRESENT" != "1" ]; then
    emit present 0
    exit 0
fi
emit present 1

# ---------------------------------------------------------------- interface --
LINK=$(iw dev "$IF" link 2>/dev/null)
INFO=$(iw dev "$IF" info 2>/dev/null)

TYPE=$(echo "$INFO" | awk '/^\ttype /{print $2; exit}')
emit type "${TYPE:-}"

if echo "$LINK" | grep -q '^Connected to'; then
    emit state associated
    emit bssid "$(echo "$LINK" | awk '/^Connected to/{print $3; exit}')"
    emit ssid  "$(echo "$LINK" | sed -n 's/^[[:space:]]*SSID: //p' | head -1)"
    emit freq  "$(echo "$LINK" | awk '/freq:/{print $2; exit}')"
    emit signal "$(echo "$LINK" | awk '/signal:/{print $2; exit}')"
    emit txrate "$(echo "$LINK" | sed -n 's/.*tx bitrate: //p' | head -1)"
    emit rxrate "$(echo "$LINK" | sed -n 's/.*rx bitrate: //p' | head -1)"
elif [ "$TYPE" = "AP" ]; then
    emit state ap
    emit ssid "$(echo "$INFO" | awk '/^\tssid /{ $1=""; sub(/^ /,""); print; exit}')"
    emit freq "$(echo "$INFO" | awk '/^\tchannel /{print $2; exit}')"
else
    emit state down
fi

emit channel   "$(echo "$INFO" | sed -n 's/^[[:space:]]*channel \([0-9]*\).*/\1/p' | head -1)"
emit chanwidth "$(echo "$INFO" | sed -n 's/.*width: \([0-9]* MHz\).*/\1/p' | head -1)"
emit mac       "$(cat /sys/class/net/$IF/address 2>/dev/null)"
emit powersave "$(iw dev "$IF" get power_save 2>/dev/null | awk -F': ' '/Power save/{print $2; exit}')"

# ---------------------------------------------------------------- addresses --
emit addr "$(ip -4 -o addr show dev "$IF" 2>/dev/null | awk '{print $4; exit}')"
emit gw   "$(ip -4 route show default dev "$IF" 2>/dev/null | awk '{print $3; exit}')"

RX=$(cat /sys/class/net/$IF/statistics/rx_bytes 2>/dev/null)
TX=$(cat /sys/class/net/$IF/statistics/tx_bytes 2>/dev/null)
emit rxbytes "${RX:-0}"
emit txbytes "${TX:-0}"

# ------------------------------------------------------------------ adapter --
# The phy index is not always 0: rtw88 resets the dongle at probe time and the
# phy comes back as phy1. Derive it instead of assuming.
PHY=$(echo "$INFO" | awk '/wiphy/{print $2; exit}')
emit phy "${PHY:-}"

DRV=$(basename "$(readlink -f "/sys/class/net/$IF/device/driver" 2>/dev/null)" 2>/dev/null)
emit driver "${DRV:-}"

USBDEV=$(readlink -f "/sys/class/net/$IF/device" 2>/dev/null)
if [ -n "$USBDEV" ]; then
    # walk up to the USB device node that carries idVendor
    D=$USBDEV
    while [ -n "$D" ] && [ "$D" != "/" ] && [ ! -f "$D/idVendor" ]; do D=$(dirname "$D"); done
    if [ -f "$D/idVendor" ]; then
        emit usbid "$(cat "$D/idVendor"):$(cat "$D/idProduct")"
        emit usbspeed "$(cat "$D/speed" 2>/dev/null)"
        emit usbpower "$(cat "$D/bMaxPower" 2>/dev/null)"
        emit usbctl "$(cat "$D/power/control" 2>/dev/null)"
    fi
fi

if [ -n "$PHY" ]; then
    emit modes "$(iw phy "phy$PHY" info 2>/dev/null \
                  | sed -n '/Supported interface modes/,/^\t[A-Za-z]/p' \
                  | sed -n 's/^[[:space:]]*\* //p' | paste -sd, - | sed 's/,/, /g')"
fi

emit country "$($RP_PATH/get_country.sh)"
emit regdomain "$(iw reg get 2>/dev/null | sed -n 's/^country \([A-Z0-9][A-Z0-9]\):.*/\1/p' | head -1)"

exit 0
