#!/bin/bash
#
# The board side of the settings page (main_menu/settings.html), called by the locations of
# main_menu/nginx.conf. Every command prints JSON.
#
#   settings.sh status        uptime, the DMA region after a reboot, the boot partition, LEDs,
#                             file system check, network interfaces
#   settings.sh stored        the stored settings of the applications
#   settings.sh remove <dir>  removes the stored settings of one application
#   settings.sh remove-all    removes the stored settings of all applications
#   settings.sh jupyter-password
#                             sets the password of Jupyter from stdin (one line; empty: no
#                             password) and restarts it
#

APPS_CONFIG=/root/.config/redpitaya/apps
APPS_WWW=/opt/redpitaya/www/apps
# the Jupyter service runs as root: its configuration is in /root/.jupyter; the JSON file is read
# after jupyter_notebook_config.py of the image and holds only the password
JUPYTER_PASSWORD_FILE=/root/.jupyter/jupyter_server_config.json

# a JSON string from a shell value
js() {
    local s=${1//\\/\\\\}
    s=${s//\"/\\\"}
    s=${s//$'\n'/ }
    printf '"%s"' "$s"
}

# address/prefix, operational state and the default gateway of an interface
iface() {
    local dev=$1
    if [ ! -e "/sys/class/net/$dev" ]; then
        printf '{"present":false}'
        return
    fi
    local state addr gw dhcp
    state=$(cat "/sys/class/net/$dev/operstate" 2>/dev/null)
    addr=$(ip -4 -o addr show dev "$dev" 2>/dev/null | awk '{print $4; exit}')
    gw=$(ip -4 route show default dev "$dev" 2>/dev/null | awk '{print $3; exit}')
    dhcp=false
    if ip -4 -o addr show dev "$dev" 2>/dev/null | grep -q ' dynamic '; then
        dhcp=true
    fi
    printf '{"present":true,"state":%s,"addr":%s,"gateway":%s,"dhcp":%s}' \
        "$(js "$state")" "$(js "$addr")" "$(js "$gw")" "$dhcp"
}

status() {
    local uptime dma_next boot_total boot_used led fsck dts
    uptime=$(awk '{printf "%d", $1}' /proc/uptime)

    # the size of the reserved region in the device tree the next boot reads (resizeDMA.sh)
    dma_next=0
    dts=$(/opt/redpitaya/bin/monitor -d 2>/dev/null)
    if [ -n "$dts" ] && [ "$dts" != "undefined" ]; then
        local reg
        reg=$(grep -A3 "buffer@2000000_b {" "/opt/redpitaya/dts/$dts/dtraw.dts" 2>/dev/null | sed -n 's/.*reg = <0x2000000 \(0x[0-9a-fA-F]*\)>;.*/\1/p' | head -1)
        if [ -n "$reg" ]; then
            dma_next=$((reg))
        fi
    fi

    read -r boot_total boot_used < <(df -B1 --output=size,used /opt/redpitaya 2>/dev/null | tail -1)

    # 1: all LEDs on, 2: some of them, 0: all off
    led=$(/opt/redpitaya/bin/led_control -z 2>/dev/null)

    fsck=false
    if /usr/sbin/tune2fs -l /dev/mmcblk0p2 2>/dev/null | grep -q 'Maximum mount count:[[:space:]]*1$'; then
        fsck=true
    fi

    # the root password is still the one of the image (root)
    local root_default=false hash
    hash=$(awk -F: '$1 == "root" {print $2}' /etc/shadow)
    if [ -n "$hash" ] && perl -e 'exit(crypt("root", $ARGV[0]) eq $ARGV[0] ? 0 : 1)' "$hash"; then
        root_default=true
    fi

    local jupyter_password=false
    if [ -s "$JUPYTER_PASSWORD_FILE" ] && grep -q '"hashed_password": *"[^"]' "$JUPYTER_PASSWORD_FILE"; then
        jupyter_password=true
    fi

    printf '{"uptime":%d,"dma_next":%d,"boot":{"total":%d,"used":%d},"led":%d,"fsck":%s,"root_default":%s,"jupyter_password":%s,"eth0":%s,"wlan0":%s}\n' \
        "${uptime:-0}" "$dma_next" "${boot_total:-0}" "${boot_used:-0}" "${led:-0}" "$fsck" "$root_default" "$jupyter_password" "$(iface eth0)" "$(iface wlan0)"
}

# the name of the application that owns a settings directory: <app>_<board> or <app>
app_name() {
    local dir=$1 app name
    for app in "${dir%_[0-9]*}" "$dir" "${dir}_manager"; do
        if [ -f "$APPS_WWW/$app/info/info.json" ]; then
            name=$(sed -n 's/.*"name"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' "$APPS_WWW/$app/info/info.json" | head -1)
            if [ -n "$name" ]; then
                printf '%s' "$name"
                return
            fi
        fi
    done
    printf '%s' "$dir"
}

stored() {
    local first=true dir name mtime
    printf '['
    if [ -d "$APPS_CONFIG" ]; then
        for path in "$APPS_CONFIG"/*/; do
            [ -d "$path" ] || continue
            dir=$(basename "$path")
            name=$(app_name "$dir")
            mtime=$(find "$path" -printf '%T@\n' 2>/dev/null | sort -n | tail -1 | cut -d. -f1)
            $first || printf ','
            first=false
            printf '{"dir":%s,"name":%s,"mtime":%d}' "$(js "$dir")" "$(js "$name")" "${mtime:-0}"
        done
    fi
    printf ']\n'
}

remove() {
    local dir=$1
    # a directory name only: no path, no dots
    if ! [[ "$dir" =~ ^[A-Za-z0-9_-]+$ ]] || [ ! -d "$APPS_CONFIG/$dir" ]; then
        printf '{"ok":false,"error":"no such settings"}\n'
        return 1
    fi
    rm -rf -- "${APPS_CONFIG:?}/$dir"
    printf '{"ok":true}\n'
}

remove_all() {
    rm -rf -- "${APPS_CONFIG:?}"/*
    printf '{"ok":true}\n'
}

# The password of Jupyter: the hash of jupyter_server (argon2) in its own configuration file; an
# empty password removes the file, Jupyter opens without one again. Restarts the service.
jupyter_password() {
    local password
    IFS= read -r password || true
    if [ -z "$password" ]; then
        rm -f "$JUPYTER_PASSWORD_FILE"
    else
        mkdir -p "$(dirname "$JUPYTER_PASSWORD_FILE")"
        local tmp="$JUPYTER_PASSWORD_FILE.new"
        ( umask 077; printf '%s' "$password" | python3 -c '
import json, sys
from jupyter_server.auth import passwd
json.dump({"PasswordIdentityProvider": {"hashed_password": passwd(sys.stdin.read())}}, sys.stdout, indent=2)
' > "$tmp" ) || { rm -f "$tmp"; printf '{"ok":false,"error":"the password could not be hashed"}\n'; return 1; }
        mv "$tmp" "$JUPYTER_PASSWORD_FILE"
    fi
    systemctl restart jupyter
    printf '{"ok":true}\n'
}

case "$1" in
    status) status ;;
    stored) stored ;;
    remove) remove "$2" ;;
    remove-all) remove_all ;;
    jupyter-password) jupyter_password ;;
    *)
        printf '{"ok":false,"error":"unknown command"}\n'
        exit 1
        ;;
esac
