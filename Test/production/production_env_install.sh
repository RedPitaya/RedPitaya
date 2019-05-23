#/bin/bash


if ! which sshpass > /dev/null; then
    echo "setup sshpass"
    apt-get install sshpass --yes
fi

if ! ntpq -p > /dev/null; then
    echo "setup ntp"
    apt-get install ntp --yes
    timedatectl set-timezone Europe/Bucharest
    service ntp restart
fi

