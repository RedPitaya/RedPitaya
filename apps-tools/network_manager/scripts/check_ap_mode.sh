#!/bin/bash

INTERFACE=$(ip link show | grep -o 'wl[^:]*' | head -1)

if [ -z "$INTERFACE" ]; then
    exit 0
fi

PHY=$(iw dev "$INTERFACE" info 2>/dev/null | awk '/wiphy/{print $2; exit}')

if [ -z "$PHY" ]; then
    exit 0
fi

# -A 10 was also too short on multi-mode drivers; take the whole modes block.
IW_CHECK=$(iw phy "phy$PHY" info 2>/dev/null \
           | sed -n '/Supported interface modes/,/^\t[A-Za-z]/p' \
           | grep -c '\* AP$')

if [ "$IW_CHECK" -gt 0 ]; then
    exit 1
fi

exit 0
