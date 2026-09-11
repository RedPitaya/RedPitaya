#!/bin/bash
#
# Generate a self-signed certificate for the board's web interface.
#
#     https_gen_cert.sh [common-name] [extra-sans] [days]
#
# The hostname, its .local name, the current IPv4 addresses and localhost are
# always in the SAN list: a board is reached by whichever of those was typed,
# and a certificate covering one of them warns on all the others.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

die() { echo "error: $1" >&2; exit "${2:-1}"; }

CN="${1:-}"
EXTRA="${2:-}"
DAYS="${3:-825}"

[ -n "$CN" ] || CN=$(hostname)
[ -n "$CN" ] || CN=redpitaya

is_hostname "$CN" || die "'$CN' is not a usable common name" 2

case "$DAYS" in
    ''|*[!0-9]*) die "validity must be a number of days" 2 ;;
esac
{ [ "$DAYS" -ge 1 ] && [ "$DAYS" -le 3650 ]; } || die "validity must be between 1 and 3650 days" 2

# Built first, de-duplicated at the end.
NAMES=("$CN" "$(hostname)" "$(hostname).local" localhost)

if [ -n "$EXTRA" ]; then
    IFS=',' read -r -a EXTRA_ARR <<< "$EXTRA"
    for e in "${EXTRA_ARR[@]}"; do
        e=$(printf '%s' "$e" | tr -d '[:space:]')
        [ -n "$e" ] || continue
        if is_hostname "$e" || printf '%s' "$e" | grep -Eq '^([0-9]{1,3}\.){3}[0-9]{1,3}$'; then
            NAMES+=("$e")
        else
            die "'$e' is not a usable name or address" 2
        fi
    done
fi

while read -r ip; do
    [ -n "$ip" ] && NAMES+=("$ip")
done < <(ip -4 -o addr show scope global 2>/dev/null | awk '{split($4,a,"/"); print a[1]}')
NAMES+=("127.0.0.1")

SAN=''
declare -A SEEN=()
for n in "${NAMES[@]}"; do
    [ -n "$n" ] || continue
    [ -n "${SEEN[$n]:-}" ] && continue
    SEEN[$n]=1
    if printf '%s' "$n" | grep -Eq '^([0-9]{1,3}\.){3}[0-9]{1,3}$'; then
        SAN="$SAN,IP:$n"
    else
        SAN="$SAN,DNS:$n"
    fi
done
SAN=${SAN#,}

mkdir -p "$SSL_DIR" || die "could not create $SSL_DIR"
chmod 700 "$SSL_DIR" 2>/dev/null

TMP_CRT=$(mktemp "$SSL_DIR/.crt.XXXXXX") || die "could not create a temporary file"
TMP_KEY=$(mktemp "$SSL_DIR/.key.XXXXXX") || { rm -f "$TMP_CRT"; die "could not create a temporary file"; }
chmod 600 "$TMP_KEY"

OUT=$(openssl req -x509 -newkey rsa:2048 -sha256 -nodes -days "$DAYS" \
        -keyout "$TMP_KEY" -out "$TMP_CRT" \
        -subj "/CN=$CN/O=Red Pitaya" \
        -addext "subjectAltName=$SAN" \
        -addext "basicConstraints=critical,CA:FALSE" \
        -addext "keyUsage=critical,digitalSignature,keyEncipherment" \
        -addext "extendedKeyUsage=serverAuth" 2>&1)
if [ $? -ne 0 ]; then
    rm -f "$TMP_CRT" "$TMP_KEY"
    die "openssl could not generate the certificate: $OUT"
fi

# Under the lock the apply path uses, so this cannot land while nginx is being
# reloaded with the old pair.
exec 9>"$LOCK_FILE"
flock -w 10 9 || { rm -f "$TMP_CRT" "$TMP_KEY"; die "could not acquire the network config lock within 10s" 3; }

mv -f "$TMP_CRT" "$SSL_CRT" && mv -f "$TMP_KEY" "$SSL_KEY" || {
    rm -f "$TMP_CRT" "$TMP_KEY"
    die "could not install the certificate into $SSL_DIR"
}
chmod 644 "$SSL_CRT"
chmod 600 "$SSL_KEY"

# Release the lock before reloading; https_reload_if_live does not take it.
exec 9>&-

conf_set cert_source self
https_reload_if_live

echo "OK self-signed certificate for $CN valid $DAYS days"
echo "SAN: $SAN"
exit 0
