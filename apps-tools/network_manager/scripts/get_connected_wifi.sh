#!/bin/bash


R8188=$(iwconfig wlan0 2>/dev/null | grep Link)
W=$(ip addr show wlan0 2>/dev/null | grep link)

if [ "$R8188" != '' ]; then
    exit 2
fi

if [ "$W" != '' ]; then
    exit 1
fi

exit 0
