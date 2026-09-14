#!/bin/bash
#
# Shared helpers for the HTTPS section of the Network Manager.
#
# Everything lives on the ext4 rootfs: an ecosystem update runs
# "rm -rf /opt/redpitaya/*", and /opt/redpitaya is read-only VFAT, which cannot
# hold a 0600 private key. The generated fragments are pulled in by wildcard
# includes, which nginx ignores when they match nothing -- so "HTTPS off" is
# the plain old configuration, not a special case.

RP_ETC=/etc/redpitaya
HTTPS_CONF=$RP_ETC/https.conf
SSL_DIR=$RP_ETC/ssl
SSL_CRT=$SSL_DIR/server.crt
SSL_KEY=$SSL_DIR/server.key
FRAG_DIR=$RP_ETC/nginx/http
FRAG80_DIR=$RP_ETC/nginx/http80
FRAG_SSL=$FRAG_DIR/https.conf
FRAG_REDIRECT=$FRAG80_DIR/redirect.conf
LOCK_FILE=/run/lock/rp-network.lock

NGINX_BIN=/opt/redpitaya/sbin/nginx
NGINX_PREFIX=/opt/redpitaya/www

# Defaults for an absent https.conf -- the normal state of a board that has
# never been given a certificate.
DEF_ENABLED=0
DEF_PORT=443
DEF_REDIRECT=0
DEF_CERT_SOURCE=self

# Last assignment wins, comments and surrounding whitespace ignored, as in
# get_ui_config.sh.
conf_get() {
    local key="$1" file="${2:-$HTTPS_CONF}"
    sed -n "s/^[[:space:]]*$key[[:space:]]*=[[:space:]]*\(.*\)$/\1/p" "$file" 2>/dev/null \
        | sed 's/[[:space:]]*$//' | grep -v '^$' | tail -1
}

# Unusable values are rejected, not repaired: a typo must not silently become a
# different working configuration.
https_load() {
    HTTPS_ENABLED=$DEF_ENABLED
    HTTPS_PORT=$DEF_PORT
    HTTPS_REDIRECT=$DEF_REDIRECT
    HTTPS_CERT_SOURCE=$DEF_CERT_SOURCE

    [ -r "$HTTPS_CONF" ] || return 0

    local v
    v=$(conf_get enabled);      case "$v" in 0|1) HTTPS_ENABLED=$v ;; esac
    v=$(conf_get redirect);     case "$v" in 0|1) HTTPS_REDIRECT=$v ;; esac
    v=$(conf_get cert_source);  case "$v" in self|upload) HTTPS_CERT_SOURCE=$v ;; esac
    v=$(conf_get port);         is_port "$v" && HTTPS_PORT=$v
    return 0
}

is_port() {
    case "$1" in
        ''|*[!0-9]*) return 1 ;;
    esac
    [ "$1" -ge 1 ] && [ "$1" -le 65535 ]
}

is_hostname() {
    [ -n "$1" ] && [ ${#1} -le 253 ] || return 1
    printf '%s' "$1" | grep -Eq '^[A-Za-z0-9]([A-Za-z0-9.-]*[A-Za-z0-9])?$'
}

ssl_module_available() {
    "$NGINX_BIN" -V 2>&1 | grep -q -- '--with-http_ssl_module'
}

nginx_test() {
    "$NGINX_BIN" -p "$NGINX_PREFIX" -t 2>&1
}

# redpitaya_nginx.service defines ExecReload; the direct call is the fallback
# when the unit is not in charge. It needs the prefix.
nginx_reload() {
    systemctl reload redpitaya_nginx 2>/dev/null && return 0
    "$NGINX_BIN" -p "$NGINX_PREFIX" -s reload 2>&1
}

# Enough escaping for certificate fields and error messages.
json_str() {
    printf '%s' "$1" | sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' | tr -d '\n'
}

# Used by the certificate scripts so the recorded source matches the
# certificate just installed, even if Apply is never pressed.
conf_set() {
    local key="$1" value="$2"
    mkdir -p "$RP_ETC" || return 1
    local old_umask
    old_umask=$(umask)
    umask 077
    if [ -f "$HTTPS_CONF" ] && grep -q "^[[:space:]]*$key[[:space:]]*=" "$HTTPS_CONF"; then
        sed -i "s|^[[:space:]]*$key[[:space:]]*=.*$|$key=$value|" "$HTTPS_CONF"
    else
        printf '%s=%s\n' "$key" "$value" >> "$HTTPS_CONF"
    fi
    local rc=$?
    umask "$old_umask"
    return $rc
}

# A freshly installed certificate is otherwise not picked up until the next
# apply or reboot.
https_reload_if_live() {
    https_load
    [ "$HTTPS_ENABLED" = "1" ] || return 0
    local out
    out=$(nginx_test)
    if [ $? -ne 0 ]; then
        echo "warning: nginx configuration test failed, not reloading: $out" >&2
        return 1
    fi
    out=$(nginx_reload)
    if [ $? -ne 0 ]; then
        echo "warning: nginx did not reload: $out" >&2
        return 1
    fi
    return 0
}
