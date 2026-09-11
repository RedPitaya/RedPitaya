#!/bin/bash
#
# Turn HTTPS off from a console.
#
#     bash /opt/redpitaya/www/apps/network_manager/scripts/https_off.sh
#
# The same thing the HTTPS section's checkbox does, for when the web interface
# is what is being debugged: it removes the generated fragments (including any
# the boot guard parked), records enabled=0, and reloads nginx if it is running.
# The certificate and the rest of the settings are left alone, so the section
# can put HTTPS back with one Apply.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

exec 9>"$LOCK_FILE"
flock -w 10 9 2>/dev/null || echo "warning: could not take the network config lock, going ahead anyway" >&2

REMOVED=0
for f in "$FRAG_SSL" "$FRAG_REDIRECT" "$FRAG_SSL.disabled" "$FRAG_REDIRECT.disabled"; do
    if [ -e "$f" ]; then
        rm -f "$f" && REMOVED=$((REMOVED + 1))
        echo "removed $f"
    fi
done
[ "$REMOVED" = "0" ] && echo "no generated fragments were installed"

conf_set enabled 0

exec 9>&-

if [ ! -x "$NGINX_BIN" ]; then
    echo "OK https off ($NGINX_BIN is missing, nothing to reload)"
    exit 0
fi

TEST_OUT=$(nginx_test)
if [ $? -ne 0 ]; then
    echo "error: nginx still does not accept its configuration, so the problem is elsewhere:" >&2
    echo "$TEST_OUT" >&2
    exit 4
fi

if ! pgrep -f 'nginx: master process' >/dev/null 2>&1; then
    echo "OK https off (nginx is not running; it will start without TLS)"
    exit 0
fi

RELOAD_OUT=$(nginx_reload)
if [ $? -ne 0 ]; then
    echo "error: nginx did not reload: $RELOAD_OUT" >&2
    echo "the fragments are gone, so a restart of redpitaya_nginx comes up without TLS" >&2
    exit 5
fi

echo "OK https off, nginx reloaded"
exit 0
