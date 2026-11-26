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
#   settings.sh web-auth-firewall on|off|boot
#                             closes (on) or opens (off) the ports of the websocket servers to the
#                             network; boot: as web_auth.json says
#   settings.sh jupyter-password
#                             sets the password of Jupyter from stdin (one line; empty: no
#                             password) and restarts it
#

APPS_CONFIG=/root/.config/redpitaya/apps
APPS_WWW=/opt/redpitaya/www/apps
# the Jupyter service runs as root: its configuration is in /root/.jupyter; the JSON file is read
# after jupyter_notebook_config.py of the image and holds only the password
JUPYTER_PASSWORD_FILE=/root/.jupyter/jupyter_server_config.json
# the web login (nginx conf/lua/auth.lua)
WEB_AUTH_FILE=/etc/redpitaya/web_auth.json
# reached through nginx only while the web login is on: 9002 (/wss), 9091 (/wsc), 9092 (/wsu),
# 9099 (/wsm), 9900 (/wsd); the servers listen on IPv4 only. The port 81 server of nginx asks for
# the login itself (nginx.conf): it listens on IPv6 too, which the kernel cannot filter.
WEB_AUTH_PORTS="9002 9091 9092 9099 9900"

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

    local web_on=false web_pw=false
    if [ -s "$WEB_AUTH_FILE" ]; then
        grep -q '"hash" *: *"pbkdf2' "$WEB_AUTH_FILE" && web_pw=true
        $web_pw && grep -q '"enabled" *: *true' "$WEB_AUTH_FILE" && web_on=true
    fi

    printf '{"uptime":%d,"dma_next":%d,"boot":{"total":%d,"used":%d},"led":%d,"fsck":%s,"root_default":%s,"jupyter_password":%s,"web_auth":{"enabled":%s,"password":%s},"eth0":%s,"wlan0":%s}\n' \
        "${uptime:-0}" "$dma_next" "${boot_total:-0}" "${boot_used:-0}" "${led:-0}" "$fsck" "$root_default" "$jupyter_password" "$web_on" "$web_pw" "$(iface eth0)" "$(iface wlan0)"
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

# Its own chain in INPUT. The kernel of the board has no nf_tables, no REJECT target and no IPv6
# tables: iptables-legacy, IPv4, DROP. The loopback stays open, nginx reaches the servers there.
web_auth_firewall() {
    local mode=$1 t p
    if [ "$mode" = boot ]; then
        mode=off
        if [ -s "$WEB_AUTH_FILE" ] && grep -q '"enabled" *: *true' "$WEB_AUTH_FILE" && grep -q '"hash" *: *"pbkdf2' "$WEB_AUTH_FILE"; then
            mode=on
        fi
    fi
    for t in /usr/sbin/iptables-legacy; do
        [ -x "$t" ] || continue
        $t -N RP_WEB_AUTH 2>/dev/null
        $t -F RP_WEB_AUTH
        while $t -D INPUT -j RP_WEB_AUTH 2>/dev/null; do :; done
        if [ "$mode" = on ]; then
            $t -A RP_WEB_AUTH -i lo -j RETURN
            for p in $WEB_AUTH_PORTS; do
                $t -A RP_WEB_AUTH -p tcp --dport "$p" -j DROP
            done
            $t -I INPUT -j RP_WEB_AUTH
        fi
    done
    printf '{"ok":true,"firewall":"%s"}\n' "$mode"
}

case "$1" in
    status) status ;;
    stored) stored ;;
    remove) remove "$2" ;;
    remove-all) remove_all ;;
    jupyter-password) jupyter_password ;;
    web-auth-firewall) web_auth_firewall "$2" ;;
    *)
        printf '{"ok":false,"error":"unknown command"}\n'
        exit 1
        ;;
esac
