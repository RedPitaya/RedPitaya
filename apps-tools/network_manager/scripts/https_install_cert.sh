#!/bin/bash
#
# Install a certificate and private key supplied by the user.
#
#     https_install_cert.sh <cert-pem-path> <key-pem-path>
#
# The endpoint writes both files under /run (nothing serves it, unlike /tmp);
# they are removed here whatever the outcome. Everything is checked before
# anything is installed -- otherwise a bad pair only shows up as a failed
# reload, with the working one already overwritten.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

CERT_IN="${1:-}"
KEY_IN="${2:-}"

cleanup() { rm -f "$CERT_IN" "$KEY_IN" 2>/dev/null; }

die() { echo "error: $1" >&2; cleanup; exit "${2:-1}"; }

[ -n "$CERT_IN" ] && [ -n "$KEY_IN" ] || die "usage: https_install_cert.sh <cert-pem-path> <key-pem-path>" 2
[ -s "$CERT_IN" ] || die "the certificate is empty -- paste the full PEM text, including the BEGIN and END lines" 2
[ -s "$KEY_IN" ]  || die "the private key is empty -- paste the full PEM text, including the BEGIN and END lines" 2

openssl x509 -in "$CERT_IN" -noout >/dev/null 2>&1 \
    || die "that is not a PEM certificate (expected a -----BEGIN CERTIFICATE----- block)"

# nginx cannot be handed a passphrase at startup, so such a key is rejected
# here rather than left to fail on the next boot.
if ! openssl pkey -in "$KEY_IN" -noout >/dev/null 2>&1; then
    if grep -q "ENCRYPTED" "$KEY_IN" 2>/dev/null; then
        die "the private key is passphrase-protected; remove the passphrase first (openssl pkey -in key.pem -out key-plain.pem)"
    fi
    die "that is not a PEM private key (expected a -----BEGIN PRIVATE KEY----- block)"
fi

CERT_PUB=$(openssl x509 -in "$CERT_IN" -noout -pubkey 2>/dev/null)
KEY_PUB=$(openssl pkey -in "$KEY_IN" -pubout 2>/dev/null)
[ -n "$CERT_PUB" ] && [ "$CERT_PUB" = "$KEY_PUB" ] \
    || die "the private key does not belong to this certificate"

if ! openssl x509 -in "$CERT_IN" -noout -checkend 0 >/dev/null 2>&1; then
    die "this certificate has already expired ($(openssl x509 -in "$CERT_IN" -noout -enddate 2>/dev/null | cut -d= -f2))"
fi

mkdir -p "$SSL_DIR" || die "could not create $SSL_DIR"
chmod 700 "$SSL_DIR" 2>/dev/null

exec 9>"$LOCK_FILE"
flock -w 10 9 || die "could not acquire the network config lock within 10s" 3

# Copy then rename: the source is on another filesystem, and the installed
# pair must stay consistent if the second copy fails.
TMP_CRT=$(mktemp "$SSL_DIR/.crt.XXXXXX") || die "could not create a temporary file"
TMP_KEY=$(mktemp "$SSL_DIR/.key.XXXXXX") || { rm -f "$TMP_CRT"; die "could not create a temporary file"; }
chmod 600 "$TMP_KEY"

cp -f "$CERT_IN" "$TMP_CRT" && cp -f "$KEY_IN" "$TMP_KEY" || {
    rm -f "$TMP_CRT" "$TMP_KEY"
    die "could not write to $SSL_DIR"
}

mv -f "$TMP_CRT" "$SSL_CRT" && mv -f "$TMP_KEY" "$SSL_KEY" || {
    rm -f "$TMP_CRT" "$TMP_KEY"
    die "could not install the certificate into $SSL_DIR"
}
chmod 644 "$SSL_CRT"
chmod 600 "$SSL_KEY"

exec 9>&-
cleanup

conf_set cert_source upload
https_reload_if_live

SUBJ=$(openssl x509 -in "$SSL_CRT" -noout -subject 2>/dev/null | sed 's/^subject= *//')
END=$(openssl x509 -in "$SSL_CRT" -noout -enddate 2>/dev/null | cut -d= -f2)
echo "OK certificate installed"
echo "subject: $SUBJ"
echo "expires: $END"
exit 0
