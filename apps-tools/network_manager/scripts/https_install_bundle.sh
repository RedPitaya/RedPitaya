#!/bin/bash
#
# Install a PKCS#12 bundle exported from another board.
#
#     https_install_bundle.sh <p12-path> <password-file>
#
# The bundle is unpacked in /run, then handed to https_install_cert.sh, which
# owns the checking and the installing -- one place decides what a usable pair
# is, whether it arrived as PEM text or inside a container.
#
# A certificate shared between boards has to name all of them: a board whose
# name is not in the SAN list will serve this certificate and every browser
# will still object. The names are printed at the end for that reason.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

P12="${1:-}"
PW_FILE="${2:-}"
TMP_PEM=/run/rp_https_bundle.pem
TMP_CRT=/run/rp_https_bundle_cert.pem
TMP_KEY=/run/rp_https_bundle_key.pem

cleanup() { rm -f "$P12" "$PW_FILE" "$TMP_PEM" "$TMP_CRT" "$TMP_KEY" 2>/dev/null; }
die() { echo "error: $1" >&2; cleanup; exit "${2:-1}"; }

[ -n "$P12" ] && [ -n "$PW_FILE" ] || die "usage: https_install_bundle.sh <p12-path> <password-file>" 2
[ -s "$P12" ] || die "the uploaded bundle is empty" 2

UMASK_OLD=$(umask); umask 077
OUT=$(openssl pkcs12 -in "$P12" -nodes -passin file:"$PW_FILE" -out "$TMP_PEM" 2>&1)
RC=$?
umask "$UMASK_OLD"

if [ $RC -ne 0 ]; then
    case "$OUT" in
        *"mac verify failure"*|*"invalid password"*|*"Mac verify error"*)
            die "wrong password for this bundle" 3 ;;
    esac
    die "this is not a PKCS#12 bundle: $OUT"
fi

openssl x509 -in "$TMP_PEM" -out "$TMP_CRT" 2>/dev/null \
    || die "the bundle holds no certificate"
openssl pkey -in "$TMP_PEM" -out "$TMP_KEY" 2>/dev/null \
    || die "the bundle holds no private key"
chmod 600 "$TMP_KEY" 2>/dev/null

SAN=$(openssl x509 -in "$TMP_CRT" -noout -ext subjectAltName 2>/dev/null \
        | sed -n '2p' | sed 's/^[[:space:]]*//')

rm -f "$P12" "$PW_FILE" "$TMP_PEM" 2>/dev/null

# https_install_cert.sh removes the two files it is given, whatever it decides.
bash "$SELF_DIR/https_install_cert.sh" "$TMP_CRT" "$TMP_KEY"
RC=$?
[ $RC -eq 0 ] || exit $RC

echo "valid for: ${SAN:-(no subjectAltName -- this certificate names no host)}"
echo "this board is $(hostname) / $(hostname).local"
exit 0
