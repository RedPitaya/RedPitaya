

# Added by DM; 2017/10/17 to check ROOT_DIR setting
if [ $ROOT_DIR ]; then 
    echo ROOT_DIR is "$ROOT_DIR"
else
    echo Error: ROOT_DIR is not set
    echo exit with error
    exit
fi

chroot $ROOT_DIR <<- EOF_CHROOT

export DEBIAN_FRONTEND=noninteractive

apt -y purge --auto-remove cmake 

cd /tmp
apt -y install build-essential libssl-dev
wget https://github.com/Kitware/CMake/releases/download/v3.22.1/cmake-3.22.1.tar.gz
tar -zxvf cmake-3.22.1.tar.gz
cd cmake-3.22.1
./bootstrap
make -j$(grep processor /proc/cpuinfo | wc -l)
make install
rm -rf /tmp/cmake-3.22.1
rm -rf /tmp/cmake-3.22.1.tar.gz


EOF_CHROOT