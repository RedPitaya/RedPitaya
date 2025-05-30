#apt update && sudo apt upgrade -y
apt remove docker docker-engine docker.io containerd runc -y
apt install -y ca-certificates curl gnupg lsb-release
mkdir -p /etc/apt/keyrings 2> /dev/null
rm /etc/apt/keyrings/docker.gpg 2> /dev/null
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /etc/apt/keyrings/docker.gpg
echo \
  "deb [arch=armhf signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null


apt update
apt install -y docker-ce docker-ce-cli containerd.io docker-compose-plugin

usermod -aG docker $USER

apt install -y iptables arptables ebtables

update-alternatives --set iptables /usr/sbin/iptables-nft
update-alternatives --set ip6tables /usr/sbin/ip6tables-legacy
update-alternatives --set arptables /usr/sbin/arptables-legacy
update-alternatives --set ebtables /usr/sbin/ebtables-legacy

systemctl restart docker
