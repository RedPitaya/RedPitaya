#!/bin/bash
#
# Pack the certificate and its private key into a PKCS#12 file, so the same
# pair can be installed on another board.
#
#     https_export_bundle.sh <password-file> <output-path>
#
# The password arrives in a file, never on the command line, where every
# process on the board could read it out of ps.
#
# PKCS#12 rather than a plain PEM pair because this file carries the private
# key: anything holding it can impersonate the board, and an encrypted
# container is the difference between a file that must not be mislaid and one
# that must not be seen.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

PW_FILE="${1:-}"
OUT="${2:-}"

cleanup() { rm -f "$PW_FILE" 2>/dev/null; }
die() { echo "error: $1" >&2; cleanup; exit "${2:-1}"; }

[ -n "$PW_FILE" ] && [ -n "$OUT" ] || die "usage: https_export_bundle.sh <password-file> <output-path>" 2
[ -r "$PW_FILE" ] || die "no password was supplied" 2
[ "$(wc -c < "$PW_FILE")" -ge 8 ] || die "the export password must be at least 8 characters" 2

https_load
cert_paths

[ -r "$CERT_FILE" ] || die "no certificate at $CERT_FILE"
[ -r "$KEY_FILE" ]  || die "no private key at $KEY_FILE"

NAME="Red Pitaya $(hostname)"

OUT_ERR=$(openssl pkcs12 -export \
            -in "$CERT_FILE" -inkey "$KEY_FILE" \
            -name "$NAME" -passout file:"$PW_FILE" -out "$OUT" 2>&1)
if [ $? -ne 0 ]; then
    rm -f "$OUT" 2>/dev/null
    die "openssl could not build the bundle: $OUT_ERR"
fi

chmod 600 "$OUT" 2>/dev/null
cleanup
echo "OK bundle written"
exit 0
