#!/bin/bash
#
# Stations associated with the access point, as JSON.
#
# `iw station dump` knows the MAC, the signal and the counters, but not the
# address - 802.11 association happens below IP. Two sources supply the address:
#
#   1. the DHCP server's own record of what it handed out, read through
#      `networkctl status`. This used to look in /var/lib/systemd/network, which
#      is not where the leases are: networkd keeps them under
#      /run/systemd/netif/leases in a private format, so every lookup missed and
#      the address column was always empty. networkctl asks networkd rather than
#      parsing that file, which is the supported way to see them.
#
#   2. the neighbour table, which knows any address the board has actually
#      exchanged traffic with - including a client that set its address by hand
#      and never asked for a lease.
#
# The lease wins where both know a MAC: a neighbour entry can be STALE and still
# name an address the client has since given up.
#
# A station with no address from either source is still listed, with an empty
# one, because "associated but no address" is worth seeing - it is what a failing
# DHCP exchange looks like from here.

# The AP's own addressing is returned alongside, so the page can show what the
# access point is actually configured to hand out rather than an empty form.
# It rides on this request because the section already makes it and the single
# nginx worker is better off with one call than two.

IF=wlan0
NET_F=/etc/systemd/network/wireless.network.ap

jesc() { printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g'; }

# iw separates a key from its value with ":" followed by spaces OR a tab.
# Splitting on ": *" leaves the tab in the value, which is a raw control
# character and makes the JSON invalid. Take everything after the first colon
# and trim whitespace of any kind.
val() { printf '%s' "${1#*:}" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//'; }

# ---------------------------------------------------------------- addresses --
# One "mac<TAB>address" line per known pairing, leases first, so the dedupe at
# the end keeps the lease when a MAC is known to both sources.
#
# networkctl prints "Offered DHCP leases: 10.0.0.5 (to aa:bb:cc:dd:ee:ff)" and
# aligns continuation lines under the label, so match the pair wherever it
# appears instead of depending on the label sharing its line.
ADDR_MAP=$(
    {
        networkctl status "$IF" --no-pager 2>/dev/null \
            | grep -oE '([0-9]{1,3}\.){3}[0-9]{1,3} \(to ([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}\)' \
            | sed -E 's/^([0-9.]+) \(to ([0-9a-fA-F:]+)\)$/\2\t\1/'

        # "192.168.128.11 dev wlan0 lladdr 3c:22:fb:1a:9e:04 REACHABLE".
        # FAILED and INCOMPLETE entries carry no lladdr and drop out here.
        ip neigh show dev "$IF" 2>/dev/null \
            | awk '$1 ~ /^([0-9]{1,3}\.){3}[0-9]{1,3}$/ {
                       for (i = 2; i < NF; i++)
                           if ($i == "lladdr") { print $(i+1) "\t" $1; break }
                   }'
    } | awk -F'\t' '{ m = tolower($1) } !seen[m]++ { print m "\t" $2 }'
)

addr_for() {
    [ -n "$ADDR_MAP" ] || return 0
    printf '%s\n' "$ADDR_MAP" \
        | awk -F'\t' -v m="$(printf '%s' "$1" | tr 'A-Z' 'a-z')" '$1 == m { print $2; exit }'
}

# ----------------------------------------------------------------- stations --
DUMP=$(iw dev "$IF" station dump 2>/dev/null)

echo '{"stations": ['
FIRST=1

emit() {
    [ $FIRST -eq 0 ] && echo ','
    FIRST=0
    printf '  {"mac":"%s","addr":"%s","signal":"%s","connected":"%s","rx":"%s","tx":"%s"}' \
        "$(jesc "$MAC")" "$(jesc "$(addr_for "$MAC")")" \
        "$(jesc "${SIG:-}")" "$(jesc "${CONN:-}")" "$(jesc "${RX:-0}")" "$(jesc "${TX:-0}")"
}

while IFS= read -r line; do
    case "$line" in
        Station\ *)
            [ -n "${MAC:-}" ] && emit
            MAC=$(echo "$line" | awk '{print $2}')
            SIG=''; CONN=''; RX=''; TX=''
            ;;
        *signal:*)          SIG=$(val "$line" | awk '{print $1}') ;;
        *connected\ time:*) CONN=$(val "$line" | awk '{print $1}') ;;
        *rx\ bytes:*)       RX=$(val "$line") ;;
        *tx\ bytes:*)       TX=$(val "$line") ;;
    esac
done <<< "$DUMP"

[ -n "${MAC:-}" ] && emit

echo
echo '],'

# -------------------------------------------------------------- AP addressing --
# The keys this app writes, read back out of the same file. "key = value" with
# arbitrary spacing around the "=", which is how the shipped file is formatted.
key() { sed -n "s/^[[:space:]]*$1[[:space:]]*=[[:space:]]*\([^[:space:]]*\).*/\1/p" "$NET_F" 2>/dev/null | tail -1; }

CIDR=$(key Address)
AP_ADDR=${CIDR%%/*}
AP_PREFIX=${CIDR#*/}
[ "$AP_PREFIX" = "$CIDR" ] && AP_PREFIX=''

# systemd stores a prefix length; the form asks for a dotted mask.
AP_MASK=''
if [ -n "$AP_PREFIX" ] && [ "$AP_PREFIX" -ge 0 ] 2>/dev/null && [ "$AP_PREFIX" -le 32 ] 2>/dev/null; then
    AP_MASK=$(awk -v p="$AP_PREFIX" 'BEGIN {
        for (i = 0; i < 4; i++) {
            b = p - i * 8; if (b < 0) b = 0; if (b > 8) b = 8
            printf "%s%d", (i ? "." : ""), (b ? 256 - 2 ^ (8 - b) : 0)
        }
    }')
fi

printf ' "net": {"address":"%s","netmask":"%s","pool_offset":"%s","pool_size":"%s","lease":"%s","dns":"%s"}\n' \
    "$(jesc "$AP_ADDR")" "$(jesc "$AP_MASK")" \
    "$(jesc "$(key PoolOffset)")" "$(jesc "$(key PoolSize)")" \
    "$(jesc "$(key DefaultLeaseTimeSec)")" "$(jesc "$(key DNS)")"

echo '}'
exit 0
