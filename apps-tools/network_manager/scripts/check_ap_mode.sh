#!/bin/bash

INTERFACE=$(ip link show | grep -o 'wl[^:]*' | head -1)

if [ -z "$INTERFACE" ]; then
    exit 0
fi

IW_CHECK=$(iw phy0 info 2>/dev/null | grep -A 10 "Supported interface modes" | grep -c "* AP")

if [ "$IW_CHECK" -gt 0 ]; then
    exit 1
fi

exit 0