#!/bin/bash


FILENAME=/opt/redpitaya/dts/$(monitor -f)/dtraw.dts
FILEDEST=/opt/redpitaya/dts/$(monitor -f)/devicetree.dtb
SEARCH_STRING="buffer@1000000 {"
REPLACE_STRING="reg = <0x1000000 "$1">;"

LINE_NUMBER=$(grep -n "$SEARCH_STRING" "$FILENAME" | cut -d: -f1)

if [ -z "$LINE_NUMBER" ]; then
  exit 1
fi
rw
TARGET_LINE=$((LINE_NUMBER + 2))

sed -i "${TARGET_LINE}s/.*/${REPLACE_STRING}/" "$FILENAME"

dtc $FILENAME -I dts -O dtb -o $FILEDEST
ro
