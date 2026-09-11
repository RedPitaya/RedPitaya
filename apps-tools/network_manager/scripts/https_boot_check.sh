#!/bin/bash
#
# Boot-time guard for the generated TLS server block, run from startup.sh
# (ordered before redpitaya_nginx.service).
#
# https_apply.sh never installs a fragment that fails "nginx -t", so this is
# for states nobody applied: a hand-edited file, a key lost with a half
# restored /etc. Any of those stops nginx from starting at all, leaving no web
# interface to fix it from.
#
# If the configuration does not load but does load without the TLS block, the
# block is moved aside and the board comes up on http. https.conf keeps saying
# "enabled" -- that is still what was asked for, and the section reports the
# difference.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

DISABLED=$FRAG_SSL.disabled

log() {
    echo "$1" >&2
    echo "network_manager: $1" > /dev/kmsg 2>/dev/null
}

[ -f "$FRAG_SSL" ] || exit 0
[ -x "$NGINX_BIN" ] || exit 0

FAIL_OUT=$(nginx_test)
[ $? -eq 0 ] && exit 0

# The first [emerg] line says what nginx choked on.
REASON=$(printf '%s\n' "$FAIL_OUT" | grep -m1 'emerg' | sed 's/^nginx: \[emerg\] *//')
[ -n "$REASON" ] || REASON="nginx -t failed"

# Find out whether the TLS block is what broke it.
mv -f "$FRAG_SSL" "$DISABLED" 2>/dev/null || exit 0
REDIRECT_MOVED=0
if [ -f "$FRAG_REDIRECT" ]; then
    mv -f "$FRAG_REDIRECT" "$FRAG_REDIRECT.disabled" 2>/dev/null && REDIRECT_MOVED=1
fi

if nginx_test >/dev/null 2>&1; then
    log "HTTPS turned off for this boot: $REASON. The web interface is on http; the old block is kept at $DISABLED."
    exit 0
fi

# Broken elsewhere: put the block back rather than leave a state nobody chose.
mv -f "$DISABLED" "$FRAG_SSL" 2>/dev/null
[ "$REDIRECT_MOVED" = "1" ] && mv -f "$FRAG_REDIRECT.disabled" "$FRAG_REDIRECT" 2>/dev/null
log "nginx configuration does not load, and the TLS block is not the cause; left untouched."
exit 0
