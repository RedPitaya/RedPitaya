#!/bin/bash
#
# Print the certificate the board is configured to serve, PEM, nothing else.
#
# Only the certificate: it is public by definition, and the file next to it is
# the private key, which must never leave the board. The path comes from the
# settings, so this follows a switch between self-signed, uploaded and ACME.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

https_load
cert_paths

[ -r "$CERT_FILE" ] || { echo "error: no certificate at $CERT_FILE" >&2; exit 1; }
openssl x509 -in "$CERT_FILE" -noout >/dev/null 2>&1 \
    || { echo "error: $CERT_FILE is not a usable certificate" >&2; exit 1; }

# Re-encoded rather than copied: a chain file would otherwise hand out the
# intermediates too, and what is wanted here is the one certificate to trust.
openssl x509 -in "$CERT_FILE"
