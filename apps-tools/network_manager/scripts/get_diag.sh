#!/bin/bash
#
# The Diagnostic WiFi section, as JSON: a list of checks and a verdict.
#
# Every check here corresponds to something that has actually gone wrong on
# this hardware, not to a generic health template:
#
#   adapter        - the USB dongle enumerates late; "no wlan0" is the first
#                    thing to rule out
#   regdb          - without regulatory.db every 5 GHz channel is passive-scan
#                    only, so networks are visible and unjoinable
#   powersave      - rtw88 leaves 802.11 power save on, which costs latency and
#                    tends to wedge the deep low-power state machine
#   usberr         - -110 (ETIMEDOUT) on register access means the adapter fell
#                    off the USB control pipe; the count since boot is the
#                    single most useful number when a link is flaky
#   config         - wpa_supplicant.conf and hostapd.conf are mutually
#                    exclusive; both present is an ambiguous configuration that
#                    leaves every unit refusing to start
#   services       - which of the two units actually took the interface
#
# Four levels, and the difference between them decides whether the page shows
# this section at all:
#
#   ok    - checked, nothing to say
#   info  - a legitimate state that is not a fault. No adapter plugged in and
#           no wireless configured yet are both perfectly normal on a board
#           used over ethernet; reporting them as errors trains people to
#           ignore the section.
#   warn  - working, but degraded, and worth acting on
#   bad   - broken; wireless will not work until it is fixed
#
# "problems" in the output counts warn and bad only. The page hides the whole
# section when that count is zero, so the section appearing is itself the
# signal.

RP_PATH=/opt/redpitaya/www/apps/network_manager/scripts
WPA_SUP=/opt/redpitaya/wpa_supplicant.conf
HOSTAPD=/opt/redpitaya/hostapd.conf
IF=wlan0

jesc() { sed 's/\\/\\\\/g; s/"/\\"/g; s/\r//g' | sed ':a;N;$!ba;s/\n/\\n/g'; }
CHECKS=''
PROBLEMS=0
add() { # level id title detail
    case "$1" in
        warn|bad) PROBLEMS=$((PROBLEMS + 1)) ;;
    esac
    [ -n "$CHECKS" ] && CHECKS="$CHECKS,"
    CHECKS="$CHECKS
  {\"level\":\"$1\",\"id\":\"$2\",\"title\":\"$3\",\"detail\":\"$(printf '%s' "$4" | jesc)\"}"
}

# ------------------------------------------------------------------ adapter --
$RP_PATH/get_connected_wifi.sh
if [ $? = 1 ]; then
    HAVE_IF=1
    DRV=$(basename "$(readlink -f "/sys/class/net/$IF/device/driver" 2>/dev/null)" 2>/dev/null)
    add ok adapter "Adapter present" "$IF - ${DRV:-unknown driver} - nl80211"
else
    HAVE_IF=0
    add info adapter "No wireless adapter" "$IF is absent. Normal on a board used over ethernet; plug a USB adapter in to use wireless"
fi

# -------------------------------------------------------------- regulatory --
# With no adapter there is no radio to regulate, so a missing database is not a
# fault yet - it only becomes one once an adapter is plugged in.
if [ -r /lib/firmware/regulatory.db ]; then
    add ok regdb "Regulatory database loaded" "/lib/firmware/regulatory.db - country $($RP_PATH/get_country.sh)"
elif [ "$HAVE_IF" = 1 ]; then
    add bad regdb "Regulatory database missing" "install wireless-regdb; without it every 5 GHz channel is passive-scan only"
else
    add info regdb "Regulatory database missing" "install wireless-regdb before using a 5 GHz adapter"
fi

# --------------------------------------------------------------- power save --
PS=$(iw dev "$IF" get power_save 2>/dev/null | awk -F': ' '/Power save/{print $2; exit}')
if [ "$PS" = "off" ]; then
    add ok powersave "Power save disabled" "iw dev $IF get power_save -> off"
elif [ -n "$PS" ]; then
    add warn powersave "Power save enabled" "iw dev $IF get power_save -> $PS; off is recommended on USB adapters"
fi

# ------------------------------------------------------------- USB timeouts --
USBERR=$(dmesg 2>/dev/null | grep -c 'failed with -110')
LASTERR=$(dmesg 2>/dev/null | grep 'failed with -110' | tail -1)
if [ "${USBERR:-0}" -eq 0 ]; then
    add ok usberr "No USB register timeouts" "nothing matching 'failed with -110' since boot"
else
    add warn usberr "USB register timeouts since boot" "$USBERR x -110 - last: ${LASTERR:-unknown}"
fi

# ------------------------------------------------------------ configuration --
# Both files at once is a real fault: every unit's ConditionPathExists then
# refuses to start and the board ends up with no wireless at all. Neither file
# is just an unconfigured board.
if [ -f "$WPA_SUP" ] && [ -f "$HOSTAPD" ]; then
    add bad config "Ambiguous configuration" "both wpa_supplicant.conf and hostapd.conf exist; every unit refuses to start"
elif [ -f "$WPA_SUP" ]; then
    add ok config "Client configured" "wpa_supplicant.conf present, hostapd.conf absent"
elif [ -f "$HOSTAPD" ]; then
    add ok config "Access point configured" "hostapd.conf present, wpa_supplicant.conf absent"
else
    add info config "Wireless not configured" "neither wpa_supplicant.conf nor hostapd.conf exists. Set up a network under Wireless or Access point"
fi

# ----------------------------------------------------------------- services --
svc() { systemctl is-active "$1" 2>/dev/null || echo unknown; }
add ok services "Services" \
    "wpa_supplicant@$IF $(svc wpa_supplicant@$IF) - wpa_supplicant_wext@$IF $(svc wpa_supplicant_wext@$IF) - hostapd@$IF $(svc hostapd@$IF)"

cat <<EOF
{
 "problems": $PROBLEMS,
 "checks": [$CHECKS
 ]
}
EOF
exit 0
