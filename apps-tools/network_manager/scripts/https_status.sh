#!/bin/bash
#
# Everything the HTTPS section needs as one JSON object: the stored settings,
# what nginx can do, and what the certificate says about itself. One request,
# because a half-loaded panel reads as a fault.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

https_load

SSL_MODULE=0
ssl_module_available && SSL_MODULE=1

# "enabled" is what was asked for, "applied" what nginx is serving; they differ
# after a failed reload or an unpressed Apply.
APPLIED=0
[ -f "$FRAG_SSL" ] && APPLIED=1
REDIRECT_APPLIED=0
[ -f "$FRAG_REDIRECT" ] && REDIRECT_APPLIED=1

# Set aside by https_boot_check.sh. Reported separately from "not applied":
# the two want different things done about them.
BOOT_DISABLED=0
[ -f "$FRAG_SSL.disabled" ] && BOOT_DISABLED=1

CERT_PRESENT=0
SUBJECT=''
ISSUER=''
NOT_AFTER=''
DAYS_LEFT=''
FINGERPRINT=''
SAN=''
SELF_SIGNED=0

if [ -r "$SSL_CRT" ] && openssl x509 -in "$SSL_CRT" -noout >/dev/null 2>&1; then
    CERT_PRESENT=1
    SUBJECT=$(openssl x509 -in "$SSL_CRT" -noout -subject 2>/dev/null | sed 's/^subject= *//')
    ISSUER=$(openssl x509 -in "$SSL_CRT" -noout -issuer 2>/dev/null | sed 's/^issuer= *//')
    NOT_AFTER=$(openssl x509 -in "$SSL_CRT" -noout -enddate 2>/dev/null | cut -d= -f2)
    FINGERPRINT=$(openssl x509 -in "$SSL_CRT" -noout -fingerprint -sha256 2>/dev/null | cut -d= -f2)
    SAN=$(openssl x509 -in "$SSL_CRT" -noout -ext subjectAltName 2>/dev/null \
            | sed -n '2p' | sed 's/^[[:space:]]*//')
    [ "$SUBJECT" = "$ISSUER" ] && SELF_SIGNED=1

    END_EPOCH=$(date -d "$NOT_AFTER" +%s 2>/dev/null)
    if [ -n "$END_EPOCH" ]; then
        DAYS_LEFT=$(( (END_EPOCH - $(date +%s)) / 86400 ))
    fi
fi

KEY_PRESENT=0
[ -r "$SSL_KEY" ] && KEY_PRESENT=1

cat <<EOF
{
    "ssl_module": $SSL_MODULE,
    "enabled": $HTTPS_ENABLED,
    "applied": $APPLIED,
    "port": $HTTPS_PORT,
    "redirect": $HTTPS_REDIRECT,
    "redirect_applied": $REDIRECT_APPLIED,
    "boot_disabled": $BOOT_DISABLED,
    "cert_source": "$(json_str "$HTTPS_CERT_SOURCE")",
    "cert_file": "$(json_str "$SSL_CRT")",
    "cert_present": $CERT_PRESENT,
    "key_present": $KEY_PRESENT,
    "self_signed": $SELF_SIGNED,
    "subject": "$(json_str "$SUBJECT")",
    "issuer": "$(json_str "$ISSUER")",
    "not_after": "$(json_str "$NOT_AFTER")",
    "days_left": "$(json_str "$DAYS_LEFT")",
    "fingerprint": "$(json_str "$FINGERPRINT")",
    "san": "$(json_str "$SAN")",
    "hostname": "$(json_str "$(hostname)")"
}
EOF
exit 0
