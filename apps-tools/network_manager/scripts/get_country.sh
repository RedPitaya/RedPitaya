#!/bin/bash
#
# Print the configured regulatory country as a two-character code.
#
# Order of preference:
#   1. what the user selected in the web UI  (/opt/redpitaya/wifi_country)
#   2. what the kernel currently has applied (iw reg get)
#   3. "00" - the world domain
#
# "00" is not a harmless default: it leaves every 5 GHz channel PASSIVE-SCAN
# only, so 5 GHz networks are visible in a scan but can never be joined, and
# 2.4 GHz transmit power is capped at the lowest common denominator.

CC_FILE=/opt/redpitaya/wifi_country
CC=''

if [ -r "$CC_FILE" ]; then
    # First line, and it has to be exactly two letters or digits. Truncating
    # whatever is in the file to two characters would turn a corrupted file into
    # a plausible-looking wrong country ("garbage" -> "GA") instead of falling
    # through to the kernel's own value.
    RAW=$(head -1 "$CC_FILE" 2>/dev/null | tr -d '[:space:]')
    case "$RAW" in
        [A-Za-z0-9][A-Za-z0-9]) CC=$(echo "$RAW" | tr 'a-z' 'A-Z') ;;
    esac
fi

if [ -z "$CC" ]; then
    CC=$(iw reg get 2>/dev/null | sed -n 's/^country \([A-Z0-9][A-Z0-9]\):.*/\1/p' | head -1)
fi

case "$CC" in
    [A-Z0-9][A-Z0-9]) echo "$CC" ;;
    *)                echo "00" ;;
esac
