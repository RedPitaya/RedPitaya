#!/bin/bash
#
# Is a usable wireless interface present?
#
#   exit 0 -> no wlan0 (no adapter plugged in)
#   exit 1 -> wlan0 present, driven through cfg80211/nl80211
#
# Exit code 2 used to mean "legacy wireless-extensions-only adapter" and drove a
# parallel iwconfig/iwlist code path throughout this app. That path targeted
# out-of-tree vendor drivers which no longer exist on this kernel: staging
# rtl8188eu was removed, RTL8188EU is served by rtl8xxxu, RTL8188CUS by
# rtl8192cu, and every RTL88xxBU/CU part by rtw88 - all of them mac80211, hence
# all nl80211. Verified on five adapters (2357:012d, 0bda:8176, 0bda:c811,
# 7392:b811, 2c4e:0102): every one classifies as nl80211 and scans through iw.
#
# Detection is by capability, not by link state: a real nl80211 device answers
# `iw dev <iface> info` whether or not it is associated.

if ! ip link show wlan0 >/dev/null 2>&1; then
    exit 0
fi

if iw dev wlan0 info >/dev/null 2>&1; then
    exit 1
fi

exit 0
