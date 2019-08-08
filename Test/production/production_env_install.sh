#/bin/bash


PS3='Please enter your choice: '
options=("production_script" "gen1" "gen2" )
select opt in "${options[@]}"
do
    case $opt in
        "production_script")
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
echo "/opt/redpitaya/bin/production_testing_script.sh" >> ~/.bashrc
echo "All done"
	    break
            ;;
        "gen1")
echo "/opt/redpitaya/bin/production_env_gen1_install.sh" >> ~/.bashrc
echo "All done"
	    break
            ;;
        "gen2")
echo "/opt/redpitaya/bin/production_env_gen2_install.sh" >> ~/.bashrc
echo "All done"
            break
            ;;
        *) echo "invalid option $REPLY";;
    esac
done
