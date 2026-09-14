#!/bin/bash
#
# Print the certificate the board is configured to serve, PEM, nothing else.
#
# Only the certificate: it is public by definition, and the file next to it is
# the private key, which must never leave the board.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

[ -r "$SSL_CRT" ] || { echo "error: no certificate at $SSL_CRT" >&2; exit 1; }
openssl x509 -in "$SSL_CRT" -noout >/dev/null 2>&1 \
    || { echo "error: $SSL_CRT is not a usable certificate" >&2; exit 1; }

# Re-encoded rather than copied: a chain file would otherwise hand out the
# intermediates too, and what is wanted here is the one certificate to trust.
openssl x509 -in "$SSL_CRT"
