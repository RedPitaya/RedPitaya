#!/bin/bash
#
# Set the regulatory country.
#
#   set_country.sh <CC>       two characters, or 00 for the world domain
#
# Persists the choice, applies it to the running kernel, and rewrites the
# country line of whichever configuration is currently active so the setting
# survives a reboot and a reconnect. Prints "ok: <CC>" or an error line.

RAW=${1:-}

# Validate the whole argument, then normalise. Sanitising first would silently
# turn "RUS" into "RU" and "RU; rm -rf /" into "RU" - no injection either way,
# but accepting a wrong input as if it were right is its own bug.
case "$RAW" in
    [A-Za-z0-9][A-Za-z0-9]) : ;;
    *) echo "error: expected exactly two letters or digits, got \"$RAW\""; exit 2 ;;
esac

CC=$(echo "$RAW" | tr 'a-z' 'A-Z')

CC_FILE=/opt/redpitaya/wifi_country
WPA_SUP=/opt/redpitaya/wpa_supplicant.conf
HOSTAPD=/opt/redpitaya/hostapd.conf

rw || { echo "error: rw remount failed"; exit 4; }

echo "$CC" > "$CC_FILE"

# --- running kernel ---
iw reg set "$CC" 2>/dev/null

# --- client mode: keep wpa_supplicant.conf in step ---
if [ -f "$WPA_SUP" ]; then
    if grep -q '^country=' "$WPA_SUP"; then
        sed -i "s/^country=.*/country=$CC/" "$WPA_SUP"
    else
        sed -i "1i country=$CC" "$WPA_SUP"
    fi
    RESTART_CLIENT=1
fi

# --- AP mode: hostapd needs country_code plus 802.11d to advertise it ---
if [ -f "$HOSTAPD" ]; then
    sed -i '/^country_code=/d; /^ieee80211d=/d' "$HOSTAPD"
    if [ "$CC" != "00" ]; then
        printf 'country_code=%s\nieee80211d=1\n' "$CC" >> "$HOSTAPD"
    fi
    RESTART_AP=1
fi

ro

# Restart after ro: the daemons do not need a writable root, and leaving the
# filesystem writable across a service restart is what makes a power loss at
# the wrong moment expensive.
[ -n "${RESTART_CLIENT:-}" ] && systemctl restart wpa_supplicant@wlan0.service wpa_supplicant_wext@wlan0.service 2>/dev/null
[ -n "${RESTART_AP:-}" ]     && systemctl restart hostapd@wlan0.service 2>/dev/null

# Report what the kernel actually took, not what we asked for - they differ when
# the code is absent from regulatory.db or a driver pins its own domain.
APPLIED=$(iw reg get 2>/dev/null | sed -n 's/^country \([A-Z0-9][A-Z0-9]\):.*/\1/p' | head -1)
if [ "$APPLIED" = "$CC" ]; then
    echo "ok: $CC"
else
    echo "warning: requested $CC, kernel reports ${APPLIED:-none}"
fi
exit 0
