#!/bin/bash


DTS_VERSION=$(monitor -d)
if [ -z "$DTS_VERSION" ] || [ "$DTS_VERSION" = "undefined" ]; then
    echo "ERROR: Failed to get DTS version"
    exit 1
fi
FILENAME=/opt/redpitaya/dts/${DTS_VERSION}/dtraw.dts
FILEDEST=/opt/redpitaya/dts/${DTS_VERSION}/devicetree.dtb

SEARCH_STRING="buffer@2000000_b {"
REPLACE_STRING="reg = <0x2000000 "$1">;"

LINE_NUMBER=$(grep -n "$SEARCH_STRING" "$FILENAME" | cut -d: -f1)

if [ -z "$LINE_NUMBER" ]; then
  exit 1
fi
rw
TARGET_LINE=$((LINE_NUMBER + 2))

sed -i "${TARGET_LINE}s/.*/${REPLACE_STRING}/" "$FILENAME"

dtc $FILENAME -I dts -O dtb -o $FILEDEST
ro
