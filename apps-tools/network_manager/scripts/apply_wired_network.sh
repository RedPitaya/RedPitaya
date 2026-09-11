#!/bin/bash


set -u

LOCK_FILE=/run/lock/rp-network.lock
FINAL_PATH=/etc/systemd/network/wired.network
TMP_PATH="${1:-}"
IFACE="${2:-eth0}"

if [ -z "$TMP_PATH" ] || [ ! -f "$TMP_PATH" ]; then
    echo "error: usage: apply_wired_network.sh <tmp-config-path> [iface]" >&2
    exit 2
fi

exec 9>"$LOCK_FILE"
if ! flock -w 10 9; then
    echo "error: could not acquire network config lock within 10s (another apply in progress?)" >&2
    rm -f "$TMP_PATH" 2>/dev/null
    exit 3
fi

# Re-assert rw here, inside the lock, even though the caller already called
# it once to be able to write $TMP_PATH -- that earlier call is unsynchronized
# and may have already been undone by another request's "ro" while we were
# waiting for the lock above. This one is what actually matters.
rw
RW_STATUS=$?
if [ "$RW_STATUS" -ne 0 ]; then
    echo "error: rw remount failed (status $RW_STATUS)" >&2
    rm -f "$TMP_PATH" 2>/dev/null
    ro
    exit 4
fi

mv -f "$TMP_PATH" "$FINAL_PATH"
MV_STATUS=$?
if [ "$MV_STATUS" -ne 0 ]; then
    echo "error: failed to install $FINAL_PATH (status $MV_STATUS)" >&2
    ro
    exit 5
fi

# Drop any address left over from whatever configuration was active before
# this one (static, DHCP-client, or DHCP-server) so the interface can't end
# up answering on both the old and the new address.
ip addr flush dev "$IFACE" scope global 2>/dev/null

systemctl restart systemd-networkd.service
RESTART_STATUS=$?

ro

# redpitaya_nginx.service defines ExecReload; the direct call is the fallback
# when the unit is not in charge, and it needs the prefix. This used to signal
# /run/nginx.pid, which never exists -- the pid file is /run/redpitaya_nginx.pid.
systemctl reload redpitaya_nginx 2>/dev/null \
    || /opt/redpitaya/sbin/nginx -p /opt/redpitaya/www -s reload 2>/dev/null

if [ "$RESTART_STATUS" -ne 0 ]; then
    echo "error: systemd-networkd restart failed (status $RESTART_STATUS)" >&2
    exit 6
fi

exit 0