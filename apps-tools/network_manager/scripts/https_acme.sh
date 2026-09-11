#!/bin/bash
#
# Request (or renew) a Let's Encrypt certificate with certbot.
#
#     https_acme.sh <domain> <email> [staging 0|1]
#
# The http-01 challenge is served from /run/acme by the location in the port 80
# server, so the board must be reachable from the internet on port 80 under the
# requested name -- the usual reason a request fails on a lab board.
#
# Renewal is left to certbot's timer; the deploy hook installed here is what
# tells nginx to pick up the renewed files.

set -u

SELF_DIR=$(dirname "$0")
# shellcheck source=https_lib.sh
. "$SELF_DIR/https_lib.sh"

die() { echo "error: $1" >&2; exit "${2:-1}"; }

DOMAIN="${1:-}"
EMAIL="${2:-}"
STAGING="${3:-0}"

command -v certbot >/dev/null 2>&1 \
    || die "certbot is not installed on this board; install it in the OS image, then request the certificate again"

is_domain "$DOMAIN" || die "'$DOMAIN' is not a valid domain name" 2
is_email "$EMAIL"   || die "'$EMAIL' is not a valid e-mail address" 2
case "$STAGING" in 0|1) ;; *) die "staging must be 0 or 1" 2 ;; esac

mkdir -p "$ACME_ROOT/.well-known/acme-challenge" || die "could not create the challenge webroot $ACME_ROOT"
chmod 755 "$ACME_ROOT" "$ACME_ROOT/.well-known" "$ACME_ROOT/.well-known/acme-challenge" 2>/dev/null

# Runs after every renewal, including the unattended ones.
mkdir -p "$(dirname "$ACME_HOOK")" 2>/dev/null
cat > "$ACME_HOOK" <<'EOF'
#!/bin/bash
# Installed by network_manager/scripts/https_acme.sh.
systemctl reload redpitaya_nginx 2>/dev/null \
    || /opt/redpitaya/sbin/nginx -p /opt/redpitaya/www -s reload
EOF
chmod 755 "$ACME_HOOK" 2>/dev/null

ARGS=(certonly --webroot -w "$ACME_ROOT" -d "$DOMAIN"
      -m "$EMAIL" --agree-tos --non-interactive --keep-until-expiring)
[ "$STAGING" = "1" ] && ARGS+=(--staging)

OUT=$(certbot "${ARGS[@]}" 2>&1)
RC=$?
if [ $RC -ne 0 ]; then
    echo "error: certbot failed (status $RC)" >&2
    echo "$OUT" >&2
    exit 6
fi

LIVE=/etc/letsencrypt/live/$DOMAIN
[ -r "$LIVE/fullchain.pem" ] && [ -r "$LIVE/privkey.pem" ] \
    || die "certbot reported success but $LIVE holds no certificate"

conf_set cert_source acme
conf_set acme_domain "$DOMAIN"
conf_set acme_email "$EMAIL"
conf_set acme_staging "$STAGING"

# Enabled rather than assumed on: a bare "apt install certbot" would otherwise
# run into an expired certificate in three months.
systemctl enable --now certbot.timer >/dev/null 2>&1

https_reload_if_live

END=$(openssl x509 -in "$LIVE/fullchain.pem" -noout -enddate 2>/dev/null | cut -d= -f2)
if [ "$STAGING" = "1" ]; then
    echo "OK staging certificate issued for $DOMAIN (not trusted by browsers -- for testing the flow only)"
else
    echo "OK certificate issued for $DOMAIN"
fi
echo "expires: $END"
exit 0
