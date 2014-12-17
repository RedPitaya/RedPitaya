#!/bin/sh

[ $# -lt 1 ] && {
    exit 255;
}
TARGET=$1

# Embed version number into scripts
DIR=sbin
SCRIPTS='lantiq_mdio bazaar'

# VERSION and REVISION environment variables should be set in global environment.
echo "VERSION: ${VERSION}"
echo "REVISION: ${REVISION}"
for file in ${SCRIPTS}; do
    sed -i ${TARGET}/${DIR}/${file} -e "s/VER=.*/VER=${VERSION}-${REVISION}/"
done
